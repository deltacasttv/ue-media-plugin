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
#include "IMediaTextureSample.h"

enum class EDeltacastEncodePixelFormat
{
	YUVK4224_8bits,
	YUVK4224_10bits,
};

class DELTACASTMEDIAOUTPUT_API FDeltacastMediaEncodeTime final
{
public:
	FDeltacastMediaEncodeTime(EDeltacastEncodePixelFormat InFormat, void* InBuffer, uint32 InPitch, uint32 InWidth, uint32 InHeight);

public:
	void Render(uint32 InHours, uint32 InMinutes, uint32 InSeconds, uint32 InFrames) const;

private:
	void DrawChar(uint32 InX, uint32 InChar) const;
	void DrawTime(uint32 InX, uint32 InTime) const;
	void SetPixelScaled(uint32 InX, uint32 InY, bool InSet, uint32 InScale) const;
	void SetPixel(uint32 InX, uint32 InY, bool InSet) const;

private:
	EDeltacastEncodePixelFormat Format;

	void* Buffer;

	uint32 Pitch;
	uint32 Width;
	uint32 Height;
};