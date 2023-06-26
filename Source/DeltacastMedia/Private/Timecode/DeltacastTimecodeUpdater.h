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
#include "HAL/Runnable.h"
#include "Misc/FrameRate.h"

#include <optional>



struct FDeltacastTimecode
{
public:
	VHD_TIMECODE Timecode;

	float FrameRate;

public:
	[[nodiscard]] int32 GetFrameNumber() const;

	[[nodiscard]] FFrameRate GetFrameRate() const;
};

class IDeltacastTimecodeUpdaterCallback
{
public:
	IDeltacastTimecodeUpdaterCallback()          = default;
	virtual ~IDeltacastTimecodeUpdaterCallback() = default;

public:
	virtual void OnInitializationCompleted(bool bSucceed) = 0;
};

struct FDeltacastTimecodeConfig final
{
	IDeltacastTimecodeUpdaterCallback *Callback;

	int32 BoardIndex;

	VHD_TIMECODE_SOURCE TimecodeSource;
};

class FDeltacastTimecodeUpdater final : public FRunnable
{
public:
	FDeltacastTimecodeUpdater(FDeltacastTimecodeConfig Config);

public: //~ FRunnable
	virtual bool   Init() override;
	virtual uint32 Run() override;
	virtual void   Exit() override;

	virtual void Stop() override;

public:
	FDeltacastTimecode GetTimecode() const;

private:
	std::optional<FDeltacastTimecode> WaitForTimecodeLocked() const;

private:
	bool bStopRequested = false;

private:
	IDeltacastTimecodeUpdaterCallback *Callback = nullptr;

	int32 BoardIndex = -1;

	VHD_TIMECODE_SOURCE TimecodeSource = VHD_TIMECODE_SOURCE::NB_VHD_TC_SRC;

private:
	VHDHandle BoardHandle = VHD::InvalidHandle;

	mutable FCriticalSection TimecodeCriticalSection;
	FDeltacastTimecode       LastTimecode = {};
};
