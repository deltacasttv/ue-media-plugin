/*
 * SPDX-FileCopyrightText: Copyright (c) DELTACAST.TV. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at * * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "DeltacastCustomTimeStep.h"

#include "DeltacastCustomTimeStepUpdater.h"
#include "DeltacastHelpers.h"
#include "DeltacastMediaSettings.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaModule.h"
#include "HAL/IConsoleManager.h"
#include "HAL/RunnableThread.h"
#include "MediaIOCoreDefinitions.h"


bool UDeltacastCustomTimeStep::Initialize(UEngine *InEngine)
{
#if WITH_EDITORONLY_DATA
	InitializedEngine = nullptr;
#endif

	bAutoDetectFormat = true;

	State = ECustomTimeStepSynchronizationState::Closed;

	if (!FDeltacast::IsInitialized())
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("The Custom Timestep '%s' can't be initialized. Deltacast is not initialized."), *GetName());
		State = ECustomTimeStepSynchronizationState::Error;
		return false;
	}

	const auto &DeltacastSdk = FDeltacast::GetSdk();

	const auto IsBoardIndexValid = DeltacastSdk.IsBoardIndexValid(BoardIndex);
	if (!IsBoardIndexValid.value_or(false))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("The Custom Timestep '%s' board index '%d' is invalid."), *GetName(), BoardIndex);
		State = ECustomTimeStepSynchronizationState::Error;
		return false;
	}

	const auto GenlockSource = [this]() -> std::optional<EDeltacastGenlockSource>
	{
		const auto Settings = GetDefault<UDeltacastMediaSettings>();
		if (Settings == nullptr)
		{
			UE_LOG(LogDeltacastMedia, Error, TEXT("Cannot get Deltacast Media Plugin Settings"));
			return {};
		}

		const auto BoardSettings = Settings->GetBoardSettings(BoardIndex);

		if (BoardSettings == nullptr)
		{
			UE_LOG(LogDeltacastMedia, Error, TEXT("Deltacast Media Plugin Settings are not filled for board index: %d"), BoardIndex);
			return {};
		}

		return BoardSettings->GenlockSource;
	}();

	if (!GenlockSource.has_value())
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("%s Cannot find Deltacast Media Plugin settings (genlock source) for board index: %d"),
		       *GetName(), BoardIndex);
		State = ECustomTimeStepSynchronizationState::Error;
		return false;
	}

	check(!Updater.IsValid());
	check(UpdaterThread == nullptr);

	FDeltacastCustomTimeStepConfig Config;
	Config.Callback      = this;
	Config.BoardIndex    = BoardIndex;

	Updater = MakeUnique<FDeltacastCustomTimeStepUpdater>(Config);
	UpdaterThread = FRunnableThread::Create(Updater.Get(), *FString::Printf(TEXT("Deltacast Custom Timestep %s"), *GetName()));
	if (UpdaterThread == nullptr)
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to start Deltacast Custom Timestep thread"));
		State = ECustomTimeStepSynchronizationState::Error;
		return false;
	}

#if WITH_EDITORONLY_DATA
	InitializedEngine = InEngine;
#endif

	State = ECustomTimeStepSynchronizationState::Synchronizing;

	return true;
}

void UDeltacastCustomTimeStep::Shutdown(UEngine *InEngine)
{
#if WITH_EDITORONLY_DATA
	InitializedEngine = nullptr;
#endif

	State = ECustomTimeStepSynchronizationState::Closed;
	ReleaseResources();
}


bool UDeltacastCustomTimeStep::UpdateTimeStep(UEngine* InEngine)
{
	if (State == ECustomTimeStepSynchronizationState::Closed)
	{
		return true;
	}

	if (State == ECustomTimeStepSynchronizationState::Error)
	{
		ReleaseResources();

#if WITH_EDITORONLY_DATA && WITH_EDITOR
		if (InitializedEngine && !GIsPlayInEditorWorld && GIsEditor)
		{
			static constexpr double TimeBetweenAttempt = 1.0;
			if (FApp::GetCurrentTime() - LastAutoSynchronizeInEditorAppTime > TimeBetweenAttempt)
			{
				Initialize(InitializedEngine);
				LastAutoSynchronizeInEditorAppTime = FApp::GetCurrentTime();
			}
		}
#endif

		return true;
	}

	check(State == ECustomTimeStepSynchronizationState::Synchronized ||
	      State == ECustomTimeStepSynchronizationState::Synchronizing);

	{
		static const auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.VSync"));
		if (!bWarnedAboutVSync)
		{
			bool bLockToVsync = CVar->GetValueOnGameThread() != 0;
			if (bLockToVsync)
			{
				UE_LOG(LogDeltacastMedia, Warning, TEXT("The Engine is using VSync and may break the 'genlock'"));
				bWarnedAboutVSync = true;
			}
		}
	}

	UpdateApplicationLastTime();

	const double TimeBeforeSync = FPlatformTime::Seconds();

	const bool bValidGenlockSignal = Updater->IsGenlockSynchronized();
	const bool bWaitedForSync = WaitForSync();

	const double TimeAfterSync = FPlatformTime::Seconds();

	if (!bWaitedForSync)
	{
		State = ECustomTimeStepSynchronizationState::Error;
		return true;
	}

	if (bValidGenlockSignal)
	{
		State = ECustomTimeStepSynchronizationState::Synchronized;
	}
	else
	{
		State = ECustomTimeStepSynchronizationState::Synchronizing;
	}

	UpdateAppTimes(TimeBeforeSync, TimeAfterSync);

	return false;
}


ECustomTimeStepSynchronizationState UDeltacastCustomTimeStep::GetSynchronizationState() const
{
	return State;
}


FFrameRate UDeltacastCustomTimeStep::GetFixedFrameRate() const
{
	return Updater.IsValid() ? Updater->GetSyncRate() : FFrameRate(30, 1);
}



bool UDeltacastCustomTimeStep::SupportsFormatAutoDetection() const
{
	return true;
}


bool UDeltacastCustomTimeStep::IsLastSyncDataValid() const
{
	return bIsPreviousSyncCountValid;
}


uint32 UDeltacastCustomTimeStep::GetLastSyncCountDelta() const
{
	return SyncCountDelta;
}

FFrameRate UDeltacastCustomTimeStep::GetSyncRate() const
{
	// If PSF you should get 2 field interrupts
	const FFrameRate SyncRate = GetFixedFrameRate();
	return SyncRate;
}



bool UDeltacastCustomTimeStep::WaitForSync()
{
	check(Updater.IsValid());

	SyncCountDelta = 1;

	const bool bIsWaitValid = Updater.Get()->WaitForSync();

	if (!bIsWaitValid)
	{
		State = ECustomTimeStepSynchronizationState::Error;
		bIsPreviousSyncCountValid = false;
		return false;
	}

	const auto SyncCount = Updater.Get()->GetSyncCount();

	if (bIsPreviousSyncCountValid)
	{
		SyncCountDelta = SyncCount - PreviousSyncCount;
		UE_CLOG(SyncCountDelta > GetExpectedSyncCountDelta(), LogDeltacastMedia, Warning, TEXT("The engine runs too slow for the CustomTimeStep, %u frames dropped"), SyncCountDelta);
	}

	PreviousSyncCount = SyncCount;

	bIsPreviousSyncCountValid = true;

	return true;
}



void UDeltacastCustomTimeStep::OnInitializationCompleted(const bool bSucceed)
{
	State = bSucceed ? ECustomTimeStepSynchronizationState::Synchronized : ECustomTimeStepSynchronizationState::Error;
}



void UDeltacastCustomTimeStep::BeginDestroy()
{
	ReleaseResources();
	Super::BeginDestroy();
}

#if WITH_EDITOR
void UDeltacastCustomTimeStep::PostEditChangeChainProperty(FPropertyChangedChainEvent &PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

#endif



void UDeltacastCustomTimeStep::ReleaseResources()
{
	if (UpdaterThread != nullptr)
	{
		UpdaterThread->Kill();
		UpdaterThread->WaitForCompletion();
		UpdaterThread = nullptr;
		Updater.Reset();
	}

	bWarnedAboutVSync = false;
}
