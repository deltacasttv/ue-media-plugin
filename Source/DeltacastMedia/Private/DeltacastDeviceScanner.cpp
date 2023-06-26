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

#include "DeltacastDeviceScanner.h"

#include "DeltacastHelpers.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaModule.h"


namespace Deltacast::Device
{
	namespace Config
	{
		bool FBasePortConfig::IsValid(const VHDHandle BoardHandle) const
		{
			check(BoardHandle != VHD::InvalidHandle);

			const auto& DeltacastSdk = FDeltacast::GetSdk();

			const auto ChannelCount = bIsInput ? DeltacastSdk.GetRxCount(BoardHandle) : DeltacastSdk.GetTxCount(BoardHandle);
			if (!ChannelCount || PortIndex >= ChannelCount)
			{
				return false;
			}

			return true;
		}

		FString FBasePortConfig::ToString() const
		{
			return FString::Printf(TEXT("B%u/%sX%u %s"),
			                       BoardIndex,
			                       bIsInput ? TEXT("R") : TEXT("T"),
			                       PortIndex,
			                       bIsEuropeanClock ? TEXT("EUR") : TEXT("US"));
		}


		bool FSdiPortConfig::IsValid(const VHDHandle BoardHandle) const
		{
			check(BoardHandle != VHD::InvalidHandle);

			if (!Base.IsValid(BoardHandle))
			{
				return false;
			}

			const auto& DeltacastSdk = FDeltacast::GetSdk();

			const auto ChannelType = DeltacastSdk.GetChannelType(BoardHandle, Base.bIsInput, Base.PortIndex);
			if (!ChannelType.has_value() ||
				!Deltacast::Helpers::IsSdi(ChannelType.value()))
			{
				return false;
			}

			// Manual fix because of a bug in the SDK
			if (!IsSingleLink() && Base.PortIndex % 4 != 0)
			{
				return false;
			}

			const auto StreamType         = GetStreamType();
			const auto IsCapableInterface = DeltacastSdk.GetBoardCapSdiInterface(BoardHandle, StreamType, Interface);
			if (!IsCapableInterface.value_or(false))
			{
				return false;
			}

			const auto IsCapableVideoStandard = DeltacastSdk.GetBoardCapSdiVideoStandard(BoardHandle, StreamType, VideoStandard);
			if (!IsCapableVideoStandard.value_or(false))
			{
				return false;
			}

			if (Base.bIsEuropeanClock && (VideoStandard == VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_NTSC_480 ||
			                              VideoStandard == VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_NTSC_487))
			{
				return false;
			}

			if (!Base.bIsEuropeanClock && VideoStandard == VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_PAL)
			{
				return false;
			}

			const auto IsFlexModule = DeltacastSdk.IsFlexModule(BoardHandle);
			if (!IsFlexModule.has_value())
			{
				return false;
			}

			if ((IsFlexModule.value() && VideoStandard == VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_NTSC_487) ||
				(!IsFlexModule.value() && VideoStandard == VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_NTSC_480))
			{
				return false;
			}
			
			return true;
		}

		FString FSdiPortConfig::ToString() const
		{
			return FString::Printf(TEXT("%s/%s/%s"),
			                       *Base.ToString(),
			                       *Deltacast::Helpers::GetVideoStandardString(VideoStandard),
			                       *Deltacast::Helpers::GetInterfaceString(Interface));
		}


		bool FSdiPortConfig::IsSingleLink() const
		{
			return Helpers::IsSingleLink(Interface);
		}

		Deltacast::Helpers::EQuadLinkType FSdiPortConfig::GetQuadLinkType() const
		{
			return Deltacast::Helpers::GetQuadLinkType(Interface);
		}


		VHD_STREAMTYPE FSdiPortConfig::GetStreamType() const
		{
			return Deltacast::Helpers::GetStreamTypeFromPortIndex(Base.bIsInput, Base.PortIndex);
		}


		bool FDvPortConfig::IsValid(const VHDHandle BoardHandle) const
		{
			check(BoardHandle != VHD::InvalidHandle);

			const auto& DeltacastSdk = FDeltacast::GetSdk();

			const auto ChannelType = DeltacastSdk.GetChannelType(BoardHandle, Base.bIsInput, Base.PortIndex);
			if (!ChannelType.has_value() ||
				!Deltacast::Helpers::IsDv(ChannelType.value()))
			{
				return false;
			}

			return Base.IsValid(BoardHandle);
		}

		FString FDvPortConfig::ToString() const
		{
			return FString::Printf(TEXT("%s/%s"),
			                       *Base.ToString(),
			                       *Deltacast::Helpers::GetVideoStandardString(VideoStandard));
		}
	}


	FSdiConfigIterator::FSdiConfigIterator(const uint32 BoardIndex): BoardIndex(BoardIndex) {}

	FSdiConfigIterator FSdiConfigIterator::end() const
	{
		FSdiConfigIterator Result(BoardIndex);

		Result.IsEnd = true;

		return Result;
	}

	Config::FSdiPortConfig FSdiConfigIterator::operator*() const
	{
		check(!IsEnd);

		Config::FSdiPortConfig Config;

		Config.Base.BoardIndex       = BoardIndex;
		Config.Base.PortIndex        = PortIndex;
		Config.Base.bIsInput         = bIsInput;
		Config.Base.bIsEuropeanClock = bIsEuropeanClock;

		Config.Interface = Helpers::SdiInterfaces[InterfaceIndex];

		if (bIsEuropeanClock)
		{
			Config.VideoStandard = Helpers::SdiInterfaceToVideoStandards[InterfaceIndex][VideoStandardIndex];
		}
		else
		{
			Config.VideoStandard = Helpers::SdiInterfaceToVideoStandardsUsClock[InterfaceIndex][VideoStandardIndex];
		}

		return Config;
	}

	FSdiConfigIterator& FSdiConfigIterator::operator++()
	{
		check(!IsEnd);

		VideoStandardIndex++;
		const auto &InterfaceToVideoStandardContainer = bIsEuropeanClock
			                                                ? Helpers::SdiInterfaceToVideoStandards
			                                                : Helpers::SdiInterfaceToVideoStandardsUsClock;
		const auto InterfaceToVideoStandardSize = InterfaceToVideoStandardContainer[InterfaceIndex].size();

		if (VideoStandardIndex == InterfaceToVideoStandardSize)
		{
			VideoStandardIndex = 0;
			InterfaceIndex++;

			if (InterfaceIndex == Helpers::SdiInterfaces.size())
			{
				InterfaceIndex = 0;

				if (bIsEuropeanClock)
				{
					bIsEuropeanClock = false;
				}
				else
				{
					bIsEuropeanClock = true;
					PortIndex++;

					if (PortIndex == VHD::MaxPortCount)
					{
						PortIndex = 0;

						if (bIsInput)
						{
							bIsInput = false;
						}
						else
						{
							IsEnd = true;
						}
					}
				}
			}
		}

		return *this;
	}

	bool operator!=(const FSdiConfigIterator &Lhs, const FSdiConfigIterator &Rhs)
	{
		if (Lhs.IsEnd || Rhs.IsEnd)
			return Lhs.IsEnd != Rhs.IsEnd;

		return Lhs.BoardIndex != Rhs.BoardIndex ||
		       Lhs.PortIndex != Rhs.PortIndex ||
		       Lhs.bIsInput != Rhs.bIsInput ||
		       Lhs.bIsEuropeanClock != Rhs.bIsEuropeanClock ||
		       Lhs.InterfaceIndex != Rhs.InterfaceIndex ||
		       Lhs.VideoStandardIndex != Rhs.VideoStandardIndex;
	}


	std::optional<FSdiDeviceScanner> FSdiDeviceScanner::GetDeviceScanner(const uint32 BoardIndex)
	{
		auto &DeltacastSdk = FDeltacast::GetSdk();

		const auto IsBoardIndexValid = DeltacastSdk.IsBoardIndexValid(BoardIndex);
		if (!IsBoardIndexValid.value_or(false))
		{
			return {};
		}

		const auto BoardHandle = DeltacastSdk.OpenBoard(BoardIndex).value_or(VHD::InvalidHandle);

		const auto InputSdiPortCount  = DeltacastSdk.GetSdiPortCount(BoardHandle, true);
		const auto OutputSdiPortCount = DeltacastSdk.GetSdiPortCount(BoardHandle, false);

		DeltacastSdk.CloseBoardHandle(BoardHandle);

		if (InputSdiPortCount.value_or(0) + OutputSdiPortCount.value_or(0) == 0)
		{
			return {};
		}

		return FSdiDeviceScanner(BoardIndex);
	}

	FSdiConfigIterator FSdiDeviceScanner::begin() const
	{
		return FSdiConfigIterator(BoardIndex);
	}

	FSdiConfigIterator FSdiDeviceScanner::end() const
	{
		return FSdiConfigIterator(BoardIndex).end();
	}

	FSdiDeviceScanner::FSdiDeviceScanner(const uint32 BoardIndex) : BoardIndex(BoardIndex) {}



	FDvConfigIterator::FDvConfigIterator(const uint32 BoardIndex) : BoardIndex(BoardIndex) {}

	FDvConfigIterator FDvConfigIterator::end() const
	{
		FDvConfigIterator Result(BoardIndex);

		Result.IsEnd = true;

		return Result;
	}

	Config::FDvPortConfig FDvConfigIterator::operator*() const
	{
		check(!IsEnd);

		Config::FDvPortConfig Config;

		Config.Base.BoardIndex       = BoardIndex;
		Config.Base.PortIndex        = PortIndex;
		Config.Base.bIsInput         = bIsInput;
		Config.Base.bIsEuropeanClock = bIsEuropeanClock;

		Config.VideoStandard = bIsEuropeanClock ? Helpers::DvVideoStandards[VideoStandardIndex] : Helpers::DvVideoStandardsUsClock[VideoStandardIndex];

		return Config;
	}

	FDvConfigIterator& FDvConfigIterator::operator++()
	{
		check(!IsEnd);

		VideoStandardIndex++;
		const auto VideoStandardSize = bIsEuropeanClock ? Helpers::DvVideoStandards.size() : Helpers::DvVideoStandardsUsClock.size();

		if (VideoStandardIndex == VideoStandardSize)
		{
			VideoStandardIndex = 0;

			if (bIsEuropeanClock)
			{
				bIsEuropeanClock = false;
			}
			else
			{
				bIsEuropeanClock = true;
				PortIndex++;

				if (PortIndex == VHD::MaxPortCount)
				{
					PortIndex = 0;

					if (bIsInput)
					{
						bIsInput = false;
					}
					else
					{
						IsEnd = true;
					}
				}
			}
		}

		return *this;
	}

	bool operator!=(const FDvConfigIterator &Lhs, const FDvConfigIterator &Rhs)
	{
		if (Lhs.IsEnd || Rhs.IsEnd)
			return Lhs.IsEnd != Rhs.IsEnd;

		return Lhs.BoardIndex != Rhs.BoardIndex ||
		       Lhs.PortIndex != Rhs.PortIndex ||
		       Lhs.bIsInput != Rhs.bIsInput ||
		       Lhs.bIsEuropeanClock != Rhs.bIsEuropeanClock ||
		       Lhs.VideoStandardIndex != Rhs.VideoStandardIndex;
	}


	std::optional<FDvDeviceScanner> FDvDeviceScanner::GetDeviceScanner(const uint32 BoardIndex)
	{
		auto& DeltacastSdk = FDeltacast::GetSdk();

		const auto IsBoardIndexValid = DeltacastSdk.IsBoardIndexValid(BoardIndex);
		if (!IsBoardIndexValid.value_or(false))
		{
			return {};
		}

		const auto BoardHandle = DeltacastSdk.OpenBoard(BoardIndex).value_or(VHD::InvalidHandle);

		const auto InputSdiPortCount = DeltacastSdk.GetDvPortCount(BoardHandle, true);
		const auto OutputSdiPortCount = DeltacastSdk.GetDvPortCount(BoardHandle, false);

		DeltacastSdk.CloseBoardHandle(BoardHandle);

		if (InputSdiPortCount.value_or(0) + OutputSdiPortCount.value_or(0) == 0)
		{
			return {};
		}

		return FDvDeviceScanner(BoardIndex);
	}

	FDvDeviceScanner::FDvDeviceScanner(const uint32 BoardIndex) : BoardIndex(BoardIndex) { }



	std::optional<FDeviceScanner> FDeviceScanner::GetDeviceScanner(const uint32 BoardIndex)
	{
		const auto &DeltacastSdk = FDeltacast::GetSdk();

		const auto IsBoardIndexValid = DeltacastSdk.IsBoardIndexValid(BoardIndex);
		if (!IsBoardIndexValid.value_or(false))
		{
			return {};
		}

		return FDeviceScanner(BoardIndex);
	}

	std::optional<FSdiDeviceScanner> FDeviceScanner::GetSdiDeviceScanner() const
	{
		return FSdiDeviceScanner::GetDeviceScanner(BoardIndex);
	}

	std::optional<FDvDeviceScanner> FDeviceScanner::GetDvDeviceScanner() const
	{
		return FDvDeviceScanner::GetDeviceScanner(BoardIndex);
	}

	FDeviceScanner::FDeviceScanner(const uint32 BoardIndex) : BoardIndex(BoardIndex) { }
}
