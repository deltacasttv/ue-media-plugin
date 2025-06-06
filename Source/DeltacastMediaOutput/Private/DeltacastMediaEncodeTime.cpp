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

#include "DeltacastMediaEncodeTime.h"

FDeltacastMediaEncodeTime::FDeltacastMediaEncodeTime(const EDeltacastEncodePixelFormat InFormat, void* InBuffer, const uint32 InPitch, const uint32 InWidth, const uint32 InHeight)
	: Format(InFormat), Buffer(InBuffer), Pitch(InPitch), Width(InWidth), Height(InHeight)
{
	check(Buffer);
}

// Monochrome version of Unreal Engine Small Font, 8x11 bitmap per character
// Contains: 0123456789:

static constexpr uint32 MaxCharacter = 11;
static constexpr uint32 CharacterHeight = 11;
static constexpr uint32 ColonCharacterIndex = 10;
static constexpr uint8 Font[MaxCharacter][CharacterHeight] =
{
	{0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00}, // 0
	{0x00, 0x08, 0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x3E, 0x00}, // 1
	{0x00, 0x3C, 0x42, 0x42, 0x40, 0x20, 0x18, 0x04, 0x02, 0x7E, 0x00}, // 2
	{0x00, 0x3C, 0x42, 0x40, 0x40, 0x38, 0x40, 0x40, 0x42, 0x3C, 0x00}, // 3
	{0x00, 0x20, 0x30, 0x28, 0x24, 0x22, 0x7E, 0x20, 0x20, 0x20, 0x00}, // 4
	{0x00, 0x7C, 0x04, 0x04, 0x04, 0x3C, 0x40, 0x40, 0x42, 0x3C, 0x00}, // 5
	{0x00, 0x38, 0x04, 0x02, 0x3E, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00}, // 6
	{0x00, 0x7E, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x08, 0x00}, // 7
	{0x00, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x42, 0x42, 0x42, 0x3C, 0x00}, // 8
	{0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x7C, 0x40, 0x20, 0x1C, 0x00}, // 9
	{0x00, 0x00, 0x08, 0x08, 0x00, 0x00, 0x00, 0x08, 0x08, 0x00, 0x00}, // :
};

void FDeltacastMediaEncodeTime::SetPixelScaled(const uint32 InX, const uint32 InY, const bool InSet, const uint32 InScale) const
{
	for (uint32 ScaleY = 0; ScaleY < InScale; ScaleY++)
	{
		for (uint32 ScaleX = 0; ScaleX < InScale; ScaleX++)
		{
			SetPixel(InX * InScale + ScaleX, InY * InScale + ScaleY, InSet);
		}
	}
}

void FDeltacastMediaEncodeTime::SetPixel(const uint32 InX, const uint32 InY, const bool InSet) const
{
	if ((InX >= Width) || (InY >= Height))
		return;

	if (Format == EDeltacastEncodePixelFormat::YUVK4224_8bits)
	{
		static constexpr uint32 BlockSize = 12;
		const uint32 Block = InX / 4;
		const uint32 Pixel = InX % 4;
		uint32* const BlockPointer = reinterpret_cast<uint32*>((reinterpret_cast<char*>(Buffer) + (Pitch * InY)) + (Block * BlockSize));

		const uint8_t Y = InSet ? 0xEB : 0x10;
		const uint8_t U = 0x80;
		const uint8_t V = 0x80;
		const uint8_t K = 0xEB;

		switch (Pixel)
		{
		case 0:
			BlockPointer[0] = (K << 24) | (V << 16) | (Y << 8) | U;
			break;
		case 1:
			BlockPointer[1] = (BlockPointer[1] & 0xFFFF0000) | (K << 8) | Y;
			break;
		case 2:
			BlockPointer[1] = (Y << 24) | (U << 16) | (BlockPointer[1] & 0x0000FFFF);
			BlockPointer[2] = (BlockPointer[2] & 0xFFFF0000) | (K << 8) | V;
			break;
		case 3:
			BlockPointer[2] = (K << 24) | (Y << 16) | (BlockPointer[2] & 0x0000FFFF);
			break;
		default:
			break;
		}
	}
	else if (Format == EDeltacastEncodePixelFormat::YUVK4224_10bits)
	{
		static constexpr uint32 BlockSize = 8;
		const uint32 Block = InX / 2;
		const uint32 Pixel = InX % 2;
		uint32* const BlockPointer = (reinterpret_cast<uint32*>((reinterpret_cast<char*>(Buffer) + (Pitch * InY)) + Block * BlockSize));

		const uint16_t Y = InSet ? 0x3C0 : 0x40;
		const uint16_t U = 0x200;
		const uint16_t V = 0x200;
		const uint16_t K = 0x3C0;

		switch (Pixel)
		{
		case 0:
			BlockPointer[0] = (V << 22) | (Y << 12) | (U << 2);
			BlockPointer[1] = (BlockPointer[1] & 0xFFFFF000) | (K << 2);
			break;
		case 1:
			BlockPointer[1] = (K << 22) | (Y << 12) | (BlockPointer[1] & 0x00000FFC);
			break;
		default:
			break;
		}
	}
}


void FDeltacastMediaEncodeTime::DrawChar(const uint32 InX, const uint32 InChar) const
{
	if (InChar >= MaxCharacter)
	{
		return;
	}

	for (int y = 0; y < MaxCharacter; y++)
	{
		for (int x = 0; x < 8; x++)
		{
			if (Font[InChar][y] & (1 << x))
			{
				SetPixelScaled(InX * 8 + x, y, true, 4);
			}
			else
			{
				SetPixelScaled(InX * 8 + x, y, false, 4);
			}
		}
	}
}


void FDeltacastMediaEncodeTime::DrawTime(const uint32 InX, const uint32 InTime) const
{
	const uint32 Hi = (InTime / 10) % 10;
	const uint32 Lo = InTime % 10;

	DrawChar(InX, Hi);
	DrawChar(InX + 1, Lo);
}


void FDeltacastMediaEncodeTime::Render(const uint32 InHours, const uint32 InMinutes, const uint32 InSeconds, const uint32 InFrames) const
{
	DrawTime(0, InHours);
	DrawChar(2, ColonCharacterIndex);
	DrawTime(3, InMinutes);
	DrawChar(5, ColonCharacterIndex);
	DrawTime(6, InSeconds);
	DrawChar(8, ColonCharacterIndex);
	DrawTime(9, InFrames);
}
