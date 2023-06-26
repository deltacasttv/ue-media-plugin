﻿/*
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

#include "DeltacastDefinition.h"

#include "Containers/UnrealString.h"

#include <array>
#include <optional>
#include <vector>


namespace Deltacast::Helpers
{
	template <typename Func>
	class TScopeExit
	{
	public:
		explicit TScopeExit(const Func& Function) : Function_(Function) {}

		~TScopeExit()
		{
			Function_();
		}

	private:
		Func Function_;
	};

	enum class EQuadLinkType { Quadrant, TwoSampleInterleaved };

	struct DELTACASTMEDIA_API FCablePacking final
	{
	public:
		explicit FCablePacking(VHD_BUFFERPACKING BufferPacking, bool IsSd);

	public:
		VHD_DV_SAMPLING Sampling;
		VHD_DV_CS       ColorSpace;
	};

	inline static constexpr auto TimecodeSleepMs = 1.0f / 5.0f;
	inline static constexpr auto RxStatusSleepMs = 0.0f;
	inline static constexpr auto GenlockStatusSleepSec = 1.0f / 20.0f;
	inline static constexpr auto GenlockWaitTimeOutMs = 50ul;
	inline static constexpr auto MaxGenlockSyncTimeSec = int32{ 15 };


	[[nodiscard]] DELTACASTMEDIA_API FString GetErrorString(VHD_ERRORCODE ErrorCode);

	[[nodiscard]] DELTACASTMEDIA_API FString GetVideoStandardString(VHD_VIDEOSTANDARD VideoStandard);
	[[nodiscard]] DELTACASTMEDIA_API FString GetVideoStandardString(VHD_DV_HDMI_VIDEOSTANDARD VideoStandard);

	[[nodiscard]] DELTACASTMEDIA_API FString GetInterfaceString(const VHD_INTERFACE Interface);

	[[nodiscard]] DELTACASTMEDIA_API FString GetGenlockSourceString(const VHD_GENLOCKSOURCE GenlockSource);


	[[nodiscard]] DELTACASTMEDIA_API bool IsValid(VHD_ERRORCODE ErrorCode);


	[[nodiscard]] DELTACASTMEDIA_API FString VersionToText(VHD::ULONG Version);

	[[nodiscard]] DELTACASTMEDIA_API VHD::ULONG GetPortIndex(VHD_STREAMTYPE StreamType);

	[[nodiscard]] DELTACASTMEDIA_API VHD_CORE_BOARDPROPERTY GetByPassFromPortIndex(int32 PortIndex);

	[[nodiscard]] DELTACASTMEDIA_API VHD_CORE_BOARDPROPERTY GetChannelStatusFromPortIndex(bool bIsInput, int32 PortIndex);

	[[nodiscard]] DELTACASTMEDIA_API VHD_CORE_BOARDPROPERTY GetChannelTypeProperty(bool bIsInput, int32 PortIndex);

	[[nodiscard]] DELTACASTMEDIA_API VHD_CORE_BOARDPROPERTY GetChannelMode(int32 PortIndex);

	[[nodiscard]] DELTACASTMEDIA_API VHD_SDI_BOARDPROPERTY GetRxVideoStandard(int32 PortIndex);

	[[nodiscard]] DELTACASTMEDIA_API VHD_SDI_BOARDPROPERTY GetRxClockDivisor(int32 PortIndex);

	[[nodiscard]] DELTACASTMEDIA_API VHD_STREAMTYPE GetStreamTypeFromPortIndex(bool bIsInput, int32 PortIndex);

	[[nodiscard]] DELTACASTMEDIA_API VHD_GENLOCKSOURCE GetGenlockSourceFromPortIndex(bool bIsLocal, bool bIsBlackBurst, int32 PortIndex);


	[[nodiscard]] DELTACASTMEDIA_API bool IsInput(VHD_STREAMTYPE StreamType);

	[[nodiscard]] DELTACASTMEDIA_API bool IsPsf(VHD_VIDEOSTANDARD VideoStandard);

	[[nodiscard]] DELTACASTMEDIA_API bool IsProgressive(VHD_VIDEOSTANDARD VideoStandard);
	[[nodiscard]] DELTACASTMEDIA_API bool IsProgressive(VHD_DV_HDMI_VIDEOSTANDARD VideoStandard);

	[[nodiscard]] DELTACASTMEDIA_API bool IsSd(VHD_DV_HDMI_VIDEOSTANDARD VideoStandard);
	[[nodiscard]] DELTACASTMEDIA_API bool Is8K(VHD_VIDEOSTANDARD VideoStandard);

	[[nodiscard]] DELTACASTMEDIA_API bool IsSingleLink(VHD_INTERFACE Interface);

	[[nodiscard]] DELTACASTMEDIA_API bool IsSdi(VHD_CHANNELTYPE ChannelType);

	[[nodiscard]] DELTACASTMEDIA_API bool IsDv(VHD_CHANNELTYPE ChannelType);

	[[nodiscard]] DELTACASTMEDIA_API bool IsAsi(VHD_CHANNELTYPE ChannelType);


	[[nodiscard]] DELTACASTMEDIA_API bool RequiresLinePadding(uint32 Width);


	[[nodiscard]] DELTACASTMEDIA_API VHD_INTERFACE GetSingleLinkInterface(VHD_VIDEOSTANDARD VideoStandard);

	[[nodiscard]] DELTACASTMEDIA_API VHD_INTERFACE GetQuadLinkInterface(VHD_VIDEOSTANDARD VideoStandard, EQuadLinkType QuadLinkType);

	[[nodiscard]] DELTACASTMEDIA_API EQuadLinkType GetQuadLinkType(VHD_INTERFACE Interface);


	[[nodiscard]] DELTACASTMEDIA_API int32 GetDeviceModeIdentifier(bool bIsEuropean, VHD_VIDEOSTANDARD VideoStandard);

	[[nodiscard]] DELTACASTMEDIA_API int32 GetDeviceModeIdentifier(bool bIsEuropean, VHD_DV_HDMI_VIDEOSTANDARD VideoStandard);

	[[nodiscard]] DELTACASTMEDIA_API VHD_VIDEOSTANDARD GetSdiVideoStandardFromDeviceModeIdentifier(int32 DeviceModeIdentifier);

	[[nodiscard]] DELTACASTMEDIA_API VHD_DV_HDMI_VIDEOSTANDARD GetDvVideoStandardFromDeviceModeIdentifier(int32 DeviceModeIdentifier);

	[[nodiscard]] DELTACASTMEDIA_API bool IsDeviceModeIdentifierEuropeanClock(int32 DeviceModeIdentifier);

	[[nodiscard]] DELTACASTMEDIA_API bool IsDeviceModeIdentifierSdi(int32 DeviceModeIdentifier);

	[[nodiscard]] DELTACASTMEDIA_API bool IsDeviceModeIdentifierDv(int32 DeviceModeIdentifier);




	struct DELTACASTMEDIA_API FVideoCharacteristics final
	{
	public:
		VHD::ULONG Width;
		VHD::ULONG Height;

		bool bIsInterlaced;

		VHD::ULONG FrameRate;

	public:
		[[nodiscard]] FString ToString() const;
	};

	[[nodiscard]] DELTACASTMEDIA_API std::optional<FVideoCharacteristics> GetVideoCharacteristics(VHD_VIDEOSTANDARD VideoStandard);
	[[nodiscard]] DELTACASTMEDIA_API std::optional<FVideoCharacteristics> GetVideoCharacteristics(VHD_DV_HDMI_VIDEOSTANDARD VideoStandard);


	struct DELTACASTMEDIA_API FStreamStatistics final
	{
	public:
		void Reset();

	public:
		bool   bUpdateProcessedFrameCount;
		uint32 ProcessedFrameCount;

		bool   bUpdateDroppedFrameCount;
		uint32 DroppedFrameCount;

		bool   bUpdateBufferFill;
		float  BufferFill;
		uint32 NumberOfDeltacastBuffers;
	};

	DELTACASTMEDIA_API void GetStreamStatistics(FStreamStatistics& Statistics, VHDHandle StreamHandle);


	inline static constexpr std::array<VHD_INTERFACE, 12> SdiInterfaces = {
		  VHD_INTERFACE::VHD_INTERFACE_SD_259,
		  VHD_INTERFACE::VHD_INTERFACE_HD_292_1,
		  VHD_INTERFACE::VHD_INTERFACE_3G_A_425_1,
		  VHD_INTERFACE::VHD_INTERFACE_6G_2081_10,
		  VHD_INTERFACE::VHD_INTERFACE_12G_2082_10,
		  VHD_INTERFACE::VHD_INTERFACE_4XHD_QUADRANT,
		  VHD_INTERFACE::VHD_INTERFACE_4X3G_A_QUADRANT,
		  VHD_INTERFACE::VHD_INTERFACE_4X3G_A_425_5,
		  VHD_INTERFACE::VHD_INTERFACE_4X6G_2081_10_QUADRANT,
		  VHD_INTERFACE::VHD_INTERFACE_4X6G_2081_12,
		  VHD_INTERFACE::VHD_INTERFACE_4X12G_2082_10_QUADRANT,
		  VHD_INTERFACE::VHD_INTERFACE_4X12G_2082_12
	};

	inline static const std::array<std::vector<VHD_VIDEOSTANDARD>, SdiInterfaces.size()> SdiInterfaceToVideoStandards =
		std::array
	{
		std::vector
		{
			// `VHD_INTERFACE_SD_259`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_PAL,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_NTSC_480,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_NTSC_487,
		},
		std::vector
		{
			// `VHD_INTERFACE_HD_292_1`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080i_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080i_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080psf_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080psf_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080psf_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048psf_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048psf_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048psf_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_30Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_3G_A_425_1`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_48Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_60Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_6G_2081_10`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_30Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_12G_2082_10`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_48Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_60Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4XHD_QUADRANT`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160psf_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160psf_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160psf_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160psf_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160psf_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160psf_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_30Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X3G_A_QUADRANT`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_48Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_60Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X3G_A_425_5`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_48Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_60Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X6G_2081_10_QUADRANT`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_30Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X6G_2081_12`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_25Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_30Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X12G_2082_10_QUADRANT`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_48Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_60Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X12G_2082_12`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_48Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_50Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_60Hz,
		}
	};

	inline static const std::array<std::vector<VHD_VIDEOSTANDARD>, SdiInterfaces.size()> SdiInterfaceToVideoStandardsUsClock =
		std::array
	{
		std::vector
		{
			// `VHD_INTERFACE_SD_259`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_PAL,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_NTSC_480,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S259M_NTSC_487,
		},
		std::vector
		{
			// `VHD_INTERFACE_HD_292_1`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S296M_720p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080i_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080psf_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080psf_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048psf_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048psf_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_30Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_3G_A_425_1`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S274M_1080p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_S2048M_2048p_60Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_6G_2081_10`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_30Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_12G_2082_10`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_60Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4XHD_QUADRANT`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160psf_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160psf_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160psf_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160psf_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_30Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X3G_A_QUADRANT`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_60Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X3G_A_425_5`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_3840x2160p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_4096x2160p_60Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X6G_2081_10_QUADRANT`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_30Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X6G_2081_12`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_30Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_24Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_30Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X12G_2082_10_QUADRANT`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_60Hz,
		},
		std::vector
		{
			// `VHD_INTERFACE_4X12G_2082_12`
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_7680x4320p_60Hz,
			VHD_VIDEOSTANDARD::VHD_VIDEOSTD_8192x4320p_60Hz,
		}
	};


	inline static constexpr std::array<VHD_DV_HDMI_VIDEOSTANDARD, 41> DvVideoStandards =
	{
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_640x480p_60Hz,

		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480i_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_60Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480i_60Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_120Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480i_120Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_240Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576i_25Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576p_50Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576i_50Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576p_100Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576i_100Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x576p_200Hz,

		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_24Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_25Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_50Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_60Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_100Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_120Hz,

		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_24Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_25Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080i_25Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_50Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080i_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_60Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080i_50Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_100Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080i_60Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_120Hz,

		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_24Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_25Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_50Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_60Hz,

		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_24Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_25Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_50Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_60Hz,
	};

	 // TODO @JV v1: improvement: C++20 use std::span instead
	inline static const auto DvVideoStandardsVector = std::vector(std::begin(DvVideoStandards), std::end(DvVideoStandards));

	inline static constexpr std::array<VHD_DV_HDMI_VIDEOSTANDARD, 23> DvVideoStandardsUsClock =
	{
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_640x480p_60Hz,

		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480i_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_60Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480i_60Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_120Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480i_120Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_720x480p_240Hz,

		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_24Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_60Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1280x720p_120Hz,

		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_24Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080i_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_60Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080i_60Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_1920x1080p_120Hz,

		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_24Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_3840x2160p_60Hz,

		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_24Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_30Hz,
		VHD_DV_HDMI_VIDEOSTANDARD::VHD_DV_HDMI_VIDEOSTD_4096x2160p_60Hz,
	};

	inline static const auto DvVideoStandardsUsClockVector = std::vector(std::begin(DvVideoStandardsUsClock), std::end(DvVideoStandardsUsClock));
}
