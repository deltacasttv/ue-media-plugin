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

#include "DeltacastMediaSource.h"

#include "IDeltacastMediaSourceModule.h"
#include "DeltacastDeviceScanner.h"
#include "DeltacastMediaOption.h"
#include "DeltacastSdk.h"
#include "MediaIOCorePlayerBase.h"



bool IsPixelFormatSupportedSdi(const EDeltacastMediaSourcePixelFormat PixelFormat)
{
	return PixelFormat != EDeltacastMediaSourcePixelFormat::PF_8BIT_RGBA;
}


UDeltacastMediaSource::UDeltacastMediaSource()
{
	MediaConfiguration.bIsInput = true;

	// `SupportsFormatAutoDetection()` == false => no choice but display uses `bAutoDetectInput`
	// `SupportsFormatAutoDetection()` == true  => choice given using `bAutoDetectInput`
	// `SupportsFormatAutoDetection()` default is false
	// `bAutoDetectInput` default is true
	bAutoDetectInput = false;
}


bool UDeltacastMediaSource::GetMediaOption(const FName &Key, const bool DefaultValue) const
{
	if (Key == DeltacastMediaOption::AutoLoadEdid)
	{
		return bAutoLoadEdid;
	}
	if (Key == DeltacastMediaOption::EncodeTimecodeInTexel)
	{
		return bEncodeTimecodeInTexel;
	}
	if (Key == DeltacastMediaOption::IsEuropeanClock)
	{
		return Deltacast::Helpers::IsDeviceModeIdentifierEuropeanClock(MediaConfiguration.MediaMode.DeviceModeIdentifier);
	}
	if (Key == DeltacastMediaOption::IsSdi)
	{
		return Deltacast::Helpers::IsDeviceModeIdentifierSdi(MediaConfiguration.MediaMode.DeviceModeIdentifier);
	}
	if (Key == DeltacastMediaOption::IsDv)
	{
		return Deltacast::Helpers::IsDeviceModeIdentifierDv(MediaConfiguration.MediaMode.DeviceModeIdentifier);
	}
	if (Key == DeltacastMediaOption::IsSRGBInput)
	{
		return bIsSRGBInput;
	}
	if (Key == DeltacastMediaOption::ErrorOnSourceLost)
	{
		return bErrorOnSourceLost;
	}
	if (Key == DeltacastMediaOption::LogDroppedFrameCount)
	{
		return bLogDroppedFrameCount;
	}

	return Super::GetMediaOption(Key, DefaultValue);
}

int64 UDeltacastMediaSource::GetMediaOption(const FName &Key, const int64 DefaultValue) const
{
	if (Key == DeltacastMediaOption::BoardIndex)
	{
		return MediaConfiguration.MediaConnection.Device.DeviceIdentifier;
	}
	if (Key == DeltacastMediaOption::LinkType)
	{
		return static_cast<int64>(MediaConfiguration.MediaConnection.TransportType);
	}
	if (Key == DeltacastMediaOption::NumberOfDeltacastBuffers)
	{
		return NumberOfDeltacastBuffers;
	}
	if (Key == DeltacastMediaOption::NumberOfEngineBuffers)
	{
		return NumberOfEngineBuffers;
	}
	if (Key == DeltacastMediaOption::PixelFormat)
	{
		return static_cast<int64>(PixelFormat);
	}
	if (Key == DeltacastMediaOption::PortIndex)
	{
		return MediaConfiguration.MediaConnection.PortIdentifier;
	}
	if (Key == DeltacastMediaOption::QuadLinkType)
	{
		return static_cast<int64>(MediaConfiguration.MediaConnection.QuadTransportType);
	}
	if (Key == DeltacastMediaOption::TimecodeFormat)
	{
		return static_cast<int64>(TimecodeFormat);
	}
	if (Key == DeltacastMediaOption::SdiVideoStandard)
	{
		return static_cast<int64>(Deltacast::Helpers::GetSdiVideoStandardFromDeviceModeIdentifier(MediaConfiguration.MediaMode.DeviceModeIdentifier));
	}
	if (Key == DeltacastMediaOption::DvVideoStandard)
	{
		return static_cast<int64>(Deltacast::Helpers::GetDvVideoStandardFromDeviceModeIdentifier(MediaConfiguration.MediaMode.DeviceModeIdentifier));
	}

	return Super::GetMediaOption(Key, DefaultValue);
}

FString UDeltacastMediaSource::GetMediaOption(const FName &Key, const FString &DefaultValue) const
{
	if (Key == FMediaIOCoreMediaOption::VideoModeName)
	{
		return MediaConfiguration.MediaMode.GetModeName().ToString();
	}

	return Super::GetMediaOption(Key, DefaultValue);
}

bool UDeltacastMediaSource::HasMediaOption(const FName &Key) const
{
	if (Key == DeltacastMediaOption::AutoLoadEdid ||
		Key == DeltacastMediaOption::BoardIndex ||
	    Key == DeltacastMediaOption::EncodeTimecodeInTexel ||
	    Key == DeltacastMediaOption::IsEuropeanClock ||
	    Key == DeltacastMediaOption::IsSdi ||
	    Key == DeltacastMediaOption::IsDv ||
	    Key == DeltacastMediaOption::IsSRGBInput ||
	    Key == DeltacastMediaOption::LinkType ||
	    Key == DeltacastMediaOption::NumberOfDeltacastBuffers ||
	    Key == DeltacastMediaOption::PixelFormat ||
	    Key == DeltacastMediaOption::PortIndex ||
	    Key == DeltacastMediaOption::QuadLinkType ||
	    Key == DeltacastMediaOption::TimecodeFormat ||
	    Key == DeltacastMediaOption::LogDroppedFrameCount ||
		Key == DeltacastMediaOption::SdiVideoStandard ||
		Key == DeltacastMediaOption::DvVideoStandard)
	{
		return true;
	}

	return Super::HasMediaOption(Key);
}


FString UDeltacastMediaSource::GetUrl() const
{
	return MediaConfiguration.MediaConnection.ToUrl();
}

bool UDeltacastMediaSource::Validate() const
{
	FString FailureReason;

	if (!MediaConfiguration.IsValid())
	{
		UE_LOG(LogDeltacastMediaSource, Warning, TEXT("The MediaConfiguration '%s' is invalid."), *GetName());
		return false;
	}

	const auto bIsSdi = Deltacast::Helpers::IsDeviceModeIdentifierSdi(MediaConfiguration.MediaMode.DeviceModeIdentifier);
	const auto bIsDv  = Deltacast::Helpers::IsDeviceModeIdentifierDv(MediaConfiguration.MediaMode.DeviceModeIdentifier);

	if (bIsSdi)
	{
		const auto VideoStandard = Deltacast::Helpers::GetSdiVideoStandardFromDeviceModeIdentifier(MediaConfiguration.MediaMode.DeviceModeIdentifier);
		const auto IsSingleLink  = MediaConfiguration.MediaConnection.TransportType == EMediaIOTransportType::SingleLink;
		const auto QuadLinkType  = [](const EMediaIOQuadLinkTransportType QuadTransportType)
		{
			switch (QuadTransportType)
			{
				case EMediaIOQuadLinkTransportType::SquareDivision:
					return Deltacast::Helpers::EQuadLinkType::Quadrant;
				case EMediaIOQuadLinkTransportType::TwoSampleInterleave:
					return Deltacast::Helpers::EQuadLinkType::TwoSampleInterleaved;
				default:
					UE_LOG(LogDeltacastMediaSource, Fatal, TEXT("Unhandled `EMediaIOQuadLinkTransportType` value: %d"), QuadTransportType);
					return Deltacast::Helpers::EQuadLinkType{};
			}
		}(MediaConfiguration.MediaConnection.QuadTransportType);

		auto PortConfig = Deltacast::Device::Config::FSdiPortConfig{};

		PortConfig.Base.bIsInput         = MediaConfiguration.bIsInput;
		PortConfig.Base.BoardIndex       = MediaConfiguration.MediaConnection.Device.DeviceIdentifier;
		PortConfig.Base.PortIndex        = MediaConfiguration.MediaConnection.PortIdentifier;
		PortConfig.Base.bIsEuropeanClock = Deltacast::Helpers::IsDeviceModeIdentifierEuropeanClock(MediaConfiguration.MediaMode.DeviceModeIdentifier);

		PortConfig.VideoStandard = VideoStandard;
		PortConfig.Interface     = IsSingleLink
			                           ? Deltacast::Helpers::GetSingleLinkInterface(VideoStandard)
			                           : Deltacast::Helpers::GetQuadLinkInterface(VideoStandard, QuadLinkType);

		auto &DeltacastSdk = FDeltacast::GetSdk();

		const auto BoardHandle = DeltacastSdk.OpenBoard(PortConfig.Base.BoardIndex).value_or(VHD::InvalidHandle);
		if (!BoardHandle)
		{
			UE_LOG(LogDeltacastMediaSource, Error, TEXT("Failed to validate Deltacast media source because board handle cannot be obtained"));
			return false;
		}

		const auto bIsValid = PortConfig.IsValid(BoardHandle);

		DeltacastSdk.CloseBoardHandle(BoardHandle);

		return bIsValid;
	}

	if (bIsDv)
	{
		auto PortConfig = Deltacast::Device::Config::FDvPortConfig{};

		PortConfig.Base.bIsInput         = MediaConfiguration.bIsInput;
		PortConfig.Base.BoardIndex       = MediaConfiguration.MediaConnection.Device.DeviceIdentifier;
		PortConfig.Base.PortIndex        = MediaConfiguration.MediaConnection.PortIdentifier;
		PortConfig.Base.bIsEuropeanClock = Deltacast::Helpers::IsDeviceModeIdentifierEuropeanClock(MediaConfiguration.MediaMode.DeviceModeIdentifier);

		PortConfig.VideoStandard = Deltacast::Helpers::GetDvVideoStandardFromDeviceModeIdentifier(MediaConfiguration.MediaMode.DeviceModeIdentifier);

		auto& DeltacastSdk = FDeltacast::GetSdk();

		const auto BoardHandle = DeltacastSdk.OpenBoard(PortConfig.Base.BoardIndex).value_or(VHD::InvalidHandle);
		if (!BoardHandle)
		{
			UE_LOG(LogDeltacastMediaSource, Error, TEXT("Failed to validate Deltacast media source because board handle cannot be obtained"));
			return false;
		}

		const auto bIsValid = PortConfig.IsValid(BoardHandle);

		DeltacastSdk.CloseBoardHandle(BoardHandle);

		return bIsValid;
	}

	return false;
}

bool UDeltacastMediaSource::SupportsFormatAutoDetection() const
{
	return false;
}


#if WITH_EDITOR
bool UDeltacastMediaSource::CanEditChange(const FProperty* InProperty) const
{
	if (!Super::CanEditChange(InProperty))
	{
		return false;
	}

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UDeltacastMediaSource, bAutoLoadEdid))
	{
		return MediaConfiguration.MediaConnection.TransportType == EMediaIOTransportType::HDMI;
	}

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UTimeSynchronizableMediaSource, bUseTimeSynchronization))
	{
		return TimecodeFormat != EMediaIOTimecodeFormat::None;
	}

	return true;
}

void UDeltacastMediaSource::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UDeltacastMediaSource, PixelFormat) ||
		 PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UDeltacastMediaSource, MediaConfiguration))
	{
		const auto DeviceIndex = MediaConfiguration.MediaConnection.Device.DeviceIdentifier;
		const auto PortIndex   = MediaConfiguration.MediaConnection.PortIdentifier;

		auto& DeltacastSdk = FDeltacast::GetSdk();

		const auto BoardHandle = DeltacastSdk.OpenBoard(DeviceIndex);

		if (BoardHandle.has_value())
		{
			const auto ChannelType = DeltacastSdk.GetChannelType(BoardHandle.value(), false, PortIndex);

			const auto IsSdi = Deltacast::Helpers::IsSdi(ChannelType.value_or(VHD_CHANNELTYPE::NB_VHD_CHANNELTYPE));

			if (IsSdi && !IsPixelFormatSupportedSdi(PixelFormat))
			{
				PixelFormat = DefaultPixelFormatSdi;
			}

			DeltacastSdk.CloseBoardHandle(BoardHandle.value());
		}
	}

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UDeltacastMediaSource, TimecodeFormat))
	{
		if (TimecodeFormat == EMediaIOTimecodeFormat::None)
		{
			bUseTimeSynchronization = false;
		}

		if (TimecodeFormat == EMediaIOTimecodeFormat::VITC)
		{
			TimecodeFormat = EMediaIOTimecodeFormat::LTC;
		}
	}

	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif
