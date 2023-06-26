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

#include "DeltacastSdk.h"
#include "IMediaModule.h"
#include "IMediaPlayerFactory.h"
#include "IDeltacastMediaSourceModule.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"


#define LOCTEXT_NAMESPACE "DeltacastMediaSourceFactoryModule"


class FDeltacastMediaSourceFactoryModule : public IMediaPlayerFactory, public IModuleInterface
{
public: //~ IMediaPlayerFactory interface
	virtual bool CanPlayUrl(const FString& Url, const IMediaOptions* /*Options*/, TArray<FText>* /*OutWarnings*/, TArray<FText>* OutErrors) const override
	{
		FString Scheme;
		FString Location;

		// check scheme
		if (!Url.Split(TEXT("://"), &Scheme, &Location, ESearchCase::CaseSensitive))
		{
			if (OutErrors != nullptr)
			{
				OutErrors->Add(LOCTEXT("NoSchemeFound", "No URI scheme found"));
			}

			return false;
		}

		if (!SupportedUriSchemes.Contains(Scheme))
		{
			if (OutErrors != nullptr)
			{
				OutErrors->Add(FText::Format(LOCTEXT("SchemeNotSupported", "The URI scheme '{0}' is not supported"), FText::FromString(Scheme)));
			}

			return false;
		}

		return true;
	}

	virtual TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> CreatePlayer(IMediaEventSink& EventSink) override
	{
		const auto MediaModule = FModuleManager::LoadModulePtr<IDeltacastMediaSourceModule>("DeltacastMediaSource");
		return MediaModule != nullptr ? MediaModule->CreatePlayer(EventSink) : nullptr;
	}

	virtual FText GetDisplayName() const override
	{
		return LOCTEXT("MediaPlayerDisplayName", "Deltacast Device Interface");
	}

	virtual FName GetPlayerName() const override
	{
		static FName PlayerName(TEXT("DeltacastMedia"));
		return PlayerName;
	}

	virtual FGuid GetPlayerPluginGUID() const override
	{
		static FGuid PlayerPluginGUID(0x6CCE8F6B, 0x9E084570, 0xA903BC7A, 0x96898154);
		return PlayerPluginGUID;
	}

	virtual const TArray<FString>& GetSupportedPlatforms() const override
	{
		return SupportedPlatforms;
	}

	virtual bool SupportsFeature(const EMediaFeature Feature) const override
	{
		return Feature == EMediaFeature::VideoSamples;
	}

public: //~ IModuleInterface interface
	virtual void StartupModule() override
	{
		SupportedPlatforms.Add(TEXT("Windows"));
		SupportedPlatforms.Add(TEXT("Linux"));

		SupportedUriSchemes.Add(FDeltacast::GetProtocolName().ToString());

		const auto MediaModule = FModuleManager::LoadModulePtr<IMediaModule>("Media");
		if (MediaModule != nullptr)
		{
			MediaModule->RegisterPlayerFactory(*this);
		}
	}

	virtual void ShutdownModule() override
	{
		const auto MediaModule = FModuleManager::GetModulePtr<IMediaModule>("Media");
		if (MediaModule != nullptr)
		{
			MediaModule->UnregisterPlayerFactory(*this);
		}
	}

private:
	TArray<FString> SupportedPlatforms;

	TArray<FString> SupportedUriSchemes;
};

IMPLEMENT_MODULE(FDeltacastMediaSourceFactoryModule, DeltacastMediaSourceFactory);

#undef LOCTEXT_NAMESPACE
