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

#include "DeltacastCustomTimeStepUpdater.h"
#include "GenlockedCustomTimeStep.h"

#include "DeltacastCustomTimeStep.generated.h"


UCLASS(Blueprintable, EditInlineNew, meta = (DisplayName = "Deltcast Custom Timestep", MediaIOCustomLayout = "Deltacast"))
class DELTACASTMEDIA_API UDeltacastCustomTimeStep : public UGenlockedCustomTimeStep,
                                                    public IDeltacastCustomTimeStepCallback
{
	GENERATED_BODY()

public:
	/**
	* Deltacast board index
	*/
	UPROPERTY(EditAnywhere, Category = "Genlock")
	int32 BoardIndex = -1;

public: //~ UFixedFrameRateCustomTimeStep
	virtual bool Initialize(UEngine *InEngine) override;
	virtual void Shutdown(UEngine *InEngine) override;

	virtual bool UpdateTimeStep(UEngine *InEngine) override;

	virtual ECustomTimeStepSynchronizationState GetSynchronizationState() const override;

	virtual FFrameRate GetFixedFrameRate() const override;

public: //~ UGenlockedCustomTimeStep
	virtual bool SupportsFormatAutoDetection() const override;

	virtual bool IsLastSyncDataValid() const override;

	virtual uint32     GetLastSyncCountDelta() const override;
	virtual FFrameRate GetSyncRate() const override;

protected: //~ UGenlockedCustomTimeStep
	virtual bool WaitForSync() override;

public:
	virtual void OnInitializationCompleted(bool bSucceed) override;

public: //~ UObject
	virtual void BeginDestroy() override;

#if WITH_EDITOR

public: //~ UObject
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent &PropertyChangedEvent) override;
#endif

private:
	void ReleaseResources();

private:
	ECustomTimeStepSynchronizationState State = ECustomTimeStepSynchronizationState::Closed;

	bool bIsPreviousSyncCountValid = false;
	uint32 SyncCountDelta = 0;
	uint32 PreviousSyncCount = 0;

	bool bWarnedAboutVSync = false;

private:
	TUniquePtr<FDeltacastCustomTimeStepUpdater> Updater;
	FRunnableThread *UpdaterThread = nullptr;

#if WITH_EDITORONLY_DATA
private:
	/** Engine used to initialize the CustomTimeStep */
	UPROPERTY(Transient)
	TObjectPtr<UEngine> InitializedEngine;

	/** When Auto synchronize is enabled, the time the last attempt was triggered. */
	double LastAutoSynchronizeInEditorAppTime = 0.0;

	double LastAutoDetectInEditorAppTime = 0.0;
#endif
};
