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

#include "MediaOutput.h"
#include "MediaIOCoreDefinitions.h"

#include "DeltacastMediaOutput.generated.h"


/**
 * Native data format.
 */
UENUM()
enum class EDeltacastMediaOutputPixelFormat : uint8
{
	PF_8BIT_RGBA UMETA(DisplayName = "8bit RGBA"),
	PF_8BIT_YUV422 UMETA(DisplayName = "8bit YUV"), 
	PF_10BIT_YUV422 UMETA(DisplayName = "10bit YUV"),
};


UCLASS(BlueprintType, meta = (MediaIOCustomLayout = "Deltacast"))
class DELTACASTMEDIAOUTPUT_API UDeltacastMediaOutput : public UMediaOutput
{
	GENERATED_BODY()

public:
	/** The device, port and video settings that correspond to the output. */
	UPROPERTY(EditAnywhere, Category = "Deltacast", meta = (DisplayName = "Configuration"))
	FMediaIOOutputConfiguration OutputConfiguration;

public:
	// Add 3G-B bool/enum here

	/** Native data format internally used by the device before being converted to output signal. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Output")
	EDeltacastMediaOutputPixelFormat PixelFormat = DefaultPixelFormatDv;

	/**
	 * Number of frame used by the Deltacast SDK.
	 * A smaller number is most likely to cause missed frame.
	 * A bigger number is most likely to increase latency.
	 */
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = "Output", meta = (ClampMin = 2, ClampMax = 32))
	int32 NumberOfDeltacastBuffers = 8;

public:
	/** Burn Frame Timecode on the output without any frame number clipping. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug", meta = (DisplayName = "Burn Frame Timecode"))
	bool bEncodeTimecodeInTexel = DefaultDebugOption;

	/** Enable the update of the number of processed frames. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AdvancedDisplay, Category = "Debug")
	bool bUpdateProcessedFrameCount = DefaultDebugOption;

	/** Number of processed frames since capture was started. */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, AdvancedDisplay, Category = "Debug", meta = (EditCondition = "bUpdateProcessedFrameCount", EditConditionHides))
	int32 ProcessedFrameCount = 0;

	/** Enable the update of the number of processed frames. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AdvancedDisplay, Category = "Debug")
	bool bUpdateRepeatedFrameCount = DefaultDebugOption;

	/** Number of dropped frames since capture was started. */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, AdvancedDisplay, Category = "Debug", meta = (EditCondition = "bUpdateRepeatedFrameCount", EditConditionHides))
	int32 RepeatedFrameCount = 0;

	/** Enable the update of the number of processed frames. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, AdvancedDisplay, Category = "Debug")
	bool bUpdateBufferFill = DefaultDebugOption;

	/** Percentage of the buffer used. */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, AdvancedDisplay, Category = "Debug", meta = (EditCondition = "bUpdateBufferFill", EditConditionHides))
	float BufferFill = 0;

public: //~ UMediaOutput
	virtual bool                             Validate(FString &OutFailureReason) const override;
	virtual FIntPoint                        GetRequestedSize() const override;
	virtual EPixelFormat                     GetRequestedPixelFormat() const override;
	virtual EMediaCaptureConversionOperation GetConversionOperation(EMediaCaptureSourceType InSourceType) const override;

protected: //~ UMediaOutput
	virtual UMediaCapture *CreateMediaCaptureImpl() override;

public: //~ UObject interface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif //WITH_EDITOR

private:
	inline static constexpr auto DefaultPixelFormatSdi = EDeltacastMediaOutputPixelFormat::PF_10BIT_YUV422;
	inline static constexpr auto DefaultPixelFormatDv = EDeltacastMediaOutputPixelFormat::PF_8BIT_YUV422;

	inline static constexpr auto DefaultDebugOption = true;
};