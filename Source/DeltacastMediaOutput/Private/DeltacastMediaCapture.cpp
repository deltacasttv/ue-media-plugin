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

#include "DeltacastMediaCapture.h"

#include "DeltacastDefinition.h"
#include "DeltacastHelpers.h"
#include "DeltacastMediaOutput.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaModule.h"
#include "IDeltacastMediaOutputModule.h"
#include "MediaIOCoreEncodeTime.h"
#include "MediaIOCoreFileWriter.h"
#include "Misc/ScopeLock.h"
#include "Slate/SceneViewport.h"
#include "Widgets/SViewport.h"

#include <cstring>

DECLARE_CYCLE_STAT(TEXT("OnFrameCaptured"), STAT_OnFrameCaptured, STATGROUP_Deltacast);

bool bDeltacastWriteInputRawDataCmdEnable = false;
static FAutoConsoleCommand DeltacastWriteInputRawDataCmd(
	TEXT("Deltacast.Capture.WriteInputRawData"),
	TEXT("Write Deltacast raw input buffer to file."),
	FConsoleCommandDelegate::CreateLambda([]() { bDeltacastWriteInputRawDataCmdEnable = true; }));


namespace DeltacastMediaCaptureUtils
{
	auto GetBufferPackingFromPixelFormat(const EDeltacastMediaOutputPixelFormat PixelFormat)
	{
		switch (PixelFormat)
		{
			case EDeltacastMediaOutputPixelFormat::PF_8BIT_RGBA:
				return VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_RGB_32;
			case EDeltacastMediaOutputPixelFormat::PF_10BIT_YUV422:
				return VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_10;
			case EDeltacastMediaOutputPixelFormat::PF_8BIT_YUV422: [[fallthrough]];
			default:
				return VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_8;
		}
	}

	std::optional<VHD_INTERFACE> GetInterface(const EMediaIOTransportType         TransportType,
	                                          const EMediaIOQuadLinkTransportType QuadTransportType,
	                                          const VHD_VIDEOSTANDARD             VideoStandard)
	{
		switch (TransportType)
		{
			case EMediaIOTransportType::SingleLink:
				return Deltacast::Helpers::GetSingleLinkInterface(VideoStandard);
			case EMediaIOTransportType::QuadLink:
				{
					const auto QuadLinkType = [QuadTransportType]() -> std::optional<Deltacast::Helpers::EQuadLinkType>
					{
						switch (QuadTransportType)
						{
							case EMediaIOQuadLinkTransportType::SquareDivision:
								return Deltacast::Helpers::EQuadLinkType::Quadrant;
							case EMediaIOQuadLinkTransportType::TwoSampleInterleave:
								return Deltacast::Helpers::EQuadLinkType::TwoSampleInterleaved;
							default:
								UE_LOG(LogDeltacastMediaOutput, Fatal, TEXT("Unhandled `EMediaIOQuadLinkTransportType`: %u."), QuadTransportType);
								return {};
						}
					}();

					if (!QuadLinkType.has_value())
					{
						return {};
					}

					return Deltacast::Helpers::GetQuadLinkInterface(VideoStandard, QuadLinkType.value());
				}
			case EMediaIOTransportType::DualLink: [[fallthrough]];
			case EMediaIOTransportType::HDMI: [[fallthrough]];
			default:
				UE_LOG(LogDeltacastMediaOutput, Fatal, TEXT("Unhandled `EMediaIOTransportType`: %u."), TransportType);
				return {};
		}
	}
}


bool UDeltacastMediaCapture::HasFinishedProcessing() const
{
	return Super::HasFinishedProcessing() || StreamHandle == VHD::InvalidHandle;
}

bool UDeltacastMediaCapture::InitializeCapture()
{
	return true;
}


bool UDeltacastMediaCapture::PostInitializeCaptureRenderTarget(UTextureRenderTarget2D *InRenderTarget)
{
	UDeltacastMediaOutput* DeltacastMediaSource = CastChecked<UDeltacastMediaOutput>(MediaOutput);
	const bool bResult = Initialize(DeltacastMediaSource);
	return bResult;
}

bool UDeltacastMediaCapture::PostInitializeCaptureViewport(TSharedPtr<FSceneViewport> &InSceneViewport)
{
	UDeltacastMediaOutput *DeltacastMediaSource = CastChecked<UDeltacastMediaOutput>(MediaOutput);

	const bool bResult = Initialize(DeltacastMediaSource);
	if (bResult)
	{
		ApplyViewportTextureAlpha(InSceneViewport);
	}

	return bResult;
}

void UDeltacastMediaCapture::StopCaptureImpl(const bool bAllowPendingFrameToBeProcess)
{
	if (!bAllowPendingFrameToBeProcess)
	{
		{
			// Prevent the rendering thread from copying while we are stopping the capture.
			FScopeLock ScopeLock(&RenderThreadCriticalSection);

			if (StreamHandle != VHD::InvalidHandle)
			{
				auto &DeltacastSdk = FDeltacast::GetSdk();

				[[maybe_unused]] const auto StopStreamResult        = DeltacastSdk.StopStream(StreamHandle);
				[[maybe_unused]] const auto CloseStreamHandleResult = DeltacastSdk.CloseStreamHandle(StreamHandle);
				StreamHandle                                        = VHD::InvalidHandle;

				const UDeltacastMediaOutput *DeltacastMediaSource = CastChecked<UDeltacastMediaOutput>(MediaOutput);
				check(DeltacastMediaSource);

				const auto PortIndex     = DeltacastMediaSource->OutputConfiguration.MediaConfiguration.MediaConnection.PortIdentifier;
				const auto TransportType = DeltacastMediaSource->OutputConfiguration.MediaConfiguration.MediaConnection.TransportType;
				const auto LinkCount     = TransportType == EMediaIOTransportType::SingleLink ||
				                           TransportType == EMediaIOTransportType::HDMI
					                           ? 1
					                           : 4;

				const int32 DeviceModeIdentifier = DeltacastMediaSource->OutputConfiguration.MediaConfiguration.MediaMode.DeviceModeIdentifier;
				const auto bIsSdiMode = Deltacast::Helpers::IsDeviceModeIdentifierSdi(DeviceModeIdentifier);

				DeltacastSdk.SetLoopbackState(BoardHandle, PortIndex, LinkCount, VHD::True);

				[[maybe_unused]] const auto CloseBoardHandleResult = DeltacastSdk.CloseBoardHandle(BoardHandle);
				BoardHandle                                        = VHD::InvalidHandle;

				ResetStatistics();
			}
		}

		RestoreViewportTextureAlpha(GetCapturingSceneViewport());
	}
}

bool UDeltacastMediaCapture::UpdateRenderTargetImpl(UTextureRenderTarget2D *InRenderTarget)
{
	RestoreViewportTextureAlpha(GetCapturingSceneViewport());
	return true;
}

bool UDeltacastMediaCapture::UpdateSceneViewportImpl(TSharedPtr<FSceneViewport> &InSceneViewport)
{
	RestoreViewportTextureAlpha(GetCapturingSceneViewport());
	ApplyViewportTextureAlpha(InSceneViewport);
	return true;
}

bool UDeltacastMediaCapture::ValidateMediaOutput() const
{
	const UDeltacastMediaOutput* DeltacastMediaOutput = Cast<UDeltacastMediaOutput>(MediaOutput);
	if (DeltacastMediaOutput == nullptr)
	{
		UE_LOG(LogDeltacastMediaOutput, Error, TEXT("Can not start the capture. MediaOutput's class is not supported."));
		return false;
	}

	return true;
}


void UDeltacastMediaCapture::OnFrameCaptured_RenderingThread(const FCaptureBaseData &InBaseData, TSharedPtr<FMediaCaptureUserData, ESPMode::ThreadSafe> InUserData, void *InBuffer, const int32 Width, const int32 Height, const int32 BytesPerRow)
{
	// Prevent the rendering thread from copying while we are stopping the capture.
	SCOPE_CYCLE_COUNTER(STAT_OnFrameCaptured);
	FScopeLock ScopeLock(&RenderThreadCriticalSection);

	if (StreamHandle != VHD::InvalidHandle)
	{
		const auto EngineBuffer = reinterpret_cast<VHD::BYTE*>(InBuffer);
		if (EngineBuffer == nullptr)
		{
			UE_LOG(LogDeltacastMediaOutput, Error, TEXT("Failed to convert buffer type"));
			return;
		}

		uint32 Stride = Width * 4;
		uint32 TimeEncodeWidth = Width;
		EMediaIOCoreEncodePixelFormat EncodePixelFormat = EMediaIOCoreEncodePixelFormat::CharBGRA;
		FString OutputFilename;

		switch (BufferPacking)
		{
			case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_RGB_32:
				Stride = Width * 4;
				TimeEncodeWidth = Width;
				EncodePixelFormat = EMediaIOCoreEncodePixelFormat::CharBGRA;
				OutputFilename = TEXT("Deltacast_Input_8_RGBA");
				break;
			case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_8:
				Stride = Width * 4;
				TimeEncodeWidth = Width * 2;
				EncodePixelFormat = EMediaIOCoreEncodePixelFormat::CharUYVY;
				OutputFilename = TEXT("Deltacast_Input_8_YUV");
				break;
			case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_10:
				Stride = Width * 16;
				TimeEncodeWidth = Width * 6;
				EncodePixelFormat = EMediaIOCoreEncodePixelFormat::YUVv210;
				OutputFilename = TEXT("Deltacast_Input_10_YUV");
				break;
			default:
				break;
		}
		
		if (bEncodeTimecodeInTexel)
		{
			const auto AlignedStride = Align(Stride, 256);
			const FMediaIOCoreEncodeTime EncodeTime(EncodePixelFormat, InBuffer, AlignedStride, TimeEncodeWidth, Height);
			const auto& Timecode = InBaseData.SourceFrameTimecode;
			EncodeTime.Render(Timecode.Hours, Timecode.Minutes, Timecode.Seconds, Timecode.Frames);
		}

		auto& DeltacastSdk = FDeltacast::GetSdk();

		VHDHandle  SlotHandle     = VHD::InvalidHandle;
		const auto LockSlotResult = DeltacastSdk.LockSlotHandle(StreamHandle, &SlotHandle);
		if (!Deltacast::Helpers::IsValid(LockSlotResult))
		{
			UE_CLOG(LockSlotResult != VHD_ERRORCODE::VHDERR_TIMEOUT, LogDeltacastMediaOutput, Error,
			        TEXT("Failed to lock slot: %s"), *Deltacast::Helpers::GetErrorString(LockSlotResult));

			static constexpr auto WarnFrameCount = uint32{ 10 };
			const auto bShouldWarn = (AdjacentFrameDropped == 0 || LastFrameDroppedWarnedCount - AdjacentFrameDropped >= WarnFrameCount) &&
				                      LockSlotResult == VHD_ERRORCODE::VHDERR_TIMEOUT;
			
			++AdjacentFrameDropped;

			if (bShouldWarn)
			{
				LastFrameDroppedWarnedCount = AdjacentFrameDropped;
				UE_LOG(LogDeltacastMediaOutput, Error, TEXT("Cannot lock slot, dropping %u frames"), AdjacentFrameDropped);
			}

			return;
		}

		AdjacentFrameDropped = 0;
		LastFrameDroppedWarnedCount = 0;

		VHD::ULONG BufferSize      = 0;
		VHD::BYTE* Buffer          = nullptr;
		const auto GetBufferResult = DeltacastSdk.GetSlotBuffer(SlotHandle, static_cast<VHD::ULONG>(VHD_SDI_BUFFERTYPE::VHD_SDI_BT_VIDEO), &Buffer,
		                                                        &BufferSize);
		if (Deltacast::Helpers::IsValid(GetBufferResult))
		{
			const auto EngineBufferSize = Height * BytesPerRow;

			if (bInterlaced && !bFieldMergingSupported)
			{
				check(Stride <= (uint32)BytesPerRow);

				const auto C = Height / 2;
				for (int Row = 0; Row < Height; Row += 2)
				{
					const auto SourceEvenLine = EngineBuffer + ((Row + 0) * BytesPerRow);
					const auto SourceOddLine  = EngineBuffer + ((Row + 1) * BytesPerRow);

					const auto DestinationEvenLine = Buffer + (((Row / 2) + C) * Stride);
					const auto DestinationOddLine  = Buffer + (((Row / 2) + 0) * Stride);

					std::memcpy(DestinationEvenLine, SourceEvenLine, Stride);
					std::memcpy(DestinationOddLine, SourceOddLine, Stride);
				}
			}
			else
			{
				if (BufferSize != EngineBufferSize)
				{
					check(Stride <= (uint32)BytesPerRow);

					for (int Row = 0; Row < Height; ++Row)
					{
						std::memcpy(Buffer + (Row * Stride), EngineBuffer + (Row * BytesPerRow), Stride);
					}
				}
				else
				{
					std::memcpy(Buffer, EngineBuffer, BufferSize);
				}
			}
		}
		else
		{
			UE_LOG(LogDeltacastMediaOutput, Error, TEXT("Failed get the sot buffer: %s"), *Deltacast::Helpers::GetErrorString(GetBufferResult));
		}

		[[maybe_unused]] const auto UnlockSlotResult = DeltacastSdk.UnlockSlotHandle(SlotHandle);

		UpdateStatistics();


		if (bDeltacastWriteInputRawDataCmdEnable)
		{
			const auto InBufferSize = Height * BytesPerRow;
			MediaIOCoreFileWriter::WriteRawFile(OutputFilename, reinterpret_cast<uint8*>(InBuffer), InBufferSize);
			bDeltacastWriteInputRawDataCmdEnable = false;
		}
	}
	else if (GetState() != EMediaCaptureState::Stopped)
	{
		SetState(EMediaCaptureState::Error);
	}
}


bool UDeltacastMediaCapture::Initialize(const UDeltacastMediaOutput *InMediaOutput)
{
	check(InMediaOutput);

	const auto& MediaModule = FModuleManager::LoadModuleChecked<IDeltacastMediaModule>(TEXT("DeltacastMedia"));
	if (!MediaModule.CanBeUsed())
	{
		UE_LOG(LogDeltacastMediaOutput, Warning,
		       TEXT("The DeltacastMediaCapture can't open MediaOutput '%s' because Deltacast board cannot be used."),
		       *InMediaOutput->GetName());
		SetState(EMediaCaptureState::Error);
		return false;
	}

	bEncodeTimecodeInTexel = InMediaOutput->bEncodeTimecodeInTexel;

	auto& DeltacastSdk = FDeltacast::GetSdk();

	const int32 DeviceModeIdentifier = InMediaOutput->OutputConfiguration.MediaConfiguration.MediaMode.DeviceModeIdentifier;

	const auto BoardIndex            = static_cast<VHD::ULONG>(InMediaOutput->OutputConfiguration.MediaConfiguration.MediaConnection.Device.DeviceIdentifier);
	const auto PortIndex             = InMediaOutput->OutputConfiguration.MediaConfiguration.MediaConnection.PortIdentifier;

	const auto bIsEuropeanClock      = Deltacast::Helpers::IsDeviceModeIdentifierEuropeanClock(DeviceModeIdentifier);
	const auto bIsSdiMode            = Deltacast::Helpers::IsDeviceModeIdentifierSdi(DeviceModeIdentifier);
	const auto bIsDvMode             = Deltacast::Helpers::IsDeviceModeIdentifierDv(DeviceModeIdentifier);
	const auto SdiVideoStandard      = bIsSdiMode
		                                   ? Deltacast::Helpers::GetSdiVideoStandardFromDeviceModeIdentifier(DeviceModeIdentifier)
		                                   : VHD_VIDEOSTANDARD::NB_VHD_VIDEOSTANDARDS;
	const auto DvVideoStandard = bIsDvMode
		                             ? Deltacast::Helpers::GetDvVideoStandardFromDeviceModeIdentifier(DeviceModeIdentifier)
		                             : VHD_DV_HDMI_VIDEOSTANDARD::NB_VHD_DV_HDMI_VIDEOSTD;
	bIsSd = Deltacast::Helpers::IsSd(DvVideoStandard);

	bInterlaced = InMediaOutput->OutputConfiguration.MediaConfiguration.MediaMode.Standard != EMediaIOStandardType::Progressive;
	const auto TransportType = InMediaOutput->OutputConfiguration.MediaConfiguration.MediaConnection.TransportType;
	const auto QuadTransportType = InMediaOutput->OutputConfiguration.MediaConfiguration.MediaConnection.QuadTransportType;
	BufferPacking = DeltacastMediaCaptureUtils::GetBufferPackingFromPixelFormat(InMediaOutput->PixelFormat);
	const auto BufferDepth = InMediaOutput->NumberOfDeltacastBuffers;
	const auto BufferPreLoad = BufferDepth / 2;
	const auto LinkCount = TransportType == EMediaIOTransportType::SingleLink ||
	                       TransportType == EMediaIOTransportType::HDMI
		                       ? 1
		                       : 4;
	const auto bIsGenlocked = bIsSdiMode && InMediaOutput->OutputConfiguration.OutputReference != EMediaIOReferenceType::FreeRun;

	const auto VideoCharacteristics = bIsSdiMode
		                            ? Deltacast::Helpers::GetVideoCharacteristics(SdiVideoStandard)
		                            : Deltacast::Helpers::GetVideoCharacteristics(DvVideoStandard);
	if (!VideoCharacteristics.has_value())
	{
		UE_LOG(LogDeltacastMediaOutput, Error, TEXT("Failed to get video standard characteristics"));
		SetState(EMediaCaptureState::Error);
		return false;
	}

	const auto bRequireLinePadding = Deltacast::Helpers::RequiresLinePadding(VideoCharacteristics->Width);

	// Clean up because StopCapture() is not called on error during initialization
	const auto BoardCleanUp = [&DeltacastSdk, bIsSdiMode, PortIndex, LinkCount, this]()
	{
		DeltacastSdk.SetLoopbackState(BoardHandle, PortIndex, LinkCount, VHD::True);

		[[maybe_unused]] const auto Result = DeltacastSdk.CloseBoardHandle(BoardHandle);
		BoardHandle                        = VHD::InvalidHandle;
	};
	const auto StreamCleanUp = [&DeltacastSdk, this]()
	{
		[[maybe_unused]] const auto Result = DeltacastSdk.CloseStreamHandle(StreamHandle);
		StreamHandle                       = VHD::InvalidHandle;
	};

	BoardHandle = DeltacastSdk.OpenBoard(BoardIndex).value_or(VHD::InvalidHandle);
	if (BoardHandle == VHD::InvalidHandle)
	{
		UE_LOG(LogDeltacastMediaOutput, Error, TEXT("Failed to open Deltacast board with index %lu."), BoardIndex);
		SetState(EMediaCaptureState::Error);
		return false;
	}

	const auto StreamType = Deltacast::Helpers::GetStreamTypeFromPortIndex(false, PortIndex);

	DeltacastSdk.SetLoopbackState(BoardHandle, PortIndex, LinkCount, VHD::False);

	if (bIsSdiMode)
	{
	
		const auto ClockDivisor = bIsEuropeanClock ? VHD_CLOCKDIVISOR::VHD_CLOCKDIV_1 : VHD_CLOCKDIVISOR::VHD_CLOCKDIV_1001;

		[[maybe_unused]] const auto SetClockDivisorResult = DeltacastSdk.SetBoardProperty(BoardHandle, VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_CLOCK_SYSTEM, static_cast<VHD::ULONG>(ClockDivisor));

		StreamHandle = DeltacastSdk.OpenStream(BoardHandle, StreamType, VHD_SDI_STREAMPROCMODE::VHD_SDI_STPROC_DISJOINED_VIDEO).value_or(VHD::InvalidHandle);
	}
	if (bIsDvMode)
	{
		StreamHandle = DeltacastSdk.OpenStream(BoardHandle, StreamType, VHD_DV_STREAMPROCMODE::VHD_DV_STPROC_DISJOINED_VIDEO).value_or(VHD::InvalidHandle);
	}

	if (StreamHandle == VHD::InvalidHandle)
	{
		UE_LOG(LogDeltacastMediaOutput, Error, TEXT("Failed to open stream on board %u."), BoardIndex);
		SetState(EMediaCaptureState::Error);

		BoardCleanUp();

		return false;
	}

	if (bIsSdiMode)
	{
		const auto Interface = DeltacastMediaCaptureUtils::GetInterface(TransportType, QuadTransportType, SdiVideoStandard);
		if (!Interface.has_value())
		{
			BoardCleanUp();
			StreamCleanUp();
			SetState(EMediaCaptureState::Error);
			return false;
		}

		[[maybe_unused]] const auto SetVideoStandardResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_SDI_STREAMPROPERTY::VHD_SDI_SP_VIDEO_STANDARD, static_cast<VHD::ULONG>(SdiVideoStandard));
		[[maybe_unused]] const auto SetInterfaceResult     = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_SDI_STREAMPROPERTY::VHD_SDI_SP_INTERFACE, static_cast<VHD::ULONG>(Interface.value()));
	}

	if (bIsDvMode)
	{
		[[maybe_unused]] const auto SetModeResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_DV_STREAMPROPERTY::VHD_DV_SP_MODE, static_cast<VHD::ULONG>(VHD_DV_MODE::VHD_DV_MODE_HDMI));

		const auto FrameRate = bIsEuropeanClock ? VideoCharacteristics->FrameRate : VideoCharacteristics->FrameRate - 1;

		[[maybe_unused]] const auto PresetResult = DeltacastSdk.PresetTimingStreamProperties(StreamHandle, VHD_DV_STANDARD::VHD_DV_STD_SMPTE, VideoCharacteristics->Width, VideoCharacteristics->Height, FrameRate, VideoCharacteristics->bIsInterlaced);

		Deltacast::Helpers::FCablePacking CablePacking(BufferPacking, bIsSd);

		[[maybe_unused]] const auto SetColorSpaceResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_DV_STREAMPROPERTY::VHD_DV_SP_CS, static_cast<VHD::ULONG>(CablePacking.ColorSpace));
		[[maybe_unused]] const auto SetSamplingResult   = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_DV_STREAMPROPERTY::VHD_DV_SP_CABLE_BIT_SAMPLING, static_cast<VHD::ULONG>(CablePacking.Sampling));
	}

	/* Configure stream video standard */
	[[maybe_unused]] const auto SetBufferQueueDepthResult   = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_BUFFERQUEUE_DEPTH, BufferDepth);
	[[maybe_unused]] const auto SetBufferQueuePreLoadResult = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_BUFFERQUEUE_PRELOAD, BufferPreLoad);
	[[maybe_unused]] const auto SetIOTimeoutResult          = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_IO_TIMEOUT, 0);
	[[maybe_unused]] const auto SetBufferPackingResult      = DeltacastSdk.SetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_BUFFER_PACKING, static_cast<VHD::ULONG>(BufferPacking));

	if (bIsGenlocked)
	{
		[[maybe_unused]] const auto SetGenlockResult = DeltacastSdk.SetStreamProperty(StreamHandle,
		                                                                              VHD_SDI_STREAMPROPERTY::VHD_SDI_SP_TX_GENLOCK, VHD::True);
	}

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
		        UE_LOG(LogDeltacastMediaOutput, Warning, TEXT("Failed to set field merging stream property: %s"),
		               *Deltacast::Helpers::GetErrorString(SetLinePaddingResult));
	 			bFieldMergingSupported = false;
	 		}
	 	}
	}

	const auto StartStreamResult = DeltacastSdk.StartStream(StreamHandle);
	if (!Deltacast::Helpers::IsValid(StartStreamResult))
	{
		UE_LOG(LogDeltacastMediaOutput, Error, TEXT("Failed to start stream with error: %s."), *Deltacast::Helpers::GetErrorString(StartStreamResult));
		SetState(EMediaCaptureState::Error);

		StreamCleanUp();
		BoardCleanUp();

		return false;
	}

	SetState(EMediaCaptureState::Capturing);

	return true;
}


void UDeltacastMediaCapture::UpdateStatistics() const
{
	UDeltacastMediaOutput* DeltacastMediaSource = CastChecked<UDeltacastMediaOutput>(MediaOutput);
	check(DeltacastMediaSource);

	Deltacast::Helpers::FStreamStatistics Statistics;

	Statistics.bUpdateProcessedFrameCount = DeltacastMediaSource->bUpdateProcessedFrameCount;
	Statistics.bUpdateDroppedFrameCount   = DeltacastMediaSource->bUpdateRepeatedFrameCount;
	Statistics.bUpdateBufferFill          = DeltacastMediaSource->bUpdateBufferFill;

	Statistics.ProcessedFrameCount        = static_cast<uint32>(DeltacastMediaSource->ProcessedFrameCount);
	Statistics.DroppedFrameCount          = static_cast<uint32>(DeltacastMediaSource->RepeatedFrameCount);
	Statistics.BufferFill                 = DeltacastMediaSource->BufferFill;

	Statistics.NumberOfDeltacastBuffers   = static_cast<uint32>(DeltacastMediaSource->NumberOfDeltacastBuffers);


	Deltacast::Helpers::GetStreamStatistics(Statistics, StreamHandle);


	if (Statistics.bUpdateProcessedFrameCount)
	{
		DeltacastMediaSource->ProcessedFrameCount = static_cast<int32>(Statistics.ProcessedFrameCount);
	}

	if (Statistics.bUpdateDroppedFrameCount)
	{
		DeltacastMediaSource->RepeatedFrameCount = static_cast<int32>(Statistics.DroppedFrameCount);
	}

	if (Statistics.bUpdateBufferFill)
	{
		DeltacastMediaSource->BufferFill = Statistics.BufferFill;
	}
}

void UDeltacastMediaCapture::ResetStatistics() const
{
	UDeltacastMediaOutput* DeltacastMediaSource = CastChecked<UDeltacastMediaOutput>(MediaOutput);
	check(DeltacastMediaSource);

	Deltacast::Helpers::FStreamStatistics Statistics;

	Statistics.Reset();

	DeltacastMediaSource->ProcessedFrameCount = static_cast<int32>(Statistics.ProcessedFrameCount);
	DeltacastMediaSource->RepeatedFrameCount   = static_cast<int32>(Statistics.DroppedFrameCount);
	DeltacastMediaSource->BufferFill          = Statistics.BufferFill;
}


void UDeltacastMediaCapture::ApplyViewportTextureAlpha(const TSharedPtr<FSceneViewport>&InSceneViewport)
{
	if (InSceneViewport.IsValid())
	{
		const TSharedPtr<SViewport> Widget(InSceneViewport->GetViewportWidget().Pin());
		if (Widget.IsValid())
		{
			bSavedIgnoreTextureAlpha = Widget->GetIgnoreTextureAlpha();

			const UDeltacastMediaOutput* DeltacastMediaSource = CastChecked<UDeltacastMediaOutput>(MediaOutput);
			if (DeltacastMediaSource->OutputConfiguration.OutputType == EMediaIOOutputType::FillAndKey)
			{
				if (bSavedIgnoreTextureAlpha)
				{
					bIgnoreTextureAlphaChanged = true;
					Widget->SetIgnoreTextureAlpha(false);
				}
			}
		}
	}
}

void UDeltacastMediaCapture::RestoreViewportTextureAlpha(const TSharedPtr<FSceneViewport> &InSceneViewport)
{
	if (bIgnoreTextureAlphaChanged)
	{
		if (InSceneViewport.IsValid())
		{
			const TSharedPtr<SViewport> Widget(InSceneViewport->GetViewportWidget().Pin());
			if (Widget.IsValid())
			{
				Widget->SetIgnoreTextureAlpha(bSavedIgnoreTextureAlpha);
			}
		}

		bIgnoreTextureAlphaChanged = false;
	}
}
