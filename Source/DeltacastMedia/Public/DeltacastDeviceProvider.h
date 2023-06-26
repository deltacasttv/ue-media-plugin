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

#include "CoreMinimal.h"
#include "IMediaIOCoreDeviceProvider.h"


class DELTACASTMEDIA_API FDeltacastDeviceProvider final : public IMediaIOCoreDeviceProvider
{
public:
	static FName GetProviderName();
	static FName GetProtocolName();

	/** Can device do fill and key */
	bool CanDeviceDoAlpha(const FMediaIODevice &InDevice) const;

public: //~ IMediaIOCoreDeviceProvider
	virtual FName GetFName() override;

	virtual TArray<FMediaIOConnection>                 GetConnections() const override;
	virtual TArray<FMediaIOConfiguration>              GetConfigurations() const override;
	virtual TArray<FMediaIOConfiguration>              GetConfigurations(bool bAllowInput, bool bAllowOutput) const override;
	virtual TArray<FMediaIOInputConfiguration>         GetInputConfigurations() const override;
	virtual TArray<FMediaIOOutputConfiguration>        GetOutputConfigurations() const override;
	virtual TArray<FMediaIOVideoTimecodeConfiguration> GetTimecodeConfigurations() const override;
	virtual TArray<FMediaIODevice>                     GetDevices() const override;
	virtual TArray<FMediaIOMode>                       GetModes(const FMediaIODevice &InDevice, bool bInOutput) const override;

	virtual FMediaIOConfiguration              GetDefaultConfiguration() const override;
	virtual FMediaIOMode                       GetDefaultMode() const override;
	virtual FMediaIOInputConfiguration         GetDefaultInputConfiguration() const override;
	virtual FMediaIOOutputConfiguration        GetDefaultOutputConfiguration() const override;
	virtual FMediaIOVideoTimecodeConfiguration GetDefaultTimecodeConfiguration() const override;

private: //~ IMediaIOCoreDeviceProvider implementation without caching
	TArray<FMediaIOConnection>                 GetConnections_Impl() const;
	TArray<FMediaIOConfiguration>              GetConfigurations_Impl(bool bAllowInput, bool bAllowOutput) const;
	TArray<FMediaIOInputConfiguration>         GetInputConfigurations_Impl() const;
	TArray<FMediaIOOutputConfiguration>        GetOutputConfigurations_Impl() const;
	TArray<FMediaIOVideoTimecodeConfiguration> GetTimecodeConfigurations_Impl() const;
	TArray<FMediaIODevice>                     GetDevices_Impl() const;
	TArray<FMediaIOMode>                       GetModes_Impl(bool bInOutput) const;

private:
	friend class FDeltacastMediaModule;

	void UpdateCache() const;

	[[nodiscard]] static TArray<uint64> ComputeCurrentHardwareIdentifier();

private:
	mutable FCriticalSection CacheCriticalSection;
	mutable TArray<uint64> CacheHardwareIdentifier;

	mutable TArray<FMediaIOConnection>                 ConnectionsCache;
	mutable TArray<FMediaIOConfiguration>              ConfigurationsInputCache;
	mutable TArray<FMediaIOConfiguration>              ConfigurationsOutputCache;
	mutable TArray<FMediaIOInputConfiguration>         InputConfigurationsCache;
	mutable TArray<FMediaIOOutputConfiguration>        OutputConfigurationsCache;
	mutable TArray<FMediaIOVideoTimecodeConfiguration> TimecodeConfigurationsCache;
	mutable TArray<FMediaIODevice>                     DevicesCache;
	mutable TArray<FMediaIOMode>                       ModesInputCache;
	mutable TArray<FMediaIOMode>                       ModesOutputCache;
};
