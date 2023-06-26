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

#include "DeltacastTimecodeUpdater.h"
#include "Engine/TimecodeProvider.h"
#include "Tickable.h"

#include "DeltacastTimecodeProvider.generated.h"


/**
 * Class to fetch a timecode via an Deltacast board.
 * When the signal is lost in the editor (not in PIE), the TimecodeProvider will try to re-synchronize every second.
 */
UCLASS(Blueprintable, editinlinenew, meta = (DisplayName = "Deltacast SDI Input", MediaIOCustomLayout = "Deltacast"))
class DELTACASTMEDIA_API UDeltacastTimecodeProvider : public UTimecodeProvider,
                                                      public FTickableGameObject,
                                                      public IDeltacastTimecodeUpdaterCallback
{
	GENERATED_BODY()

public:
	UDeltacastTimecodeProvider(const FObjectInitializer& ObjectInitializer);

public: //~ UTimecodeProvider interface
	virtual bool Initialize(UEngine *InEngine) override;
	virtual void Shutdown(UEngine *InEngine) override;

	virtual void FetchAndUpdate() override;
	virtual bool FetchTimecode(FQualifiedFrameTime &OutFrameTime) override;

	virtual FQualifiedFrameTime GetQualifiedFrameTime() const override;

	virtual ETimecodeProviderSynchronizationState GetSynchronizationState() const override { return State; }

	virtual bool IsAutoDetected() const override { return false; }
	virtual bool SupportsAutoDetected() const override { return false; }

public: //~ FTickableGameObject interface
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableWhenPaused() const override { return true; }
	virtual bool IsTickableInEditor() const override { return true; }
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UDeltacastTimecodeProvider, STATGROUP_Tickables); }

public: //~ UObject interface
	virtual void BeginDestroy() override;

public: //~ IDeltacastTimecodeUpdaterCallback
	virtual void OnInitializationCompleted(bool bSucceed) override;

public:
	/**
	 * Deltacast board index
	 */
	UPROPERTY(EditAnywhere, Category = "Timecode")
	int32 BoardIndex = -1;

private:
	void ReleaseResources();

private:
#if WITH_EDITORONLY_DATA
private:
	/** Engine used to initialize the Provider */
	UPROPERTY(Transient)
	UEngine* InitializedEngine;

	/** The time the last attempt to auto synchronize was triggered. */
	double LastAutoSynchronizeInEditorAppTime;
#endif

private:
	TUniquePtr<FDeltacastTimecodeUpdater> Updater;
	FRunnableThread *UpdaterThread = nullptr;

private:
	/** The current SynchronizationState of the TimecodeProvider*/
	ETimecodeProviderSynchronizationState State;
};
