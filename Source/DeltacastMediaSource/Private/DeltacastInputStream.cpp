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

#include "DeltacastInputStream.h"

#include "DeltacastMediaSettings.h"
#include "DeltacastMediaTextureSample.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaSourceModule.h"
#include "HAL/UnrealMemory.h"


FDeltacastInputStream::FDeltacastInputStream(const FDeltacastInputStreamConfig &Config)
	: Callback(Config.Callback),
	  bIsSdi(Config.bIsSdi),
	  SdiPortConfig(Config.SdiPortConfig),
	  DvPortConfig(Config.DvPortConfig),
	  bAutoLoadEdid(Config.bAutoLoadEdid),
	  TimecodeFormat(Config.TimecodeFormat),
	  bErrorOnSourceLost(Config.bErrorOnSourceLost)
{
	check(Callback);

	StreamStatistics.bUpdateProcessedFrameCount = false;
	StreamStatistics.bUpdateDroppedFrameCount   = Config.bLogDroppedFrameCount;
	StreamStatistics.bUpdateBufferFill          = false;
	StreamStatistics.NumberOfDeltacastBuffers   = BasePortConfig().BufferDepth;
}


bool FDeltacastInputStream::Init()
{
	auto &DeltacastSdk = FDeltacast::GetSdk();

	const auto& BaseConfig = BasePortConfig();

	BoardHandle = DeltacastSdk.OpenBoard(BaseConfig.BoardIndex).value_or(VHD::InvalidHandle);
	if (BoardHandle == VHD::InvalidHandle)
	{
		Callback->OnInitializationCompleted(false);
		Exit();
		return false;
	}

	ComputeConstants();
	ComputeTimecodeSource();
	RegisterSettingsEvent();


	const auto bRequireLinePadding = Deltacast::Helpers::RequiresLinePadding(Width);

	bInterlaced = bIsSdi
		              ? !Deltacast::Helpers::IsProgressive(SdiPortConfig.VideoStandard)
		              : !Deltacast::Helpers::IsProgressive(DvPortConfig.VideoStandard);

	const auto bIsDv = !bIsSdi;

	StreamStatistics.Reset();

	const auto ChannelType = DeltacastSdk.GetChannelType(BoardHandle, true, BaseConfig.PortIndex);
	if (Deltacast::Helpers::IsAsi(ChannelType.value_or(VHD_CHANNELTYPE::NB_VHD_CHANNELTYPE)) &&
		Deltacast::Helpers::IsSdi(ChannelType.value_or(VHD_CHANNELTYPE::NB_VHD_CHANNELTYPE)))
	{
		const auto ChannelMode = Deltacast::Helpers::GetChannelMode(BaseConfig.PortIndex);
		DeltacastSdk.SetBoardProperty(BoardHandle, ChannelMode, static_cast<VHD::ULONG>(VHD_CHANNEL_MODE::VHD_CHANNEL_MODE_SDI));
	}

	if (bIsSdi)
	{
		DeltacastSdk.SetByPassRelay(BoardHandle, BaseConfig.PortIndex, SdiPortConfig.IsSingleLink() ? 1 : 4, VHD::False);
	}

	const auto StreamType = Deltacast::Helpers::GetStreamTypeFromPortIndex(BaseConfig.bIsInput, BaseConfig.PortIndex);

	if (bIsSdi)
	{
		StreamHandle = DeltacastSdk.OpenStream(BoardHandle, StreamType, VHD_SDI_STREAMPROCMODE::VHD_SDI_STPROC_DISJOINED_VIDEO).value_or(VHD::InvalidHandle);
	}
	if (bIsDv)
	{
		StreamHandle = DeltacastSdk.OpenStream(BoardHandle, StreamType, VHD_DV_STREAMPROCMODE::VHD_DV_STPROC_DISJOINED_VIDEO).value_or(VHD::InvalidHandle);
	}

	if (StreamHandle == VHD::InvalidHandle)
	{
		Callback->OnInitializationCompleted(false);
		Exit();
		return false;
	}

	if (bIsDv)
	{
		const auto LoadEdidPropertyValue = !bAutoLoadEdid ? VHD::True : VHD::False;
		[[maybe_unused]] const auto SetAutoLoadEdidResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_DV_STREAMPROPERTY::VHD_DV_SP_DISABLE_EDID_AUTO_LOAD, LoadEdidPropertyValue);

		[[maybe_unused]] const auto SetModeResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_DV_STREAMPROPERTY::VHD_DV_SP_MODE, static_cast<VHD::ULONG>(VHD_DV_MODE::VHD_DV_MODE_HDMI));

		const auto VideoCharacteristics = Deltacast::Helpers::GetVideoCharacteristics(DvPortConfig.VideoStandard);
		if (!VideoCharacteristics.has_value())
		{
			UE_LOG(LogDeltacastMediaSource, Error, TEXT("Failed to get video characteristics"));
			Callback->OnInitializationCompleted(false);
			Exit();
			return false;
		}

		[[maybe_unused]] const auto SetWidthResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_DV_STREAMPROPERTY::VHD_DV_SP_ACTIVE_WIDTH, VideoCharacteristics->Width);
		[[maybe_unused]] const auto SetHeightResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_DV_STREAMPROPERTY::VHD_DV_SP_ACTIVE_HEIGHT, VideoCharacteristics->Height);
		[[maybe_unused]] const auto SetInterlacedResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_DV_STREAMPROPERTY::VHD_DV_SP_INTERLACED, VideoCharacteristics->bIsInterlaced);
		[[maybe_unused]] const auto SetRefreshRateResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_DV_STREAMPROPERTY::VHD_DV_SP_REFRESH_RATE, VideoCharacteristics->FrameRate);

		Deltacast::Helpers::FCablePacking CablePacking(BaseConfig.BufferPacking, Deltacast::Helpers::IsSd(DvPortConfig.VideoStandard));

		[[maybe_unused]] const auto SetColorSpaceResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_DV_STREAMPROPERTY::VHD_DV_SP_CS, static_cast<VHD::ULONG>(CablePacking.ColorSpace));
	}

	if (bIsSdi)
	{
		[[maybe_unused]] const auto SetVideoStandardResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_SDI_STREAMPROPERTY::VHD_SDI_SP_VIDEO_STANDARD, static_cast<VHD::ULONG>(SdiPortConfig.VideoStandard));
		[[maybe_unused]] const auto SetInterfaceResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_SDI_STREAMPROPERTY::VHD_SDI_SP_INTERFACE, static_cast<VHD::ULONG>(SdiPortConfig.Interface));
	}

	[[maybe_unused]] const auto SetBufferQueueDepthResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_BUFFERQUEUE_DEPTH, BaseConfig.BufferDepth);
	[[maybe_unused]] const auto SetBufferPackingResult    = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_BUFFER_PACKING, static_cast<VHD::ULONG>(BaseConfig.BufferPacking));

	[[maybe_unused]] const auto SetTransferSchemeResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_TRANSFER_SCHEME, static_cast<VHD::ULONG>(VHD_TRANSFERSCHEME::VHD_TRANSFER_SLAVED));
	[[maybe_unused]] const auto SetIOTimeoutResult      = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_IO_TIMEOUT, 500);

	if (bRequireLinePadding)
	{
		[[maybe_unused]] const auto SetLinePaddingResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_LINE_PADDING, 128);
	}

	if (bInterlaced)
	{
		bFieldMergingSupported = DeltacastSdk.IsFieldMergingSupported(BoardHandle).value_or(false);

		if (bFieldMergingSupported)
		{
			const auto SetLinePaddingResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_FIELD_MERGE, VHD::True);
			if (!Deltacast::Helpers::IsValid(SetLinePaddingResult))
			{
				UE_LOG(LogDeltacastMediaSource, Warning, TEXT("Failed to set field merging stream property: %s"),
					*Deltacast::Helpers::GetErrorString(SetLinePaddingResult));
				bFieldMergingSupported = false;
			}
		}
	}

	const auto StartStreamResult = DeltacastSdk.StartStream(StreamHandle);
	if (!Deltacast::Helpers::IsValid(StartStreamResult))
	{
		UE_LOG(LogDeltacastMediaSource, Error, TEXT("Failed to start stream: %s"), *Deltacast::Helpers::GetErrorString(StartStreamResult));
		Callback->OnInitializationCompleted(false);
		Exit();
		return false;
	}

	Callback->OnInitializationCompleted(true);

	return true;
}

uint32 FDeltacastInputStream::Run()
{
	static constexpr auto BufferType = static_cast<VHD::ULONG>(VHD_SDI_BUFFERTYPE::VHD_SDI_BT_VIDEO);

	auto& DeltacastSdk = FDeltacast::GetSdk();

	const auto ChannelStatus = Deltacast::Helpers::GetChannelStatusFromPortIndex(true, BasePortConfig().PortIndex);

	WaitForChannelLocked(ChannelStatus);
	
	while (!bStopRequested)
	{
		VHDHandle SlotHandle = VHD::InvalidHandle;

		const auto LockSlotResult = DeltacastSdk.LockSlotHandle(StreamHandle, &SlotHandle);
		if (!Deltacast::Helpers::IsValid(LockSlotResult))
		{
			const auto LockResult = static_cast<VHD_ERRORCODE>(LockSlotResult);
			UE_CLOG(LockResult != VHD_ERRORCODE::VHDERR_TIMEOUT, LogDeltacastMediaSource, Error,
			        TEXT("Failed to lock the slot for media '%s' with error: %s"),
			       *ConfigString(), *Deltacast::Helpers::GetErrorString(LockSlotResult));
			UE_CLOG(LockResult == VHD_ERRORCODE::VHDERR_TIMEOUT, LogDeltacastMediaSource, Error,
			        TEXT("Timeout when locking the slot for media '%s'"), *ConfigString());

			if (LockResult == VHD_ERRORCODE::VHDERR_TIMEOUT)
			{
				 if (bErrorOnSourceLost)
				 {
					 bSourceError = true;
					 break;
				 }
				 else
				 {
					 VHD::ULONG Status = 0;
					 const auto Result = DeltacastSdk.GetBoardProperty(BoardHandle, ChannelStatus, &Status);
					 if (!Deltacast::Helpers::IsValid(Result) || (Status & VHD::VHD_CORE_RXSTS_UNLOCKED))
					 {
						 Callback->OnSourceStopped();

						 WaitForChannelLocked(ChannelStatus);
						 
						 if (!bStopRequested)
						 {
						 	Callback->OnSourceResumed();
						 }
					 }
				 }
			}

			continue;
		}

		VHD_TIMECODE Timecode{};
		if (TimecodeFormat == EMediaIOTimecodeFormat::LTC)
		{
			const auto TimecodeResult = DeltacastSdk.GetSlotTimecode(SlotHandle, TimecodeSource, &Timecode);

			if (!Deltacast::Helpers::IsValid(TimecodeResult))
			{
				UE_LOG(LogDeltacastMediaSource, Error, TEXT("Failed to get slot timecode: %s"), *Deltacast::Helpers::GetErrorString(TimecodeResult));
			}
		}

		const auto CleanUp = [&]()
		{
			[[maybe_unused]] const auto UnlockSlotResult = DeltacastSdk.UnlockSlotHandle(SlotHandle);

			SlotHandle = VHD::InvalidHandle;
		};

		VHD::BYTE* Buffer     = nullptr;
		VHD::ULONG BufferSize = 0;

		const auto GetBufferResult = DeltacastSdk.GetSlotBuffer(SlotHandle, BufferType, &Buffer, &BufferSize);
		if (!Deltacast::Helpers::IsValid(GetBufferResult))
		{
			UE_LOG(LogDeltacastMediaSource, Error, TEXT("Failed to get slot buffer for media '%s' with error: %s"),
			       *SdiPortConfig.ToString(), *Deltacast::Helpers::GetErrorString(GetBufferResult));
			CleanUp();
			continue;
		}

		auto RequestedBuffer = FDeltacastRequestedBuffer{};
		auto RequestBuffer = FDeltacastRequestBuffer{};
		RequestBuffer.VideoBufferSize = BufferSize;

		if (Callback->OnRequestInputBuffer(RequestBuffer, RequestedBuffer))
		{
			auto VideoFrameData = FDeltacastVideoFrameData{};

			VideoFrameData.VideoBufferSize = BufferSize;

			VideoFrameData.Width  = Width;
			VideoFrameData.Height = Height;
			VideoFrameData.Stride = Stride;

			VideoFrameData.bIsProgressive = !bInterlaced;

			VideoFrameData.MetaData.Timecode = Timecode;

			VideoFrameData.MetaData.FrameCount = StreamStatistics.ProcessedFrameCount;
			VideoFrameData.MetaData.DropCount  = StreamStatistics.DroppedFrameCount;
			VideoFrameData.MetaData.BufferFill = StreamStatistics.BufferFill;

			if (RequestedBuffer.VideoBuffer != nullptr)
			{
				if (bInterlaced && !bFieldMergingSupported)
				{
					const auto &EngineBuffer = RequestedBuffer.VideoBuffer;
					const auto  BytesPerRow  = Stride;

					const auto C = Height / 2;
					for (uint32 Row = 0; Row < Height; Row += 2)
					{
						const auto SourceEvenLine = Buffer + (((Row / 2) + C) * BytesPerRow);
						const auto SourceOddLine  = Buffer + (((Row / 2) + 0) * BytesPerRow);

						const auto DestinationEvenLine = EngineBuffer + ((Row + 0) * BytesPerRow);
						const auto DestinationOddLine  = EngineBuffer + ((Row + 1) * BytesPerRow);

						std::memcpy(DestinationEvenLine, SourceEvenLine, BytesPerRow);
						std::memcpy(DestinationOddLine, SourceOddLine, BytesPerRow);
					}
				}
				else
				{
					FMemory::Memcpy(RequestedBuffer.VideoBuffer, Buffer, BufferSize);
				}

				VideoFrameData.VideoBuffer = RequestedBuffer.VideoBuffer;

				CleanUp();

				Callback->OnInputFrameReceived(VideoFrameData);
			}
			else
			{
				VideoFrameData.VideoBuffer = Buffer;

				Callback->OnInputFrameReceived(VideoFrameData);

				CleanUp();
			}
		}
		else
		{
			UE_LOG(LogDeltacastMediaSource, Error, TEXT("Failed to get input buffer for media %s"), *SdiPortConfig.ToString());
			CleanUp();
		}

		Deltacast::Helpers::GetStreamStatistics(StreamStatistics, StreamHandle);
	}

	return 0;
}

void FDeltacastInputStream::Exit()
{
	if (BoardHandle != VHD::InvalidHandle && StreamHandle != VHD::InvalidHandle)
	{
		auto& DeltacastSdk = FDeltacast::GetSdk();

		[[maybe_unused]] const auto StopStreamResult = DeltacastSdk.StopStream(StreamHandle);
		[[maybe_unused]] const auto CloseStreamResult = DeltacastSdk.CloseStreamHandle(StreamHandle);

		StreamHandle = VHD::InvalidHandle;

		if (bIsSdi)
		{
			DeltacastSdk.SetByPassRelay(BoardHandle, BasePortConfig().PortIndex, SdiPortConfig.IsSingleLink() ? 1 : 4, VHD::True);
		}

		[[maybe_unused]] const auto CloseBoardHandleResult = DeltacastSdk.CloseBoardHandle(BoardHandle);
		BoardHandle = VHD::InvalidHandle;

		StreamStatistics.Reset();
	}

	Callback->OnCompletion(!bSourceError);
}


void FDeltacastInputStream::Stop()
{
	bStopRequested = true;
}



void FDeltacastInputStream::WaitForChannelLocked(const VHD_CORE_BOARDPROPERTY ChannelStatus) const
{
	const auto &DeltacastSdk = FDeltacast::GetSdk();

	VHD::ULONG Status = 0;

	VHD_ERRORCODE Result;

	do
	{
		Result = DeltacastSdk.GetBoardProperty(BoardHandle, ChannelStatus, &Status);

		FPlatformProcess::Sleep(Deltacast::Helpers::RxStatusSleepMs);
	}
	while (!bStopRequested && Deltacast::Helpers::IsValid(Result) && (Status & VHD::VHD_CORE_RXSTS_UNLOCKED));
}


void FDeltacastInputStream::ComputeConstants()
{
	const auto VideoCharacteristics = bIsSdi
		                                  ? Deltacast::Helpers::GetVideoCharacteristics(SdiPortConfig.VideoStandard)
		                                  : Deltacast::Helpers::GetVideoCharacteristics(DvPortConfig.VideoStandard);
	if (!VideoCharacteristics.has_value())
	{
		[[maybe_unused]] const auto VideoStandardString = bIsSdi
			                                                  ? Deltacast::Helpers::GetVideoStandardString(SdiPortConfig.VideoStandard)
			                                                  : Deltacast::Helpers::GetVideoStandardString(DvPortConfig.VideoStandard);
		UE_LOG(LogDeltacastMediaSource, Error, TEXT("Failed to get video characteristics: %s"), *VideoStandardString);
	}

	const auto& BaseConfig = BasePortConfig();

	Stride = VideoCharacteristics->Width * 4;
	Height = VideoCharacteristics->Height;
	Width  = VideoCharacteristics->Width;
	
	switch (BaseConfig.BufferPacking)
	{
		case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_RGB_32:
			Stride = VideoCharacteristics->Width * 4;
			break;
		case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_8:
			Stride = VideoCharacteristics->Width * 2;
			break;
		case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_10:
			if (Deltacast::Helpers::RequiresLinePadding(VideoCharacteristics->Width))
			{
				Stride = Align(VideoCharacteristics->Width * 8 / 3, 128);
			}
			else
			{
				Stride = VideoCharacteristics->Width * 8 / 3;
			}
			break;
		default:
			UE_LOG(LogDeltacastMediaSource, Fatal, TEXT("Unhandled pixel format: %u"), BaseConfig.BufferPacking);
			break;
	}
}

void FDeltacastInputStream::ComputeTimecodeSource()
{
	const auto Settings = GetDefault<UDeltacastMediaSettings>();
	if (Settings == nullptr)
	{
		UE_LOG(LogDeltacastMediaSource, Warning, TEXT("Cannot get Deltacast Media Plugin Settings, timecode will be ignored"));
		return;
	}

	const auto BoardIndex = BasePortConfig().BoardIndex;

	const auto BoardSettings = Settings->GetBoardSettings(BoardIndex);

	if (BoardSettings == nullptr)
	{
		UE_LOG(LogDeltacastMediaSource, Warning, TEXT("Deltacast Media Plugin Settings are not filled for board index: %d, timecode will be ignored"),
		       BoardIndex);
		return;
	}

	switch (BoardSettings->TimecodeSource)
	{
		case EDeltacastTimecodeSource::OnBoard:
			TimecodeSource = VHD_TIMECODE_SOURCE::VHD_TC_SRC_LTC_ONBOARD;
			return;
		case EDeltacastTimecodeSource::CompanionCard:
			TimecodeSource = VHD_TIMECODE_SOURCE::VHD_TC_SRC_LTC_COMPANION_CARD;
			return;
		default:
			UE_LOG(LogDeltacastMediaSource, Fatal, TEXT("Unhandled `DeltacastTimecodeSource`: %d, timecode will be ignored"),
			       BoardSettings->TimecodeSource);
			return;
	}
}

FString FDeltacastInputStream::ConfigString() const
{
	return bIsSdi ? SdiPortConfig.ToString() : DvPortConfig.ToString();
}




void FDeltacastInputStream::RegisterSettingsEvent()
{
#if WITH_EDITOR
	UpdateSettingsDelegateHandle = UDeltacastMediaSettings::OnSettingsChanged().AddThreadSafeSP(this, &FDeltacastInputStream::UpdateSettings);
#endif
}

void FDeltacastInputStream::UnregisterSettingsEvent()
{
#if WITH_EDITOR
	UDeltacastMediaSettings::OnSettingsChanged().Remove(UpdateSettingsDelegateHandle);
	UpdateSettingsDelegateHandle.Reset();
#endif
}


#if WITH_EDITOR
void FDeltacastInputStream::UpdateSettings(const UDeltacastMediaSettings* Settings)
{
	ComputeTimecodeSource();
}
#endif


const FDeltacastInputStream::FDcBaseConfig& FDeltacastInputStream::BasePortConfig() const
{
	return bIsSdi ? SdiPortConfig.Base : DvPortConfig.Base;
}
