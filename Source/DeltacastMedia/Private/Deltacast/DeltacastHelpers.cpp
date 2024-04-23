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

#include "DeltacastHelpers.h"

#include "DeltacastSdk.h"
#include "IDeltacastMediaModule.h"
#include "UObject/Class.h"
#include "UObject/ReflectedTypeAccessors.h"

#include <algorithm>
#include <type_traits>


namespace Deltacast::Helpers
{
	FCablePacking::FCablePacking(const VHD_BUFFERPACKING BufferPacking, const bool IsSd)
	{
		switch (BufferPacking)
		{
			case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_8: [[fallthrough]];
			case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_10:
				Sampling = VHD_DV_SAMPLING::VHD_DV_SAMPLING_4_2_2_12BITS;
				ColorSpace = IsSd ? VHD_DV_CS::VHD_DV_CS_YUV601 : VHD_DV_CS::VHD_DV_CS_YUV709;
				break;
			case VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_RGB_32:
				Sampling = VHD_DV_SAMPLING::VHD_DV_SAMPLING_4_4_4_8BITS;
				ColorSpace = VHD_DV_CS::VHD_DV_CS_RGB_FULL;
				break;
			default:
				UE_LOG(LogDeltacastMedia, Fatal, TEXT("Unsupported buffer packing: %u"), BufferPacking);
				Sampling   = VHD_DV_SAMPLING::NB_VHD_DV_SAMPLING;
				ColorSpace = VHD_DV_CS::NB_VHD_DV_CS;
				break;
		}
	}


	template <typename Enum>
	FString GetEnumString(const Enum EnumValue)
	{
		static const UEnum* UEnum = StaticEnum<Enum>();
		auto                Value = UEnum->GetNameStringByValue(static_cast<int64>(EnumValue));
		return Value.IsEmpty() ? FString::FromInt(static_cast<int32>(EnumValue)) : Value;
	}

	FString GetErrorString(const VHD_ERRORCODE ErrorCode)
	{
		return GetEnumString(ErrorCode);
	}

	FString GetVideoStandardString(const VHD_VIDEOSTANDARD VideoStandard)
	{
		return GetEnumString(VideoStandard);
	}

	FString GetVideoStandardString(const VHD_DV_HDMI_VIDEOSTANDARD VideoStandard)
	{
		return GetEnumString(VideoStandard);
	}

	FString GetInterfaceString(const VHD_INTERFACE Interface)
	{
		return GetEnumString(Interface);
	}

	FString GetGenlockSourceString(const VHD_GENLOCKSOURCE GenlockSource)
	{
		return GetEnumString(GenlockSource);
	}



	bool IsValid(const VHD_ERRORCODE ErrorCode)
	{
		return ErrorCode == VHD_ERRORCODE::VHDERR_NOERROR;
	}


	FString VersionToText(const VHD::ULONG Version)
	{
		const auto VersionSeparator = TEXT(".");
		FString VersionString = TEXT("");

		VersionString.AppendInt(Version >> 24);
		VersionString.Append(VersionSeparator);
		VersionString.AppendInt((Version >> 16) & 0xFF);
		VersionString.Append(VersionSeparator);
		VersionString.AppendInt(Version & 0xFFFF);

		return VersionString;
	}


	VHD::ULONG GetPortIndex(const VHD_STREAMTYPE StreamType)
	{
		switch (StreamType)
		{
			case VHD_STREAMTYPE::VHD_ST_RX0: return 0;
			case VHD_STREAMTYPE::VHD_ST_RX1: return 1;
			case VHD_STREAMTYPE::VHD_ST_RX2: return 2;
			case VHD_STREAMTYPE::VHD_ST_RX3: return 3;
			case VHD_STREAMTYPE::VHD_ST_RX4: return 4;
			case VHD_STREAMTYPE::VHD_ST_RX5: return 5;
			case VHD_STREAMTYPE::VHD_ST_RX6: return 6;
			case VHD_STREAMTYPE::VHD_ST_RX7: return 7;
			case VHD_STREAMTYPE::VHD_ST_RX8: return 8;
			case VHD_STREAMTYPE::VHD_ST_RX9: return 9;
			case VHD_STREAMTYPE::VHD_ST_RX10: return 10;
			case VHD_STREAMTYPE::VHD_ST_RX11: return 11;
			case VHD_STREAMTYPE::VHD_ST_TX0: return 0;
			case VHD_STREAMTYPE::VHD_ST_TX1: return 1;
			case VHD_STREAMTYPE::VHD_ST_TX2: return 2;
			case VHD_STREAMTYPE::VHD_ST_TX3: return 3;
			case VHD_STREAMTYPE::VHD_ST_TX4: return 4;
			case VHD_STREAMTYPE::VHD_ST_TX5: return 5;
			case VHD_STREAMTYPE::VHD_ST_TX6: return 6;
			case VHD_STREAMTYPE::VHD_ST_TX7: return 7;
			case VHD_STREAMTYPE::VHD_ST_TX8: return 8;
			case VHD_STREAMTYPE::VHD_ST_TX9: return 9;
			case VHD_STREAMTYPE::VHD_ST_TX10: return 10;
			case VHD_STREAMTYPE::VHD_ST_TX11: return 11;
			case VHD_STREAMTYPE::NB_VHD_STREAMTYPES: [[fallthrough]];
			default:
				return static_cast<VHD::ULONG>(VHD_STREAMTYPE::NB_VHD_STREAMTYPES);
		}
	}

	VHD_CORE_BOARDPROPERTY GetFWLoopbackFromPortIndex(const int32 PortIndex)
	{
		switch (PortIndex)
		{
		case 0:
			return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_FIRMWARE_LOOPBACK_0;
		case 1:
			return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_FIRMWARE_LOOPBACK_1;
		default:
			return VHD_CORE_BOARDPROPERTY::NB_VHD_CORE_BOARDPROPERTIES;
		}
	}

	VHD_CORE_BOARDPROPERTY GetByPassFromPortIndex(const int32 PortIndex)
	{
		switch (PortIndex)
		{
			case 0:
				return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_BYPASS_RELAY_0;
			case 1:
				return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_BYPASS_RELAY_1;
			case 2:
				return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_BYPASS_RELAY_2;
			case 3:
				return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_BYPASS_RELAY_3;
			default:
				return VHD_CORE_BOARDPROPERTY::NB_VHD_CORE_BOARDPROPERTIES;
		}
	}

	VHD_CORE_BOARDPROPERTY GetChannelStatusFromPortIndex(const bool bIsInput, const int32 PortIndex)
	{
		if (bIsInput)
		{
			switch (PortIndex)
			{
				case 0:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX0_STATUS;
				case 1:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX1_STATUS;
				case 2:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX2_STATUS;
				case 3:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX3_STATUS;
				case 4:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX4_STATUS;
				case 5:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX5_STATUS;
				case 6:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX6_STATUS;
				case 7:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX7_STATUS;
				case 8:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX8_STATUS;
				case 9:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX9_STATUS;
				case 10:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX10_STATUS;
				case 11:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX11_STATUS;
				default:
					return VHD_CORE_BOARDPROPERTY::NB_VHD_CORE_BOARDPROPERTIES;
			}
		}
		else
		{
			switch (PortIndex)
			{
				case 0:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX0_STATUS;
				case 1:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX1_STATUS;
				case 2:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX2_STATUS;
				case 3:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX3_STATUS;
				case 4:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX4_STATUS;
				case 5:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX5_STATUS;
				case 6:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX6_STATUS;
				case 7:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX7_STATUS;
				case 8:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX8_STATUS;
				case 9:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX9_STATUS;
				case 10:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX10_STATUS;
				case 11:
					return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX11_STATUS;
				default:
					return VHD_CORE_BOARDPROPERTY::NB_VHD_CORE_BOARDPROPERTIES;
			}
		}
	}

	VHD_CORE_BOARDPROPERTY GetChannelTypeProperty(const bool bIsInput, const int32 PortIndex)
	{
		if (bIsInput)
		{
			switch (PortIndex)
			{
				case 0: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX0_TYPE;
				case 1: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX1_TYPE;
				case 2: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX2_TYPE;
				case 3: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX3_TYPE;
				case 4: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX4_TYPE;
				case 5: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX5_TYPE;
				case 6: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX6_TYPE;
				case 7: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX7_TYPE;
				case 8: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX8_TYPE;
				case 9: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX9_TYPE;
				case 10: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX10_TYPE;
				case 11: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX11_TYPE;
				default: return VHD_CORE_BOARDPROPERTY::NB_VHD_CORE_BOARDPROPERTIES;
			}
		}
		else
		{
			switch (PortIndex)
			{
				case 0: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX0_TYPE;
				case 1: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX1_TYPE;
				case 2: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX2_TYPE;
				case 3: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX3_TYPE;
				case 4: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX4_TYPE;
				case 5: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX5_TYPE;
				case 6: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX6_TYPE;
				case 7: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX7_TYPE;
				case 8: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX8_TYPE;
				case 9: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX9_TYPE;
				case 10: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX10_TYPE;
				case 11: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_TX11_TYPE;
				default: return VHD_CORE_BOARDPROPERTY::NB_VHD_CORE_BOARDPROPERTIES;
			}
		}
	}

	VHD_CORE_BOARDPROPERTY GetChannelMode(const int32 PortIndex)
	{
		switch (PortIndex)
		{
			case 0: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX0_MODE;
			case 1: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX1_MODE;
			case 2: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX2_MODE;
			case 3: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX3_MODE;
			case 4: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX4_MODE;
			case 5: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX5_MODE;
			case 6: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX6_MODE;
			case 7: return VHD_CORE_BOARDPROPERTY::VHD_CORE_BP_RX7_MODE;
			default: return VHD_CORE_BOARDPROPERTY::NB_VHD_CORE_BOARDPROPERTIES;
		}
	}

	VHD_SDI_BOARDPROPERTY GetRxVideoStandard(const int32 PortIndex)
	{
		switch (PortIndex)
		{
			case 0: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX0_STANDARD;
			case 1: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX1_STANDARD;
			case 2: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX2_STANDARD;
			case 3: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX3_STANDARD;
			case 4: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX4_STANDARD;
			case 5: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX5_STANDARD;
			case 6: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX6_STANDARD;
			case 7: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX7_STANDARD;
			default: return VHD_SDI_BOARDPROPERTY::NB_VHD_SDI_BOARDPROPERTIES;
		}
	}

	VHD_SDI_BOARDPROPERTY GetRxClockDivisor(const int32 PortIndex)
	{
		switch (PortIndex)
		{
			case 0: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX0_CLOCK_DIV;
			case 1: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX1_CLOCK_DIV;
			case 2: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX2_CLOCK_DIV;
			case 3: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX3_CLOCK_DIV;
			case 4: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX4_CLOCK_DIV;
			case 5: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX5_CLOCK_DIV;
			case 6: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX6_CLOCK_DIV;
			case 7: return VHD_SDI_BOARDPROPERTY::VHD_SDI_BP_RX7_CLOCK_DIV;
			default: return VHD_SDI_BOARDPROPERTY::NB_VHD_SDI_BOARDPROPERTIES;
		}
	}

	VHD_STREAMTYPE GetStreamTypeFromPortIndex(const bool bIsInput, const int32 PortIndex)
	{
		if (bIsInput)
		{
			switch (PortIndex)
			{
				case 0: return VHD_STREAMTYPE::VHD_ST_RX0;
				case 1: return VHD_STREAMTYPE::VHD_ST_RX1;
				case 2: return VHD_STREAMTYPE::VHD_ST_RX2;
				case 3: return VHD_STREAMTYPE::VHD_ST_RX3;
				case 4: return VHD_STREAMTYPE::VHD_ST_RX4;
				case 5: return VHD_STREAMTYPE::VHD_ST_RX5;
				case 6: return VHD_STREAMTYPE::VHD_ST_RX6;
				case 7: return VHD_STREAMTYPE::VHD_ST_RX7;
				case 8: return VHD_STREAMTYPE::VHD_ST_RX8;
				case 9: return VHD_STREAMTYPE::VHD_ST_RX9;
				case 10: return VHD_STREAMTYPE::VHD_ST_RX10;
				case 11: return VHD_STREAMTYPE::VHD_ST_RX11;
				default:
					return VHD_STREAMTYPE::NB_VHD_STREAMTYPES;
			}
		}
		else
		{
			switch (PortIndex)
			{
				case 0: return VHD_STREAMTYPE::VHD_ST_TX0;
				case 1: return VHD_STREAMTYPE::VHD_ST_TX1;
				case 2: return VHD_STREAMTYPE::VHD_ST_TX2;
				case 3: return VHD_STREAMTYPE::VHD_ST_TX3;
				case 4: return VHD_STREAMTYPE::VHD_ST_TX4;
				case 5: return VHD_STREAMTYPE::VHD_ST_TX5;
				case 6: return VHD_STREAMTYPE::VHD_ST_TX6;
				case 7: return VHD_STREAMTYPE::VHD_ST_TX7;
				case 8: return VHD_STREAMTYPE::VHD_ST_TX8;
				case 9: return VHD_STREAMTYPE::VHD_ST_TX9;
				case 10: return VHD_STREAMTYPE::VHD_ST_TX10;
				case 11: return VHD_STREAMTYPE::VHD_ST_TX11;
				default:
					return VHD_STREAMTYPE::NB_VHD_STREAMTYPES;
			}
		}
	}

	VHD_GENLOCKSOURCE GetGenlockSourceFromPortIndex(const bool bIsLocal, const bool bIsBlackBurst, const int32 PortIndex)
	{
		check(!(bIsLocal && bIsBlackBurst));

		if (bIsLocal)
		{
			return VHD_GENLOCKSOURCE::VHD_GENLOCK_LOCAL;
		}
		else if (bIsBlackBurst)
		{
			return VHD_GENLOCKSOURCE::VHD_GENLOCK_BB0;
		}
		else
		{
			switch (PortIndex)
			{
				case 0: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX0;
				case 1: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX1;
				case 2: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX2;
				case 3: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX3;
				case 4: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX4;
				case 5: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX5;
				case 6: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX6;
				case 7: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX7;
				case 8: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX8;
				case 9: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX9;
				case 10: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX10;
				case 11: return VHD_GENLOCKSOURCE::VHD_GENLOCK_RX11;
				default:
					return VHD_GENLOCKSOURCE::NB_VHD_GENLOCKSOURCES;
			}
		}
	}


	bool IsInput(const VHD_STREAMTYPE StreamType)
	{
		switch (StreamType)
		{
			case VHD_STREAMTYPE::VHD_ST_RX0: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX1: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX2: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX3: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX4: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX5: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX6: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX7: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX8: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX9: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX10: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_RX11:
				return true;
			case VHD_STREAMTYPE::VHD_ST_TX0: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX1: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX2: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX3: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX4: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX5: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX6: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX7: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX8: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX9: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX10: [[fallthrough]];
			case VHD_STREAMTYPE::VHD_ST_TX11:
				return false;
			case VHD_STREAMTYPE::NB_VHD_STREAMTYPES: [[fallthrough]];
			default:
				UE_LOG(LogDeltacastMedia, Fatal, TEXT("`IsInput` unhandled stream type: %u"), StreamType);
				return false;
		}
	}

	bool IsPsf(const VHD_VIDEOSTANDARD VideoStandard)
	{
		switch (VideoStandard)
		{
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080psf_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080psf_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080psf_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048psf_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048psf_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048psf_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160psf_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160psf_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160psf_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160psf_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160psf_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160psf_30Hz:
				return true;
			default:
				return false;
		}
	}

	bool IsProgressive(const VHD_VIDEOSTANDARD VideoStandard)
	{
		switch (VideoStandard)
		{
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_50Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_60Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_60Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_50Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_48Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_50Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_60Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_50Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_60Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_48Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_50Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_60Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_50Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_60Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_48Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_50Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_60Hz:
				return true;
			default:
				return false;
		}
	}

	bool IsProgressive(const VHD_DV_HDMI_VIDEOSTANDARD VideoStandard)
	{
		switch (VideoStandard)
		{
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_640x480p_60Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_60Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_60Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_60Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576p_50Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_50Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_50Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_24Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_25Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_30Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_100Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576p_100Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_120Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_120Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576p_200Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_240Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_24Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_25Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_30Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_120Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_100Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_30Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_25Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_24Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_24Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_50Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_60Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_25Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_30Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_50Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_60Hz:
				return true;
			default:
				return false;
		}
	}

	bool IsSd(const VHD_DV_HDMI_VIDEOSTANDARD VideoStandard)
	{
		switch (VideoStandard)
		{
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_640x480p_60Hz:  [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480i_30Hz:  [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_60Hz:  [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480i_60Hz:  [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_120Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480i_120Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_240Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576i_25Hz:  [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576p_50Hz:  [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576i_50Hz:  [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576p_100Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576i_100Hz: [[fallthrough]];
			case VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576p_200Hz:
			default:
				return false;
		}
	}

	bool Is8K(const VHD_VIDEOSTANDARD VideoStandard)
	{
		switch (VideoStandard)
		{
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_50Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_60Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_24Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_25Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_30Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_48Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_50Hz: [[fallthrough]];
			case VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_60Hz:
				return true;
			default:
				return false;
		}
	}

	bool IsSingleLink(const VHD_INTERFACE Interface)
	{
		switch (Interface)
		{
			case VHD_INTERFACE::VHD_INTERFACE_SD_259: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_HD_292_1: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_3G_A_425_1: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_6G_2081_10: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_12G_2082_10:
				return true;
			case VHD_INTERFACE::VHD_INTERFACE_4XHD_QUADRANT: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X3G_A_QUADRANT: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X3G_A_425_5: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X6G_2081_10_QUADRANT: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X6G_2081_12: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X12G_2082_10_QUADRANT: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X12G_2082_12:
				return false;
			case VHD_INTERFACE::NB_VHD_INTERFACE: [[fallthrough]];
			default:
				UE_LOG(LogDeltacastMedia, Fatal, TEXT("`IsSingleLink` unhandled interface: %u"), Interface);
				return false;
		}
	}

	bool IsSdi(const VHD_CHANNELTYPE ChannelType)
	{
		switch (ChannelType) {
			case VHD_CHANNELTYPE::VHD_CHNTYPE_HDSDI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_3GSDI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_12GSDI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_3GSDI_ASI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_12GSDI_ASI:
				return true;
			case VHD_CHANNELTYPE::VHD_CHNTYPE_ASI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_DVI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_DISPLAYPORT: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_HDMI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_DISABLE: [[fallthrough]];
			case VHD_CHANNELTYPE::NB_VHD_CHANNELTYPE:
				return false;
			default:
				UE_LOG(LogDeltacastMedia, Fatal, TEXT("`IsSdi` unhandled channel type: %u"), ChannelType);
				return false;
		}	
	}

	bool IsDv(const VHD_CHANNELTYPE ChannelType)
	{
		switch (ChannelType)
		{
			case VHD_CHANNELTYPE::VHD_CHNTYPE_DVI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_DISPLAYPORT: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_HDMI:
				return true;
			case VHD_CHANNELTYPE::VHD_CHNTYPE_ASI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_HDSDI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_3GSDI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_12GSDI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_3GSDI_ASI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_12GSDI_ASI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_DISABLE: [[fallthrough]];
			case VHD_CHANNELTYPE::NB_VHD_CHANNELTYPE:
				return false;
			default:
				UE_LOG(LogDeltacastMedia, Fatal, TEXT("`IsDv` unhandled channel type: %u"), ChannelType);
				return false;
		}
	}

	bool IsAsi(const VHD_CHANNELTYPE ChannelType)
	{
		switch (ChannelType)
		{
			case VHD_CHANNELTYPE::VHD_CHNTYPE_ASI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_3GSDI_ASI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_12GSDI_ASI:
				return true;
			case VHD_CHANNELTYPE::VHD_CHNTYPE_DVI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_DISPLAYPORT: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_HDMI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_HDSDI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_3GSDI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_12GSDI: [[fallthrough]];
			case VHD_CHANNELTYPE::VHD_CHNTYPE_DISABLE: [[fallthrough]];
			case VHD_CHANNELTYPE::NB_VHD_CHANNELTYPE:
				return false;
			default:
				UE_LOG(LogDeltacastMedia, Fatal, TEXT("`IsDv` unhandled channel type: %u"), ChannelType);
				return false;
		}
	}


	bool RequiresLinePadding(const uint32 Width)
	{
		return Width % 48 != 0;
	}


	VHD_INTERFACE GetSingleLinkInterface(const VHD_VIDEOSTANDARD VideoStandard)
	{
		static constexpr auto NbSdiInterfaces = SdiInterfaces.size();
		
		for (int i = 0; i < NbSdiInterfaces; ++i)
		{
			if (!IsSingleLink(SdiInterfaces[i]))
			{
				continue;
			}

			if (std::find(SdiInterfaceToVideoStandards[i].cbegin(), SdiInterfaceToVideoStandards[i].cend(), VideoStandard) !=
			    SdiInterfaceToVideoStandards[i].cend())
			{
				return SdiInterfaces[i];
			}
		}

		UE_LOG(LogDeltacastMedia, Fatal, TEXT("Unhandled `VHD_VIDEOSTANDARD`: %u"), VideoStandard);
		return VHD_INTERFACE::NB_VHD_INTERFACE;
	}

	VHD_INTERFACE GetQuadLinkInterface(const VHD_VIDEOSTANDARD VideoStandard, const EQuadLinkType QuadLinkType)
	{
		static constexpr auto NbSdiInterfaces = SdiInterfaces.size();

		for (int i = 0; i < NbSdiInterfaces; ++i)
		{
			if (IsSingleLink(SdiInterfaces[i]))
			{
				continue;
			}

			const auto& VideoStandards = SdiInterfaceToVideoStandards[i];

			if (std::find(VideoStandards.cbegin(), VideoStandards.cend(), VideoStandard) != VideoStandards.cend() &&
			    QuadLinkType == GetQuadLinkType(SdiInterfaces[i]))
			{
				return SdiInterfaces[i];
			}
		}

		UE_LOG(LogDeltacastMedia, Fatal, TEXT("Unhandled `VHD_VIDEOSTANDARD`: %u"), VideoStandard);
		return VHD_INTERFACE::NB_VHD_INTERFACE;
	}

	EQuadLinkType GetQuadLinkType(const VHD_INTERFACE Interface)
	{
		switch (Interface)
		{
			case VHD_INTERFACE::VHD_INTERFACE_4XHD_QUADRANT: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X3G_A_QUADRANT: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X6G_2081_10_QUADRANT: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X12G_2082_10_QUADRANT:
				return EQuadLinkType::Quadrant;
			case VHD_INTERFACE::VHD_INTERFACE_4X3G_A_425_5: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X6G_2081_12: [[fallthrough]];
			case VHD_INTERFACE::VHD_INTERFACE_4X12G_2082_12:
				return EQuadLinkType::TwoSampleInterleaved;
			case VHD_INTERFACE::NB_VHD_INTERFACE: [[fallthrough]];
			default:
				{
					const auto IsQuadLink = !IsSingleLink(Interface);
					UE_CLOG(!IsQuadLink, LogDeltacastMedia, Fatal, TEXT("Given interface `IsSingleLink` == true: %u"), Interface);
					UE_CLOG(IsQuadLink, LogDeltacastMedia, Fatal, TEXT("`GetQuadLinkType` unhandled interface: %u"), Interface);
					return Helpers::EQuadLinkType::Quadrant;
				}
		}
	}


	namespace Internal
	{
		using SdiVideoStandardType = std::underlying_type_t<VHD_VIDEOSTANDARD>;
		using DvVideoStandardType = std::underlying_type_t<VHD_DV_HDMI_VIDEOSTANDARD>;

		static_assert(std::is_same_v<SdiVideoStandardType, DvVideoStandardType>);

		inline static constexpr auto TypeSize = sizeof(SdiVideoStandardType);

		inline static constexpr auto FlagOffset   = ((8 * TypeSize) - 2);
		inline static constexpr auto TypeFlagBit  = 0b10;
		inline static constexpr auto ClockFlagBit = 0b01;
		inline static constexpr auto FlagBits     = TypeFlagBit | ClockFlagBit;

		inline static constexpr auto SdiTypeFlag = 0b10;
		inline static constexpr auto DvTypeFlag  = 0b00;

		inline static constexpr auto EuropeanTypeFlag = 0b01;
		inline static constexpr auto UsTypeFlag       = 0b00;
	}

	int32 GetDeviceModeIdentifier(const bool bIsEuropean, const VHD_VIDEOSTANDARD VideoStandard)
	{
		static constexpr auto TypeFlag  = Internal::SdiTypeFlag;
		const auto            ClockFlag = bIsEuropean ? Internal::EuropeanTypeFlag : Internal::UsTypeFlag;

		const auto FlagBits   = (TypeFlag | ClockFlag) << Internal::FlagOffset;
		const auto Identifier = FlagBits | static_cast<Internal::SdiVideoStandardType>(VideoStandard);

		return static_cast<int32>(Identifier);
	}

	int32 GetDeviceModeIdentifier(const bool bIsEuropean, const VHD_DV_HDMI_VIDEOSTANDARD VideoStandard)
	{
		static constexpr auto TypeFlag  = Internal::DvTypeFlag;
		const auto            ClockFlag = bIsEuropean ? Internal::EuropeanTypeFlag : Internal::UsTypeFlag;

		const auto FlagBits   = (TypeFlag | ClockFlag) << Internal::FlagOffset;
		const auto Identifier = FlagBits | static_cast<Internal::DvVideoStandardType>(VideoStandard);

		return static_cast<int32>(Identifier);
	}

	VHD_VIDEOSTANDARD GetSdiVideoStandardFromDeviceModeIdentifier(const int32 DeviceModeIdentifier)
	{
		static constexpr auto ExpectedTypeMask = Internal::SdiTypeFlag << Internal::FlagOffset;

		[[maybe_unused]] const auto TypeBits      = DeviceModeIdentifier & (Internal::TypeFlagBit << Internal::FlagOffset);
		const auto                  VideoStandard = DeviceModeIdentifier & ~(Internal::FlagBits << Internal::FlagOffset);

		check(TypeBits == ExpectedTypeMask);

		return static_cast<VHD_VIDEOSTANDARD>(VideoStandard);
	}

	VHD_DV_HDMI_VIDEOSTANDARD GetDvVideoStandardFromDeviceModeIdentifier(const int32 DeviceModeIdentifier)
	{
		static constexpr auto ExpectedTypeMask = Internal::DvTypeFlag << Internal::FlagOffset;

		[[maybe_unused]] const auto TypeBits      = DeviceModeIdentifier & (Internal::TypeFlagBit << Internal::FlagOffset);
		const auto                  VideoStandard = DeviceModeIdentifier & ~(Internal::FlagBits << Internal::FlagOffset);

		check(TypeBits == ExpectedTypeMask);

		return static_cast<VHD_DV_HDMI_VIDEOSTANDARD>(VideoStandard);
	}

	bool IsDeviceModeIdentifierEuropeanClock(const int32 DeviceModeIdentifier)
	{
		const auto FlagBits = DeviceModeIdentifier & (Internal::FlagBits << Internal::FlagOffset);

		return (FlagBits & (Internal::ClockFlagBit << Internal::FlagOffset)) != 0;
	}

	bool IsDeviceModeIdentifierSdi(const int32 DeviceModeIdentifier)
	{
		const auto FlagBits = DeviceModeIdentifier & (Internal::FlagBits << Internal::FlagOffset);

		return (FlagBits & (Internal::SdiTypeFlag << Internal::FlagOffset)) != 0;
	}

	bool IsDeviceModeIdentifierDv(const int32 DeviceModeIdentifier)
	{
		return !IsDeviceModeIdentifierSdi(DeviceModeIdentifier);
	}


	FString FVideoCharacteristics::ToString() const
	{
		return FString::Printf(TEXT("%ux%u%s%u"),
		                       Width, Height,
		                       bIsInterlaced ? TEXT("i") : TEXT("p"),
		                       FrameRate);
	}


	std::optional<FVideoCharacteristics> GetVideoCharacteristics(const VHD_VIDEOSTANDARD VideoStandard)
	{
		const auto &DeltacastSdk = FDeltacast::GetSdk();

		FVideoCharacteristics VideoCharacteristics{};

		const auto Result = DeltacastSdk.GetVideoCharacteristics_Internal(VideoStandard, &VideoCharacteristics.Width, &VideoCharacteristics.Height,
		                                                         &VideoCharacteristics.bIsInterlaced, &VideoCharacteristics.FrameRate);

		if (!Deltacast::Helpers::IsValid(Result))
		{
			return {};
		}

		return VideoCharacteristics;
	}

	std::optional<FVideoCharacteristics> GetVideoCharacteristics(const VHD_DV_HDMI_VIDEOSTANDARD VideoStandard)
	{
		const auto& DeltacastSdk = FDeltacast::GetSdk();

		FVideoCharacteristics VideoCharacteristics{};

		const auto Result = DeltacastSdk.GetVideoCharacteristics_Internal(VideoStandard, &VideoCharacteristics.Width, &VideoCharacteristics.Height,
		                                                                  &VideoCharacteristics.bIsInterlaced, &VideoCharacteristics.FrameRate);

		if (!Deltacast::Helpers::IsValid(Result))
		{
			return {};
		}

		return VideoCharacteristics;
	}


	void FStreamStatistics::Reset()
	{
		ProcessedFrameCount = 0;
		DroppedFrameCount   = 0;
		BufferFill          = 0.0f;
	}

	void GetStreamStatistics(FStreamStatistics& Statistics, const VHDHandle StreamHandle)
	{
		check(StreamHandle != VHD::InvalidHandle);

		const FDeltacastSdk& DeltacastSdk = FDeltacast::GetSdk();

		if (Statistics.bUpdateProcessedFrameCount)
		{
			const auto NewProcessedFrameCount = DeltacastSdk.GetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_SLOTS_COUNT);
			Statistics.ProcessedFrameCount = static_cast<uint32>(NewProcessedFrameCount.value_or(Statistics.ProcessedFrameCount));
		}

		if (Statistics.bUpdateDroppedFrameCount)
		{
			const auto NewDroppedFrameCount = DeltacastSdk.GetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_SLOTS_DROPPED);
			Statistics.DroppedFrameCount = static_cast<uint32>(NewDroppedFrameCount.value_or(Statistics.DroppedFrameCount));
		}

		if (Statistics.bUpdateBufferFill)
		{
			const auto NewBufferFill = DeltacastSdk.GetStreamProperty(StreamHandle, VHD_CORE_STREAMPROPERTY::VHD_CORE_SP_BUFFERQUEUE_FILLING);
			if (NewBufferFill.has_value())
			{
				const auto BufferFillPercentage = static_cast<float>(NewBufferFill.value()) / static_cast<float>(Statistics.NumberOfDeltacastBuffers);
				Statistics.BufferFill = 100.0f * BufferFillPercentage;
			}
		}
	}
}
