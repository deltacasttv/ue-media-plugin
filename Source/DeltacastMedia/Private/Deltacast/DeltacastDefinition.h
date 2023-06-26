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


#include "HAL/Platform.h"
#include "UObject/ObjectMacros.h"


/*
 * Can abstract VHDHandle lifetime management with:
 *	- Creation: `VHDXXX XXX = DeltacastSdk.OpenXXX(...);`
 *	- Deletion: `using VHDXXX = std::unique_ptr<VHDHandle, XXXDeleter>;`
 * Don't need to change the wrapper API, still pass `XXX.get()`/`*XXX`
 */
using VHDHandle = void*;

namespace VHD
{
	using Bool = int;
	using BYTE = unsigned char;
	using ULONG = uint32;

	inline static constexpr VHDHandle InvalidHandle = nullptr;

	inline static constexpr VHD::Bool False = 0;
	inline static constexpr VHD::Bool True  = 1;

	inline static constexpr auto MaxPortCount = 12;

	inline static constexpr auto VHD_CORE_RXSTS_UNLOCKED = 0x2;

	inline static constexpr auto VHD_SDI_GNLKSTS_NOREF    = 0x1;
	inline static constexpr auto VHD_SDI_GNLKSTS_UNLOCKED = 0x2;
}


UENUM()
enum class VHD_ERRORCODE : uint32
{
	VHDERR_NOERROR = 0,

	VHDERR_STREAMUSED = 18,
	VHDERR_TIMEOUT = 22,

	VHDERR_BADARG = 6,
	VHDERR_BADCONFIG = 33,

	VHDERR_LTCSOURCEUNLOCKED = 50,
};


#pragma region Board

enum class VHD_CORE_BOARDPROPERTY : uint32
{
	VHD_CORE_BP_NB_RXCHANNELS = 16777229,
	VHD_CORE_BP_NB_TXCHANNELS = 16777230,

	VHD_CORE_BP_RX0_STATUS = 16777235,
	VHD_CORE_BP_RX1_STATUS = 16777236,
	VHD_CORE_BP_RX2_STATUS = 16777237,
	VHD_CORE_BP_RX3_STATUS = 16777238,
	VHD_CORE_BP_RX4_STATUS = 16777255,
	VHD_CORE_BP_RX5_STATUS = 16777256,
	VHD_CORE_BP_RX6_STATUS = 16777257,
	VHD_CORE_BP_RX7_STATUS = 16777258,
	VHD_CORE_BP_RX8_STATUS = 16777309,
	VHD_CORE_BP_RX9_STATUS = 16777310,
	VHD_CORE_BP_RX10_STATUS = 16777311,
	VHD_CORE_BP_RX11_STATUS = 16777312,

	VHD_CORE_BP_TX0_STATUS = 16777239,
	VHD_CORE_BP_TX1_STATUS = 16777240,
	VHD_CORE_BP_TX2_STATUS = 16777241,
	VHD_CORE_BP_TX3_STATUS = 16777242,
	VHD_CORE_BP_TX4_STATUS = 16777267,
	VHD_CORE_BP_TX5_STATUS = 16777268,
	VHD_CORE_BP_TX6_STATUS = 16777269,
	VHD_CORE_BP_TX7_STATUS = 16777270,
	VHD_CORE_BP_TX8_STATUS = 16777317,
	VHD_CORE_BP_TX9_STATUS = 16777318,
	VHD_CORE_BP_TX10_STATUS = 16777319,
	VHD_CORE_BP_TX11_STATUS = 16777320,

	VHD_CORE_BP_BOARD_TYPE = 16777220,

	VHD_CORE_BP_SERIALNUMBER_LSW = 16777224,
	VHD_CORE_BP_SERIALNUMBER_MSW = 16777225,

	VHD_CORE_BP_RX0_TYPE = 16777243,
	VHD_CORE_BP_RX1_TYPE = 16777244,
	VHD_CORE_BP_RX2_TYPE = 16777245,
	VHD_CORE_BP_RX3_TYPE = 16777246,
	VHD_CORE_BP_RX4_TYPE = 16777259,
	VHD_CORE_BP_RX5_TYPE = 16777260,
	VHD_CORE_BP_RX6_TYPE = 16777261,
	VHD_CORE_BP_RX7_TYPE = 16777262,
	VHD_CORE_BP_RX8_TYPE = 16777305,
	VHD_CORE_BP_RX9_TYPE = 16777306,
	VHD_CORE_BP_RX10_TYPE = 16777307,
	VHD_CORE_BP_RX11_TYPE = 16777308,

	VHD_CORE_BP_TX0_TYPE = 16777247,
	VHD_CORE_BP_TX1_TYPE = 16777248,
	VHD_CORE_BP_TX2_TYPE = 16777249,
	VHD_CORE_BP_TX3_TYPE = 16777250,
	VHD_CORE_BP_TX4_TYPE = 16777271,
	VHD_CORE_BP_TX5_TYPE = 16777272,
	VHD_CORE_BP_TX6_TYPE = 16777273,
	VHD_CORE_BP_TX7_TYPE = 16777274,
	VHD_CORE_BP_TX8_TYPE = 16777313,
	VHD_CORE_BP_TX9_TYPE = 16777314,
	VHD_CORE_BP_TX10_TYPE = 16777315,
	VHD_CORE_BP_TX11_TYPE = 16777316,

	VHD_CORE_BP_RX0_MODE = 16777294,
	VHD_CORE_BP_RX1_MODE = 16777295,
	VHD_CORE_BP_RX2_MODE = 16777296,
	VHD_CORE_BP_RX3_MODE = 16777297,
	VHD_CORE_BP_RX4_MODE = 16777298,
	VHD_CORE_BP_RX5_MODE = 16777299,
	VHD_CORE_BP_RX6_MODE = 16777300,
	VHD_CORE_BP_RX7_MODE = 16777301,

	VHD_CORE_BP_BYPASS_RELAY_0 = 16777231,
	VHD_CORE_BP_BYPASS_RELAY_1 = 16777232,
	VHD_CORE_BP_BYPASS_RELAY_2 = 16777265,
	VHD_CORE_BP_BYPASS_RELAY_3 = 16777266,

	VHD_CORE_BOARD_CAP_LTC_ONBOARD = 16777238,

	NB_VHD_CORE_BOARDPROPERTIES = 16777327,
};

enum class VHD_CORE_BOARD_CAPABILITY : uint32
{
	VHD_CORE_BOARD_CAP_FIELD_MERGING = 16777232, /*! Return true if the board supports field merging handling feature */
};

enum class VHD_BOARDTYPE : uint32
{
	VHD_BOARDTYPE_HD = 0,             /*! DELTA-hd board */
	VHD_BOARDTYPE_MIXEDINTERFACE = 5, /*! Mixed interfaces board */
	VHD_BOARDTYPE_3G = 6,             /*! DELTA-3G board */
	VHD_BOARDTYPE_3GKEY = 7,          /*! DELTA-key 3G board */
	VHD_BOARDTYPE_ASI = 10,           /*! DELTA-asi board */
	VHD_BOARDTYPE_HDMI20 = 12,        /*! DELTA-h4k2 board */
	VHD_BOARDTYPE_FLEX_DP = 13,       /*! FLEX-dp board*/
	VHD_BOARDTYPE_FLEX_SDI = 14,      /*! FLEX-sdi board*/
	VHD_BOARDTYPE_12G = 15,           /*! DELTA-12G board */
	VHD_BOARDTYPE_FLEX_HMI = 16,      /*! FLEX-hmi board */
	NB_VHD_BOARDTYPES = 17
};

enum class VHD_SDI_BOARDPROPERTY : uint32
{
	VHD_SDI_BP_CLOCK_SYSTEM = 33554436,
	
	VHD_SDI_BP_GENLOCK_SOURCE = 33554437,
	VHD_SDI_BP_GENLOCK_STATUS = 33554439,
	VHD_SDI_BP_GENLOCK_VIDEO_STANDARD = 33554440,

	VHD_SDI_BP_RX0_STANDARD = 33554432,
	VHD_SDI_BP_RX1_STANDARD = 33554433,
	VHD_SDI_BP_RX2_STANDARD = 33554434,
	VHD_SDI_BP_RX3_STANDARD = 33554435,
	VHD_SDI_BP_RX4_STANDARD = 33554456,
	VHD_SDI_BP_RX5_STANDARD = 33554457,
	VHD_SDI_BP_RX6_STANDARD = 33554458,
	VHD_SDI_BP_RX7_STANDARD = 33554459,
	VHD_SDI_BP_RX8_STANDARD = 33554491,
	VHD_SDI_BP_RX9_STANDARD = 33554492,
	VHD_SDI_BP_RX10_STANDARD = 33554493,
	VHD_SDI_BP_RX11_STANDARD = 33554494,

	VHD_SDI_BP_RX0_CLOCK_DIV = 33554442,
	VHD_SDI_BP_RX1_CLOCK_DIV = 33554443,
	VHD_SDI_BP_RX2_CLOCK_DIV = 33554444,
	VHD_SDI_BP_RX3_CLOCK_DIV = 33554445,
	VHD_SDI_BP_RX4_CLOCK_DIV = 33554460,
	VHD_SDI_BP_RX5_CLOCK_DIV = 33554461,
	VHD_SDI_BP_RX6_CLOCK_DIV = 33554462,
	VHD_SDI_BP_RX7_CLOCK_DIV = 33554463,
	VHD_SDI_BP_RX8_CLOCK_DIV = 33554495,
	VHD_SDI_BP_RX9_CLOCK_DIV = 33554495,
	VHD_SDI_BP_RX10_CLOCK_DIV = 33554495,
	VHD_SDI_BP_RX11_CLOCK_DIV = 33554495,

	NB_VHD_SDI_BOARDPROPERTIES = 33554511
};

#pragma endregion


#pragma region Stream

enum class VHD_STREAMTYPE : uint32
{
	VHD_ST_RX0 = 0,
	VHD_ST_RX1 = 1,
	VHD_ST_RX2 = 9,
	VHD_ST_RX3 = 10,
	VHD_ST_RX4 = 20,
	VHD_ST_RX5 = 21,
	VHD_ST_RX6 = 22,
	VHD_ST_RX7 = 23,
	VHD_ST_RX8 = 44,
	VHD_ST_RX9 = 45,
	VHD_ST_RX10 = 46,
	VHD_ST_RX11 = 47,
	VHD_ST_TX0 = 2,
	VHD_ST_TX1 = 3,
	VHD_ST_TX2 = 11,
	VHD_ST_TX3 = 12,
	VHD_ST_TX4 = 33,
	VHD_ST_TX5 = 34,
	VHD_ST_TX6 = 35,
	VHD_ST_TX7 = 36,
	VHD_ST_TX8 = 48,
	VHD_ST_TX9 = 49,
	VHD_ST_TX10 = 50,
	VHD_ST_TX11 = 51,
	NB_VHD_STREAMTYPES = 0x00000800
};

enum class VHD_CORE_STREAMPROPERTY : uint32
{
	VHD_CORE_SP_TRANSFER_SCHEME = 16777218, /*! Affect reception stream only (default is VHD_TRANSFER_UNCONSTRAINED) */

	VHD_CORE_SP_IO_TIMEOUT = 16777219, /*! Slot locking time-out value, in milliseconds (default is 100ms) */

	VHD_CORE_SP_SLOTS_COUNT = 16777221, /*! Counts the number of slots transferred since the stream has been started */
	VHD_CORE_SP_SLOTS_DROPPED = 16777222, /*! Counts the number of dropped slots since the stream has been started */

	VHD_CORE_SP_BUFFERQUEUE_DEPTH = 16777223, /*! Stream driver buffers queue depth, in number of slots [2, 32], default = 4 */
	VHD_CORE_SP_BUFFERQUEUE_FILLING = 16777224, /*! Current filling level of stream driver buffer queue, in number of slots */
	VHD_CORE_SP_BUFFERQUEUE_PRELOAD = 16777225, /*! TX buffer preloaded before channel start. [0, BUFFERQUEUE_DEPTH + 1], default = 0 */

	VHD_CORE_SP_BUFFER_PACKING = 16777226, /*! Stream buffers data packing, default is VHD_BUFPACK_VIDEO_YUV422_8 */
	VHD_CORE_SP_FIELD_MERGE = 16777230, /*! Activate field merging for interlaced stream (default is FALSE) */
	VHD_CORE_SP_LINE_PADDING = 16777231, /*! Activate 64 or 128 bytes line padding (default is 0) */
};


enum class VHD_SDI_STREAMPROCMODE : uint32
{
	VHD_SDI_STPROC_DISJOINED_VIDEO = 33554434, /*! Disjoined processed mode (video handling only) */
};

enum class VHD_SDI_STREAMPROPERTY : uint32
{
	VHD_SDI_SP_TX_GENLOCK = 33554432,

	VHD_SDI_SP_VIDEO_STANDARD = 33554433,
	VHD_SDI_SP_INTERFACE = 33554437,
};


enum class VHD_DV_STREAMPROCMODE : uint32
{
	VHD_DV_STPROC_DISJOINED_VIDEO = 268435456,          /*! Single Video processing (default) */
	NB_VHD_DV_STREAMPROCMODE = 268435460
};

enum class VHD_DV_STREAMPROPERTY : uint32
{
	VHD_DV_SP_ACTIVE_WIDTH = 268435457,
	VHD_DV_SP_ACTIVE_HEIGHT = 268435458,
	VHD_DV_SP_INTERLACED = 268435459,
	VHD_DV_SP_REFRESH_RATE = 268435460,

	VHD_DV_SP_CS = 268435477,
	VHD_DV_SP_CABLE_BIT_SAMPLING = 268435480,

	VHD_DV_SP_MODE = 268435456,

	VHD_DV_SP_DISABLE_EDID_AUTO_LOAD = 268435476,

	NB_VHD_DV_STREAMPROPERTIES = 268435481
};

#pragma endregion


#pragma region SDI

UENUM()
enum class VHD_VIDEOSTANDARD : uint32
{
	VHD_VIDEOSTD_S274M_1080p_25Hz = 0,    /*! SMPTE 274M - HD 1080p @ 25Hz standard */
	VHD_VIDEOSTD_S274M_1080p_30Hz = 1,    /*! SMPTE 274M - HD 1080p @ 30Hz standard (default) */
	VHD_VIDEOSTD_S274M_1080i_50Hz = 2,    /*! SMPTE 274M - HD 1080i @ 50Hz standard */
	VHD_VIDEOSTD_S274M_1080i_60Hz = 3,    /*! SMPTE 274M - HD 1080i @ 60Hz standard */
	VHD_VIDEOSTD_S296M_720p_50Hz = 4,     /*! SMPTE 296M - HD 720p @ 50Hz standard */
	VHD_VIDEOSTD_S296M_720p_60Hz = 5,     /*! SMPTE 296M - HD 720p @ 60Hz standard */
	VHD_VIDEOSTD_S259M_PAL = 6,           /*! SMPTE 259M - SD PAL standard */
	VHD_VIDEOSTD_S259M_NTSC_487 = 7,      /*! SMPTE 259M - SD NTSC standard */
	VHD_VIDEOSTD_S274M_1080p_24Hz = 8,    /*! SMPTE 274M - HD 1080p @ 24Hz standard */
	VHD_VIDEOSTD_S274M_1080p_60Hz = 9,    /*! SMPTE 274M - 3G 1080p @ 60Hz standard */
	VHD_VIDEOSTD_S274M_1080p_50Hz = 10,   /*! SMPTE 274M - 3G 1080p @ 50Hz standard */
	VHD_VIDEOSTD_S274M_1080psf_24Hz = 11, /*! SMPTE 274M - HD 1080psf @ 24Hz standard */
	VHD_VIDEOSTD_S274M_1080psf_25Hz = 12, /*! SMPTE 274M - HD 1080psf @ 25Hz standard */
	VHD_VIDEOSTD_S274M_1080psf_30Hz = 13, /*! SMPTE 274M - HD 1080psf @ 30Hz standard */
	VHD_VIDEOSTD_S296M_720p_24Hz = 14,    /*! SMPTE 296M - HD 720p @ 24Hz standard */
	VHD_VIDEOSTD_S296M_720p_25Hz = 15,    /*! SMPTE 296M - HD 720p @ 25Hz standard */
	VHD_VIDEOSTD_S296M_720p_30Hz = 16,    /*! SMPTE 296M - HD 720p @ 30Hz standard */
	VHD_VIDEOSTD_S2048M_2048p_24Hz = 17,  /*! SMPTE 2048M - HD 2048p @ 24 Hz standard */
	VHD_VIDEOSTD_S2048M_2048p_25Hz = 18,  /*! SMPTE 2048M - HD 2048p @ 25 Hz standard */
	VHD_VIDEOSTD_S2048M_2048p_30Hz = 19,  /*! SMPTE 2048M - HD 2048p @ 30 Hz standard */
	VHD_VIDEOSTD_S2048M_2048psf_24Hz = 20,/*! SMPTE 2048M - HD 2048psf @ 24 Hz standard */
	VHD_VIDEOSTD_S2048M_2048psf_25Hz = 21,/*! SMPTE 2048M - HD 2048psf @ 25 Hz standard */
	VHD_VIDEOSTD_S2048M_2048psf_30Hz = 22,/*! SMPTE 2048M - HD 2048psf @ 30 Hz standard */
	VHD_VIDEOSTD_S2048M_2048p_60Hz = 23,  /*! SMPTE 2048M - 3G 2048p @ 60Hz standard */
	VHD_VIDEOSTD_S2048M_2048p_50Hz = 24,  /*! SMPTE 2048M - 3G 2048p @ 50Hz standard */
	VHD_VIDEOSTD_S2048M_2048p_48Hz = 25,  /*! SMPTE 2048M - 3G 2048p @ 50Hz standard */
	VHD_VIDEOSTD_3840x2160p_24Hz = 26,    /*! 3840x2160 - 4x HD 1080p @ 24Hz merged */
	VHD_VIDEOSTD_3840x2160p_25Hz = 27,    /*! 3840x2160 - 4x HD 1080p @ 25Hz merged */
	VHD_VIDEOSTD_3840x2160p_30Hz = 28,    /*! 3840x2160 - 4x HD 1080p @ 30Hz merged */
	VHD_VIDEOSTD_3840x2160p_50Hz = 29,    /*! 3840x2160 - 4x 3G 1080p @ 50Hz merged */
	VHD_VIDEOSTD_3840x2160p_60Hz = 30,    /*! 3840x2160 - 4x 3G 1080p @ 60Hz merged */
	VHD_VIDEOSTD_4096x2160p_24Hz = 31,    /*! 4096x2160 - 4x HD 2048p @ 24Hz merged */
	VHD_VIDEOSTD_4096x2160p_25Hz = 32,    /*! 4096x2160 - 4x HD 2048p @ 25Hz merged */
	VHD_VIDEOSTD_4096x2160p_30Hz = 33,    /*! 4096x2160 - 4x HD 2048p @ 30Hz merged */
	VHD_VIDEOSTD_4096x2160p_48Hz = 34,    /*! 4096x2160 - 4x 3G 2048p @ 48Hz merged */
	VHD_VIDEOSTD_4096x2160p_50Hz = 35,    /*! 4096x2160 - 4x 3G 2048p @ 50Hz merged */
	VHD_VIDEOSTD_4096x2160p_60Hz = 36,    /*! 4096x2160 - 4x 3G 2048p @ 60Hz merged */
	VHD_VIDEOSTD_S259M_NTSC_480 = 37,     /*! SMPTE 259M - SD NTSC standard - 480 active lines */
	VHD_VIDEOSTD_7680x4320p_24Hz = 38,    /*! 7680x4320 - 4x 6G 3840x2160 @ 24Hz merged */
	VHD_VIDEOSTD_7680x4320p_25Hz = 39,    /*! 7680x4320 - 4x 6G 3840x2160 @ 25Hz merged */
	VHD_VIDEOSTD_7680x4320p_30Hz = 40,    /*! 7680x4320 - 4x 6G 3840x2160 @ 30Hz merged */
	VHD_VIDEOSTD_7680x4320p_50Hz = 41,    /*! 7680x4320 - 4x 12G 3840x2160 @ 50Hz merged */
	VHD_VIDEOSTD_7680x4320p_60Hz = 42,    /*! 7680x4320 - 4x 12G 3840x2160 @ 60Hz merged */
	VHD_VIDEOSTD_3840x2160psf_24Hz = 43,  /*! 3840x2160 - 4x HD 1080psf @ 24Hz merged */
	VHD_VIDEOSTD_3840x2160psf_25Hz = 44,  /*! 3840x2160 - 4x HD 1080psf @ 25Hz merged */
	VHD_VIDEOSTD_3840x2160psf_30Hz = 45,  /*! 3840x2160 - 4x HD 1080psf @ 30Hz merged */
	VHD_VIDEOSTD_4096x2160psf_24Hz = 46,  /*! 4096x2160 - 4x HD 2048psf @ 24Hz merged */
	VHD_VIDEOSTD_4096x2160psf_25Hz = 47,  /*! 4096x2160 - 4x HD 2048psf @ 25Hz merged */
	VHD_VIDEOSTD_4096x2160psf_30Hz = 48,  /*! 4096x2160 - 4x HD 2048psf @ 30Hz merged */
	VHD_VIDEOSTD_8192x4320p_24Hz = 49,    /*! 8192x4320 - 4x 6G 2048psf @ 24Hz merged */
	VHD_VIDEOSTD_8192x4320p_25Hz = 50,    /*! 8192x4320 - 4x 6G 2048psf @ 25Hz merged */
	VHD_VIDEOSTD_8192x4320p_30Hz = 51,    /*! 8192x4320 - 4x 6G 2048psf @ 30Hz merged */
	VHD_VIDEOSTD_8192x4320p_48Hz = 52,    /*! 8192x4320 - 4x 12G 2048psf @ 48Hz merged */
	VHD_VIDEOSTD_8192x4320p_50Hz = 53,    /*! 8192x4320 - 4x 12G 2048psf @ 50Hz merged */
	VHD_VIDEOSTD_8192x4320p_60Hz = 54,    /*! 8192x4320 - 4x 12G 2048psf @ 60Hz merged */
	NB_VHD_VIDEOSTANDARDS = 55
};

UENUM()
enum class VHD_INTERFACE : uint32
{
	VHD_INTERFACE_SD_259 = 1,				   /*! SD SMPTE 259 interface */
	VHD_INTERFACE_HD_292_1 = 2,				   /*! HD SMPTE 291-1 interface */
	VHD_INTERFACE_3G_A_425_1 = 4,			   /*! 3G-A SMPTE 425-1 interface */
	VHD_INTERFACE_6G_2081_10 = 23,			   /*! 6G over SMPTE 2081-10 interface */
	VHD_INTERFACE_12G_2082_10 = 24,			   /*! 12G over SMPTE 2082-10 interface */
	VHD_INTERFACE_4XHD_QUADRANT = 5,		   /*! 4xHD interface (4K image is split into 4 quadrants for transport) */
	VHD_INTERFACE_4X3G_A_QUADRANT = 6,		   /*! 4x3G-A interface (4K image is split into 4 quadrants for transport) */
	VHD_INTERFACE_4X3G_A_425_5 = 12,		   /*! 4x3G-A SMPTE 425-5 interface (4K image is split into 4 images with the 2-sample interleave division rule for transport) */
	VHD_INTERFACE_4X6G_2081_10_QUADRANT = 26,  /*! 4x6G over SMPTE 2081-10 interface (8K image is split into 4 quadrants for transport) */
	VHD_INTERFACE_4X6G_2081_12 = 30,		   /*! 4x6G over SMPTE 2081-12 */
	VHD_INTERFACE_4X12G_2082_10_QUADRANT = 27, /*! 4x12G over SMPTE 2082-10 interface (8K image is split into 4 quadrants for transport) */
	VHD_INTERFACE_4X12G_2082_12 = 31,		   /*! 4x12G over SMPTE 2082-12 */
	NB_VHD_INTERFACE = 32,
};

enum class VHD_CLOCKDIVISOR : uint32
{
	VHD_CLOCKDIV_1 = 0, /* Produce a frame rate of 24, 25, 30, 50, or 60 fps (default) */
	VHD_CLOCKDIV_1001,  /* Produce a frame rate of 23.98, 29.97 or 59.94 fps */
	NB_VHD_CLOCKDIVISORS
};

#pragma endregion


#pragma region Dv

UENUM()
enum class VHD_DV_HDMI_VIDEOSTANDARD : uint32
{
	VHD_DV_HDMI_VIDEOSTD_640x480p_60Hz = 0,     /*! 640x480p     @  60Hz VIC=1  */
	VHD_DV_HDMI_VIDEOSTD_720x480p_60Hz = 1,     /*! 720x480p     @  60Hz VIC=3  */
	VHD_DV_HDMI_VIDEOSTD_1280x720p_60Hz = 2,    /*! 1280x720p    @  60Hz VIC=4  */
	VHD_DV_HDMI_VIDEOSTD_1920x1080i_30Hz = 3,   /*! 1920x1080i   @  30Hz VIC=5  */
	VHD_DV_HDMI_VIDEOSTD_720x480i_30Hz = 4,     /*! 720x480i     @  30Hz VIC=7  */
	VHD_DV_HDMI_VIDEOSTD_1920x1080p_60Hz = 5,   /*! 1920x1080p   @  60Hz VIC=16 */
	VHD_DV_HDMI_VIDEOSTD_720x576p_50Hz = 6,     /*! 720x576p     @  50Hz VIC=18 */
	VHD_DV_HDMI_VIDEOSTD_1280x720p_50Hz = 7,    /*! 280x720p     @  50Hz VIC=19 */
	VHD_DV_HDMI_VIDEOSTD_1920x1080i_25Hz = 8,   /*! 1920x1080i   @  25Hz VIC=20 */
	VHD_DV_HDMI_VIDEOSTD_720x576i_25Hz = 9,     /*! 720x576i     @  25Hz VIC=22 */
	VHD_DV_HDMI_VIDEOSTD_1920x1080p_50Hz = 10,  /*! 1920x1080p   @  50Hz VIC=31 */
	VHD_DV_HDMI_VIDEOSTD_1920x1080p_24Hz = 11,  /*! 1920x1080p   @  24Hz VIC=32 */
	VHD_DV_HDMI_VIDEOSTD_1920x1080p_25Hz = 12,  /*! 1920x1080p   @  25Hz VIC=33 */
	VHD_DV_HDMI_VIDEOSTD_1920x1080p_30Hz = 13,  /*! 1920x1080p   @  30Hz VIC=34 */
	VHD_DV_HDMI_VIDEOSTD_1920x1080i_50Hz = 14,  /*! 1920x1080i   @  50Hz VIC=40 */
	VHD_DV_HDMI_VIDEOSTD_1280x720p_100Hz = 15,  /*! 1280x720p    @ 100Hz VIC=41 */
	VHD_DV_HDMI_VIDEOSTD_720x576p_100Hz = 16,   /*! 720x576p     @ 100Hz VIC=43 */
	VHD_DV_HDMI_VIDEOSTD_720x576i_50Hz = 17,    /*! 720x576i     @  50Hz VIC=45 */
	VHD_DV_HDMI_VIDEOSTD_1920x1080i_60Hz = 18,  /*! 1920x1080i   @  60Hz VIC=46 */
	VHD_DV_HDMI_VIDEOSTD_1280x720p_120Hz = 19,  /*! 1280x720p    @ 120Hz VIC=47 */
	VHD_DV_HDMI_VIDEOSTD_720x480p_120Hz = 20,   /*! 720x480p     @ 120Hz VIC=49 */
	VHD_DV_HDMI_VIDEOSTD_720x480i_60Hz = 21,    /*! 720x480i     @  60Hz VIC=51 */
	VHD_DV_HDMI_VIDEOSTD_720x576p_200Hz = 22,   /*! 720x576p     @ 200Hz VIC=53 */
	VHD_DV_HDMI_VIDEOSTD_720x576i_100Hz = 23,   /*! 720x576i     @ 100Hz VIC=55 */
	VHD_DV_HDMI_VIDEOSTD_720x480p_240Hz = 24,   /*! 720x480p     @ 240Hz VIC=57 */
	VHD_DV_HDMI_VIDEOSTD_720x480i_120Hz = 25,   /*! 720x480i     @ 120Hz VIC=59 */
	VHD_DV_HDMI_VIDEOSTD_1280x720p_24Hz = 26,   /*! 1280x720p    @  24Hz VIC=60 */
	VHD_DV_HDMI_VIDEOSTD_1280x720p_25Hz = 27,   /*! 1280x720p    @  25Hz VIC=61 */
	VHD_DV_HDMI_VIDEOSTD_1280x720p_30Hz = 28,   /*! 1280x720p    @  30Hz VIC=62 */
	VHD_DV_HDMI_VIDEOSTD_1920x1080p_120Hz = 29, /*! 1920x1080p   @ 120Hz VIC=63 */
	VHD_DV_HDMI_VIDEOSTD_1920x1080p_100Hz = 30, /*! 1920x1080p   @ 100Hz VIC=64 */
	VHD_DV_HDMI_VIDEOSTD_3840x2160p_30Hz = 31,  /*! 3840x2160p   @  30Hz VIC=95 */
	VHD_DV_HDMI_VIDEOSTD_3840x2160p_25Hz = 32,  /*! 3840x2160p   @  25Hz VIC=94 */
	VHD_DV_HDMI_VIDEOSTD_3840x2160p_24Hz = 33,  /*! 3840x2160p   @  24Hz VIC=93 */
	VHD_DV_HDMI_VIDEOSTD_4096x2160p_24Hz = 34,  /*! 4096x2160p   @  24Hz VIC=98 */
	VHD_DV_HDMI_VIDEOSTD_3840x2160p_50Hz = 35,  /*! 3840x2160p   @  50Hz VIC=96 */
	VHD_DV_HDMI_VIDEOSTD_3840x2160p_60Hz = 36,  /*! 3840x2160p   @  60Hz VIC=97 */
	VHD_DV_HDMI_VIDEOSTD_4096x2160p_25Hz = 37,  /*! 4096x2160p   @  25Hz VIC=99 */
	VHD_DV_HDMI_VIDEOSTD_4096x2160p_30Hz = 38,  /*! 4096x2160p   @  30Hz VIC=100*/
	VHD_DV_HDMI_VIDEOSTD_4096x2160p_50Hz = 39,  /*! 4096x2160p   @  50Hz VIC=101*/
	VHD_DV_HDMI_VIDEOSTD_4096x2160p_60Hz = 40,  /*! 4096x2160p   @  60Hz VIC=102*/
	NB_VHD_DV_HDMI_VIDEOSTD
};

enum class VHD_DV_CS : uint32
{
	VHD_DV_CS_RGB_FULL = 0, /*! RGB full color space */
	VHD_DV_CS_YUV601 = 2,   /*! YUV 601 (SD) color space */
	VHD_DV_CS_YUV709 = 3,   /*! YUV 709 (HD) color space */
	NB_VHD_DV_CS = 17
};

enum class VHD_DV_SAMPLING : uint32
{
	VHD_DV_SAMPLING_4_2_2_12BITS = 6, /*! 4-2-2 12-bit sampling */
	VHD_DV_SAMPLING_4_4_4_8BITS = 9,  /*! 4-4-4  8-bit sampling */
	NB_VHD_DV_SAMPLING = 13,
};

enum class VHD_DV_STANDARD : uint32
{
	VHD_DV_STD_SMPTE = 3,             /*! SMPTE standard (CEA-861) */
	NB_VHD_DV_STANDARD = 6
};

enum class VHD_DV_MODE : uint32
{
	VHD_DV_MODE_HDMI = 2,        /*! HDMI mode */
	VHD_DV_MODE_DISPLAYPORT = 3, /*! DisplayPort mode */
	NB_VHD_DV_MODES = 5
};

#pragma endregion


#pragma region Buffer

enum class VHD_SDI_BUFFERTYPE : uint32
{
	VHD_SDI_BT_VIDEO = 0, /*! SDI video frames buffer type */
};

enum class VHD_BUFFERPACKING : uint32
{
	VHD_BUFPACK_VIDEO_YUV422_8 = 0,
	VHD_BUFPACK_VIDEO_YUV422_10 = 2,
	VHD_BUFPACK_VIDEO_RGB_32 = 8,
};

#pragma endregion


#pragma region Genlock

enum class VHD_TIMER_SOURCE : uint32
{
	VHD_TIMER_SOURCE_GENLOCK = 0,
	NB_VHD_TIMER_SOURCE = 2
};

UENUM()
enum class VHD_GENLOCKSOURCE : uint32
{
	VHD_GENLOCK_LOCAL = 0,

	VHD_GENLOCK_BB0 = 3,
	VHD_GENLOCK_BB1 = 10,

	VHD_GENLOCK_RX0 = 1,
	VHD_GENLOCK_RX1 = 2,
	VHD_GENLOCK_RX2 = 4,
	VHD_GENLOCK_RX3 = 5,
	VHD_GENLOCK_RX4 = 6,
	VHD_GENLOCK_RX5 = 7,
	VHD_GENLOCK_RX6 = 8,
	VHD_GENLOCK_RX7 = 9,
	VHD_GENLOCK_RX8 = 16,
	VHD_GENLOCK_RX9 = 17,
	VHD_GENLOCK_RX10 = 18,
	VHD_GENLOCK_RX11 = 19,

	NB_VHD_GENLOCKSOURCES = 21
};

#pragma endregion


#pragma region Timecode

enum class VHD_TIMECODE_SOURCE : uint32
{
	VHD_TC_SRC_LTC_COMPANION_CARD = 0, /*! A-LTC companion card timecode source*/
	VHD_TC_SRC_LTC_ONBOARD = 1,        /*! On board timecode source*/
	NB_VHD_TC_SRC = 2
};

struct VHD_TIMECODE
{
	VHD::BYTE  Hour;         /*! Timecode hour component */
	VHD::BYTE  Minute;       /*! Timecode minute component */
	VHD::BYTE  Second;       /*! Timecode second component */
	VHD::BYTE  Frame;        /*! Timecode frame component */
	VHD::ULONG BinaryGroups; /*! Timecode binary groups component. This field comprises 8 groups of 4-bit, stored in a 32-bit variable with LSB being the LSB of BG1 until MSB being MSB of BG8 */
	VHD::BYTE  Flags;        /*! Timecode 6 flag bits, as specified by SMPTE 12M
							  * Bit 0: Drop frame flag
							  * Bit 1: Color frame flag
							  * Bit 2: Field identification flag
							  * Bit 3: Binary group flag BGF0
							  * Bit 4: Binary group flag BGF1
							  * Bit 5: Binary group flag BGF2
						      */
	VHD::BYTE  pDBB[2];      /*! Timecode distributed binary bit groups component (ATC only) */
};

enum class VHD_COMPANION_CARD_TYPE : uint32
{
	VHD_LTC_COMPANION_CARD = 0,    /*! A-LTC companion card*/
	NB_VHD_COMPANION_CARD_TYPE = 1
};

#pragma endregion


#pragma region Misc

enum class VHD_CHANNELTYPE : uint32
{
	VHD_CHNTYPE_DISABLE = 0,     /*! Channel not present in the board layout */
	VHD_CHNTYPE_HDSDI = 2,       /*! HD-SDI channel */
	VHD_CHNTYPE_3GSDI = 3,       /*! 3G-SDI channel */
	VHD_CHNTYPE_DVI = 4,         /*! DVI channel */
	VHD_CHNTYPE_ASI = 5,         /*! ASI channel */
	VHD_CHNTYPE_HDMI = 6,        /*! HDMI channel */
	VHD_CHNTYPE_DISPLAYPORT = 7, /*! Display port channel */
	VHD_CHNTYPE_12GSDI = 8,      /*! 12G-SDI channel */
	VHD_CHNTYPE_3GSDI_ASI = 10,  /*! 3G-SDI/ASI channel */
	VHD_CHNTYPE_12GSDI_ASI = 11, /*! 12G-SDI/ASI channel */
	NB_VHD_CHANNELTYPE = 12
};

enum class VHD_CHANNEL_MODE :uint32
{
	VHD_CHANNEL_MODE_SDI = 0, /*! SDI channel mode */
	NB_VHD_CHANNEL_MODE = 2
};

enum class VHD_TRANSFERSCHEME : uint32
{
	VHD_TRANSFER_SLAVED = 1, /*! Reception stream always provides oldest captured slot. No effect in transmission */
};

#pragma endregion


#pragma region Function definitions

typedef VHD::ULONG (*VHD_GetApiInfo)(VHD::ULONG *ApiVersion, VHD::ULONG *NbBoards);
typedef VHD::ULONG (*VHD_GetVideoCharacteristics)(VHD_VIDEOSTANDARD VideoStandard, VHD::ULONG *Width, VHD::ULONG *Height, VHD::Bool *Interlaced, VHD::ULONG *FrameRate);
typedef VHD::ULONG (*VHD_GetHdmiVideoCharacteristics)(VHD_DV_HDMI_VIDEOSTANDARD VideoStd, VHD::ULONG *Width, VHD::ULONG *Height, VHD::Bool *Interlaced, VHD::ULONG *FrameRate);

typedef VHD::ULONG (*VHD_OpenBoardHandle)(VHD::ULONG BoardIndex, VHDHandle *BoardHandle, VHDHandle OnStateChangeEvent, VHD::ULONG StateChangeMask);
typedef VHD::ULONG (*VHD_CloseBoardHandle)(VHDHandle BoardHandle);

typedef const char* (*VHD_GetBoardModel)(VHD::ULONG BoardIndex);
typedef VHD::ULONG(*VHD_GetBoardProperty)(VHDHandle BoardHandle, VHD::ULONG Property, VHD::ULONG* Value);
typedef VHD::ULONG(*VHD_GetBoardCapability)(VHDHandle BoardHandle, VHD_CORE_BOARD_CAPABILITY BoardCapability, VHD::ULONG* Value);
typedef VHD::ULONG(*VHD_GetBoardCapSDIVideoStandard)(VHDHandle BoardHandle, VHD_STREAMTYPE StreamType, VHD_VIDEOSTANDARD VideoStandard, VHD::Bool* IsCapable);
typedef VHD::ULONG(*VHD_GetBoardCapSDIInterface)(VHDHandle BoardHandle, VHD_STREAMTYPE StreamType, VHD_INTERFACE Interface, VHD::Bool* IsCapable);

typedef VHD::ULONG (*VHD_SetBoardProperty)(VHDHandle BoardHandle, VHD::ULONG Property, VHD::ULONG Value);

typedef VHD::ULONG (*VHD_OpenStreamHandle)(VHDHandle BoardHandle, VHD::ULONG StreamType, VHD::ULONG ProcessingMode, VHD::Bool *SetupLock, VHDHandle *StreamHandle, VHDHandle OnDataReadyEvent);
typedef VHD::ULONG (*VHD_CloseStreamHandle)(VHDHandle StreamHandle);

typedef VHD::ULONG (*VHD_GetStreamProperty)(VHDHandle StreamHandle, VHD::ULONG Property, VHD::ULONG *Value);

typedef VHD::ULONG (*VHD_SetStreamProperty)(VHDHandle StreamHandle, VHD::ULONG Property, VHD::ULONG Value);

typedef VHD::ULONG(*VHD_SetStreamProperty)(VHDHandle StreamHandle, VHD::ULONG Property, VHD::ULONG Value);
typedef VHD::ULONG(*VHD_PresetTimingStreamProperties)(VHDHandle StreamHandle, VHD_DV_STANDARD VideoStandard, VHD::ULONG ActiveWidth, VHD::ULONG ActiveHeight, VHD::ULONG RefreshRate, VHD::Bool Interlaced);

typedef VHD::ULONG (*VHD_StartStream)(VHDHandle StreamHandle);
typedef VHD::ULONG (*VHD_StopStream)(VHDHandle StreamHandle);

typedef VHD::ULONG (*VHD_LockSlotHandle)(VHDHandle StreamHandle, VHDHandle *SlotHandle);
typedef VHD::ULONG (*VHD_UnlockSlotHandle)(VHDHandle SlotHandle);

typedef VHD::ULONG (*VHD_GetSlotBuffer)(VHDHandle SlotHandle, VHD::ULONG BufferType, VHD::BYTE **Buffer, VHD::ULONG *BufferSize);

typedef VHD::ULONG(*VHD_GetSlotTimecode)(VHDHandle SlotHandle, VHD_TIMECODE_SOURCE TimecodeSource, VHD_TIMECODE* TimeCode);
typedef VHD::ULONG (*VHD_GetTimecode)(VHDHandle BoardHandle, VHD_TIMECODE_SOURCE TcSource, VHD::Bool *Locked, float *FrameRate, VHD_TIMECODE *TimeCode);
typedef VHD::ULONG (*VHD_DetectCompanionCard)(VHDHandle BoardHandle, VHD_COMPANION_CARD_TYPE CompanionCardType, VHD::Bool *IsPresent);

typedef VHD::ULONG (*VHD_StartTimer)(VHDHandle BoardHandle, VHD_TIMER_SOURCE Source, VHDHandle *TimerHandle);
typedef VHD::ULONG (*VHD_WaitOnNextTimerTick)(VHDHandle TimerHandle, VHD::ULONG Timeout);
typedef VHD::ULONG (*VHD_StopTimer)(VHDHandle TimerHandle);

#pragma endregion
