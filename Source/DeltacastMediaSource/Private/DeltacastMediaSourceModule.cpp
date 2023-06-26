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

#include "CoreMinimal.h"
#include "DeltacastMediaPlayer.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaSourceModule.h"
#include "IMediaPlayer.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"

#define LOCTEXT_NAMESPACE "DeltacastMediaSource"

DEFINE_LOG_CATEGORY(LogDeltacastMediaSource);


class FDeltacastMediaSourceModule : public IDeltacastMediaSourceModule
{
public: //~ IDeltacastMediaSourceModule
	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer(IMediaEventSink& EventSink) override
	{
		if (!FDeltacast::IsInitialized())
		{
			return nullptr;
		}

		return MakeShared<FDeltacastMediaPlayer, ESPMode::ThreadSafe>(EventSink);
	}

	virtual TSharedPtr<FSlateStyleSet> GetStyle() override
	{
		return StyleInstance;
	}

public: //~IModuleInterface interface
	virtual void StartupModule() override
	{
		CreateStyle();
	}

private:
	void CreateStyle()
	{
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleInstance->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

		static const FName     StyleName(TEXT("DeltacastMediaSourceStyle"));
		static const FVector2D Icon64x64(64.0f, 64.0f);

		StyleInstance = MakeShared<FSlateStyleSet>(StyleName);

		const auto Plugin = IPluginManager::Get().FindPlugin(TEXT("DeltacastMedia"));
		if (Plugin.IsValid())
		{
			StyleInstance->SetContentRoot(FPaths::Combine(Plugin->GetContentDir(), TEXT("Editor/Icons")));
		}

		StyleInstance->Set("DeltacastMediaIcon", new IMAGE_BRUSH("DeltacastMediaSource_64x64", Icon64x64));

#undef IMAGE_BRUSH
	}

private:
	TSharedPtr<FSlateStyleSet> StyleInstance;
};

IMPLEMENT_MODULE(FDeltacastMediaSourceModule, DeltacastMediaSource)

#undef LOCTEXT_NAMESPACE