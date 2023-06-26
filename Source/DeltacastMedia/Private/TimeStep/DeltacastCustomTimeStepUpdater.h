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
#include "HAL/Event.h"
#include "HAL/Runnable.h"
#include "Misc/FrameRate.h"


class IDeltacastCustomTimeStepCallback
{
public:
	IDeltacastCustomTimeStepCallback() = default;
	virtual ~IDeltacastCustomTimeStepCallback() = default;

public:
	virtual void OnInitializationCompleted(bool bSucceed) = 0;
};

struct FDeltacastCustomTimeStepConfig final
{
	IDeltacastCustomTimeStepCallback* Callback;

	int32 BoardIndex;
};

class FDeltacastCustomTimeStepUpdater final : public FRunnable
{
public:
	FDeltacastCustomTimeStepUpdater(FDeltacastCustomTimeStepConfig Config);

public: //~ FRunnable
	virtual bool   Init() override;
	virtual uint32 Run() override;
	virtual void   Exit() override;

	virtual void Stop() override;

public:
	[[nodiscard]] bool WaitForSync() const;

	[[nodiscard]] uint32 GetSyncCount() const;

	[[nodiscard]] bool IsGenlockSynchronized() const;

	[[nodiscard]] FFrameRate GetSyncRate() const;

private:
	void WaitAndDetectGenlock();

private:
	bool bStopRequested = false;

	bool bIsSynchronized = false;
	uint32 SyncCount = 0;

	bool              bIsEuropeanClock          = true;
	VHD_VIDEOSTANDARD GenlockVideoStandard = VHD_VIDEOSTANDARD::NB_VHD_VIDEOSTANDARDS;

private:
	IDeltacastCustomTimeStepCallback* Callback = nullptr;

	int32 BoardIndex = -1;

private:
	VHDHandle BoardHandle = VHD::InvalidHandle;
	VHDHandle TimerHandle = VHD::InvalidHandle;

	mutable FEvent *WaitForSyncEvent = nullptr;
};