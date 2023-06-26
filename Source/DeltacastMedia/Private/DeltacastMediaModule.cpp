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

#include "IDeltacastMediaModule.h"

#include "DeltacastDeviceProvider.h"
#include "DeltacastGenlockSourceUpdater.h"
#include "DeltacastSdk.h"
#include "DeltacastMediaSettings.h"
#include "IMediaIOCoreModule.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif
#include "Misc/App.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"

#define LOCTEXT_NAMESPACE "FDeltacastMediaModule"

DEFINE_LOG_CATEGORY(LogDeltacastMedia);



class FDeltacastMediaModule : public IDeltacastMediaModule
{
public: //~ IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

public: //~ IDeltacastMediaModule
	/** @return true if the Deltacast module and its dependencies could be loaded */
	virtual bool IsInitialized() const override;

	/** @return true if the Deltacast card can be used */
	virtual bool CanBeUsed() const override;

private:
	FDeltacastDeviceProvider DeviceProvider;

	FDeltacast Deltacast;

	TSharedRef<FDeltacastGenlockSourceUpdater> GenlockSourceUpdater = MakeShared<FDeltacastGenlockSourceUpdater>();
};


static const FName SettingsContainerName = "Project";
static const FName SettingsCategoryName = "Plugins";
static const FName SettingsSectionName = "Deltacast Media";


void FDeltacastMediaModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	if (!Deltacast.Initialize())
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to initialize Deltacast SDK access"));
		return;
	}

#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(SettingsContainerName, SettingsCategoryName, SettingsSectionName,
			LOCTEXT("DeltacastMediaSettingsName", "Deltacast Media"),
			LOCTEXT("DeltacastmediaSettingsDescription", "Configure Deltacast plug-in"),
			GetMutableDefault<UDeltacastMediaSettings>()
		);
	}
#endif

	GenlockSourceUpdater->InitializeGenlockSources();

	DeviceProvider.UpdateCache();

	if (IMediaIOCoreModule::IsAvailable())
	{
		IMediaIOCoreModule::Get().RegisterDeviceProvider(&DeviceProvider);
	}
}

void FDeltacastMediaModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (IMediaIOCoreModule::IsAvailable())
	{
		IMediaIOCoreModule::Get().UnregisterDeviceProvider(&DeviceProvider);
	}

	GenlockSourceUpdater->ShutdownGenlockSources();

#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings(SettingsContainerName, SettingsCategoryName, SettingsSectionName);
	}
#endif

	Deltacast.Shutdown();
}



bool FDeltacastMediaModule::IsInitialized() const
{
	return Deltacast.IsInitialized();
}

bool FDeltacastMediaModule::CanBeUsed() const
{
	return FApp::CanEverRender();
}


IMPLEMENT_MODULE(FDeltacastMediaModule, DeltacastMedia)

#undef LOCTEXT_NAMESPACE
