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
#include "MediaCapture.h"

#include "DeltacastMediaCapture.generated.h"


class UDeltacastMediaOutput;

/**
 * Output Media for Deltacast streams.
 * The output format could be any of EDeltacastMediaOutputPixelFormat.
 */
UCLASS(BlueprintType)
class DELTACASTMEDIAOUTPUT_API UDeltacastMediaCapture : public UMediaCapture
{
private:
	GENERATED_BODY()

public: //~ UMediaCapture interface
	virtual bool HasFinishedProcessing() const override;

protected: //~ UMediaCapture interface
	virtual bool InitializeCapture() override;
	virtual bool PostInitializeCaptureRenderTarget(UTextureRenderTarget2D *InRenderTarget) override;
	virtual bool PostInitializeCaptureViewport(TSharedPtr<FSceneViewport> &InSceneViewport) override;
	virtual void StopCaptureImpl(bool bAllowPendingFrameToBeProcess) override;
	virtual bool UpdateRenderTargetImpl(UTextureRenderTarget2D *InRenderTarget) override;
	virtual bool UpdateSceneViewportImpl(TSharedPtr<FSceneViewport> &InSceneViewport) override;
	virtual bool ValidateMediaOutput() const override;

protected: //~ UMediaCapture interface
	virtual void OnFrameCaptured_RenderingThread(const FCaptureBaseData &InBaseData, TSharedPtr<FMediaCaptureUserData, ESPMode::ThreadSafe> InUserData, void *InBuffer, int32 Width, int32 Height, int32 BytesPerRow) override;

private:
	bool Initialize(const UDeltacastMediaOutput *InMediaOutput);


	void UpdateStatistics() const;

	void ResetStatistics() const;

private:
	void ApplyViewportTextureAlpha(const TSharedPtr<FSceneViewport> &InSceneViewport);
	void RestoreViewportTextureAlpha(const TSharedPtr<FSceneViewport> &InSceneViewport);

private:
	FCriticalSection RenderThreadCriticalSection;

	bool bSavedIgnoreTextureAlpha   = false;
	bool bIgnoreTextureAlphaChanged = false;

private:
	VHD_BUFFERPACKING BufferPacking = VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_8;

	bool bEncodeTimecodeInTexel = false;
	bool bInterlaced            = false;

	bool bIsSd = false;

	bool bFieldMergingSupported = false;

private:
	VHDHandle BoardHandle = VHD::InvalidHandle;
	VHDHandle StreamHandle = VHD::InvalidHandle;

	uint32 AdjacentFrameDropped = 0;
	uint32 LastFrameDroppedWarnedCount = 0;
};