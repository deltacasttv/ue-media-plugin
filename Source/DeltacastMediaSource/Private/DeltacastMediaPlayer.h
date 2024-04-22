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

#include "DeltacastInputStream.h"
#include "DeltacastMediaSource.h"
#include "DeltacastMediaTextureSample.h"
#include "IMediaEventSink.h"
#include "MediaIOCorePlayerBase.h"


class FDeltacastMediaPlayer : public FMediaIOCorePlayerBase,
                              public IDeltacastInputStreamCallback
{
public:
	FDeltacastMediaPlayer(IMediaEventSink &InEventSink);
	virtual ~FDeltacastMediaPlayer() override;

public: //~ FMediaIOCorePlayerBase
	virtual bool Open(const FString &Url, const IMediaOptions *Options) override;
	virtual void Close() override;

	virtual void TickFetch(FTimespan DeltaTime, FTimespan Timecode) override;
	virtual void TickInput(FTimespan DeltaTime, FTimespan Timecode) override;

	virtual FString GetStats() const override;

	virtual FGuid GetPlayerPluginGUID() const override;

#if WITH_EDITOR
	virtual const FSlateBrush *GetDisplayIcon() const override;
#endif

public: //~ IDeltacastInputStreamCallback
	virtual void OnInitializationCompleted(bool bSucceed) override;
	virtual bool OnRequestInputBuffer(const FDeltacastRequestBuffer& RequestBuffer, FDeltacastRequestedBuffer& RequestedBuffer) override;
	virtual bool OnInputFrameReceived(const FDeltacastVideoFrameData& VideoFrame) override;
	virtual void OnSourceResumed() override;
	virtual void OnSourceStopped() override;
	virtual void OnCompletion(bool bSucceed) override;

protected: //~ FMediaIOCorePlayerBase
	virtual bool IsHardwareReady() const override;
	virtual void SetupSampleChannels() override;
	virtual TSharedPtr<FMediaIOCoreTextureSampleBase> AcquireTextureSample_AnyThread() const override;

private:
	void VerifyFrameDropCount();

private:
	using Super = FMediaIOCorePlayerBase;

private:
	/** The media event handler. */
	IMediaEventSink &EventSink;

	bool bLogDroppedFrameCount  = false;
	bool bEncodeTimecodeInTexel = false;
	bool bIsSRGBInput           = false;
	bool bUseFrameTimecode      = false;

	uint32 MaxVideoFrameBufferCount = 8;

	VHD_BUFFERPACKING BufferPacking = VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_8;

private:
	TSharedPtr<FDeltacastInputStream> InputChannel = nullptr;
	TUniquePtr<FRunnableThread>       Thread       = nullptr;

	EMediaState ThreadMediaState = EMediaState::Closed;

private:
	TUniquePtr<FDeltacastMediaTextureSamplePool> TextureSamplePool;

	TSharedPtr<FDeltacastMediaTextureSample, ESPMode::ThreadSafe> CurrentTextureSample;

private:
	uint32 ThreadFrameProcessedCount = 0;
	uint32 ThreadFrameDropCount      = 0;
	float  ThreadBufferFill          = 0;

#if !NO_LOGGING
	uint32 DroppedFrameCountAccumulator = 0;
#endif

	uint32 LastAdjacentFrameDropCount = 0;
	uint32 LastFrameDropCount         = 0;

	uint32 LastVideoFrameDropCount = 0;
};
