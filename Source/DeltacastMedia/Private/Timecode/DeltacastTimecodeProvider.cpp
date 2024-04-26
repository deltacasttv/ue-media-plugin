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

#include "DeltacastTimecodeProvider.h"

#include "DeltacastHelpers.h"
#include "DeltacastMediaSettings.h"
#include "DeltacastSdk.h"
#include "DeltacastTimecodeUpdater.h"
#include "IDeltacastMediaModule.h"
#include "HAL/RunnableThread.h"
#include "Misc/App.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "DeltacastTimecodeProvider"

DECLARE_CYCLE_STAT(TEXT("Deltacast Timecode Provider GetQualifiedFrameTime"), STAT_Deltacast_GetQualifiedFrameTime, STATGROUP_Deltacast);


UDeltacastTimecodeProvider::UDeltacastTimecodeProvider(const FObjectInitializer &ObjectInitializer)
	: Super(ObjectInitializer),
#if WITH_EDITORONLY_DATA
	  InitializedEngine(nullptr),
	  LastAutoSynchronizeInEditorAppTime(0.0),
#endif
	  State(ETimecodeProviderSynchronizationState::Closed) {}


bool UDeltacastTimecodeProvider::Initialize(UEngine* InEngine)
{
#if WITH_EDITORONLY_DATA
	InitializedEngine = nullptr;
#endif

	State = ETimecodeProviderSynchronizationState::Closed;

	const auto& MediaModule = FModuleManager::LoadModuleChecked<IDeltacastMediaModule>(TEXT("DeltacastMedia"));
	if (!MediaModule.IsInitialized())
	{
		State = ETimecodeProviderSynchronizationState::Error;
		UE_LOG(LogDeltacastMedia, Warning, TEXT("Can't validate MediaOutput '%s'. The Deltacast library was not initialized."), *GetName());
		return false;
	}

	if (!MediaModule.CanBeUsed())
	{
		UE_LOG(LogDeltacastMedia, Warning, TEXT("Can't validate MediaOutput '%s' because Deltacast board cannot be used."), *GetName());
		return false;
	}

	const auto& DeltacastSdk = FDeltacast::GetSdk();

	const auto IsBoardIndexValid = DeltacastSdk.IsBoardIndexValid(BoardIndex);
	if (!IsBoardIndexValid.value_or(false))
	{
		State = ETimecodeProviderSynchronizationState::Error;
		UE_LOG(LogDeltacastMedia, Warning, TEXT("The TimecodeProvider '%s' configuration is invalid because of the board index '%d'."), *GetName(), BoardIndex);
		return false;
	}

	check(!Updater.IsValid());
	check(UpdaterThread == nullptr);

	const auto TimecodeSource = [this]() -> std::optional<VHD_TIMECODE_SOURCE>
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

		switch (BoardSettings->TimecodeSource)
		{
			case EDeltacastTimecodeSource::OnBoard: return VHD_TIMECODE_SOURCE::VHD_TC_SRC_LTC_ONBOARD;
			case EDeltacastTimecodeSource::CompanionCard: return VHD_TIMECODE_SOURCE::VHD_TC_SRC_LTC_COMPANION_CARD;
			default:
				UE_LOG(LogDeltacastMedia, Fatal, TEXT("Unhandled `DeltacastTimecodeSource`: %d"), BoardSettings->TimecodeSource);
				return {};
		}
	}();

	if (!TimecodeSource.has_value())
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Cannot find Deltacast Media Plugin settings for board index: %d"), BoardIndex);
		State = ETimecodeProviderSynchronizationState::Error;
		return false;
	}	

	FDeltacastTimecodeConfig Config;
	Config.Callback       = this;
	Config.BoardIndex     = BoardIndex;
	Config.TimecodeSource = TimecodeSource.value();

	Updater = MakeUnique<FDeltacastTimecodeUpdater>(Config);
	UpdaterThread = FRunnableThread::Create(Updater.Get(), *FString::Printf(TEXT("Deltacast Timecode Provider %s"), *GetName()));
	if (UpdaterThread == nullptr)
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to start Deltacast timecode thread"));
		State = ETimecodeProviderSynchronizationState::Error;
		return false;
	}

#if WITH_EDITORONLY_DATA
	InitializedEngine = InEngine;
#endif

	State = ETimecodeProviderSynchronizationState::Synchronizing;

	return true;
}

void UDeltacastTimecodeProvider::Shutdown(UEngine* InEngine)
{
#if WITH_EDITORONLY_DATA
	InitializedEngine = nullptr;
#endif

	State = ETimecodeProviderSynchronizationState::Closed;
	ReleaseResources();
}


void UDeltacastTimecodeProvider::FetchAndUpdate()
{
}

bool UDeltacastTimecodeProvider::FetchTimecode(FQualifiedFrameTime &OutFrameTime)
{
	OutFrameTime = GetQualifiedFrameTime();
	return State == ETimecodeProviderSynchronizationState::Synchronized;
}


FQualifiedFrameTime UDeltacastTimecodeProvider::GetQualifiedFrameTime() const
{
	SCOPE_CYCLE_COUNTER(STAT_Deltacast_GetQualifiedFrameTime);
	FQualifiedFrameTime Result;

	if (State != ETimecodeProviderSynchronizationState::Synchronized)
	{
		return Result;
	}

	const auto Timecode = Updater.Get()->GetTimecode();

	Result.Rate = Timecode.GetFrameRate();
	Result.Time = FFrameTime(Timecode.GetFrameNumber());

	return Result;
}


ETickableTickType UDeltacastTimecodeProvider::GetTickableTickType() const
{
#if WITH_EDITORONLY_DATA && WITH_EDITOR
	return ETickableTickType::Conditional;
#else
	return ETickableTickType::Never;
#endif
}

bool UDeltacastTimecodeProvider::IsTickable() const
{
	return State == ETimecodeProviderSynchronizationState::Error;
}

void UDeltacastTimecodeProvider::Tick(float DeltaTime)
{
#if WITH_EDITORONLY_DATA && WITH_EDITOR
	if (State == ETimecodeProviderSynchronizationState::Error)
	{
		ReleaseResources();

		// In Editor only, when not in pie, reinitialized the device
		if (InitializedEngine && !GIsPlayInEditorWorld && GIsEditor)
		{
			constexpr double TimeBetweenAttempt = 1.0;
			if (FApp::GetCurrentTime() - LastAutoSynchronizeInEditorAppTime > TimeBetweenAttempt)
			{
				Initialize(InitializedEngine);

				LastAutoSynchronizeInEditorAppTime = FApp::GetCurrentTime();
			}
		}
	}
#endif
}

void UDeltacastTimecodeProvider::BeginDestroy()
{
	ReleaseResources();
	Super::BeginDestroy();
}


void UDeltacastTimecodeProvider::OnInitializationCompleted(const bool bSucceed)
{
	State = bSucceed ? ETimecodeProviderSynchronizationState::Synchronized : ETimecodeProviderSynchronizationState::Error;
}


void UDeltacastTimecodeProvider::ReleaseResources()
{
	if (UpdaterThread != nullptr)
	{
		UpdaterThread->Kill();
		UpdaterThread->WaitForCompletion();
		UpdaterThread = nullptr;
		Updater.Reset();
	}
}

#undef LOCTEXT_NAMESPACE
