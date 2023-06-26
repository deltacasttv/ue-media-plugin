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

#pragma once

#include "DeltacastDefinition.h"
#include "DynamicLibraryLoader.h"

#include <limits>
#include <optional>



class DELTACASTMEDIA_API FDeltacastSdk final : public Deltacast::DynamicLibrary::DynamicLibraryLoader
{
	/* Wrapped functions const-correctness is determined by logical const-correctness.
	 * This is done because all wrapped functions can be const despite modifying the underlying objects
	 */
public:
	FDeltacastSdk() = default;

	FDeltacastSdk(const FDeltacastSdk &Other)     = delete;
	FDeltacastSdk(FDeltacastSdk &&Other) noexcept = delete;

public:
	// General
	[[nodiscard]] VHD_ERRORCODE GetApiInfo(VHD::ULONG *ApiVersion, VHD::ULONG *NbBoards) const;

	[[nodiscard]] std::optional<bool> IsBoardIndexValid(VHD::ULONG BoardIndex) const;

	[[nodiscard]] const char *GetBoardModel(VHD::ULONG BoardIndex) const;

	// Internal use only, use `Deltacast::Helpers::GetVideoCharacteristics` instead
	[[nodiscard]] VHD_ERRORCODE GetVideoCharacteristics_Internal(VHD_VIDEOSTANDARD VideoStandard, VHD::ULONG *Width, VHD::ULONG *Height, bool *Interlaced, VHD::ULONG *FrameRate) const;
	// Internal use only, use `Deltacast::Helpers::GetVideoCharacteristics` instead
	[[nodiscard]] VHD_ERRORCODE GetVideoCharacteristics_Internal(VHD_DV_HDMI_VIDEOSTANDARD VideoStandard, VHD::ULONG *Width, VHD::ULONG *Height, bool *Interlaced, VHD::ULONG *FrameRate) const;

public: // Board
	[[nodiscard]] std::optional<VHDHandle> OpenBoard(VHD::ULONG BoardIndex);
	VHD_ERRORCODE CloseBoardHandle(VHDHandle BoardHandle);

	[[nodiscard]] FORCEINLINE VHD_ERRORCODE GetBoardProperty(VHDHandle BoardHandle, VHD_CORE_BOARDPROPERTY Property, VHD::ULONG *Value) const;
	[[nodiscard]] FORCEINLINE VHD_ERRORCODE GetBoardProperty(VHDHandle BoardHandle, VHD_SDI_BOARDPROPERTY Property, VHD::ULONG *Value) const;
	[[nodiscard]] VHD_ERRORCODE GetBoardCapability(VHDHandle BoardHandle, VHD_CORE_BOARD_CAPABILITY BoardCapability, VHD::ULONG *Value) const;
	[[nodiscard]] VHD_ERRORCODE GetBoardCapSdiVideoStandard(VHDHandle BoardHandle, VHD_STREAMTYPE StreamType, VHD_VIDEOSTANDARD VideoStandard, bool *IsCapable) const;
	[[nodiscard]] VHD_ERRORCODE GetBoardCapSdiInterface(VHDHandle BoardHandle, VHD_STREAMTYPE StreamType, VHD_INTERFACE Interface, bool *IsCapable) const;

	FORCEINLINE VHD_ERRORCODE SetBoardProperty(VHDHandle BoardHandle, VHD_CORE_BOARDPROPERTY Property, VHD::ULONG Value);
	FORCEINLINE VHD_ERRORCODE SetBoardProperty(VHDHandle BoardHandle, VHD_SDI_BOARDPROPERTY Property, VHD::ULONG Value);

public: // Stream
	[[nodiscard]] std::optional<VHDHandle> OpenStream(VHDHandle BoardHandle, VHD_STREAMTYPE StreamType, VHD_SDI_STREAMPROCMODE ProcessingMode);
	[[nodiscard]] std::optional<VHDHandle> OpenStream(VHDHandle BoardHandle, VHD_STREAMTYPE StreamType, VHD_DV_STREAMPROCMODE ProcessingMode);
	VHD_ERRORCODE CloseStreamHandle(VHDHandle StreamHandle);

	FORCEINLINE VHD_ERRORCODE SetStreamProperty(const VHDHandle StreamHandle, const VHD_CORE_STREAMPROPERTY Property, const VHD::ULONG Value);
	FORCEINLINE VHD_ERRORCODE SetStreamProperty(const VHDHandle StreamHandle, const VHD_SDI_STREAMPROPERTY Property, const VHD::ULONG Value);
	FORCEINLINE VHD_ERRORCODE SetStreamProperty(const VHDHandle StreamHandle, const VHD_DV_STREAMPROPERTY Property, const VHD::ULONG Value);

	[[nodiscard]] VHD_ERRORCODE PresetTimingStreamProperties(VHDHandle StreamHandle, VHD_DV_STANDARD VideoStandard, VHD::ULONG ActiveWidth, VHD::ULONG ActiveHeight, VHD::ULONG RefreshRate, bool bInterlaced);

	[[nodiscard]] VHD_ERRORCODE StartStream(VHDHandle StreamHandle);
	VHD_ERRORCODE StopStream(VHDHandle StreamHandle);

public: // Slot & Buffer
	[[nodiscard]] VHD_ERRORCODE LockSlotHandle(VHDHandle StreamHandle, VHDHandle* SlotHandle);
	VHD_ERRORCODE UnlockSlotHandle(VHDHandle SlotHandle);

	[[nodiscard]] VHD_ERRORCODE GetSlotBuffer(VHDHandle SlotHandle, VHD::ULONG BufferType, VHD::BYTE** Buffer, VHD::ULONG* BufferSize) const;

public: // Timecode
	[[nodiscard]] VHD_ERRORCODE GetSlotTimecode(VHDHandle SlotHandle, VHD_TIMECODE_SOURCE TimecodeSource, VHD_TIMECODE* TimeCode) const;

	[[nodiscard]] VHD_ERRORCODE GetTimecode(VHDHandle BoardHandle, VHD_TIMECODE_SOURCE TcSource, bool *Locked, float *FrameRate, VHD_TIMECODE *TimeCode) const;

	[[nodiscard]] VHD_ERRORCODE DetectCompanionCard(VHDHandle BoardHandle, VHD_COMPANION_CARD_TYPE CompanionCardType, bool *IsPresent) const;

public: // Genlock
	[[nodiscard]] VHD_ERRORCODE StartTimer(VHDHandle BoardHandle, VHD_TIMER_SOURCE Source, VHDHandle *TimerHandle);
	VHD_ERRORCODE StopTimer(VHDHandle TimerHandle);

	[[nodiscard]] VHD_ERRORCODE WaitOnNextTimerTick(VHDHandle TimerHandle, VHD::ULONG Timeout) const;

public: // Utils for detection
	[[nodiscard]] std::optional<VHD::ULONG> GetNbBoards() const;

	[[nodiscard]] std::optional<VHD::ULONG> GetRxCount(VHDHandle BoardHandle) const;
	[[nodiscard]] std::optional<VHD::ULONG> GetTxCount(VHDHandle BoardHandle) const;

	[[nodiscard]] std::optional<VHD::ULONG> GetSdiPortCount(VHDHandle BoardHandle, bool IsInput) const;
	[[nodiscard]] std::optional<VHD::ULONG> GetDvPortCount(VHDHandle BoardHandle, bool IsInput) const;

	[[nodiscard]] std::optional<VHD_CHANNELTYPE> GetChannelType(VHDHandle BoardHandle, bool IsInput, VHD::ULONG PortIndex) const;

	[[nodiscard]] std::optional<bool> GetBoardCapSdiVideoStandard(VHDHandle BoardHandle, VHD_STREAMTYPE StreamType, VHD_VIDEOSTANDARD VideoStandard) const;
	[[nodiscard]] std::optional<bool> GetBoardCapSdiInterface(VHDHandle BoardHandle, VHD_STREAMTYPE StreamType, VHD_INTERFACE Interface) const;

	[[nodiscard]] std::optional<bool> IsFlexModule(VHDHandle BoardHandle) const;

	[[nodiscard]] std::optional<bool> IsFieldMergingSupported(VHDHandle BoardHandle) const;

	[[nodiscard]] FORCEINLINE std::optional<VHD::ULONG> GetStreamProperty(VHDHandle StreamHandle, VHD_CORE_STREAMPROPERTY Property) const;

public: // Utils for modification
	void SetByPassRelay(VHDHandle BoardHandle, VHD::ULONG PortIndex, VHD::ULONG LinkCount, int Value);


protected: //~ Deltacast::DynamicLibrary::DynamicLibraryLoader
	virtual Deltacast::DynamicLibrary::DynamicLibraryStatus LoadAllFunctionAddress() override;


private: // Hidden SDK board (wrapper with improved/simplified API)
	[[nodiscard]] VHD_ERRORCODE OpenBoardHandle(VHD::ULONG BoardIndex, VHDHandle* BoardHandle, VHDHandle OnStateChangeEvent, VHD::ULONG StateChangeMask);

	[[nodiscard]] VHD_ERRORCODE GetBoardProperty(VHDHandle BoardHandle, VHD::ULONG Property, VHD::ULONG* Value) const;
	[[nodiscard]] VHD_ERRORCODE SetBoardProperty(VHDHandle BoardHandle, VHD::ULONG Property, VHD::ULONG Value);

private: // Hidden SDK stream (wrapper with improved/simplified API)
	[[nodiscard]] VHD_ERRORCODE OpenStreamHandle(VHDHandle BoardHandle, VHD::ULONG StreamType, VHD::ULONG ProcessingMode, VHDHandle* StreamHandle);

	[[nodiscard]] std::optional<VHD::ULONG> GetStreamProperty(VHDHandle StreamHandle, VHD::ULONG Property) const;
	[[nodiscard]] VHD_ERRORCODE GetStreamProperty(VHDHandle StreamHandle, VHD::ULONG Property, VHD::ULONG* Value) const;

	[[nodiscard]] VHD_ERRORCODE SetStreamProperty(VHDHandle StreamHandle, VHD::ULONG Property, VHD::ULONG Value);

private: // SDK fixes
	static bool GetVideoCharacteristics_SdkFix(VHD_VIDEOSTANDARD VideoStandard, VHD::ULONG *Width, VHD::ULONG *Height, VHD::Bool *Interlaced, VHD::ULONG *FrameRate);

private:
	VHD_GetApiInfo Wrapper_GetApiInfo = nullptr;
	VHD_GetVideoCharacteristics Wrapper_GetVideoCharacteristics = nullptr;
	VHD_GetHdmiVideoCharacteristics Wrapper_GetHdmiVideoCharacteristics = nullptr;

	VHD_OpenBoardHandle Wrapper_OpenBoardHandle = nullptr;
	VHD_CloseBoardHandle Wrapper_CloseBoardHandle = nullptr;

	VHD_GetBoardModel Wrapper_GetBoardModel = nullptr;
	VHD_GetBoardProperty Wrapper_GetBoardProperty = nullptr;
	VHD_GetBoardCapability Wrapper_GetBoardCapability = nullptr;
	VHD_GetBoardCapSDIVideoStandard Wrapper_GetBoardCapSDIVideoStandard = nullptr;
	VHD_GetBoardCapSDIInterface Wrapper_GetBoardCapSDIInterface = nullptr;

	VHD_SetBoardProperty Wrapper_SetBoardProperty = nullptr;

	VHD_OpenStreamHandle Wrapper_OpenStreamHandle = nullptr;
	VHD_CloseStreamHandle Wrapper_CloseStreamHandle = nullptr;

	VHD_GetStreamProperty Wrapper_GetStreamProperty = nullptr;

	VHD_SetStreamProperty Wrapper_SetStreamProperty = nullptr;

	VHD_PresetTimingStreamProperties Wrapper_PresetTimingStreamProperties = nullptr;

	VHD_StartStream Wrapper_StartStream = nullptr;
	VHD_StartStream Wrapper_StopStream = nullptr;

	VHD_LockSlotHandle Wrapper_LockSlotHandle = nullptr;
	VHD_UnlockSlotHandle Wrapper_UnlockSlotHandle = nullptr;

	VHD_GetSlotBuffer Wrapper_GetSlotBuffer = nullptr;

	VHD_GetSlotTimecode Wrapper_GetSlotTimecode = nullptr;
	VHD_GetTimecode Wrapper_GetTimecode = nullptr;
	VHD_DetectCompanionCard Wrapper_DetectCompanionCard = nullptr;

	VHD_StartTimer Wrapper_StartTimer = nullptr;
	VHD_StopTimer Wrapper_StopTimer = nullptr;
	VHD_WaitOnNextTimerTick Wrapper_WaitOnNextTimerTick = nullptr;
	
private:
	inline static constexpr VHD_ERRORCODE FunctionNotLoaded = static_cast<VHD_ERRORCODE>(std::numeric_limits<VHD::ULONG>::max());
};



class DELTACASTMEDIA_API FDeltacast
{
public:
	static bool Initialize();

	static void Shutdown();

public:
	static bool IsInitialized();

	static FDeltacastSdk& GetSdk();

	static FName GetProtocolName();

private:
#ifdef WIN32
	inline static const auto LibraryName = L"VideoMasterHD";
#else
	inline static const auto LibraryName = L"libvideomasterhd.so";
#endif
};



VHD_ERRORCODE FDeltacastSdk::GetBoardProperty(const VHDHandle BoardHandle, const VHD_CORE_BOARDPROPERTY Property, VHD::ULONG* Value) const
{
	return GetBoardProperty(BoardHandle, static_cast<VHD::ULONG>(Property), Value);
}

VHD_ERRORCODE FDeltacastSdk::GetBoardProperty(const VHDHandle BoardHandle, const VHD_SDI_BOARDPROPERTY Property, VHD::ULONG* Value) const
{
	return GetBoardProperty(BoardHandle, static_cast<VHD::ULONG>(Property), Value);
}



VHD_ERRORCODE FDeltacastSdk::SetBoardProperty(const VHDHandle BoardHandle, const VHD_CORE_BOARDPROPERTY Property, const VHD::ULONG Value)
{
	return SetBoardProperty(BoardHandle, static_cast<VHD::ULONG>(Property), Value);
}

VHD_ERRORCODE FDeltacastSdk::SetBoardProperty(const VHDHandle BoardHandle, const VHD_SDI_BOARDPROPERTY Property, const VHD::ULONG Value)
{
	return SetBoardProperty(BoardHandle, static_cast<VHD::ULONG>(Property), Value);
}



std::optional<VHD::ULONG> FDeltacastSdk::GetStreamProperty(const VHDHandle StreamHandle, const VHD_CORE_STREAMPROPERTY Property) const
{
	return GetStreamProperty(StreamHandle, static_cast<VHD::ULONG>(Property));
}


VHD_ERRORCODE FDeltacastSdk::SetStreamProperty(const VHDHandle StreamHandle, const VHD_CORE_STREAMPROPERTY Property, const VHD::ULONG Value)
{
	return SetStreamProperty(StreamHandle, static_cast<VHD::ULONG>(Property), Value);
}

VHD_ERRORCODE FDeltacastSdk::SetStreamProperty(const VHDHandle StreamHandle, const VHD_SDI_STREAMPROPERTY Property, const VHD::ULONG Value)
{
	return SetStreamProperty(StreamHandle, static_cast<VHD::ULONG>(Property), Value);
}

VHD_ERRORCODE FDeltacastSdk::SetStreamProperty(const VHDHandle StreamHandle, const VHD_DV_STREAMPROPERTY Property, const VHD::ULONG Value)
{
	return SetStreamProperty(StreamHandle, static_cast<VHD::ULONG>(Property), Value);
}