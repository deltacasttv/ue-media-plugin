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

#include "DeltacastTimecodeUpdater.h"

#include "DeltacastHelpers.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaModule.h"
#include "Misc/AssertionMacros.h"
#include "Misc/ScopeLock.h"

#include <algorithm>
#include <array>
#include <utility>

DECLARE_CYCLE_STAT(TEXT("Deltacast SDK GetTimecode"), STAT_Deltacast_SDK_GetTimecode, STATGROUP_Deltacast);


int32 FDeltacastTimecode::GetFrameNumber() const
{
	static constexpr int32 SecondConstant = 1;
	static constexpr int32 MinuteConstant = SecondConstant * 60;
	static constexpr int32 HourConstant   = MinuteConstant * 60;

	const auto UeFrameRate = GetFrameRate();

	const int32 Seconds = Timecode.Second * SecondConstant +
	                      Timecode.Minute * MinuteConstant +
	                      Timecode.Hour * HourConstant;

	const int32 FrameNumber = Timecode.Frame + ((Seconds * UeFrameRate.Numerator) / UeFrameRate.Denominator);

	return FrameNumber;
}

FFrameRate FDeltacastTimecode::GetFrameRate() const
{
	using FrameRatePair = std::pair<uint32, uint32>;
	static constexpr std::array<std::pair<float, FrameRatePair>, 10> Fps = {
		std::pair{24.0f, FrameRatePair{24, 1}},
		std::pair{25.0f, FrameRatePair{25, 1}},
		std::pair{30.0f, FrameRatePair{30, 1}},
		std::pair{48.0f, FrameRatePair{48, 1}},
		std::pair{50.0f, FrameRatePair{50, 1}},
		std::pair{60.0f, FrameRatePair{60, 1}},
		std::pair{24.0f * 1000.0f / 1001.0f, FrameRatePair{24000, 1001}},
		std::pair{30.0f * 1000.0f / 1001.0f, FrameRatePair{30000, 1001}},
		std::pair{48.0f * 1000.0f / 1001.0f, FrameRatePair{48000, 1001}},
		std::pair{60.0f * 1000.0f / 1001.0f, FrameRatePair{60000, 1001}},
	};

	const auto MinIterator = std::min_element(Fps.cbegin(), Fps.cend(),
	                                          [FrameRate = this->FrameRate](const std::pair<float, FrameRatePair> &Lhs,
	                                                                        const std::pair<float, FrameRatePair> &Rhs)
	                                          {
		                                          const auto LhsTransformed = std::abs(FrameRate - Lhs.first);
		                                          const auto RhsTransformed = std::abs(FrameRate - Rhs.first);
		                                          return LhsTransformed < RhsTransformed;
	                                          });

	if (MinIterator == Fps.cend())
	{
		return FFrameRate();
	}

	return FFrameRate(MinIterator->second.first, MinIterator->second.second);
}



FDeltacastTimecodeUpdater::FDeltacastTimecodeUpdater(const FDeltacastTimecodeConfig Config)
	: Callback(Config.Callback),
	  BoardIndex(Config.BoardIndex),
	  TimecodeSource(Config.TimecodeSource)
{
	check(Callback);
}


bool FDeltacastTimecodeUpdater::Init()
{
	auto &DeltacastSdk = FDeltacast::GetSdk();

	BoardHandle = DeltacastSdk.OpenBoard(BoardIndex).value_or(VHD::InvalidHandle);
	if (BoardHandle == VHD::InvalidHandle)
	{
		Callback->OnInitializationCompleted(false);
		Exit();
		return false;
	}

	if (TimecodeSource == VHD_TIMECODE_SOURCE::VHD_TC_SRC_LTC_COMPANION_CARD)
	{
		bool       IsPresent           = false;
		const auto CompanionCardResult = DeltacastSdk.DetectCompanionCard(BoardHandle, VHD_COMPANION_CARD_TYPE::VHD_LTC_COMPANION_CARD, &IsPresent);
		if (!Deltacast::Helpers::IsValid(CompanionCardResult))
		{
			UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to detect companion card: %s"), *Deltacast::Helpers::GetErrorString(CompanionCardResult));
			Callback->OnInitializationCompleted(false);
			Exit();
			return false;
		}
	}
	else if (TimecodeSource == VHD_TIMECODE_SOURCE::VHD_TC_SRC_LTC_ONBOARD)
	{
		VHD::ULONG IsSupported     = 0;
		const auto SupportedResult = DeltacastSdk.GetBoardProperty(BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BOARD_CAP_LTC_ONBOARD, &IsSupported);
		if (!Deltacast::Helpers::IsValid(SupportedResult) || !IsSupported)
		{
			UE_LOG(LogDeltacastMedia, Error, TEXT("Board %d doesn't support LTC onboard: %s"),
			       BoardIndex, *Deltacast::Helpers::GetErrorString(SupportedResult));
			Callback->OnInitializationCompleted(false);
			Exit();
			return false;
		}
	}
	else
	{
		UE_LOG(LogDeltacastMedia, Fatal, TEXT("Unhandled timecode source: %u"), TimecodeSource);
		Callback->OnInitializationCompleted(false);
		Exit();
		return false;
	}

	return true;
}

uint32 FDeltacastTimecodeUpdater::Run()
{
	static constexpr auto UpdateSleepSecond = 1.0f / 120.0f;

	const auto &DeltacastSdk = FDeltacast::GetSdk();

	bool bHasLoggedLastError = false;

	[[maybe_unused]] const auto InitialTimecode = WaitForTimecodeLocked();

	if (InitialTimecode.has_value())
	{
		FScopeLock Guard(&TimecodeCriticalSection);

		LastTimecode.Timecode  = InitialTimecode.value().Timecode;
		LastTimecode.FrameRate = InitialTimecode.value().FrameRate;
	}

	Callback->OnInitializationCompleted(!bStopRequested);

	while (!bStopRequested)
	{
		FPlatformProcess::Sleep(UpdateSleepSecond);

		VHD_TIMECODE Timecode{};
		bool         bLocked    = false;
		float        FrameRate = 0.0f;

		{
			SCOPE_CYCLE_COUNTER(STAT_Deltacast_SDK_GetTimecode);
			const auto Result = DeltacastSdk.GetTimecode(BoardHandle, TimecodeSource, &bLocked, &FrameRate, &Timecode);

			if (Result == VHD_ERRORCODE::VHDERR_LTCSOURCEUNLOCKED ||
			    (Deltacast::Helpers::IsValid(Result) && !bLocked))
			{
				UE_CLOG(!bHasLoggedLastError, LogDeltacastMedia, Error, TEXT("LTC source is unlocked"));
				bHasLoggedLastError = true;
				continue;
			}
			else if (!Deltacast::Helpers::IsValid(Result))
			{
				UE_CLOG(!bHasLoggedLastError, LogDeltacastMedia, Error,
				        TEXT("Failed to get timecode %s"), *Deltacast::Helpers::GetErrorString(Result));
				bHasLoggedLastError = true;
				continue;
			}
		}

		// Reset on success
		bHasLoggedLastError = false;

		{
			FScopeLock Guard(&TimecodeCriticalSection);

			LastTimecode.Timecode  = Timecode;
			LastTimecode.FrameRate = FrameRate;
		}
	}

	return 0;
}

void FDeltacastTimecodeUpdater::Exit()
{
	auto &DeltacastSdk = FDeltacast::GetSdk();

	if (BoardHandle != VHD::InvalidHandle)
	{
		DeltacastSdk.CloseBoardHandle(BoardHandle);
		BoardHandle = VHD::InvalidHandle;
	}
}

void FDeltacastTimecodeUpdater::Stop()
{
	bStopRequested = true;
}


FDeltacastTimecode FDeltacastTimecodeUpdater::GetTimecode() const
{
	FScopeLock Guard(&TimecodeCriticalSection);

	return LastTimecode;
}


std::optional<FDeltacastTimecode> FDeltacastTimecodeUpdater::WaitForTimecodeLocked() const
{
	const auto &DeltacastSdk = FDeltacast::GetSdk();

	VHD_ERRORCODE Result;

	bool bLocked    = false;
	auto DcTimecode = FDeltacastTimecode();

	do
	{
		FPlatformProcess::Sleep(Deltacast::Helpers::TimecodeSleepMs);

		SCOPE_CYCLE_COUNTER(STAT_Deltacast_SDK_GetTimecode);

		Result = DeltacastSdk.GetTimecode(BoardHandle, TimecodeSource, &bLocked, &DcTimecode.FrameRate, &DcTimecode.Timecode);
	}
	while (!bStopRequested && !Deltacast::Helpers::IsValid(Result) && !bLocked);

	if (bStopRequested)
	{
		return {};
	}

	return DcTimecode;
}