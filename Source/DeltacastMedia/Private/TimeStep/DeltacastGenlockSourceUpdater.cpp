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

#include "DeltacastGenlockSourceUpdater.h"

#include "DeltacastHelpers.h"
#include "DeltacastMediaSettings.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaModule.h"



FDeltacastGenlockSourceUpdater::~FDeltacastGenlockSourceUpdater()
{
	ShutdownGenlockSources();
}


void FDeltacastGenlockSourceUpdater::InitializeGenlockSources()
{
	const auto& DeltacastSdk = FDeltacast::GetSdk();

	BoardCount = DeltacastSdk.GetNbBoards().value_or(0);

	const auto Settings = GetDefault<UDeltacastMediaSettings>();
	if (Settings != nullptr)
	{
		for (const auto& BoardSettings : Settings->BoardSettings)
		{
			UpdateGenlockSource(BoardSettings);
		}
	}

#if WITH_EDITOR
	UpdateSettingsDelegateHandle = UDeltacastMediaSettings::OnSettingsChanged().AddThreadSafeSP(this, &FDeltacastGenlockSourceUpdater::UpdateSettings);
#endif
}

void FDeltacastGenlockSourceUpdater::ShutdownGenlockSources()
{
#if WITH_EDITOR
	auto& DeltacastSdk = FDeltacast::GetSdk();

	UDeltacastMediaSettings::OnSettingsChanged().Remove(UpdateSettingsDelegateHandle);
	UpdateSettingsDelegateHandle.Reset();

	for (const auto& [BoardIndex, BoardGenlock] : BoardGenlockSource)
	{
		DeltacastSdk.CloseBoardHandle(BoardGenlock.BoardHandle);
	}

	BoardGenlockSource.Reset();
#endif
}




VHD_GENLOCKSOURCE Convert(const EDeltacastGenlockSource GenlockSource)
{
	const bool bIsLocal = GenlockSource == EDeltacastGenlockSource::Local;
	const bool bIsBlackBurst = GenlockSource == EDeltacastGenlockSource::BlackBurst;

	const auto PortIndex = [GenlockSource]() -> int32
	{
		switch (GenlockSource)
		{
		case EDeltacastGenlockSource::Local: [[fallthrough]];
		case EDeltacastGenlockSource::BlackBurst: return -1;
		case EDeltacastGenlockSource::RX0: return 0;
		case EDeltacastGenlockSource::RX1: return 1;
		case EDeltacastGenlockSource::RX2: return 2;
		case EDeltacastGenlockSource::RX3: return 3;
		case EDeltacastGenlockSource::RX4: return 4;
		case EDeltacastGenlockSource::RX5: return 5;
		case EDeltacastGenlockSource::RX6: return 6;
		case EDeltacastGenlockSource::RX7: return 7;
		case EDeltacastGenlockSource::RX8: return 8;
		case EDeltacastGenlockSource::RX9: return 9;
		case EDeltacastGenlockSource::RX10: return 10;
		case EDeltacastGenlockSource::RX11: return 11;
		default:
			checkNoEntry();
			return -1;
		}
	}();

	const auto VhdGenlockSource = Deltacast::Helpers::GetGenlockSourceFromPortIndex(bIsLocal, bIsBlackBurst, PortIndex);

	return VhdGenlockSource;
}

void FDeltacastGenlockSourceUpdater::UpdateGenlockSource(const FDeltacastBoardSettings& BoardSettings)
{
	if (0 > BoardSettings.BoardIndex || BoardSettings.BoardIndex >= BoardCount)
	{
		UE_LOG(LogDeltacastMedia, Display, TEXT("Ignoring board index %d, out of range [0, %lld)"), BoardSettings.BoardIndex, BoardCount);
		return;
	}

	auto& DeltacastSdk = FDeltacast::GetSdk();

	const auto NewGenlockSource = Convert(BoardSettings.GenlockSource);

#if WITH_EDITOR
	const auto ExistingBoardGenlock = BoardGenlockSource.Find(BoardSettings.BoardIndex);
	if (ExistingBoardGenlock == nullptr)
#endif
	{
		// New board index
		const auto BoardHandle = DeltacastSdk.OpenBoard(BoardSettings.BoardIndex).value_or(VHD::InvalidHandle);
		if (BoardHandle == VHD::InvalidHandle)
		{
			UE_LOG(LogDeltacastMedia, Error, TEXT("Cannot open board %d, genlock source won't be set"), BoardSettings.BoardIndex);
			return;
		}

		// TODO @JV v1: improvement: on error (popup? show in settings the current applied settings?)
		SetGenlockSource(BoardHandle, NewGenlockSource);

#if WITH_EDITOR
		BoardGenlockSource.Add(BoardSettings.BoardIndex, FBoardGenlock{ NewGenlockSource, BoardHandle });
#endif
	}
#if WITH_EDITOR
	else
	{
		// Existing board index
		const auto CurrentGenlockSource = ExistingBoardGenlock->GenlockSource;
		if (CurrentGenlockSource == NewGenlockSource)
		{
			return;
		}

		// TODO @JV v1: improvement: on error (popup? show in settings the current applied settings?)
		SetGenlockSource(ExistingBoardGenlock->BoardHandle, NewGenlockSource);

		ExistingBoardGenlock->GenlockSource = NewGenlockSource;
	}
#endif
}

bool FDeltacastGenlockSourceUpdater::SetGenlockSource(const VHDHandle BoardHandle, const VHD_GENLOCKSOURCE GenlockSource)
{
	auto& DeltacastSdk = FDeltacast::GetSdk();

	const auto SetGenlockResult = DeltacastSdk.SetBoardProperty(BoardHandle, VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_GENLOCK_SOURCE,
                                                               static_cast<VHD::ULONG>(GenlockSource));

	if (!Deltacast::Helpers::IsValid(SetGenlockResult))
	{
		UE_LOG(LogDeltacastMedia, Warning, TEXT("Cannot update the genlock source: %s"), *Deltacast::Helpers::GetGenlockSourceString(GenlockSource));
		return false;
	}

	return true;
}



void FDeltacastGenlockSourceUpdater::UpdateSettings(const UDeltacastMediaSettings* Settings)
{
	for (const auto& BoardSettings : Settings->BoardSettings)
	{
		UpdateGenlockSource(BoardSettings);
	}
}
