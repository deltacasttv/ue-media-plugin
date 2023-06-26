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

#include "UObject/Object.h"

#include "DeltacastMediaSettings.generated.h"


UENUM()
enum class EDeltacastTimecodeSource
{
	OnBoard UMETA(DisplayName = "On board"),
	CompanionCard UMETA(DisplayName = "Companion Card"),
};

UENUM()
enum class EDeltacastGenlockSource
{
	Local = -2 UMETA(DisplayName = "Local"),
	BlackBurst = -1 UMETA(DisplayName = "Black Burst"),
	RX0 = 0 UMETA(DisplayName = "RX 0"),
	RX1 = 1 UMETA(DisplayName = "RX 1"),
	RX2 = 2 UMETA(DisplayName = "RX 2"),
	RX3 = 3 UMETA(DisplayName = "RX 3"),
	RX4 = 4 UMETA(DisplayName = "RX 4"),
	RX5 = 5 UMETA(DisplayName = "RX 5"),
	RX6 = 6 UMETA(DisplayName = "RX 6"),
	RX7 = 7 UMETA(DisplayName = "RX 7"),
	RX8 = 8 UMETA(DisplayName = "RX 8"),
	RX9 = 9 UMETA(DisplayName = "RX 9"),
	RX10 = 10 UMETA(DisplayName = "RX 10"),
	RX11 = 11 UMETA(DisplayName = "RX 11"),
};

USTRUCT()
struct DELTACASTMEDIA_API FDeltacastBoardSettings
{
	GENERATED_BODY()

public:
	/**
	 * Deltacast board index
	 */
	UPROPERTY(config, EditAnywhere, Category = "General")
	int32 BoardIndex = -1;

public: // Genlock
	/**
	 * Use RX port as genlock source
	 */
	UPROPERTY(config, EditAnywhere, Category = "Genlock")
	EDeltacastGenlockSource GenlockSource = EDeltacastGenlockSource::Local;

public: // Timecode
	/**
	* Use companion card as timecode source
	*/
	UPROPERTY(config, EditAnywhere, Category = "Timecode")
	EDeltacastTimecodeSource TimecodeSource = EDeltacastTimecodeSource::OnBoard;
};


UCLASS(config=DeviceProfiles)
class DELTACASTMEDIA_API UDeltacastMediaSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category = "Boards", meta = (EditFixedOrder))
	TArray<FDeltacastBoardSettings> BoardSettings;

public:
	const FDeltacastBoardSettings *GetBoardSettings(int32 BoardIndex) const;

#if WITH_EDITOR
public:
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent &PropertyChangedEvent) override;

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeltacastMediaSettingsChanged, const UDeltacastMediaSettings*);

	static FOnDeltacastMediaSettingsChanged& OnSettingsChanged();

protected:
	static FOnDeltacastMediaSettingsChanged SettingsChangedDelegate;
#endif
};