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

#include "MediaIOCoreTextureSampleBase.h"
#include "MediaShaders.h"


struct FDeltacastVideoFrameMetaData
{
	VHD_TIMECODE Timecode;

	uint32 FrameCount;
	uint32 DropCount;
	float  BufferFill;
};

struct FDeltacastVideoFrameData
{
	uint8 *VideoBuffer;
	uint32 VideoBufferSize;

	uint32 Stride;
	uint32 Width;
	uint32 Height;

	bool bIsProgressive;

	FDeltacastVideoFrameMetaData MetaData;
};


class FDeltacastMediaTextureSample : public FMediaIOCoreTextureSampleBase
{
	using Super = FMediaIOCoreTextureSampleBase;

public:
	bool InitializeProgressive(const FDeltacastVideoFrameData &VideoData,
	                           const EMediaTextureSampleFormat TextureSampleFormat,
	                           const FTimespan                 Timespan,
	                           const FFrameRate &              FrameRate,
	                           const TOptional<FTimecode> &    OptionalTimecode,
	                           const bool                      bInIsSRGB)
	{
		bIsSd = IsSd(VideoData);
		return Super::Initialize(VideoData.VideoBuffer,
		                         VideoData.VideoBufferSize,
		                         VideoData.Stride,
		                         VideoData.Width,
		                         VideoData.Height,
		                         TextureSampleFormat,
		                         Timespan,
		                         FrameRate,
		                         OptionalTimecode,
		                         bInIsSRGB);
	}

	bool InitializeInterlaced_Half(const FDeltacastVideoFrameData &VideoData,
	                               const EMediaTextureSampleFormat TextureSampleFormat,
	                               const FTimespan                 Timespan,
	                               const FFrameRate &              FrameRate,
	                               const TOptional<FTimecode> &    OptionalTimecode,
	                               const bool                      bIsSRGB,
	                               const bool                      bIsEven)
	{
		bIsSd = IsSd(VideoData);
		return Super::InitializeWithEvenOddLine(bIsEven,
		                                        VideoData.VideoBuffer,
		                                        VideoData.VideoBufferSize,
		                                        VideoData.Stride,
		                                        VideoData.Width,
		                                        VideoData.Height,
		                                        TextureSampleFormat,
		                                        Timespan,
		                                        FrameRate,
		                                        OptionalTimecode,
		                                        bIsSRGB);
	}

	virtual const FMatrix& GetYUVToRGBMatrix() const override
	{
		return bIsSd ? MediaShaders::YuvToRgbRec601Scaled : MediaShaders::YuvToRgbRec709Scaled;
	}

private:
	static bool IsSd(const FDeltacastVideoFrameData& VideoData)
	{
		return (VideoData.Width == 720 && (VideoData.Height == 480 || VideoData.Height == 576)) ||
		       (VideoData.Width == 640 && VideoData.Height == 480);
	}

private:
	bool bIsSd = false;
};

class FDeltacastMediaTextureSamplePool : public TMediaObjectPool<FDeltacastMediaTextureSample> { };
