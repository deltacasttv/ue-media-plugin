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

#include "DeltacastCustomTimeStepUpdater.h"

#include "DeltacastHelpers.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaModule.h"



FDeltacastCustomTimeStepUpdater::FDeltacastCustomTimeStepUpdater(const FDeltacastCustomTimeStepConfig Config)
	: Callback(Config.Callback),
	  BoardIndex(Config.BoardIndex)
{
	check(Callback);
}



bool FDeltacastCustomTimeStepUpdater::Init()
{
	auto& DeltacastSdk = FDeltacast::GetSdk();

	check(WaitForSyncEvent == nullptr);

	WaitForSyncEvent = FPlatformProcess::GetSynchEventFromPool();
	if (WaitForSyncEvent == nullptr)
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to create sync event"));
		Callback->OnInitializationCompleted(false);
		Exit();
		return false;
	}

	BoardHandle = DeltacastSdk.OpenBoard(BoardIndex).value_or(VHD::InvalidHandle);
	if (BoardHandle == VHD::InvalidHandle)
	{
		Callback->OnInitializationCompleted(false);
		Exit();
		return false;
	}

	const auto StartResult = DeltacastSdk.StartTimer(BoardHandle, VHD_TIMER_SOURCE::VHD_TIMER_SOURCE_GENLOCK, &TimerHandle);
	if (!Deltacast::Helpers::IsValid(StartResult))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to start the genlock timer: %s"), *Deltacast::Helpers::GetErrorString(StartResult));
		Callback->OnInitializationCompleted(false);
		Exit();
		return false;
	}
	
	return true;
}

uint32 FDeltacastCustomTimeStepUpdater::Run()
{
	const auto& DeltacastSdk = FDeltacast::GetSdk();

	WaitAndDetectGenlock();

	bIsSynchronized = true;
	Callback->OnInitializationCompleted(!bStopRequested);

	while (!bStopRequested)
	{
		const auto WaitResult = DeltacastSdk.WaitOnNextTimerTick(TimerHandle, Deltacast::Helpers::GenlockWaitTimeOutMs);

		VHD::ULONG Status              = 0;
		const auto GenlockStatusResult = DeltacastSdk.GetBoardProperty(BoardHandle, VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_GENLOCK_STATUS, &Status);

		if (!Deltacast::Helpers::IsValid(GenlockStatusResult))
		{
			UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get genlock status: %s"), *Deltacast::Helpers::GetErrorString(GenlockStatusResult));
			bIsSynchronized = false;
			continue;
		}

		const auto bHasReference = (Status & VHD::VHD_SDI_GNLKSTS_NOREF) == 0;
		const auto bIsLocked     = (Status & VHD::VHD_SDI_GNLKSTS_UNLOCKED) == 0;

		bIsSynchronized = WaitResult == VHD_ERRORCODE::VHDERR_NOERROR && bHasReference && bIsLocked;


		if (!bIsSynchronized)
		{
			WaitAndDetectGenlock();
			bIsSynchronized = !bStopRequested;
		}
		else
		{
			WaitForSyncEvent->Trigger();
			++SyncCount;
		}
	}

	return 0;
}

void FDeltacastCustomTimeStepUpdater::Exit()
{
	auto& DeltacastSdk = FDeltacast::GetSdk();

	if (BoardHandle != VHD::InvalidHandle)
	{
		if (TimerHandle != VHD::InvalidHandle)
		{
			DeltacastSdk.StopTimer(TimerHandle);
			TimerHandle = VHD::InvalidHandle;
		}

		DeltacastSdk.CloseBoardHandle(BoardHandle);
		BoardHandle = VHD::InvalidHandle;
	}

	FPlatformProcess::ReturnSynchEventToPool(WaitForSyncEvent);
	WaitForSyncEvent = nullptr;
}


void FDeltacastCustomTimeStepUpdater::Stop()
{
	bStopRequested = true;
}



bool FDeltacastCustomTimeStepUpdater::WaitForSync() const
{
	while (!bStopRequested && bIsSynchronized && !WaitForSyncEvent->Wait(Deltacast::Helpers::GenlockWaitTimeOutMs)){}

	return !bStopRequested;
}


uint32 FDeltacastCustomTimeStepUpdater::GetSyncCount() const
{
	return SyncCount;
}

bool FDeltacastCustomTimeStepUpdater::IsGenlockSynchronized() const
{
	return bIsSynchronized;
}

FFrameRate FDeltacastCustomTimeStepUpdater::GetSyncRate() const
{
	const auto VideoCharacteristics = Deltacast::Helpers::GetVideoCharacteristics(GenlockVideoStandard);
	if (!VideoCharacteristics.has_value())
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get video characteristics"));
		return FFrameRate(30, 1);
	}

	const auto CorrectedFrameRate = VideoCharacteristics->bIsInterlaced ? VideoCharacteristics->FrameRate * 2 : VideoCharacteristics->FrameRate;

	if (bIsEuropeanClock)
	{
		return FFrameRate(CorrectedFrameRate, 1);
	}
	else
	{
		return FFrameRate(CorrectedFrameRate * 1000, 1001);
	}
}


void FDeltacastCustomTimeStepUpdater::WaitAndDetectGenlock()
{
	auto &DeltacastSdk = FDeltacast::GetSdk();

	VHD::ULONG Status = 0;

	do
	{
		FPlatformProcess::Sleep(Deltacast::Helpers::GenlockStatusSleepSec);

		const auto GenlockStatusResult = DeltacastSdk.GetBoardProperty(BoardHandle, VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_GENLOCK_STATUS, &Status);

		if (!Deltacast::Helpers::IsValid(GenlockStatusResult))
		{
			UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get genlock status: %s"), *Deltacast::Helpers::GetErrorString(GenlockStatusResult));
			continue;
		}

		const auto bHasReference = (Status & VHD::VHD_SDI_GNLKSTS_NOREF) == 0;
		const auto bIsLocked     = (Status & VHD::VHD_SDI_GNLKSTS_UNLOCKED) == 0;

		if (!bHasReference)
		{
			// Wait until reference is found
			continue;
		}

		if (bHasReference && !bIsLocked)
		{
			VHD::ULONG VideoStandard              = 0;
			const auto GenlockVideoStandardResult = DeltacastSdk.GetBoardProperty(BoardHandle,
			                                                                      VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_GENLOCK_VIDEO_STANDARD,
			                                                                      &VideoStandard);
			if (!Deltacast::Helpers::IsValid(GenlockVideoStandardResult))
			{
				UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get genlock video standard: %s"),
				       *Deltacast::Helpers::GetErrorString(GenlockVideoStandardResult));
				continue;
			}

			VHD::ULONG ClockDivisor              = 0;
			const auto GenlockClockDivisorResult = DeltacastSdk.GetBoardProperty(BoardHandle,
			                                                                     VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_CLOCK_SYSTEM, &ClockDivisor);
			if (!Deltacast::Helpers::IsValid(GenlockClockDivisorResult))
			{
				UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get genlock clock divisor: %s"),
				       *Deltacast::Helpers::GetErrorString(GenlockClockDivisorResult));
				continue;
			}


			const auto SetGenlockClockDivisorResult = DeltacastSdk.SetBoardProperty(BoardHandle,
			                                                                        VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_CLOCK_SYSTEM, ClockDivisor);
			if (!Deltacast::Helpers::IsValid(SetGenlockClockDivisorResult))
			{
				UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to set genlock clock divisor: %s"),
				       *Deltacast::Helpers::GetErrorString(SetGenlockClockDivisorResult));
				continue;
			}

			const auto SetGenlockVideoStandardResult = DeltacastSdk.SetBoardProperty(BoardHandle,
			                                                                         VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_GENLOCK_VIDEO_STANDARD,
			                                                                         VideoStandard);
			if (!Deltacast::Helpers::IsValid(SetGenlockVideoStandardResult))
			{
				UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to set genlock video standard: %s"),
				       *Deltacast::Helpers::GetErrorString(SetGenlockVideoStandardResult));
				continue;
			}

			bIsEuropeanClock = static_cast<VHD_CLOCKDIVISOR>(ClockDivisor) == VHD_CLOCKDIVISOR::VHD_CLOCKDIV_1;
			GenlockVideoStandard = static_cast<VHD_VIDEOSTANDARD>(VideoStandard);

			UE_LOG(LogDeltacastMedia, Display, TEXT("Waiting for lock genlock"));

			bool  bWaitHasReference = false;
			bool  bWaitIsLocked     = false;
			int32 RetryCounter      = static_cast<float>(Deltacast::Helpers::MaxGenlockSyncTimeSec) / Deltacast::Helpers::GenlockStatusSleepSec;
			do
			{
				FPlatformProcess::Sleep(Deltacast::Helpers::GenlockStatusSleepSec);

				const auto WaitGenlockStatusResult = DeltacastSdk.GetBoardProperty(BoardHandle, VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_GENLOCK_STATUS, &Status);

				if (!Deltacast::Helpers::IsValid(WaitGenlockStatusResult))
				{
					UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get genlock status: %s"), *Deltacast::Helpers::GetErrorString(WaitGenlockStatusResult));
					continue;
				}

				bWaitHasReference = (Status & VHD::VHD_SDI_GNLKSTS_NOREF) == 0;
				bWaitIsLocked     = (Status & VHD::VHD_SDI_GNLKSTS_UNLOCKED) == 0;

				--RetryCounter;
			} while (!bStopRequested && bWaitHasReference && !bWaitIsLocked && RetryCounter >= 0);

			UE_CLOG(RetryCounter < 0, LogDeltacastMedia, Error, TEXT("Failed to lock genlock"));

			continue;
		}

		if (bHasReference && bIsLocked)
		{
			break;
		}
	}
	while (!bStopRequested);
}
