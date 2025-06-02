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

#include "DeltacastSdk.h"

#include "DeltacastHelpers.h"
#include "IDeltacastMediaModule.h"

#include <map>
#include <string_view>

// Define this to true to enable the time logging macro on the SDK wrapper while in DEBUG
#define DC_WRAPPER_LOG false

#if defined(UE_BUILD_DEBUG) && DC_WRAPPER_LOG
#define WRAPPER_LOG_TIME() SCOPE_LOG_TIME_FUNC()
#else
#define WRAPPER_LOG_TIME()
#endif


template<typename T>
void Set(T* Variable, T Value)
{
	if (Variable != nullptr)
	{
		*Variable = Value;
	}
}


namespace Deltacast::Utils
{
	VHD::Bool Convert(const bool Value)
	{
		return Value ? VHD::True : VHD::False;
	}

	bool Convert(const VHD::Bool Value)
	{
		return Value != VHD::False;
	}
}



VHD_ERRORCODE FDeltacastSdk::GetApiInfo(VHD::ULONG *ApiVersion, VHD::ULONG *NbBoards) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetApiInfo != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_GetApiInfo(ApiVersion, NbBoards));

	return FunctionNotLoaded;
}

std::optional<bool> FDeltacastSdk::IsBoardIndexValid(const VHD::ULONG BoardIndex) const
{
	VHD::ULONG      NbBoards = 0;
	const auto ApiInfoResult = GetApiInfo(nullptr, &NbBoards);

	if (!Deltacast::Helpers::IsValid(ApiInfoResult))
		return {};

	if (BoardIndex >= NbBoards)
		return false;

	return true;
}


VHD_ERRORCODE FDeltacastSdk::GetVideoCharacteristics_Internal(const VHD_VIDEOSTANDARD VideoStandard, VHD::ULONG *Width, VHD::ULONG *Height, bool *Interlaced, VHD::ULONG *FrameRate) const
{
	WRAPPER_LOG_TIME();

	auto InterlacedValue = Deltacast::Utils::Convert(*Interlaced);

	if (Deltacast::Helpers::Is8K(VideoStandard))
	{
		if (GetVideoCharacteristics_SdkFix(VideoStandard, Width, Height, &InterlacedValue, FrameRate))
		{
			Set(Interlaced, Deltacast::Utils::Convert(InterlacedValue));
			return VHD_ERRORCODE::VHDERR_NOERROR;
		}
		else
		{
			checkNoEntry();
		}
	}

	if (Wrapper_GetVideoCharacteristics != nullptr)
	{
		const auto Result = static_cast<VHD_ERRORCODE>(Wrapper_GetVideoCharacteristics(VideoStandard, Width, Height, &InterlacedValue, FrameRate));
		Set(Interlaced, Deltacast::Utils::Convert(InterlacedValue));
		return Result;
	}

	return FunctionNotLoaded;
}

VHD_ERRORCODE FDeltacastSdk::GetVideoCharacteristics_Internal(const VHD_DV_HDMI_VIDEOSTANDARD VideoStandard, VHD::ULONG *Width, VHD::ULONG *Height, bool *Interlaced, VHD::ULONG *FrameRate) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetVideoCharacteristics != nullptr)
	{
		auto InterlacedValue = Deltacast::Utils::Convert(*Interlaced);

		const auto Result = static_cast<VHD_ERRORCODE>(Wrapper_GetHdmiVideoCharacteristics(VideoStandard, Width, Height, &InterlacedValue, FrameRate));
		Set(Interlaced, Deltacast::Utils::Convert(InterlacedValue));
		return Result;
	}

	return FunctionNotLoaded;
}


const char* FDeltacastSdk::GetBoardModel(const VHD::ULONG BoardIndex) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetBoardModel != nullptr)
		return Wrapper_GetBoardModel(BoardIndex);

	return nullptr;
}



std::optional<VHDHandle> FDeltacastSdk::OpenBoard(const VHD::ULONG BoardIndex)
{
	VHDHandle  BoardHandle = VHD::InvalidHandle;
	const auto OpenBoardResult = OpenBoardHandle(BoardIndex, &BoardHandle, VHD::InvalidHandle, 0);
	if (!Deltacast::Helpers::IsValid(OpenBoardResult))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to open Deltacast board %u: %s"), BoardIndex , *Deltacast::Helpers::GetErrorString(OpenBoardResult));
		return {};
	}

	return BoardHandle;
}

VHD_ERRORCODE FDeltacastSdk::CloseBoardHandle(const VHDHandle BoardHandle)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_CloseBoardHandle != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_CloseBoardHandle(BoardHandle));

	return FunctionNotLoaded;
}


VHD_ERRORCODE FDeltacastSdk::GetBoardCapSdiVideoStandard(const VHDHandle BoardHandle, const VHD_STREAMTYPE StreamType, const VHD_VIDEOSTANDARD VideoStandard, bool *IsCapable) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetBoardCapSDIVideoStandard != nullptr)
	{
		auto IsCapableValue = Deltacast::Utils::Convert(*IsCapable);

		const auto Result = static_cast<VHD_ERRORCODE>(Wrapper_GetBoardCapSDIVideoStandard(BoardHandle, StreamType, VideoStandard, &IsCapableValue));
		Set(IsCapable, Deltacast::Utils::Convert(IsCapableValue));
		return Result;
	}

	return FunctionNotLoaded;
}

VHD_ERRORCODE FDeltacastSdk::GetBoardCapSdiInterface(const VHDHandle BoardHandle, const VHD_STREAMTYPE StreamType, const VHD_INTERFACE Interface, bool *IsCapable) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetBoardCapSDIInterface != nullptr)
	{
		auto IsCapableValue = Deltacast::Utils::Convert(*IsCapable);

		const auto Result = static_cast<VHD_ERRORCODE>(Wrapper_GetBoardCapSDIInterface(BoardHandle, StreamType, Interface, &IsCapableValue));
		Set(IsCapable, Deltacast::Utils::Convert(IsCapableValue));
		return Result;
	}

	return FunctionNotLoaded;
}

VHD_ERRORCODE FDeltacastSdk::GetBoardCapBufferPacking(const VHDHandle BoardHandle, const VHD_BUFFERPACKING BufferPacking, bool *IsCapable) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetBoardCapBufferPacking != nullptr)
	{
		auto IsCapableValue = Deltacast::Utils::Convert(*IsCapable);

		const auto Result = static_cast<VHD_ERRORCODE>(Wrapper_GetBoardCapBufferPacking(BoardHandle, BufferPacking, &IsCapableValue));
		Set(IsCapable, Deltacast::Utils::Convert(IsCapableValue));
		return Result;
	}

	return FunctionNotLoaded;
}


std::optional<VHDHandle> FDeltacastSdk::OpenStream(const VHDHandle BoardHandle, const VHD_STREAMTYPE StreamType, const VHD_SDI_STREAMPROCMODE ProcessingMode)
{
	auto StreamHandle = VHD::InvalidHandle;
	const auto Result = OpenStreamHandle(BoardHandle, static_cast<VHD::ULONG>(StreamType), static_cast<VHD::ULONG>(ProcessingMode), &StreamHandle);
	if (!Deltacast::Helpers::IsValid(Result))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to open Deltacast stream: %s"), *Deltacast::Helpers::GetErrorString(Result));
		return {};
	}

	return StreamHandle;
}

std::optional<VHDHandle> FDeltacastSdk::OpenStream(const VHDHandle BoardHandle, const VHD_STREAMTYPE StreamType, const VHD_DV_STREAMPROCMODE ProcessingMode)
{
	auto StreamHandle = VHD::InvalidHandle;
	const auto Result = OpenStreamHandle(BoardHandle, static_cast<VHD::ULONG>(StreamType), static_cast<VHD::ULONG>(ProcessingMode), &StreamHandle);
	if (!Deltacast::Helpers::IsValid(Result))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to open Deltacast stream: %s"), *Deltacast::Helpers::GetErrorString(Result));
		return {};
	}

	return StreamHandle;
}

VHD_ERRORCODE FDeltacastSdk::CloseStreamHandle(const VHDHandle StreamHandle)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_CloseStreamHandle != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_CloseStreamHandle(StreamHandle));

	return FunctionNotLoaded;
}



VHD_ERRORCODE FDeltacastSdk::PresetTimingStreamProperties(const VHDHandle StreamHandle, const VHD_DV_STANDARD VideoStandard, const VHD::ULONG ActiveWidth, const VHD::ULONG ActiveHeight, const VHD::ULONG RefreshRate, const bool bInterlaced)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_PresetTimingStreamProperties != nullptr)
	{
		const auto InterlacedValue = Deltacast::Utils::Convert(bInterlaced);

		const auto Result = static_cast<VHD_ERRORCODE>(Wrapper_PresetTimingStreamProperties(StreamHandle, VideoStandard, ActiveWidth, ActiveHeight, RefreshRate, InterlacedValue));
		return Result;
	}

	return FunctionNotLoaded;
}



VHD_ERRORCODE FDeltacastSdk::StartStream(const VHDHandle StreamHandle)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_StartStream != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_StartStream(StreamHandle));

	return FunctionNotLoaded;
}

VHD_ERRORCODE FDeltacastSdk::StopStream(const VHDHandle StreamHandle)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_StopStream != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_StopStream(StreamHandle));

	return FunctionNotLoaded;
}



VHD_ERRORCODE FDeltacastSdk::LockSlotHandle(const VHDHandle StreamHandle, VHDHandle *SlotHandle)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_LockSlotHandle != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_LockSlotHandle(StreamHandle, SlotHandle));

	return FunctionNotLoaded;
}

VHD_ERRORCODE FDeltacastSdk::UnlockSlotHandle(const VHDHandle SlotHandle)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_UnlockSlotHandle != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_UnlockSlotHandle(SlotHandle));

	return FunctionNotLoaded;
}


VHD_ERRORCODE FDeltacastSdk::GetSlotBuffer(const VHDHandle SlotHandle, const VHD::ULONG BufferType, VHD::BYTE **Buffer, VHD::ULONG *BufferSize) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetSlotBuffer != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_GetSlotBuffer(SlotHandle, BufferType, Buffer, BufferSize));

	return FunctionNotLoaded;
}



VHD_ERRORCODE FDeltacastSdk::GetSlotTimecode(VHDHandle SlotHandle, VHD_TIMECODE_SOURCE TimecodeSource, VHD_TIMECODE* TimeCode) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetSlotTimecode != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_GetSlotTimecode(SlotHandle, TimecodeSource, TimeCode));

	return FunctionNotLoaded;
}

VHD_ERRORCODE FDeltacastSdk::GetTimecode(const VHDHandle BoardHandle, const VHD_TIMECODE_SOURCE TcSource, bool *Locked, float *FrameRate, VHD_TIMECODE *TimeCode) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetTimecode != nullptr)
	{
		auto LockedValue = Deltacast::Utils::Convert(*Locked);

		const auto Result = static_cast<VHD_ERRORCODE>(Wrapper_GetTimecode(BoardHandle, TcSource, &LockedValue, FrameRate, TimeCode));
		Set(Locked, Deltacast::Utils::Convert(LockedValue));
		return Result;
	}

	return FunctionNotLoaded;
}

VHD_ERRORCODE FDeltacastSdk::DetectCompanionCard(const VHDHandle BoardHandle, const VHD_COMPANION_CARD_TYPE CompanionCardType, bool *IsPresent) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_DetectCompanionCard != nullptr)
	{
		auto IsPresentValue = Deltacast::Utils::Convert(*IsPresent);

		const auto Result = static_cast<VHD_ERRORCODE>(Wrapper_DetectCompanionCard(BoardHandle, CompanionCardType, &IsPresentValue));
		Set(IsPresent, Deltacast::Utils::Convert(IsPresentValue));
		return Result;
	}

	return FunctionNotLoaded;
}




VHD_ERRORCODE FDeltacastSdk::StartTimer(const VHDHandle BoardHandle, const VHD_TIMER_SOURCE Source, VHDHandle *TimerHandle)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_StartTimer != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_StartTimer(BoardHandle, Source, TimerHandle));

	return FunctionNotLoaded;
}

VHD_ERRORCODE FDeltacastSdk::StopTimer(const VHDHandle TimerHandle)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_StopTimer != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_StopTimer(TimerHandle));

	return FunctionNotLoaded;
}


VHD_ERRORCODE FDeltacastSdk::WaitOnNextTimerTick(const VHDHandle TimerHandle, const VHD::ULONG Timeout) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_WaitOnNextTimerTick != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_WaitOnNextTimerTick(TimerHandle, Timeout));

	return FunctionNotLoaded;
}


std::optional<VHD::ULONG> FDeltacastSdk::GetNbBoards() const
{
	WRAPPER_LOG_TIME();

	VHD::ULONG      NbBoards = 0;
	const auto ApiInfoResult = GetApiInfo(nullptr, &NbBoards);
	if (!Deltacast::Helpers::IsValid(ApiInfoResult))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get the number of Deltacast boards: %s"), *Deltacast::Helpers::GetErrorString(ApiInfoResult));
		return {};
	}

	return NbBoards;
}



std::optional<VHD::ULONG> FDeltacastSdk::GetRxCount(const VHDHandle BoardHandle) const
{
	WRAPPER_LOG_TIME();

	VHD::ULONG      RxCount = 0;
	const auto Result  = GetBoardProperty(BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_NB_RXCHANNELS, &RxCount);
	if (!Deltacast::Helpers::IsValid(Result))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get the number of RX channels: %s"),
		       *Deltacast::Helpers::GetErrorString(Result));
		return {};
	}

	return RxCount;
}

std::optional<VHD::ULONG> FDeltacastSdk::GetTxCount(const VHDHandle BoardHandle) const
{
	WRAPPER_LOG_TIME();

	VHD::ULONG      TxCount = 0;
	const auto Result  = GetBoardProperty(BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_NB_TXCHANNELS, &TxCount);
	if (!Deltacast::Helpers::IsValid(Result))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get the number of TX channels: %s"),
		       *Deltacast::Helpers::GetErrorString(Result));
		return {};
	}

	return TxCount;
}


std::optional<VHD::ULONG> FDeltacastSdk::GetSdiPortCount(const VHDHandle BoardHandle, const bool IsInput) const
{
	if (BoardHandle == VHD::InvalidHandle)
	{
		return {};
	}

	VHD::ULONG Count = 0;

	for (int PortIndex = 0; PortIndex < VHD::MaxPortCount; ++PortIndex)
	{
		const auto ChannelType = GetChannelType(BoardHandle, IsInput, PortIndex);
		if (!ChannelType.has_value())
		{
			continue;
		}

		if (!Deltacast::Helpers::IsSdi(ChannelType.value()))
		{
			continue;
		}

		++Count;
	}

	return Count;
}

std::optional<VHD::ULONG> FDeltacastSdk::GetDvPortCount(const VHDHandle BoardHandle, const bool IsInput) const
{
	if (BoardHandle == VHD::InvalidHandle)
	{
		return {};
	}

	VHD::ULONG Count = 0;

	for (int PortIndex = 0; PortIndex < VHD::MaxPortCount; ++PortIndex)
	{
		const auto ChannelType = GetChannelType(BoardHandle, IsInput, PortIndex);
		if (!ChannelType.has_value())
		{
			continue;
		}

		if (!Deltacast::Helpers::IsDv(ChannelType.value()))
		{
			continue;
		}

		++Count;
	}

	return Count;
}


std::optional<VHD_CHANNELTYPE> FDeltacastSdk::GetChannelType(const VHDHandle BoardHandle, const bool IsInput, const VHD::ULONG PortIndex) const
{
	WRAPPER_LOG_TIME();

	const auto ChannelTypeProperty = Deltacast::Helpers::GetChannelTypeProperty(IsInput, PortIndex);

	VHD::ULONG ChannelType = 0;
	const auto Result      = GetBoardProperty(BoardHandle, ChannelTypeProperty, &ChannelType);
	if (!Deltacast::Helpers::IsValid(Result))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get the channel type of channel %u: %s"),
		       PortIndex, *Deltacast::Helpers::GetErrorString(Result));
		return {};
	}

	return static_cast<VHD_CHANNELTYPE>(ChannelType);
}


std::optional<bool> FDeltacastSdk::GetBoardCapSdiVideoStandard(const VHDHandle BoardHandle, const VHD_STREAMTYPE StreamType, const VHD_VIDEOSTANDARD VideoStandard) const
{
	WRAPPER_LOG_TIME();

	bool       IsCapable = false;
	const auto Result    = GetBoardCapSdiVideoStandard(BoardHandle, StreamType, VideoStandard, &IsCapable);

	if (!Deltacast::Helpers::IsValid(Result))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get Deltacast board SDI video format capabilites: %s"),
		       *Deltacast::Helpers::GetErrorString(Result));
		return {};
	}

	return IsCapable;
}

std::optional<bool> FDeltacastSdk::GetBoardCapSdiInterface(const VHDHandle     BoardHandle, const VHD_STREAMTYPE StreamType,
                                                           const VHD_INTERFACE Interface) const
{
	WRAPPER_LOG_TIME();

	bool       IsCapable = false;
	const auto Result    = GetBoardCapSdiInterface(BoardHandle, StreamType, Interface, &IsCapable);

	if (!Deltacast::Helpers::IsValid(Result))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get Deltacast board SDI interface capabilites: %s"),
		       *Deltacast::Helpers::GetErrorString(Result));
		return {};
	}

	return IsCapable;
}

std::optional<bool> FDeltacastSdk::GetBoardCapBufferPacking(const VHDHandle BoardHandle, const VHD_BUFFERPACKING BufferPacking) const
{
	WRAPPER_LOG_TIME();

	bool       IsCapable = false;
	const auto Result    = GetBoardCapBufferPacking(BoardHandle, BufferPacking, &IsCapable);

	if (!Deltacast::Helpers::IsValid(Result))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get Deltacast board buffer packing capabilites: %s"),
		       *Deltacast::Helpers::GetErrorString(Result));
		return {};
	}

	return IsCapable;
}


std::optional<bool> FDeltacastSdk::IsFlexModule(const VHDHandle BoardHandle) const
{
	WRAPPER_LOG_TIME();

	VHD::ULONG Value  = 0;
	const auto Result = GetBoardProperty(BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_BOARD_TYPE, &Value);

	if (!Deltacast::Helpers::IsValid(Result))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get Deltacast board SDI interface capabilites: %s"),
		       *Deltacast::Helpers::GetErrorString(Result));
		return {};
	}

	switch (static_cast<VHD_BOARDTYPE>(Value))
	{
		case VHD_BOARDTYPE::VHD_BOARDTYPE_HD: [[fallthrough]];
		case VHD_BOARDTYPE::VHD_BOARDTYPE_3G: [[fallthrough]];
		case VHD_BOARDTYPE::VHD_BOARDTYPE_3GKEY: [[fallthrough]];
		case VHD_BOARDTYPE::VHD_BOARDTYPE_12G: [[fallthrough]];
		case VHD_BOARDTYPE::VHD_BOARDTYPE_ASI: [[fallthrough]];
		case VHD_BOARDTYPE::VHD_BOARDTYPE_HDMI20: [[fallthrough]];
		case VHD_BOARDTYPE::VHD_BOARDTYPE_MIXEDINTERFACE:
			return false;
		case VHD_BOARDTYPE::VHD_BOARDTYPE_FLEX_DP: [[fallthrough]];
		case VHD_BOARDTYPE::VHD_BOARDTYPE_FLEX_SDI: [[fallthrough]];
		case VHD_BOARDTYPE::VHD_BOARDTYPE_FLEX_HMI:
			return true;
		case VHD_BOARDTYPE::NB_VHD_BOARDTYPES: [[fallthrough]];
		default:
			UE_LOG(LogDeltacastMedia, Fatal, TEXT("Unhandled `VHD_BOARDTYPE` in `IsFlexModule`: %u"), Value);
			return {};
	}
}


std::optional<bool> FDeltacastSdk::IsFieldMergingSupported(const VHDHandle BoardHandle) const
{
	VHD::ULONG Value  = VHD::False;
	const auto Result = GetBoardCapability(BoardHandle, VHD_CORE_BOARD_CAPABILITY::VHD_CORE_BOARD_CAP_FIELD_MERGING, &Value);

	 if (!Deltacast::Helpers::IsValid(Result))
	 {
		 UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get Deltacast board capabilites: %s"),
			 *Deltacast::Helpers::GetErrorString(Result));
		return {};
	 }

	return Value != VHD::False;
}


static const std::map<std::string_view, int> SingleLinkKeyOffsetByCard =
{
	{ "DELTA-12G-elp-h 2c", 4 },
	{ "DELTA-12G2c-asi8c-elp-h", 4 },
	{ "DELTA-12G-elp-h 04", 4 },
	{ "DELTA-12G-elp-h 4c", 4 },
	{ "DELTA-12G-elp 4c", 2 },
	{ "DELTA-12G2c-hmi10-elp", 2 },
	{ "DELTA-12G-e-h 2i1c", 2 },
	{ "DELTA-12G-e-h 4i2c", 2 },
	{ "DELTA-12G-e-h 4o2c", 2 },
	{ "DELTA-12G-elp 2c", 1 },
};

int FDeltacastSdk::GetSingleLinkKeyOffset(VHD::ULONG BoardIndex) const
{
	const auto BoardName = GetBoardModel(BoardIndex);
	const auto it = SingleLinkKeyOffsetByCard.find(std::string_view{ BoardName });
	if (it == SingleLinkKeyOffsetByCard.cend())
		return 1;

	return it->second;
}


void FDeltacastSdk::SetLoopbackState(const VHDHandle BoardHandle, const VHD::ULONG PortIndex, const VHD::ULONG LinkCount, const int State)
{
	VHD::ULONG has_passive_loopback;
	VHD::ULONG has_active_loopback;
	VHD::ULONG has_firmware_loopback;

	check(BoardHandle != VHD::InvalidHandle);
	check(State == VHD::True || State == VHD::False);

	[[maybe_unused]] const auto GetCapPassiveLoopbackRes = GetBoardCapability(BoardHandle, VHD_CORE_BOARD_CAPABILITY::VHD_CORE_BOARD_CAP_PASSIVE_LOOPBACK, &has_passive_loopback);
	[[maybe_unused]] const auto GetCapActiveLoopbackRes = GetBoardCapability(BoardHandle, VHD_CORE_BOARD_CAPABILITY::VHD_CORE_BOARD_CAP_ACTIVE_LOOPBACK, &has_active_loopback);
	[[maybe_unused]] const auto GetCapFirmwareLoopbackRes = GetBoardCapability(BoardHandle, VHD_CORE_BOARD_CAPABILITY::VHD_CORE_BOARD_CAP_FIRMWARE_LOOPBACK, &has_firmware_loopback);

	if ((VHD::Bool)has_firmware_loopback)
	{
		auto fwloopback = Deltacast::Helpers::GetFWLoopbackFromPortIndex(PortIndex);
		if (fwloopback != VHD_CORE_BOARDPROPERTY::NB_VHD_CORE_BOARDPROPERTIES)
			[[maybe_unused]] const auto SetFWLoopbackStateResult = SetBoardProperty(BoardHandle, fwloopback, State);
	}

	if ((VHD::Bool)has_active_loopback && PortIndex == 0)
		[[maybe_unused]] const auto SetActiveLoopbackStateResult = SetBoardProperty(
			BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_ACTIVE_LOOPBACK_0, State
		);

	check(LinkCount == 1 || LinkCount == 4);

	if (has_passive_loopback)
	{
		if (PortIndex != 0 || LinkCount != 4)
		{
			const auto ByPassRelay = Deltacast::Helpers::GetByPassFromPortIndex(PortIndex);
			if (ByPassRelay != VHD_CORE_BOARDPROPERTY::NB_VHD_CORE_BOARDPROPERTIES)
				[[maybe_unused]] const auto SetByPassResult = SetBoardProperty(BoardHandle, ByPassRelay, State);
		}
		else
		{
			[[maybe_unused]] const auto SetByPass0Result = SetBoardProperty(BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_BYPASS_RELAY_0, State);
			[[maybe_unused]] const auto SetByPass1Result = SetBoardProperty(BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_BYPASS_RELAY_1, State);
			[[maybe_unused]] const auto SetByPass2Result = SetBoardProperty(BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_BYPASS_RELAY_2, State);
			[[maybe_unused]] const auto SetByPass3Result = SetBoardProperty(BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_BYPASS_RELAY_3, State);
		}
	}
}



Deltacast::DynamicLibrary::DynamicLibraryStatus FDeltacastSdk::LoadAllFunctionAddress()
{
#define LoadFunction(FunctionName) \
    { \
        Wrapper_##FunctionName = GetFunctionPointer<VHD_##FunctionName>(L"VHD_" #FunctionName); \
		if (Wrapper_##FunctionName == nullptr) \
		{ \
		    UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to load: %s"), TEXT("VHD_" #FunctionName)); \
		    return Deltacast::DynamicLibrary::DynamicLibraryStatus::get_function_address_failed; \
		} \
    }

	LoadFunction(GetApiInfo);
	LoadFunction(GetVideoCharacteristics);
	LoadFunction(GetHdmiVideoCharacteristics);

	LoadFunction(OpenBoardHandle);
	LoadFunction(CloseBoardHandle);

	LoadFunction(GetBoardModel);
	LoadFunction(GetBoardProperty);
	LoadFunction(GetBoardCapability);
	LoadFunction(GetBoardCapSDIVideoStandard);
	LoadFunction(GetBoardCapSDIInterface);
	LoadFunction(GetBoardCapBufferPacking);

	LoadFunction(SetBoardProperty);

	LoadFunction(OpenStreamHandle);
	LoadFunction(CloseStreamHandle);

	LoadFunction(GetStreamProperty);

	LoadFunction(SetStreamProperty);

	LoadFunction(PresetTimingStreamProperties);

	LoadFunction(StartStream);
	LoadFunction(StopStream);

	LoadFunction(LockSlotHandle);
	LoadFunction(UnlockSlotHandle);

	LoadFunction(GetSlotBuffer);

	LoadFunction(GetSlotTimecode);
	LoadFunction(GetTimecode);
	LoadFunction(DetectCompanionCard);

	LoadFunction(StartTimer);
	LoadFunction(StopTimer);
	LoadFunction(WaitOnNextTimerTick);

#undef LoadFunction

	return Deltacast::DynamicLibrary::DynamicLibraryStatus::ok;
}



VHD_ERRORCODE FDeltacastSdk::OpenBoardHandle(const VHD::ULONG BoardIndex, VHDHandle *BoardHandle, const VHDHandle OnStateChangeEvent, const VHD::ULONG StateChangeMask)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_OpenBoardHandle != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_OpenBoardHandle(BoardIndex, BoardHandle, OnStateChangeEvent, StateChangeMask));

	return FunctionNotLoaded;
}


VHD_ERRORCODE FDeltacastSdk::GetBoardCapability(const VHDHandle BoardHandle, const VHD_CORE_BOARD_CAPABILITY BoardCapability, VHD::ULONG* Value) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetBoardCapability != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_GetBoardCapability(BoardHandle, BoardCapability, Value));

	return FunctionNotLoaded;
}


VHD_ERRORCODE FDeltacastSdk::GetBoardProperty(const VHDHandle BoardHandle, const VHD::ULONG Property, VHD::ULONG* Value) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetBoardProperty != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_GetBoardProperty(BoardHandle, Property, Value));

	return FunctionNotLoaded;
}

VHD_ERRORCODE FDeltacastSdk::SetBoardProperty(const VHDHandle BoardHandle, const VHD::ULONG Property, const VHD::ULONG Value)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_SetBoardProperty != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_SetBoardProperty(BoardHandle, Property, Value));

	return FunctionNotLoaded;
}



VHD_ERRORCODE FDeltacastSdk::OpenStreamHandle(const VHDHandle BoardHandle, const VHD::ULONG StreamType, const VHD::ULONG ProcessingMode, VHDHandle* StreamHandle)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_OpenStreamHandle != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_OpenStreamHandle(BoardHandle, StreamType, ProcessingMode, nullptr, StreamHandle, VHD::InvalidHandle));

	return FunctionNotLoaded;
}



std::optional<VHD::ULONG> FDeltacastSdk::GetStreamProperty(const VHDHandle StreamHandle, const VHD::ULONG Property) const
{
	VHD::ULONG Value = 0;

	const auto Result = GetStreamProperty(StreamHandle, Property, &Value);

	return Deltacast::Helpers::IsValid(Result) ? Value : std::optional<VHD::ULONG>{};
}

VHD_ERRORCODE FDeltacastSdk::GetStreamProperty(const VHDHandle StreamHandle, const VHD::ULONG Property, VHD::ULONG* Value) const
{
	WRAPPER_LOG_TIME();

	if (Wrapper_GetStreamProperty != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_GetStreamProperty(StreamHandle, Property, Value));

	return FunctionNotLoaded;
}


VHD_ERRORCODE FDeltacastSdk::SetStreamProperty(const VHDHandle StreamHandle, const VHD::ULONG Property, const VHD::ULONG Value)
{
	WRAPPER_LOG_TIME();

	if (Wrapper_SetStreamProperty != nullptr)
		return static_cast<VHD_ERRORCODE>(Wrapper_SetStreamProperty(StreamHandle, Property, Value));

	return FunctionNotLoaded;
}



bool FDeltacastSdk::GetVideoCharacteristics_SdkFix(const VHD_VIDEOSTANDARD VideoStandard, VHD::ULONG* Width, VHD::ULONG* Height, VHD::Bool* Interlaced, VHD::ULONG* FrameRate)
{
	switch (VideoStandard)
	{
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_24Hz: Set(Width, VHD::ULONG{ 7680 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 24 }); return true;
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_25Hz: Set(Width, VHD::ULONG{ 7680 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 25 }); return true;
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_30Hz: Set(Width, VHD::ULONG{ 7680 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 30 }); return true;
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_50Hz: Set(Width, VHD::ULONG{ 7680 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 50 }); return true;
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_60Hz: Set(Width, VHD::ULONG{ 7680 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 60 }); return true;
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_24Hz: Set(Width, VHD::ULONG{ 8192 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 24 }); return true;
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_25Hz: Set(Width, VHD::ULONG{ 8192 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 25 }); return true;
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_30Hz: Set(Width, VHD::ULONG{ 8192 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 30 }); return true;
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_48Hz: Set(Width, VHD::ULONG{ 8192 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 48 }); return true;
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_50Hz: Set(Width, VHD::ULONG{ 8192 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 50 }); return true;
		case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_60Hz: Set(Width, VHD::ULONG{ 8192 }); Set(Height, VHD::ULONG{ 4320 }); Set(Interlaced, VHD::False); Set(FrameRate, VHD::ULONG{ 60 }); return true;
		default:
			return false;
	}
}



bool FDeltacast::Initialize()
{
	auto& DeltacastSdk = GetSdk();

	const auto LoadStatus = DeltacastSdk.Load(LibraryName);
	if (LoadStatus != Deltacast::DynamicLibrary::DynamicLibraryStatus::ok)
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to load"));
		return false;
	}

	VHD::ULONG NbBoards      = 0;
	VHD::ULONG ApiVersion    = 0;
	const auto ApiInfoResult = DeltacastSdk.GetApiInfo(&ApiVersion, &NbBoards);
	if (Deltacast::Helpers::IsValid(ApiInfoResult))
	{
		UE_LOG(LogDeltacastMedia, Log, TEXT("VideoMaster version: %s, Number of boards detected: %u"),
		       *Deltacast::Helpers::VersionToText(ApiVersion), NbBoards);
	}
	else
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Cannot get VideoMaster general information"));
	}

	return true;
}

void FDeltacast::Shutdown()
{
	GetSdk().Unload();
}


bool FDeltacast::IsInitialized()
{
	return GetSdk().IsLoaded();
}

FDeltacastSdk& FDeltacast::GetSdk()
{
	static FDeltacastSdk DeltacastSdk;

	return DeltacastSdk;
}

FName FDeltacast::GetProtocolName()
{
	static const FName ProtocolName = TEXT("deltacast");
	return ProtocolName;
}
