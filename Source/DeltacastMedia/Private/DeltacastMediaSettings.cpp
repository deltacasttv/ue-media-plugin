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

#include "DeltacastMediaSettings.h"



const FDeltacastBoardSettings *UDeltacastMediaSettings::GetBoardSettings(const int32 BoardIndex) const
{
	const auto Settings = BoardSettings.FindByPredicate([BoardIndex](const FDeltacastBoardSettings &Setting)
	{
		return Setting.BoardIndex == BoardIndex;
	});

	return Settings;
}


#if WITH_EDITOR

void UDeltacastMediaSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent &PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(FDeltacastBoardSettings, BoardIndex))
	{
		const auto BoardSettingsIndex = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_STRING_CHECKED(UDeltacastMediaSettings, BoardSettings));
		if (BoardSettingsIndex >= 0)
		{
			const auto BoardIndex = BoardSettings[BoardSettingsIndex].BoardIndex;
			if (BoardIndex >= 0)
			{
				for (int i = 0; i < BoardSettings.Num(); ++i)
				{
					if (i == BoardSettingsIndex)
					{
						continue;
					}

					if (BoardSettings[i].BoardIndex == BoardIndex)
					{
						BoardSettings[BoardSettingsIndex].BoardIndex = -1;
						break;
					}
				}
			}
		}
	}

	SettingsChangedDelegate.Broadcast(this);
}

UDeltacastMediaSettings::FOnDeltacastMediaSettingsChanged& UDeltacastMediaSettings::OnSettingsChanged()
{
	return SettingsChangedDelegate;
}

UDeltacastMediaSettings::FOnDeltacastMediaSettingsChanged UDeltacastMediaSettings::SettingsChangedDelegate;

#endif