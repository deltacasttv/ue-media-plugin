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

#include "DeltacastMediaOutput.h"

#include "DeltacastDeviceScanner.h"
#include "DeltacastHelpers.h"
#include "DeltacastMediaCapture.h"
#include "DeltacastMediaSettings.h"
#include "DeltacastSdk.h"
#include "IDeltacastMediaModule.h"
#include "IDeltacastMediaOutputModule.h"
#include "MediaCapture.h"
#include "Modules/ModuleManager.h"


#define LOCTEXT_NAMESPACE "DeltacastMediaOutput"


bool IsPixelFormatSupportedSdi(const EDeltacastMediaOutputPixelFormat PixelFormat)
{
	return PixelFormat != EDeltacastMediaOutputPixelFormat::PF_8BIT_RGBA;
}


bool UDeltacastMediaOutput::Validate(FString& OutFailureReason) const
{
	if (!Super::Validate(OutFailureReason))
	{
		return false;
	}

	if (!OutputConfiguration.IsValid())
	{
		OutFailureReason = FString::Printf(TEXT("The Configuration of '%s' is invalid ."), *GetName());
		return false;
	}

	const auto &MediaModule = FModuleManager::LoadModuleChecked<IDeltacastMediaModule>(TEXT("DeltacastMedia"));
	if (!MediaModule.IsInitialized())
	{
		OutFailureReason = FString::Printf(TEXT("Can't validate MediaOutput '%s'. The Deltacast library was not initialized."), *GetName());
		return false;
	}

	if (!MediaModule.CanBeUsed())
	{
		OutFailureReason = FString::Printf(TEXT("Can't validate MediaOutput '%s' because Deltacast board cannot be used."), *GetName());
		return false;
	}

	auto& DeltacastSdk = FDeltacast::GetSdk();

	const auto BoardIndex = static_cast<VHD::ULONG>(OutputConfiguration.MediaConfiguration.MediaConnection.Device.DeviceIdentifier);

	const auto IsBoardIndexValid = DeltacastSdk.IsBoardIndexValid(BoardIndex);
	if (!IsBoardIndexValid.value_or(false))
	{
		OutFailureReason = FString::Printf(TEXT("The MediaOutput '%s' use the board index '%d' that doesn't exist on this machine."), *GetName(), OutputConfiguration.MediaConfiguration.MediaConnection.Device.DeviceIdentifier);
		return false;
	}

	const auto DeviceModeIdentifier = OutputConfiguration.MediaConfiguration.MediaMode.DeviceModeIdentifier;

	const auto bIsSdi = Deltacast::Helpers::IsDeviceModeIdentifierSdi(DeviceModeIdentifier);
	const auto bIsDv = Deltacast::Helpers::IsDeviceModeIdentifierDv(DeviceModeIdentifier);

	Deltacast::Device::Config::FBasePortConfig Base{};

	Base.bIsInput         = false;
	Base.BoardIndex       = OutputConfiguration.MediaConfiguration.MediaConnection.Device.DeviceIdentifier;
	Base.PortIndex        = OutputConfiguration.MediaConfiguration.MediaConnection.PortIdentifier;
	Base.bIsEuropeanClock = Deltacast::Helpers::IsDeviceModeIdentifierEuropeanClock(DeviceModeIdentifier);

	if (bIsSdi)
	{
		const auto VideoStandard = Deltacast::Helpers::GetSdiVideoStandardFromDeviceModeIdentifier(DeviceModeIdentifier);
		const auto IsSingleLink = OutputConfiguration.MediaConfiguration.MediaConnection.TransportType == EMediaIOTransportType::SingleLink;
		const auto QuadLinkType = [](const EMediaIOQuadLinkTransportType QuadTransportType)
		{
			switch (QuadTransportType)
			{
			case EMediaIOQuadLinkTransportType::SquareDivision:
				return Deltacast::Helpers::EQuadLinkType::Quadrant;
			case EMediaIOQuadLinkTransportType::TwoSampleInterleave:
				return Deltacast::Helpers::EQuadLinkType::TwoSampleInterleaved;
			default:
				UE_LOG(LogDeltacastMediaOutput, Fatal, TEXT("Unhandled `EMediaIOQuadLinkTransportType` value: %d"), QuadTransportType);
				return Deltacast::Helpers::EQuadLinkType{};
			}
		}(OutputConfiguration.MediaConfiguration.MediaConnection.QuadTransportType);

		auto PortConfig = Deltacast::Device::Config::FSdiPortConfig{};

		PortConfig.Base = Base;

		PortConfig.VideoStandard = VideoStandard;
		PortConfig.Interface = IsSingleLink
			? Deltacast::Helpers::GetSingleLinkInterface(VideoStandard)
			: Deltacast::Helpers::GetQuadLinkInterface(VideoStandard, QuadLinkType);

		const auto BoardHandle = DeltacastSdk.OpenBoard(PortConfig.Base.BoardIndex).value_or(VHD::InvalidHandle);
		if (!BoardHandle)
		{
			OutFailureReason = FString::Printf(TEXT("Failed to validate Deltacast media source because board handle cannot be obtained"));
			return false;
		}

		const auto bIsValid = PortConfig.IsValid(BoardHandle);

		DeltacastSdk.CloseBoardHandle(BoardHandle);

		if (!bIsValid)
		{
			OutFailureReason = FString::Printf(TEXT("Invalid port configuration: %s"), *PortConfig.ToString());
		}

		return bIsValid;
	}

	if (bIsDv)
	{
		auto PortConfig = Deltacast::Device::Config::FDvPortConfig{};

		PortConfig.Base = Base;

		PortConfig.VideoStandard = Deltacast::Helpers::GetDvVideoStandardFromDeviceModeIdentifier(OutputConfiguration.MediaConfiguration.MediaMode.DeviceModeIdentifier);

		const auto BoardHandle = DeltacastSdk.OpenBoard(PortConfig.Base.BoardIndex).value_or(VHD::InvalidHandle);
		if (!BoardHandle)
		{
			OutFailureReason = FString::Printf(TEXT("Failed to validate Deltacast media source because board handle cannot be obtained"));
			return false;
		}

		const auto bIsValid = PortConfig.IsValid(BoardHandle);

		DeltacastSdk.CloseBoardHandle(BoardHandle);

		if (!bIsValid)
		{
			OutFailureReason = FString::Printf(TEXT("Invalid port configuration: %s"), *PortConfig.ToString());
		}

		return bIsValid;
	}

	return false;
}

FIntPoint UDeltacastMediaOutput::GetRequestedSize() const
{
	return OutputConfiguration.MediaConfiguration.MediaMode.Resolution;
}

EPixelFormat UDeltacastMediaOutput::GetRequestedPixelFormat() const
{
	EPixelFormat Result = EPixelFormat::PF_B8G8R8A8;
	switch (PixelFormat)
	{
		case EDeltacastMediaOutputPixelFormat::PF_8BIT_RGBA:
			Result = EPixelFormat::PF_B8G8R8A8;
			break;
		case EDeltacastMediaOutputPixelFormat::PF_8BIT_YUV422:
			Result = EPixelFormat::PF_B8G8R8A8;
			break;
		case EDeltacastMediaOutputPixelFormat::PF_10BIT_YUV422:
			Result = EPixelFormat::PF_A2B10G10R10;
			break;
		default:
			break;
	}
	return Result;
}

EMediaCaptureConversionOperation UDeltacastMediaOutput::GetConversionOperation(EMediaCaptureSourceType InSourceType) const
{
	EMediaCaptureConversionOperation Result = EMediaCaptureConversionOperation::NONE;

	switch (PixelFormat)
	{
		case EDeltacastMediaOutputPixelFormat::PF_8BIT_RGBA:
			Result = EMediaCaptureConversionOperation::NONE;
			break;
		case EDeltacastMediaOutputPixelFormat::PF_8BIT_YUV422:
			Result = EMediaCaptureConversionOperation::RGBA8_TO_YUV_8BIT;
			break;
		case EDeltacastMediaOutputPixelFormat::PF_10BIT_YUV422:
			Result = EMediaCaptureConversionOperation::RGB10_TO_YUVv210_10BIT;
			break;
		default:
			break;
	}

	return Result;
}


UMediaCapture * UDeltacastMediaOutput::CreateMediaCaptureImpl()
{
	UMediaCapture* Result = NewObject<UDeltacastMediaCapture>();
	if (Result)
	{
		Result->SetMediaOutput(this);
	}

	return Result;
}


#if WITH_EDITOR
bool UDeltacastMediaOutput::CanEditChange(const FProperty* InProperty) const
{
	if (!Super::CanEditChange(InProperty))
	{
		return false;
	}

	return true;
}

void UDeltacastMediaOutput::PostEditChangeChainProperty(FPropertyChangedChainEvent &PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UDeltacastMediaOutput, PixelFormat) ||
		 PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UDeltacastMediaOutput, OutputConfiguration))
	{
		const auto DeviceIndex = OutputConfiguration.MediaConfiguration.MediaConnection.Device.DeviceIdentifier;
		const auto PortIndex   = OutputConfiguration.MediaConfiguration.MediaConnection.PortIdentifier;

		auto &DeltacastSdk = FDeltacast::GetSdk();

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

	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}
#endif

#undef LOCTEX_NAMESPACE
