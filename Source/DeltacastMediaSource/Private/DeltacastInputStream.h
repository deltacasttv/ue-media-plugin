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

#if WITH_EDITOR
#include "Delegates/IDelegateInstance.h"
#endif
#include "DeltacastDefinition.h"
#include "DeltacastDeviceScanner.h"
#include "DeltacastMediaSettings.h"
#include "DeltacastMediaTextureSample.h"
#include "MediaIOCoreDefinitions.h"
#include "HAL/Runnable.h"



struct FDeltacastRequestBuffer
{
	uint32_t VideoBufferSize;

	bool bIsProgressive;
};

struct FDeltacastRequestedBuffer
{
	uint8_t *VideoBuffer;
};


class IDeltacastInputStreamCallback
{
public:
	IDeltacastInputStreamCallback() = default;
	virtual ~IDeltacastInputStreamCallback() = default;

public:
	virtual void OnInitializationCompleted(bool bSucceed) = 0;

	virtual bool OnRequestInputBuffer(const FDeltacastRequestBuffer& RequestBuffer, FDeltacastRequestedBuffer& RequestedBuffer) = 0;
	virtual bool OnInputFrameReceived(const FDeltacastVideoFrameData& VideoFrame) = 0;

	virtual void OnSourceResumed() = 0;
	virtual void OnSourceStopped() = 0;

	virtual void OnCompletion(bool bSucceed) = 0;
};


struct FDeltacastInputStreamConfig final
{
	IDeltacastInputStreamCallback *Callback = nullptr;

	bool bIsSdi = true;
	Deltacast::Device::Config::FSdiPortConfig SdiPortConfig = {};
	Deltacast::Device::Config::FDvPortConfig  DvPortConfig  = {};

	bool bAutoLoadEdid = true;

	bool bErrorOnSourceLost = true;

	bool bLogDroppedFrameCount = false;

	EMediaIOTimecodeFormat TimecodeFormat = EMediaIOTimecodeFormat::None;
};


class FDeltacastInputStream final : public FRunnable,
                                    public TSharedFromThis<FDeltacastInputStream, ESPMode::ThreadSafe>
{
public:
	FDeltacastInputStream(const FDeltacastInputStreamConfig &Config);

public: //~ FRunnable
	virtual bool   Init() override;
	virtual uint32 Run() override;
	virtual void   Exit() override;

	virtual void Stop() override;

private:
	void WaitForChannelLocked(VHD_CORE_BOARDPROPERTY ChannelStatus) const;

	void ComputeConstants();
	void ComputeTimecodeSource();

	[[nodiscard]]FString ConfigString() const;

private:
	void RegisterSettingsEvent();
	void UnregisterSettingsEvent();

#if WITH_EDITOR
	void UpdateSettings(const UDeltacastMediaSettings *Settings);

	FDelegateHandle UpdateSettingsDelegateHandle;
#endif

private:
	using Super = FRunnable;

	using FDcBaseConfig = Deltacast::Device::Config::FBasePortConfig;
	using FDcSdiConfig = Deltacast::Device::Config::FSdiPortConfig;
	using FDcDvConfig  = Deltacast::Device::Config::FDvPortConfig;

private:
	bool bStopRequested = false;
	bool bSourceError = false;

private:
	IDeltacastInputStreamCallback *Callback = nullptr;

	bool bIsSdi;

	[[nodiscard]] const FDcBaseConfig& BasePortConfig() const;

	FDcSdiConfig SdiPortConfig{};
	FDcDvConfig  DvPortConfig{};

	bool bAutoLoadEdid = true;

	EMediaIOTimecodeFormat TimecodeFormat = EMediaIOTimecodeFormat::None;
	VHD_TIMECODE_SOURCE    TimecodeSource = VHD_TIMECODE_SOURCE::NB_VHD_TC_SRC;

private:
	uint32 Width  = 0;
	uint32 Height = 0;
	uint32 Stride = 0;

private:
	Deltacast::Helpers::FStreamStatistics StreamStatistics;

	bool bInterlaced            = false;
	bool bFieldMergingSupported = false;

	bool bErrorOnSourceLost = true;

private:
	VHDHandle BoardHandle  = VHD::InvalidHandle;
	VHDHandle StreamHandle = VHD::InvalidHandle;
};
