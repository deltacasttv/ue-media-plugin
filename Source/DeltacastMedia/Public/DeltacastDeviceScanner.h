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

#include "DeltacastDefinition.h"
#include "DeltacastHelpers.h"
#include "HAL/Platform.h"

#include <optional>


/**
 * Native data format.
 */
UENUM()
enum class EDeltacastMediaPixelFormat : uint8
{
	PF_8BIT_RGBA UMETA(DisplayName = "8bit RGBA"),
	PF_8BIT_YUV422 UMETA(DisplayName = "8bit YUV"),
	PF_10BIT_YUV422 UMETA(DisplayName = "10bit YUV"),
};


namespace Deltacast::Device
{
	namespace Config
	{
		class DELTACASTMEDIA_API FBasePortConfig
		{
		public:
			[[nodiscard]] bool IsValid(VHDHandle BoardHandle) const;

			[[nodiscard]] FString ToString() const;

		public:
			uint32 BoardIndex = 0;
			uint32 PortIndex  = 0;

			bool bIsInput = false;

			bool bIsEuropeanClock = true;

		public:
			uint32 BufferDepth = 8;

			VHD_BUFFERPACKING BufferPacking = VHD_BUFFERPACKING::VHD_BUFPACK_VIDEO_YUV422_8;
		};

		class DELTACASTMEDIA_API FSdiPortConfig
		{
		public:
			[[nodiscard]] bool IsValid(VHDHandle BoardHandle) const;

			[[nodiscard]] FString ToString() const;


			[[nodiscard]] bool IsSingleLink() const;

			[[nodiscard]] bool IsDual() const;

			[[nodiscard]] Deltacast::Helpers::EQuadLinkType GetQuadLinkType() const;

			[[nodiscard]] VHD_STREAMTYPE GetStreamType() const;

		public:
			FBasePortConfig Base;

			VHD_VIDEOSTANDARD VideoStandard;
			VHD_INTERFACE Interface;
		};

		class DELTACASTMEDIA_API FDvPortConfig
		{
		public:
			[[nodiscard]] bool IsValid(VHDHandle BoardHandle) const;

			[[nodiscard]] FString ToString() const;

		public:
			FBasePortConfig Base;

			VHD_DV_HDMI_VIDEOSTANDARD VideoStandard;
		};
	}


	class DELTACASTMEDIA_API FSdiConfigIterator
	{
	public:
		explicit FSdiConfigIterator(const uint32 BoardIndex);

		FSdiConfigIterator end() const;

	public:
		Config::FSdiPortConfig operator*() const;

		FSdiConfigIterator& operator++();

	public:
		friend bool operator!=(const FSdiConfigIterator &Lhs, const FSdiConfigIterator &Rhs);

	private:
		uint32 BoardIndex = 0;

		bool bIsInput = true;
		uint32 PortIndex = 0;

		bool bIsEuropeanClock = true;

		size_t InterfaceIndex = 0;
		size_t VideoStandardIndex = 0;

	private:
		bool IsEnd = false;
	};


	class DELTACASTMEDIA_API FSdiDeviceScanner
	{
	public:
		static std::optional<FSdiDeviceScanner> GetDeviceScanner(uint32 BoardIndex);

	public:
		FSdiConfigIterator begin() const;

		FSdiConfigIterator end() const;

	private:
		explicit FSdiDeviceScanner(uint32 BoardIndex);

	private:
		uint32 BoardIndex;
	};


	class DELTACASTMEDIA_API FDvConfigIterator
	{
	public:
		explicit FDvConfigIterator(const uint32 BoardIndex);

		FDvConfigIterator end() const;

	public:
		Config::FDvPortConfig operator*() const;

		FDvConfigIterator& operator++();

	public:
		friend bool operator!=(const FDvConfigIterator& Lhs, const FDvConfigIterator& Rhs);

	private:
		uint32 BoardIndex = 0;

		bool bIsInput = true;
		uint32 PortIndex = 0;

		bool bIsEuropeanClock = true;

		uint32 VideoStandardIndex = 0;

	private:
		bool IsEnd = false;
	};

	class DELTACASTMEDIA_API FDvDeviceScanner
	{
	public:
		static std::optional<FDvDeviceScanner> GetDeviceScanner(uint32 BoardIndex);

	public:
		FDvConfigIterator begin() const
		{
			return FDvConfigIterator(BoardIndex);
		}
		
		FDvConfigIterator end() const
		{
			return FDvConfigIterator(BoardIndex).end();
		}

	private:
		explicit FDvDeviceScanner(uint32 BoardIndex);

	private:
		uint32 BoardIndex;
	};


	class DELTACASTMEDIA_API FDeviceScanner
	{
	public:
		static std::optional<FDeviceScanner> GetDeviceScanner(uint32 BoardIndex);

	public:
		[[nodiscard]] std::optional<FSdiDeviceScanner> GetSdiDeviceScanner() const;

		[[nodiscard]] std::optional<FDvDeviceScanner> GetDvDeviceScanner() const;

	private:
		explicit FDeviceScanner(uint32 BoardIndex);

	private:
		uint32 BoardIndex;
	};
}
