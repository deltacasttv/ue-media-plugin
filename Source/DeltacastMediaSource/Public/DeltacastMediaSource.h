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

#include "MediaIOCoreDefinitions.h"
#include "TimeSynchronizableMediaSource.h"

#include "DeltacastMediaSource.generated.h"


/**
 * Native data format.
 */
UENUM()
enum class EDeltacastMediaSourcePixelFormat : uint8
{
	PF_8BIT_RGBA UMETA(DisplayName = "8bit RGBA"),
	PF_8BIT_YUV422 UMETA(DisplayName = "8bit YUV"),
	PF_10BIT_YUV422 UMETA(DisplayName = "10bit YUV"),
};


UCLASS(BlueprintType, HideCategories = (Platforms, Object), meta = (MediaIOCustomLayout = "Deltacast"))
class DELTACASTMEDIASOURCE_API UDeltacastMediaSource : public UTimeSynchronizableMediaSource
{
	GENERATED_BODY()

public:
	UDeltacastMediaSource();

public:
	/** The device, port and video settings that correspond to the input. */
	UPROPERTY(EditAnywhere, Category = "Deltacast", meta = (DisplayName = "Configuration"))
	FMediaIOConfiguration MediaConfiguration;

	/** Use the time code embedded in the input stream. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Deltacast")
	EMediaIOTimecodeFormat TimecodeFormat = EMediaIOTimecodeFormat::None;

	/** Auto-load the EDID */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Deltacast")
	bool bAutoLoadEdid = false;

public:
	/** Native data format internally used by the device after being converted from input signal. */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Video")
	EDeltacastMediaSourcePixelFormat PixelFormat = DefaultPixelFormatDv;

	/**
	 * Whether the video input is in sRGB color space.
	 * A sRGB to Linear conversion will be applied resulting in a texture in linear space.
	 * @Note If the texture is not in linear space, it won't look correct in the editor. Another pass will be required either through Composure or other means.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Video", meta = (EditCondition = "PixelFormat == EDeltacastMediaSourcePixelFormat::PF_8BIT_RGBA", EditConditionHides))
	bool bIsSRGBInput = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Video")
	bool bErrorOnSourceLost = true;

	/**
	 * Number of frame used by the Deltacast SDK.
	 * A smaller number is most likely to cause missed frame.
	 * A bigger number is most likely to increase latency.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, AdvancedDisplay, Category = "Video", meta = (ClampMin = "2", ClampMax = "32"))
	int32 NumberOfDeltacastBuffers = 4;

	/**
	 * Number of frame used by Unreal Engine.
	 * A smaller number is most likely to cause missed frame.
	 * A bigger number is most likely to increase latency.
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, AdvancedDisplay, Category = "Video", meta = (ClampMin = "1", ClampMax = "32"))
	int32 NumberOfEngineBuffers = 4;

public:
	/** Burn Frame Timecode on the output without any frame number clipping. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug", meta = (DisplayName = "Burn Frame Timecode"))
	bool bEncodeTimecodeInTexel = DefaultDebugOption;

	/** Enable the update of the number of processed frames. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AdvancedDisplay, Category = "Debug")
	bool bLogDroppedFrameCount = DefaultDebugOption;


public: //~ IMediaOptions interface
	virtual bool    GetMediaOption(const FName &Key, bool DefaultValue) const override;
	virtual int64   GetMediaOption(const FName &Key, int64 DefaultValue) const override;
	virtual FString GetMediaOption(const FName &Key, const FString &DefaultValue) const override;
	virtual bool    HasMediaOption(const FName &Key) const override;

public: //~ UMediaSource interface
	virtual FString GetUrl() const override;
	virtual bool    Validate() const override;

public: //~ UTimeSynchronizableMediaSource
	virtual bool SupportsFormatAutoDetection() const override;

public: //~ UObject interface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty *InProperty) const override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent &PropertyChangedEvent) override;
#endif

private:
	inline static constexpr auto DefaultPixelFormatSdi = EDeltacastMediaSourcePixelFormat::PF_10BIT_YUV422;
	inline static constexpr auto DefaultPixelFormatDv  = EDeltacastMediaSourcePixelFormat::PF_8BIT_YUV422;

	inline static constexpr auto DefaultDebugOption = true;
};
