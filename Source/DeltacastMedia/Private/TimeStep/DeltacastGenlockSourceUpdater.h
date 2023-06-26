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
#include "DeltacastMediaSettings.h"
#include "Templates/SharedPointer.h"



class FDeltacastGenlockSourceUpdater : public TSharedFromThis<FDeltacastGenlockSourceUpdater, ESPMode::ThreadSafe>
{
public:
	~FDeltacastGenlockSourceUpdater();

public:
	void InitializeGenlockSources();

	void ShutdownGenlockSources();

private:
	void UpdateGenlockSource(const FDeltacastBoardSettings& BoardSettings);

	static bool SetGenlockSource(VHDHandle BoardHandle, VHD_GENLOCKSOURCE GenlockSource);

private:
	void UpdateSettings(const UDeltacastMediaSettings* Settings);

private:
	int64 BoardCount = 0;

#if WITH_EDITOR
private:
	struct FBoardGenlock
	{
		VHD_GENLOCKSOURCE GenlockSource = VHD_GENLOCKSOURCE::NB_VHD_GENLOCKSOURCES;

		VHDHandle BoardHandle = VHD::InvalidHandle;
	};

	TMap<int32, FBoardGenlock> BoardGenlockSource;

	FDelegateHandle UpdateSettingsDelegateHandle;
#endif
};
