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

#include "CoreGlobals.h"
#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "UObject/UObjectBase.h"

#define LOCTEXT_NAMESPACE "FDeltacastMediaEditorModule"


class FDeltacastMediaEditorModule : public IModuleInterface
{
public: //~IModuleInterface interface
	virtual void StartupModule() override
	{
		RegisterStyle();
	}

	virtual void ShutdownModule() override
	{
		if (UObjectInitialized() && !IsEngineExitRequested())
		{
			UnregisterStyle();
		}
	}

private:
	void RegisterStyle()
	{
#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(StyleInstance->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

		static const FVector2D Icon20x20(20.0f, 20.0f);
		static const FVector2D Icon64x64(64.0f, 64.0f);

		StyleInstance = MakeUnique<FSlateStyleSet>("DeltacastMediaStyle");

		const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("DeltacastMedia"));
		if (Plugin.IsValid())
		{
			StyleInstance->SetContentRoot(FPaths::Combine(Plugin->GetContentDir(), TEXT("Editor/Icons")));
		}


		StyleInstance->Set("ClassThumbnail.DeltacastMediaSource", new IMAGE_BRUSH("DeltacastMediaSource_64x64", Icon64x64));
		StyleInstance->Set("ClassIcon.DeltacastMediaSource", new IMAGE_BRUSH("DeltacastMediaSource_20x20", Icon20x20));
		StyleInstance->Set("ClassThumbnail.DeltacastMediaOutput", new IMAGE_BRUSH("DeltacastMediaOutput_64x64", Icon64x64));
		StyleInstance->Set("ClassIcon.DeltacastMediaOutput", new IMAGE_BRUSH("DeltacastMediaOutput_20x20", Icon20x20));

		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance.Get());

#undef IMAGE_BRUSH
	}

	void UnregisterStyle()
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance.Get());
		StyleInstance.Reset();
	}

private:
	TUniquePtr<FSlateStyleSet> StyleInstance;
};


IMPLEMENT_MODULE(FDeltacastMediaEditorModule, DeltacastMediaEditor);
	
#undef LOCTEXT_NAMESPACE