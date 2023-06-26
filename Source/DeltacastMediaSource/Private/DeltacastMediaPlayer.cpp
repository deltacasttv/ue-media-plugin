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

#include "DeltacastMediaPlayer.h"

#include "DeltacastDeviceScanner.h"
#include "DeltacastHelpers.h"
#include "DeltacastInputStream.h"
#include "DeltacastMediaOption.h"
#include "DeltacastMediaSource.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaModule.h"
#include "IDeltacastMediaSourceModule.h"
#include "IMediaEventSink.h"
#include "IMediaOptions.h"
#include "HAL/RunnableThread.h"
#include "MediaIOCoreDefinitions.h"
#include "MediaIOCoreEncodeTime.h"
#include "MediaIOCoreFileWriter.h"
#include "MediaIOCoreSamples.h"


#define LOCTEXT_NAMESPACE "FDeltacastMediaPlayer"

DECLARE_CYCLE_STAT(TEXT("Deltacast MediaPlayer Request frame"), STAT_Deltacast_MediaPlayer_RequestFrame, STATGROUP_Deltacast);
DECLARE_CYCLE_STAT(TEXT("Deltacast MediaPlayer Process frame"), STAT_Deltacast_MediaPlayer_ProcessFrame, STATGROUP_Deltacast);

bool bDeltacastMediaSourceWriteOutputRawDataCmdEnable = false;
static FAutoConsoleCommand DeltacastWriteOutputRawDataCmd(
	TEXT("Deltacast.Source.WriteOutputRawData"),
	TEXT("Write Deltacast raw output buffer to file."),
	FConsoleCommandDelegate::CreateLambda([]() { bDeltacastMediaSourceWriteOutputRawDataCmdEnable = true; })
);


VHD_BUFFERPACKING SourcePixelFormatToDcBufferPacking(const EDeltacastMediaSourcePixelFormat PixelFormat)
{
	switch (PixelFormat)
	{
	case EDeltacastMediaSourcePixelFormat::PF_8BIT_RGBA:
		return VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_RGB_32;
	case EDeltacastMediaSourcePixelFormat::PF_8BIT_YUV422:
		return VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_8;
	case EDeltacastMediaSourcePixelFormat::PF_10BIT_YUV422:
		return VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_10;
	default:
		UE_LOG(LogDeltacastMediaSource, Fatal, TEXT("Unhandled `EDeltacastMediaSourcePixelFormat`: %d"), PixelFormat);
		return VHD_BUFFERPACKING{};
		break;
	}
}


FDeltacastMediaPlayer::FDeltacastMediaPlayer(IMediaEventSink &InEventSink)
	: Super(InEventSink),
	  EventSink(InEventSink),
	  TextureSamplePool(MakeUnique<FDeltacastMediaTextureSamplePool>()) { }

FDeltacastMediaPlayer::~FDeltacastMediaPlayer()
{
	FDeltacastMediaPlayer::Close();
}


bool FDeltacastMediaPlayer::Open(const FString &Url, const IMediaOptions *Options)
{
	const auto& MediaModule = FModuleManager::LoadModuleChecked<IDeltacastMediaModule>(TEXT("DeltacastMedia"));
	if (!MediaModule.IsInitialized())
	{
		UE_LOG(LogDeltacastMediaSource, Error, TEXT("Can't open media player '%s'. The Deltacast library was not initialized."), *GetMediaName().ToString());
		return false;
	}

	if (!MediaModule.CanBeUsed())
	{
		UE_LOG(LogDeltacastMediaSource, Error, TEXT("The DeltacastMediaPlayer can't open the URL '%s' because Deltacast board cannot be used."), *Url);
		return false;
	}

	if (!Super::Open(Url, Options))
	{
		return false;
	}

	const auto BoardIndex  = Options->GetMediaOption(DeltacastMediaOption::BoardIndex, int64{ 0 });
	const auto PortIndex   = Options->GetMediaOption(DeltacastMediaOption::PortIndex, int64{ 0 });
	const auto BufferDepth = Options->GetMediaOption(DeltacastMediaOption::NumberOfDeltacastBuffers, int64{ 8 });

	const auto LinkType      = static_cast<EMediaIOTransportType>(Options->GetMediaOption(DeltacastMediaOption::LinkType, int64{ 0 }));
	const auto QuadLinkType  = static_cast<EMediaIOQuadLinkTransportType>(Options->GetMediaOption(DeltacastMediaOption::QuadLinkType, int64{ 0 }));
	const auto PixelFormat   = static_cast<EDeltacastMediaSourcePixelFormat>(Options->GetMediaOption(DeltacastMediaOption::PixelFormat, int64{ static_cast<VHD::ULONG>(EDeltacastMediaSourcePixelFormat::PF_8BIT_YUV422) }));

	const auto bIsEuropeanClock = Options->GetMediaOption(DeltacastMediaOption::IsEuropeanClock, true);
	const auto bIsSdi           = Options->GetMediaOption(DeltacastMediaOption::IsSdi, true);
	const auto bIsDv            = Options->GetMediaOption(DeltacastMediaOption::IsDv, false);

	check(bIsSdi != bIsDv);

	const auto TimecodeFormat = static_cast<EMediaIOTimecodeFormat>(Options->GetMediaOption(DeltacastMediaOption::TimecodeFormat, int64{ 0 }));
	bUseFrameTimecode = TimecodeFormat == EMediaIOTimecodeFormat::LTC;

	const auto bAutoLoadEdid = Options->GetMediaOption(DeltacastMediaOption::AutoLoadEdid, true);

	const auto bErrorOnSourceLost = Options->GetMediaOption(DeltacastMediaOption::ErrorOnSourceLost, true);

	const auto IsSingleLink = [=]()
	{
		switch (LinkType)
		{
			case EMediaIOTransportType::SingleLink: [[fallthrough]];
			case EMediaIOTransportType::HDMI:
				return true;
			case EMediaIOTransportType::DualLink: [[fallthrough]];
			case EMediaIOTransportType::QuadLink: [[fallthrough]];
			default:
				return false;
		}
	}();
	const auto DcQuadLinkType = [=]()
	{
		switch (QuadLinkType)
		{
			case EMediaIOQuadLinkTransportType::SquareDivision:
				return Deltacast::Helpers::EQuadLinkType::Quadrant;
			case EMediaIOQuadLinkTransportType::TwoSampleInterleave:
				return Deltacast::Helpers::EQuadLinkType::TwoSampleInterleaved;
			default:
				UE_LOG(LogDeltacastMediaSource, Fatal, TEXT("Unhandled `EMediaIOQuadLinkTransportType` value: %d"), QuadLinkType);
				return Deltacast::Helpers::EQuadLinkType{};
		}
	}();

	SetupSampleChannels();

	bEncodeTimecodeInTexel = Options->GetMediaOption(DeltacastMediaOption::EncodeTimecodeInTexel, false);
	bIsSRGBInput           = Options->GetMediaOption(DeltacastMediaOption::IsSRGBInput, false);

	BufferPacking            = SourcePixelFormatToDcBufferPacking(PixelFormat);
	MaxVideoFrameBufferCount = Options->GetMediaOption(DeltacastMediaOption::NumberOfEngineBuffers, int64{ 8 });
	bLogDroppedFrameCount = Options->GetMediaOption(DeltacastMediaOption::LogDroppedFrameCount, false);

	Samples->EnableTimedDataChannels(this, EMediaIOSampleType::Video);

	check(!InputChannel.IsValid());
	check(!Thread.IsValid());

	const auto InputStreamConfig = [&]()
	{
		FDeltacastInputStreamConfig Config{};

		Deltacast::Device::Config::FBasePortConfig Base;

		Base.bIsInput         = true;
		Base.BoardIndex       = BoardIndex;
		Base.PortIndex        = PortIndex;
		Base.bIsEuropeanClock = bIsEuropeanClock;
		Base.BufferDepth      = BufferDepth;
		Base.BufferPacking    = SourcePixelFormatToDcBufferPacking(PixelFormat);

		Config.Callback       = this;
		Config.bIsSdi         = bIsSdi;
		Config.TimecodeFormat = TimecodeFormat;

		if (bIsSdi)
		{
			Config.SdiPortConfig.Base = Base;

			const auto SdiVideoStandard = static_cast<VHD_VIDEOSTANDARD>(Options->GetMediaOption(DeltacastMediaOption::SdiVideoStandard, int64{ 0 }));

			Config.SdiPortConfig.VideoStandard = SdiVideoStandard;
			Config.SdiPortConfig.Interface     = IsSingleLink
				                                     ? Deltacast::Helpers::GetSingleLinkInterface(SdiVideoStandard)
				                                     : Deltacast::Helpers::GetQuadLinkInterface(SdiVideoStandard, DcQuadLinkType);
		}

		if (bIsDv)
		{
			Config.DvPortConfig.Base = Base;

			const auto DvVideoStandard = static_cast<VHD_DV_HDMI_VIDEOSTANDARD>(Options->GetMediaOption(DeltacastMediaOption::DvVideoStandard, int64{ 0 }));

			Config.DvPortConfig.VideoStandard = DvVideoStandard;
		}

		Config.bAutoLoadEdid = bAutoLoadEdid;

		Config.bErrorOnSourceLost = bErrorOnSourceLost;

		Config.bLogDroppedFrameCount = bLogDroppedFrameCount;

		return Config;
	}();

	InputChannel = MakeShared<FDeltacastInputStream>(InputStreamConfig);
	Thread.Reset(FRunnableThread::Create(InputChannel.Get(), *FString::Printf(TEXT("Deltacast Media Player %s"), *GetMediaName().ToString())));
	if (Thread == nullptr)
	{
		UE_LOG(LogDeltacastMediaSource, Error, TEXT("Failed to start Deltacast input channel thread"));
		CurrentState = EMediaState::Error;
		return false;
	}

	return true;
}

void FDeltacastMediaPlayer::Close()
{
	if (InputChannel.IsValid())
	{
		Thread->Kill();
		Thread->WaitForCompletion();
		Thread.Reset();
		InputChannel.Reset();
	}

	Samples->EnableTimedDataChannels(this, EMediaIOSampleType::None);

	TextureSamplePool.Get()->Reset();
	CurrentTextureSample.Reset();

	Super::Close();
}


void FDeltacastMediaPlayer::TickFetch(const FTimespan DeltaTime, const FTimespan Timecode)
{
	Super::TickFetch(DeltaTime, Timecode);

	if (IsHardwareReady() &&
		CurrentState == EMediaState::Playing)
	{
		VerifyFrameDropCount();
	}
}

void FDeltacastMediaPlayer::TickInput(FTimespan DeltaTime, FTimespan Timecode)
{
	const EMediaState NewState = InputChannel.IsValid() ? ThreadMediaState : EMediaState::Closed;

	if (NewState != CurrentState)
	{
		CurrentState = NewState;
		if (CurrentState == EMediaState::Playing)
		{
			EventSink.ReceiveMediaEvent(EMediaEvent::TracksChanged);
			EventSink.ReceiveMediaEvent(EMediaEvent::MediaOpened);
			EventSink.ReceiveMediaEvent(EMediaEvent::PlaybackResumed);
		}
		else if (NewState == EMediaState::Error)
		{
			EventSink.ReceiveMediaEvent(EMediaEvent::MediaOpenFailed);
			Close();
		}
	}

	if (CurrentState != EMediaState::Playing)
	{
		return;
	}

	TickTimeManagement();
}


FString FDeltacastMediaPlayer::GetStats() const
{
	FString Stats;

	Stats += FString::Printf(TEXT("\t\tInput port: %s\n"), *GetUrl());
	Stats += FString::Printf(TEXT("\t\tFrame rate: %s\n"), *VideoFrameRate.ToPrettyText().ToString());
	Stats += FString::Printf(TEXT("\t\tBoard Mode: %s\n"), *VideoTrackFormat.TypeName);

	Stats += TEXT("\nStatus Deltacast\n");
	Stats += FString::Printf(TEXT("\t\tFrames processed: %u\n"), ThreadFrameProcessedCount);
	Stats += FString::Printf(TEXT("\t\tFrames dropped:   %u\n"), ThreadFrameDropCount);
	Stats += FString::Printf(TEXT("\t\tBuffer fill:      %f\n"), ThreadBufferFill);

	Stats += TEXT("\nStatus Media Source\n");
	Stats += FString::Printf(TEXT("\t\tBuffered video frames: %d\n"), GetSamples().NumVideoSamples());

	return Stats;
}


FGuid FDeltacastMediaPlayer::GetPlayerPluginGUID() const
{
	static FGuid PlayerPluginGUID(0x6CCE8F6B, 0x9E084570, 0xA903BC7A, 0x96898154);
	return PlayerPluginGUID;
}

#if WITH_EDITOR
const FSlateBrush *FDeltacastMediaPlayer::GetDisplayIcon() const
{
	return IDeltacastMediaSourceModule::Get().GetStyle()->GetBrush("DeltacastMediaIcon");
}
#endif


void FDeltacastMediaPlayer::OnInitializationCompleted(const bool bSucceed)
{
	ThreadMediaState = bSucceed ? EMediaState::Playing : EMediaState::Error;
}

bool FDeltacastMediaPlayer::OnRequestInputBuffer(const FDeltacastRequestBuffer& RequestBuffer, FDeltacastRequestedBuffer& RequestedBuffer)
{
	SCOPE_CYCLE_COUNTER(STAT_Deltacast_MediaPlayer_RequestFrame);

	if (ThreadMediaState != EMediaState::Playing)
	{
		return false;
	}

	if (RequestBuffer.VideoBufferSize > 0 && RequestBuffer.bIsProgressive)
	{
		CurrentTextureSample        = TextureSamplePool->AcquireShared();
		RequestedBuffer.VideoBuffer = static_cast<uint8_t*>(CurrentTextureSample->RequestBuffer(RequestBuffer.VideoBufferSize));
	}

	return true;
}

bool FDeltacastMediaPlayer::OnInputFrameReceived(const FDeltacastVideoFrameData& VideoFrame)
{
	SCOPE_CYCLE_COUNTER(STAT_Deltacast_MediaPlayer_ProcessFrame);

	if (ThreadMediaState != EMediaState::Playing)
	{
		return false;
	}

	ThreadFrameProcessedCount= VideoFrame.MetaData.FrameCount;
	ThreadFrameDropCount = VideoFrame.MetaData.DropCount;
	ThreadBufferFill = VideoFrame.MetaData.BufferFill;

	FTimespan DecodedTime   = FTimespan::FromSeconds(GetPlatformSeconds());
	FTimespan DecodedTimeF2 = DecodedTime + FTimespan::FromSeconds(VideoFrameRate.AsInterval());

	TOptional<FTimecode> DecodedTimecode;
	TOptional<FTimecode> DecodedTimecodeF2;

	if (bUseFrameTimecode)
	{
		DecodedTimecode = [](const VHD_TIMECODE &Timecode)
		{
			const auto bDropFrame = (Timecode.Flags & 0b1) != 0;
			return FTimecode(Timecode.Hour, Timecode.Minute, Timecode.Second, Timecode.Frame, bDropFrame);
		}(VideoFrame.MetaData.Timecode);

		DecodedTimecodeF2 = DecodedTimecode;
		++DecodedTimecodeF2->Frames;

		if (bUseTimeSynchronization)
		{
			const FFrameNumber ConvertedFrameNumber = DecodedTimecode.GetValue().ToFrameNumber(VideoFrameRate);
			const double       NumberOfSeconds      = ConvertedFrameNumber.Value * VideoFrameRate.AsInterval();
			const FTimespan    TimecodeDecodedTime  = FTimespan::FromSeconds(NumberOfSeconds);

			DecodedTime   = TimecodeDecodedTime;
			DecodedTimeF2 = TimecodeDecodedTime + FTimespan::FromSeconds(VideoFrameRate.AsInterval());
		}
	}

	EMediaTextureSampleFormat VideoSampleFormat = EMediaTextureSampleFormat::CharBGRA;
	EMediaIOCoreEncodePixelFormat EncodePixelFormat = EMediaIOCoreEncodePixelFormat::CharBGRA;
	FString OutputFilename;

	switch (BufferPacking)
	{
		case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_RGB_32:
			VideoSampleFormat = EMediaTextureSampleFormat::CharBGRA;
			EncodePixelFormat = EMediaIOCoreEncodePixelFormat::CharBGRA;
			OutputFilename = TEXT("Deltacast_Input_8_RGBA");
			break;
		case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_8:
			VideoSampleFormat = EMediaTextureSampleFormat::CharUYVY;
			EncodePixelFormat = EMediaIOCoreEncodePixelFormat::CharUYVY;
			OutputFilename = TEXT("Deltacast_Input_8_YUV");
			break;
		case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_10:
			VideoSampleFormat = EMediaTextureSampleFormat::YUVv210;
			EncodePixelFormat = EMediaIOCoreEncodePixelFormat::YUVv210;
			OutputFilename = TEXT("Deltacast_Input_10_YUV");
			break;
		default:
			break;
	}

	if (bEncodeTimecodeInTexel && DecodedTimecode.IsSet() && VideoFrame.bIsProgressive)
	{
		const FTimecode SetTimecode = DecodedTimecode.GetValue();
		const FMediaIOCoreEncodeTime EncodeTime(EncodePixelFormat, VideoFrame.VideoBuffer, VideoFrame.Stride, VideoFrame.Width, VideoFrame.Height);
		EncodeTime.Render(SetTimecode.Hours, SetTimecode.Minutes, SetTimecode.Seconds, SetTimecode.Frames);
	}

	if (bDeltacastMediaSourceWriteOutputRawDataCmdEnable)
	{
		MediaIOCoreFileWriter::WriteRawFile(OutputFilename, VideoFrame.VideoBuffer, VideoFrame.Stride * VideoFrame.Height);
		bDeltacastMediaSourceWriteOutputRawDataCmdEnable = false;
	}

	if (CurrentTextureSample.IsValid())
	{
		if (CurrentTextureSample->SetProperties(VideoFrame.Stride, VideoFrame.Width, VideoFrame.Height, VideoSampleFormat, DecodedTime, VideoFrameRate, DecodedTimecode, bIsSRGBInput))
		{
			Samples->AddVideo(CurrentTextureSample.ToSharedRef());
		}
	}
	else
	{
		if (VideoFrame.bIsProgressive)
		{
			const auto TextureSample = TextureSamplePool->AcquireShared();
			if (TextureSample->InitializeProgressive(VideoFrame, VideoSampleFormat, DecodedTime, VideoFrameRate, DecodedTimecode, bIsSRGBInput))
			{
				Samples->AddVideo(TextureSample);
			}
		}
		else
		{
			const auto bIsEven = GFrameCounterRenderThread % 2 == 1;

			const auto TextureSampleFirstHalf = TextureSamplePool->AcquireShared();
			if (TextureSampleFirstHalf->InitializeInterlaced_Half(VideoFrame, VideoSampleFormat, DecodedTime, VideoFrameRate, DecodedTimecode, bIsSRGBInput, bIsEven))
			{
				Samples->AddVideo(TextureSampleFirstHalf);
			}

			const auto TextureSampleSecondHalf = TextureSamplePool->AcquireShared();
			if (TextureSampleSecondHalf->InitializeInterlaced_Half(VideoFrame, VideoSampleFormat, DecodedTimeF2, VideoFrameRate, DecodedTimecodeF2, bIsSRGBInput, !bIsEven))
			{
				Samples->AddVideo(TextureSampleSecondHalf);
			}
		}
	}

	CurrentTextureSample.Reset();

	return true;
}


void FDeltacastMediaPlayer::OnSourceResumed()
{
	ThreadMediaState = EMediaState::Playing;
}

void FDeltacastMediaPlayer::OnSourceStopped()
{
	ThreadMediaState = EMediaState::Stopped;
}

void FDeltacastMediaPlayer::OnCompletion(const bool bSucceed)
{
	ThreadMediaState = bSucceed ? EMediaState::Closed : EMediaState::Error;
}


bool FDeltacastMediaPlayer::IsHardwareReady() const
{
	return InputChannel.IsValid() && ThreadMediaState == EMediaState::Playing;
}

void FDeltacastMediaPlayer::SetupSampleChannels()
{
	FMediaIOSamplingSettings VideoSettings = BaseSettings;
	VideoSettings.BufferSize = MaxVideoFrameBufferCount;
	Samples->InitializeVideoBuffer(VideoSettings);
}


void FDeltacastMediaPlayer::VerifyFrameDropCount()
{
	if (bLogDroppedFrameCount)
	{
		const auto FrameDropCount = ThreadFrameDropCount;
		const auto DeltaFrameDropCount = static_cast<int32>(FrameDropCount) - static_cast<int32>(LastFrameDropCount);
		LastFrameDropCount = static_cast<uint32>(FrameDropCount);
		if (DeltaFrameDropCount > 0)
		{
			static constexpr auto FrameCountBeforeWarning = 50;

			[[maybe_unused]] const auto bIsFirstDrop = LastAdjacentFrameDropCount == 0;

			LastAdjacentFrameDropCount += static_cast<uint32>(DeltaFrameDropCount);

#if !NO_LOGGING
			DroppedFrameCountAccumulator += static_cast<uint32>(DeltaFrameDropCount);
			const auto ShouldLog = bIsFirstDrop || DroppedFrameCountAccumulator >= FrameCountBeforeWarning;
			if (ShouldLog)
			{
				DroppedFrameCountAccumulator = DroppedFrameCountAccumulator % FrameCountBeforeWarning;
			}
#else
			static constexpr auto ShouldLog = false;
#endif

			UE_CLOG(ShouldLog, LogDeltacastMediaSource, Warning,
			        TEXT("Loosing frames on Deltacast input %s. The current count is %u."),
			        *GetUrl(), LastAdjacentFrameDropCount);
		}
		else if (LastAdjacentFrameDropCount > 0)
		{
			// Adjacent drop end, log one last time
			UE_LOG(LogDeltacastMediaSource, Warning,
			       TEXT("Lost %u frames on input %s. Unreal Engine's frame rate is too slow and the capture card was not able to send the frame(s) to Unreal."),
			       LastAdjacentFrameDropCount, *GetUrl());
			LastAdjacentFrameDropCount = 0;
		}

		const auto CurrentVideoDropCount = Samples->GetVideoFrameDropCount();
		const auto DeltaVideoDropCount   = CurrentVideoDropCount - static_cast<int32>(LastVideoFrameDropCount);
		LastVideoFrameDropCount          = static_cast<uint32>(CurrentVideoDropCount);
		UE_CLOG(DeltaVideoDropCount > 0, LogDeltacastMediaSource, Warning,
		        TEXT("Lost %d video frames on input %s. Frame rate is either too slow or buffering capacity is too small."),
		        DeltaVideoDropCount, *GetUrl());
	}
}

#undef LOCTEXT_NAMESPACE
