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

#include "DeltacastDeviceProvider.h"

#include "DeltacastDeviceScanner.h"
#include "DeltacastHelpers.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaModule.h"

DECLARE_CYCLE_STAT(TEXT("GetConfigurations"), STAT_DeltacastDeviceConfigurations, STATGROUP_Deltacast);

#define LOCTEXT_NAMESPACE "DeltacastDeviceProvider"


namespace DeltacastDeviceProvider
{
	FMediaIOMode ToMediaMode(const Deltacast::Device::Config::FSdiPortConfig& SdiDescriptor)
	{
		FMediaIOMode MediaMode;

		const auto VideoCharacteristics = Deltacast::Helpers::GetVideoCharacteristics(SdiDescriptor.VideoStandard);
		if (!VideoCharacteristics.has_value())
		{
			UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get video characteristics"));

			MediaMode.DeviceModeIdentifier = -1;
			return MediaMode;
		}

		const auto CorrectedFrameRate = VideoCharacteristics->bIsInterlaced ? VideoCharacteristics->FrameRate * 2 : VideoCharacteristics->FrameRate;

		if (SdiDescriptor.Base.bIsEuropeanClock)
		{
			MediaMode.FrameRate = FFrameRate(CorrectedFrameRate, 1);
		}
		else
		{
			MediaMode.FrameRate = FFrameRate(CorrectedFrameRate * 1000, 1001);
		}
			  
		MediaMode.Resolution = FIntPoint(VideoCharacteristics->Width, VideoCharacteristics->Height);
		MediaMode.DeviceModeIdentifier = Deltacast::Helpers::GetDeviceModeIdentifier(SdiDescriptor.Base.bIsEuropeanClock, SdiDescriptor.VideoStandard);

		if (Deltacast::Helpers::IsPsf(SdiDescriptor.VideoStandard))
		{
			MediaMode.Standard = EMediaIOStandardType::ProgressiveSegmentedFrame;
		}
		else if (VideoCharacteristics->bIsInterlaced)
		{
			MediaMode.Standard = EMediaIOStandardType::Interlaced;
		}
		else
		{
			MediaMode.Standard = EMediaIOStandardType::Progressive;
		}

		return MediaMode;
	}

	FMediaIOMode ToMediaMode(const Deltacast::Device::Config::FDvPortConfig& DvDescriptor)
	{
		FMediaIOMode MediaMode;

		const auto VideoCharacteristics = Deltacast::Helpers::GetVideoCharacteristics(DvDescriptor.VideoStandard);
		if (!VideoCharacteristics.has_value())
		{
			UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get video characteristics"));
			MediaMode.DeviceModeIdentifier = -1;
			return MediaMode;
		}

		const auto CorrectedFrameRate = VideoCharacteristics->bIsInterlaced ? VideoCharacteristics->FrameRate * 2 : VideoCharacteristics->FrameRate;

		if (DvDescriptor.Base.bIsEuropeanClock)
		{
			MediaMode.FrameRate = FFrameRate(CorrectedFrameRate, 1);
		}
		else
		{
			MediaMode.FrameRate = FFrameRate(CorrectedFrameRate * 1000, 1001);
		}
		
		MediaMode.Resolution = FIntPoint(VideoCharacteristics->Width, VideoCharacteristics->Height);
		MediaMode.DeviceModeIdentifier = Deltacast::Helpers::GetDeviceModeIdentifier(DvDescriptor.Base.bIsEuropeanClock, DvDescriptor.VideoStandard);
		
		if (VideoCharacteristics->bIsInterlaced)
		{
			MediaMode.Standard = EMediaIOStandardType::Interlaced;
		}
		else
		{
			MediaMode.Standard = EMediaIOStandardType::Progressive;
		}

		return MediaMode;
	}
}



FName FDeltacastDeviceProvider::GetProviderName()
{
	static FName ProviderName = "deltacast";
	return ProviderName;
}

FName FDeltacastDeviceProvider::GetProtocolName()
{
	return FDeltacast::GetProtocolName();
}



bool FDeltacastDeviceProvider::CanDeviceDoAlpha(const FMediaIODevice &InDevice) const
{
	if (!FDeltacast::IsInitialized() || !InDevice.IsValid())
	{
		return false;
	}

	auto& DeltacastSdk = FDeltacast::GetSdk();

	const auto BoardIndex = InDevice.DeviceIdentifier;
	const auto BoardHandle = DeltacastSdk.OpenBoard(BoardIndex).value_or(VHD::InvalidHandle);
	if (BoardHandle == VHD::InvalidHandle)
	{
		UE_LOG(LogDeltacastMedia, Warning, TEXT("Failed to open board index %d"), BoardIndex);
		return false;
	}

	Deltacast::Helpers::TScopeExit CloseBoardHandle([&]() { DeltacastSdk.CloseBoardHandle(BoardHandle); });

	const auto IsYuvkSupported = DeltacastSdk.GetBoardCapBufferPacking(BoardHandle, VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUVK4224_8);
	const auto TxCount = DeltacastSdk.GetTxCount(BoardHandle);

	if (TxCount.value_or(0) < 2 || !IsYuvkSupported.value_or(false))
		return false;
	
	return true;
}



FName FDeltacastDeviceProvider::GetFName()
{
	return GetProviderName();
}


TArray<FMediaIOConnection> FDeltacastDeviceProvider::GetConnections() const
{
	if (!FDeltacast::IsInitialized())
	{
		return {};
	}

	UpdateCache();

	return ConnectionsCache;
}

TArray<FMediaIOConfiguration> FDeltacastDeviceProvider::GetConfigurations() const
{
	return GetConfigurations(true, true);
}

TArray<FMediaIOConfiguration> FDeltacastDeviceProvider::GetConfigurations(const bool bAllowInput, const bool bAllowOutput) const
{
	if (!FDeltacast::IsInitialized())
	{
		return {};
	}

	UpdateCache();

	TArray<FMediaIOConfiguration> Results;

	if (bAllowInput)
	{
		Results.Append(ConfigurationsInputCache);
	}

	if (bAllowOutput)
	{
		Results.Append(ConfigurationsOutputCache);
	}

	return Results;
}

TArray<FMediaIOInputConfiguration> FDeltacastDeviceProvider::GetInputConfigurations() const
{
	if (!FDeltacast::IsInitialized())
	{
		return {};
	}

	UpdateCache();

	return InputConfigurationsCache;
}

TArray<FMediaIOOutputConfiguration> FDeltacastDeviceProvider::GetOutputConfigurations() const
{
	if (!FDeltacast::IsInitialized())
	{
		return {};
	}

	UpdateCache();

	return OutputConfigurationsCache;
}

TArray<FMediaIOVideoTimecodeConfiguration> FDeltacastDeviceProvider::GetTimecodeConfigurations() const
{
	if (!FDeltacast::IsInitialized())
	{
		return {};
	}

	UpdateCache();

	return TimecodeConfigurationsCache;
}

TArray<FMediaIODevice> FDeltacastDeviceProvider::GetDevices() const
{
	if (!FDeltacast::IsInitialized())
	{
		return {};
	}

	UpdateCache();

	return DevicesCache;
}

TArray<FMediaIOMode> FDeltacastDeviceProvider::GetModes(const FMediaIODevice &InDevice, const bool bInOutput) const
{
	if (!FDeltacast::IsInitialized() || !InDevice.IsValid())
	{
		return {};
	}

	UpdateCache();

	return bInOutput ? ModesInputCache : ModesOutputCache;
}



FMediaIOConfiguration FDeltacastDeviceProvider::GetDefaultConfiguration() const
{
	FMediaIOConfiguration Result;

	Result.bIsInput                                = true;
	Result.MediaConnection.Device.DeviceIdentifier = 0;
	Result.MediaConnection.Protocol                = GetProtocolName();
	Result.MediaConnection.PortIdentifier          = 0;
	Result.MediaConnection.TransportType           = EMediaIOTransportType::SingleLink;
	Result.MediaMode                               = GetDefaultMode();

	return Result;
}

FMediaIOMode FDeltacastDeviceProvider::GetDefaultMode() const
{
	FMediaIOMode Mode;

	Mode.DeviceModeIdentifier = Deltacast::Helpers::GetDeviceModeIdentifier(true, VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_60Hz);
	Mode.FrameRate            = FFrameRate(60, 1);
	Mode.Resolution           = FIntPoint(1920, 1080);
	Mode.Standard             = EMediaIOStandardType::Progressive;

	return Mode;
}

FMediaIOInputConfiguration FDeltacastDeviceProvider::GetDefaultInputConfiguration() const
{
	FMediaIOInputConfiguration Configuration;

	Configuration.MediaConfiguration          = GetDefaultConfiguration();
	Configuration.MediaConfiguration.bIsInput = true;
	Configuration.InputType                   = EMediaIOInputType::Fill;

	return Configuration;
}

FMediaIOOutputConfiguration FDeltacastDeviceProvider::GetDefaultOutputConfiguration() const
{
	FMediaIOOutputConfiguration Configuration;

	Configuration.MediaConfiguration          = GetDefaultConfiguration();
	Configuration.MediaConfiguration.bIsInput = false;
	Configuration.OutputReference             = EMediaIOReferenceType::FreeRun;
	Configuration.OutputType                  = EMediaIOOutputType::Fill;
	Configuration.KeyPortIdentifier           = -1;
	Configuration.ReferencePortIdentifier     = -1;

	return Configuration;
}

FMediaIOVideoTimecodeConfiguration FDeltacastDeviceProvider::GetDefaultTimecodeConfiguration() const
{
	FMediaIOVideoTimecodeConfiguration Configuration;

	Configuration.MediaConfiguration = GetDefaultConfiguration();
	Configuration.TimecodeFormat     = EMediaIOAutoDetectableTimecodeFormat::None;

	return Configuration;
}


TArray<FMediaIOConfiguration> FDeltacastDeviceProvider::GetOutputKeyConfigurations() const
{
	if (!FDeltacast::IsInitialized())
	{
		return {};
	}

	UpdateCache();

	return ConfigurationsKeyOutputCache;
}



TArray<FMediaIOConnection> FDeltacastDeviceProvider::GetConnections_Impl() const
{
	TArray<FMediaIOConnection> Results;

	if (!FDeltacast::IsInitialized())
	{
		return Results;
	}

	auto& DeltacastSdk = FDeltacast::GetSdk();

	VHD::ULONG NbBoards = 0;
	VHD::ULONG ApiVersion = 0;
	const auto ApiInfoResult = DeltacastSdk.GetApiInfo(&ApiVersion, &NbBoards);
	if (!Deltacast::Helpers::IsValid(ApiInfoResult))
	{
		return Results;
	}

	for (int32 BoardIndex = 0; BoardIndex < static_cast<int32>(NbBoards); ++BoardIndex)
	{
		const auto BoardModel = DeltacastSdk.GetBoardModel(BoardIndex);
		if (BoardModel == nullptr)
		{
			UE_LOG(LogDeltacastMedia, Warning, TEXT("Failed to get the board model for board index %u"), BoardIndex);
			continue;
		}

		const auto BoardHandle = DeltacastSdk.OpenBoard(BoardIndex).value_or(VHD::InvalidHandle);
		if (BoardHandle == VHD::InvalidHandle)
		{
			UE_LOG(LogDeltacastMedia, Warning, TEXT("Failed to open board index %d"), BoardIndex);
			continue;
		}

		Deltacast::Helpers::TScopeExit CloseBoardHandle([&]() {DeltacastSdk.CloseBoardHandle(BoardHandle); });

		FMediaIOConnection MediaConnection;

		MediaConnection.Device.DeviceIdentifier = BoardIndex;
		MediaConnection.Device.DeviceName = FName(BoardModel);
		MediaConnection.Protocol = GetProtocolName();

		const auto NbRxChannels = DeltacastSdk.GetRxCount(BoardHandle);
		if (!NbRxChannels.has_value())
		{
			UE_LOG(LogDeltacastMedia, Warning, TEXT("Failed to get board %d RX count"), BoardIndex);
			continue;
		}

		for (VHD::ULONG RxPortIndex = 0; RxPortIndex < NbRxChannels.value(); ++RxPortIndex)
		{
			const auto ChannelType = DeltacastSdk.GetChannelType(BoardHandle, true, RxPortIndex);
			if (!ChannelType)
				continue;

			MediaConnection.PortIdentifier = static_cast<int32>(RxPortIndex);

			const auto bIsSdi = Deltacast::Helpers::IsSdi(ChannelType.value());
			const auto bIsDv = Deltacast::Helpers::IsDv(ChannelType.value());

			if (bIsSdi)
			{
				MediaConnection.TransportType = EMediaIOTransportType::SingleLink;
			}
			else if (bIsDv)
			{
				MediaConnection.TransportType = EMediaIOTransportType::HDMI;
			}
			else
			{
				continue;
			}

			Results.Add(MediaConnection);
		}
	}

	return Results;
}

TTuple<TArray<FMediaIOConfiguration>, TArray<FMediaIOConfiguration>> FDeltacastDeviceProvider::GetConfigurations_Impl(const bool bAllowInput, const bool bAllowOutput) const
{
	TArray<FMediaIOConfiguration> Results;
	TArray<FMediaIOConfiguration> KeyResults;

	if (!FDeltacast::IsInitialized())
	{
		return TTuple<TArray<FMediaIOConfiguration>, TArray<FMediaIOConfiguration>>(Results, KeyResults);
	}

	auto& DeltacastSdk = FDeltacast::GetSdk();

	const auto NumberOfBoards = DeltacastSdk.GetNbBoards();
	if (!NumberOfBoards)
	{
		return TTuple<TArray<FMediaIOConfiguration>, TArray<FMediaIOConfiguration>>(Results, KeyResults);
	}

	for (VHD::ULONG BoardIndex = 0; BoardIndex < NumberOfBoards.value(); ++BoardIndex)
	{
		FMediaIOConfiguration MediaConfiguration;
		MediaConfiguration.MediaConnection.Device.DeviceIdentifier = BoardIndex;
		MediaConfiguration.MediaConnection.Protocol = GetProtocolName();

		const auto IsDualSupported = CanDeviceDoAlpha(MediaConfiguration.MediaConnection.Device);

		const auto BoardModel = DeltacastSdk.GetBoardModel(BoardIndex);
		if (BoardModel == nullptr)
		{
			UE_LOG(LogDeltacastMedia, Warning, TEXT("Failed to get the board model for board index %u"), BoardIndex);
			continue;
		}

		const auto BoardHandle = DeltacastSdk.OpenBoard(BoardIndex).value_or(VHD::InvalidHandle);
		if (!BoardHandle)
		{
			continue;
		}

		Deltacast::Helpers::TScopeExit CloseBoardHandle([&]() {DeltacastSdk.CloseBoardHandle(BoardHandle); });

		MediaConfiguration.MediaConnection.Device.DeviceName = FName(BoardModel);

		auto DeviceScanner = Deltacast::Device::FDeviceScanner::GetDeviceScanner(BoardIndex);
		if (!DeviceScanner)
		{
			continue;
		}

		auto SdiDeviceScanner = DeviceScanner.value().GetSdiDeviceScanner();

		if (SdiDeviceScanner.has_value())
		{
			for (auto SdiDescriptor : SdiDeviceScanner.value())
			{
				if ((!bAllowInput && SdiDescriptor.Base.bIsInput) ||
					(!bAllowOutput && !SdiDescriptor.Base.bIsInput))
				{
					continue;
				}

				if (!SdiDescriptor.IsValid(BoardHandle))
				{
					continue;
				}

				MediaConfiguration.bIsInput = SdiDescriptor.Base.bIsInput;
				MediaConfiguration.MediaConnection.PortIdentifier = static_cast<int32>(SdiDescriptor.Base.PortIndex);

				if (SdiDescriptor.IsSingleLink())
				{
					MediaConfiguration.MediaConnection.TransportType = EMediaIOTransportType::SingleLink;
					MediaConfiguration.MediaConnection.QuadTransportType = {};
				}
				else
				{
					MediaConfiguration.MediaConnection.TransportType = EMediaIOTransportType::QuadLink;
					const auto QuadLinkType = SdiDescriptor.GetQuadLinkType();
					switch (QuadLinkType)
					{
					case Deltacast::Helpers::EQuadLinkType::Quadrant:
						MediaConfiguration.MediaConnection.QuadTransportType = EMediaIOQuadLinkTransportType::SquareDivision;
						break;
					case Deltacast::Helpers::EQuadLinkType::TwoSampleInterleaved:
						MediaConfiguration.MediaConnection.QuadTransportType = EMediaIOQuadLinkTransportType::TwoSampleInterleave;
						break;
					default:
						UE_LOG(LogDeltacastMedia, Fatal, TEXT("Unhandled QuadLinkType: %d"), QuadLinkType);
						break;
					}
				}

				MediaConfiguration.MediaMode = DeltacastDeviceProvider::ToMediaMode(SdiDescriptor);

				if (SdiDescriptor.IsDual())
				{
					const auto IsDualSd = SdiDescriptor.Interface == VHD_INTERFACE::VHD_INTERFACE_SD_DUAL;
					if (!SdiDescriptor.Base.bIsInput && IsDualSupported && !IsDualSd)
						KeyResults.Add(MediaConfiguration);
				}
				else
				{
					Results.Add(MediaConfiguration);
				}
			}
		}


		auto DvDeviceScanner = DeviceScanner.value().GetDvDeviceScanner();

		if (DvDeviceScanner.has_value())
		{
			for (auto DvDescriptor : DvDeviceScanner.value())
			{
				if ((!bAllowInput && DvDescriptor.Base.bIsInput) ||
					(!bAllowOutput && !DvDescriptor.Base.bIsInput))
				{
					continue;
				}

				if (!DvDescriptor.IsValid(BoardHandle))
				{
					continue;
				}

				MediaConfiguration.bIsInput = DvDescriptor.Base.bIsInput;
				MediaConfiguration.MediaConnection.PortIdentifier = static_cast<int32>(DvDescriptor.Base.PortIndex);

				{
					MediaConfiguration.MediaConnection.TransportType = EMediaIOTransportType::HDMI;
					MediaConfiguration.MediaConnection.QuadTransportType = {};
				}

				MediaConfiguration.MediaMode = DeltacastDeviceProvider::ToMediaMode(DvDescriptor);

				Results.Add(MediaConfiguration);
			}
		}
	}

#if false
	const auto StandardToString = [](const EMediaIOStandardType Standard)
	{
		switch (Standard)
		{
		case EMediaIOStandardType::Progressive: return TEXT("p");
		case EMediaIOStandardType::Interlaced: return TEXT("i");
		case EMediaIOStandardType::ProgressiveSegmentedFrame: return TEXT("psf");
		default: return TEXT("<EMediaIOStandardType>");
		}
	};

	const auto TransportTypeToString = [](const EMediaIOTransportType Type)
	{
		switch (Type)
		{
		case EMediaIOTransportType::SingleLink: return TEXT("SingleLink");
		case EMediaIOTransportType::DualLink: return TEXT("DualLink");
		case EMediaIOTransportType::QuadLink: return TEXT("QuadLink");
		case EMediaIOTransportType::HDMI: return TEXT("HDMI");
		default: return TEXT("<EMediaIOTransportType>");
		}
	};
	const auto QuadTransportTypeToString = [](const EMediaIOQuadLinkTransportType Type)
	{
		switch (Type)
		{
		case EMediaIOQuadLinkTransportType::SquareDivision: return TEXT("SquareDivision");
		case EMediaIOQuadLinkTransportType::TwoSampleInterleave: return TEXT("TwoSampleInterleave");
		default: return TEXT("<EMediaIOQuadLinkTransportType>");
		}
	};

	for (const auto& MediaConfiguration : Results)
	{
		const auto& MediaMode = MediaConfiguration.MediaMode;
		const auto& MediaConnection = MediaConfiguration.MediaConnection;

		UE_LOG(LogDeltacastMedia, Display, TEXT("[%s: %dx%d%s%s (mode=%d), port=%d %s]"),
			MediaConfiguration.bIsInput ? TEXT("Input") : TEXT("Output"),
			MediaMode.Resolution.X,
			MediaMode.Resolution.Y,
			StandardToString(MediaMode.Standard),
			*FText::AsNumber(MediaMode.FrameRate.AsDecimal(), &FNumberFormattingOptions::DefaultNoGrouping()).ToString(),
			MediaMode.DeviceModeIdentifier,
			MediaConnection.PortIdentifier,
			TransportTypeToString(MediaConnection.TransportType));

		// UE_LOG(LogDeltacastMedia, Display, TEXT("[%s: %s(%d) %dx%d@%s %s (id=%d), %d %s %s %s]"),
		//        MediaConfiguration.bIsInput ? TEXT("Input") : TEXT("Output"),
		//        *MediaConnection.Device.DeviceName.ToString(),
		//        MediaConnection.Device.DeviceIdentifier,
		//        MediaMode.Resolution.X,
		//        MediaMode.Resolution.Y,
		//        StandardToString(MediaMode.Standard),
		//        *MediaMode.FrameRate.ToPrettyText().ToString(),
		//        MediaMode.DeviceModeIdentifier,
		//        MediaConnection.PortIdentifier,
		//        *MediaConnection.Protocol.ToString(),
		//        TransportTypeToString(MediaConnection.TransportType),
		//        QuadTransportTypeToString(MediaConnection.QuadTransportType));
	}
#endif

	return TTuple<TArray<FMediaIOConfiguration>, TArray<FMediaIOConfiguration>>(Results, KeyResults);
}

TArray<FMediaIOInputConfiguration> FDeltacastDeviceProvider::GetInputConfigurations_Impl() const
{
	TArray<FMediaIOInputConfiguration> Results;
	TArray<FMediaIOConfiguration>      InputConfigurations = GetConfigurations(true, false);

	FMediaIOInputConfiguration DefaultInputConfiguration = GetDefaultInputConfiguration();
	Results.Reset(InputConfigurations.Num() * 2);

	int32 LastDeviceIndex = INDEX_NONE;
	bool  bCanDoKeyAndFill = false;

	for (const FMediaIOConfiguration& InputConfiguration : InputConfigurations)
	{
		// Update the Device Info
		if (InputConfiguration.MediaConnection.Device.DeviceIdentifier != LastDeviceIndex)
		{
			LastDeviceIndex = InputConfiguration.MediaConnection.Device.DeviceIdentifier;
			bCanDoKeyAndFill = CanDeviceDoAlpha(InputConfiguration.MediaConnection.Device);
		}

		DefaultInputConfiguration.MediaConfiguration = InputConfiguration;

		// Build the list for fill
		DefaultInputConfiguration.InputType = EMediaIOInputType::Fill;
		Results.Add(DefaultInputConfiguration);
	}

	return Results;
}

TArray<FMediaIOOutputConfiguration> FDeltacastDeviceProvider::GetOutputConfigurations_Impl() const
{
	TArray<FMediaIOOutputConfiguration> Results;
	TArray<FMediaIOConfiguration>       OutputConfigurations    = GetConfigurations(false, true);
	TArray<FMediaIOConfiguration>       OutputKeyConfigurations = GetOutputKeyConfigurations();

	FMediaIOOutputConfiguration DefaultOutputConfiguration = GetDefaultOutputConfiguration();
	Results.Reset(OutputConfigurations.Num() + OutputKeyConfigurations.Num());

	auto BuildList = [&DefaultOutputConfiguration, &Results](const FMediaIOConfiguration& OutputConfiguration)
		{
			DefaultOutputConfiguration.MediaConfiguration = OutputConfiguration;

			DefaultOutputConfiguration.OutputReference = EMediaIOReferenceType::FreeRun;
			Results.Add(DefaultOutputConfiguration);

			if (OutputConfiguration.MediaConnection.TransportType != EMediaIOTransportType::HDMI)
			{
				DefaultOutputConfiguration.OutputReference = EMediaIOReferenceType::External;
				Results.Add(DefaultOutputConfiguration);
			}
		};

	DefaultOutputConfiguration.OutputType = EMediaIOOutputType::Fill;
	for (const FMediaIOConfiguration& OutputConfiguration : OutputConfigurations)
	{		
		BuildList(OutputConfiguration);
	}

	const auto& DeltacastSdk = FDeltacast::GetSdk();

	DefaultOutputConfiguration.OutputType = EMediaIOOutputType::FillAndKey;
	for (const FMediaIOConfiguration& OutputConfiguration : OutputKeyConfigurations)
	{
		const auto bIsQuadLink = OutputConfiguration.MediaConnection.TransportType == EMediaIOTransportType::QuadLink;
		const auto PortIdentifier = OutputConfiguration.MediaConnection.PortIdentifier;
		const auto KeyOffset = bIsQuadLink ? 4 : DeltacastSdk.GetSingleLinkKeyOffset(OutputConfiguration.MediaConnection.Device.DeviceIdentifier);
		DefaultOutputConfiguration.KeyPortIdentifier = PortIdentifier + KeyOffset;

		BuildList(OutputConfiguration);
	}

	return Results;
}

TArray<FMediaIOVideoTimecodeConfiguration> FDeltacastDeviceProvider::GetTimecodeConfigurations_Impl() const
{
	TArray<FMediaIOConfiguration> InputConfigurations = GetConfigurations(true, false);

	TArray<FMediaIOVideoTimecodeConfiguration> MediaConfigurations;
	MediaConfigurations.Reset(InputConfigurations.Num() * 2);

	FMediaIOVideoTimecodeConfiguration DefaultTimecodeConfiguration;

	for (const FMediaIOConfiguration& InputConfiguration : InputConfigurations)
	{
		DefaultTimecodeConfiguration.MediaConfiguration = InputConfiguration;
		DefaultTimecodeConfiguration.TimecodeFormat = EMediaIOAutoDetectableTimecodeFormat::LTC;
		MediaConfigurations.Add(DefaultTimecodeConfiguration);

		// Not supported
		// DefaultTimecodeConfiguration.TimecodeFormat = EMediaIOAutoDetectableTimecodeFormat::VITC;
		// MediaConfigurations.Add(DefaultTimecodeConfiguration);
	}

	return MediaConfigurations;
}

TArray<FMediaIODevice> FDeltacastDeviceProvider::GetDevices_Impl() const
{
	TArray<FMediaIODevice> Results;

	if (!FDeltacast::IsInitialized())
	{
		return Results;
	}

	const auto& DeltacastSdk = FDeltacast::GetSdk();

	VHD::ULONG NbBoards = 0;
	VHD::ULONG ApiVersion = 0;
	const auto ApiInfoResult = DeltacastSdk.GetApiInfo(&ApiVersion, &NbBoards);
	if (!Deltacast::Helpers::IsValid(ApiInfoResult))
	{
		UE_LOG(LogDeltacastMedia, Error, TEXT("Cannot get available board count"));
		return Results;
	}

	for (VHD::ULONG BoardIndex = 0; BoardIndex < NbBoards; ++BoardIndex)
	{
		const auto BoardModel = DeltacastSdk.GetBoardModel(BoardIndex);
		if (BoardModel == nullptr)
		{
			UE_LOG(LogDeltacastMedia, Warning, TEXT("Failed to get the board model for board index %u"), BoardIndex);
			continue;
		}

		FMediaIODevice Device;

		Device.DeviceIdentifier = static_cast<int32>(BoardIndex);
		Device.DeviceName = BoardModel;

		Results.Add(Device);
	}

	return Results;
}

TArray<FMediaIOMode> FDeltacastDeviceProvider::GetModes_Impl(const bool bInOutput) const
{
	TArray<FMediaIOMode> Results;

	if (!FDeltacast::IsInitialized())
	{
		return Results;
	}

	auto& DeltacastSdk = FDeltacast::GetSdk();

	const auto NbBoards = DeltacastSdk.GetNbBoards();
	if (!NbBoards)
	{
		return Results;
	}

	for (int32 BoardIndex = 0; BoardIndex < static_cast<int32>(NbBoards.value()); ++BoardIndex)
	{
		const auto BoardModel = DeltacastSdk.GetBoardModel(BoardIndex);
		if (BoardModel == nullptr)
		{
			UE_LOG(LogDeltacastMedia, Warning, TEXT("Failed to get the board model for board index %u"), BoardIndex);
			continue;
		}

		const auto BoardHandle = DeltacastSdk.OpenBoard(BoardIndex).value_or(VHD::InvalidHandle);
		if (BoardHandle == VHD::InvalidHandle)
		{
			UE_LOG(LogDeltacastMedia, Warning, TEXT("Failed to open board index %d"), BoardIndex);
			continue;
		}

		const auto NbPorts = bInOutput ? DeltacastSdk.GetRxCount(BoardHandle) : DeltacastSdk.GetTxCount(BoardHandle);

		{
			auto SdiConfig = Deltacast::Device::Config::FSdiPortConfig{};
			SdiConfig.Base.BoardIndex = BoardIndex;
			SdiConfig.Base.bIsInput = bInOutput;

			for (uint32 PortIndex = 0; PortIndex < NbPorts.value_or(0); ++PortIndex)
			{
				SdiConfig.Base.PortIndex = PortIndex;

				for (int Clock = 0; Clock < 2; ++Clock)
				{
					SdiConfig.Base.bIsEuropeanClock = Clock == 0;
					const auto& InterfaceToVideoStandards = SdiConfig.Base.bIsEuropeanClock
						? Deltacast::Helpers::SdiInterfaceToVideoStandards
						: Deltacast::Helpers::SdiInterfaceToVideoStandardsUsClock;

					for (int i = 0; i < InterfaceToVideoStandards.size(); ++i)
					{
						const auto& VideoStandards = InterfaceToVideoStandards[i];

						SdiConfig.Interface = Deltacast::Helpers::SdiInterfaces[i];
						for (const auto VideoStandard : VideoStandards)
						{
							SdiConfig.VideoStandard = VideoStandard;

							if (!SdiConfig.IsValid(BoardHandle))
							{
								continue;
							}

							const auto MediaMode = DeltacastDeviceProvider::ToMediaMode(SdiConfig);

							Results.Add(MediaMode);
						}
					}
				}
			}
		}

		{
			auto DvConfig = Deltacast::Device::Config::FDvPortConfig{};
			DvConfig.Base.BoardIndex = BoardIndex;
			DvConfig.Base.bIsInput = bInOutput;

			for (uint32 PortIndex = 0; PortIndex < NbPorts.value_or(0); ++PortIndex)
			{
				DvConfig.Base.PortIndex = PortIndex;

				for (int Clock = 0; Clock < 2; ++Clock)
				{
					DvConfig.Base.bIsEuropeanClock = Clock == 0;
					const auto VideoStandards = DvConfig.Base.bIsEuropeanClock
						? Deltacast::Helpers::DvVideoStandardsVector
						: Deltacast::Helpers::DvVideoStandardsUsClockVector;

					for (const auto VideoStandard : VideoStandards)
					{
						DvConfig.VideoStandard = VideoStandard;

						if (!DvConfig.IsValid(BoardHandle))
						{
							continue;
						}

						const auto MediaMode = DeltacastDeviceProvider::ToMediaMode(DvConfig);

						Results.Add(MediaMode);
					}
				}
			}
		}

		[[maybe_unused]] const auto CloseBoardResult = DeltacastSdk.CloseBoardHandle(BoardHandle);
	}

	return Results;
}


void FDeltacastDeviceProvider::UpdateCache() const
{
	FScopeLock Guard(&CacheCriticalSection);

	const auto HardwareIdentifier = ComputeCurrentHardwareIdentifier();

	if (CacheHardwareIdentifier != HardwareIdentifier)
	{
		CacheHardwareIdentifier = HardwareIdentifier;

		ConfigurationsInputCache    = GetConfigurations_Impl(true, false).Key;
		const auto OutputConfigurations = GetConfigurations_Impl(false, true);
		ConfigurationsOutputCache   = OutputConfigurations.Key;
		ConfigurationsKeyOutputCache = OutputConfigurations.Value;
		InputConfigurationsCache    = GetInputConfigurations_Impl();
		OutputConfigurationsCache   = GetOutputConfigurations_Impl();
		TimecodeConfigurationsCache = GetTimecodeConfigurations_Impl();
		DevicesCache                = GetDevices_Impl();
		ModesInputCache             = GetModes_Impl(true);
		ModesOutputCache            = GetModes_Impl(false);
	}
}

TArray<uint64> FDeltacastDeviceProvider::ComputeCurrentHardwareIdentifier()
{
	auto& DeltacastSdk = FDeltacast::GetSdk();

	const auto NbBoards = DeltacastSdk.GetNbBoards().value_or(0);

	TArray<uint64> Identifier;
	Identifier.Reserve(NbBoards);

	for (VHD::ULONG BoardIndex = 0; BoardIndex < NbBoards; ++BoardIndex)
	{
		const auto BoardHandle = DeltacastSdk.OpenBoard(BoardIndex).value_or(VHD::InvalidHandle);
		if (BoardHandle == VHD::InvalidHandle)
		{
			continue;
		}

		Deltacast::Helpers::TScopeExit CloseBoardHandle([BoardHandle, &DeltacastSdk]()
		{
			DeltacastSdk.CloseBoardHandle(BoardHandle);
		});

		VHD::ULONG SerialLower = 0;
		VHD::ULONG SerialUpper = 0;

		const auto SerialLowerResult = DeltacastSdk.GetBoardProperty(BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_SERIALNUMBER_LSW, &SerialLower);
		const auto SerialUpperResult = DeltacastSdk.GetBoardProperty(BoardHandle, VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_SERIALNUMBER_MSW, &SerialUpper);

		if (!Deltacast::Helpers::IsValid(SerialLowerResult) ||
			!Deltacast::Helpers::IsValid(SerialUpperResult))
		{
			UE_LOG(LogDeltacastMedia, Error, TEXT("Failed to get serial number of board %u: %s/%s"), BoardIndex,
			       *Deltacast::Helpers::GetErrorString(SerialLowerResult), *Deltacast::Helpers::GetErrorString(SerialUpperResult));
			continue;
		}

		const uint64 Serial = SerialLower + (uint64{ SerialUpper } << 32);

		Identifier.Add(Serial);
	}

	return Identifier;
}

#undef LOCTEXT_NAMESPACE
