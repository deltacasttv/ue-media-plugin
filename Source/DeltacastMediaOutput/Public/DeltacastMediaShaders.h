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
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

 /** Struct of common parameters used in media capture shaders to do RGB to YUV conversions */
BEGIN_SHADER_PARAMETER_STRUCT(FRGBAToYUVKConversion, DELTACASTMEDIAOUTPUT_API)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
	SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
	SHADER_PARAMETER(FMatrix44f, ColorTransform)
	SHADER_PARAMETER(uint32, DoLinearToSrgb)
	SHADER_PARAMETER(float, OnePixelDeltaX)
END_SHADER_PARAMETER_STRUCT()

/**
 * Pixel shader to convert RGBA 8 bits to YUVK 4224 8 bits
 */
class FRGBA8toYUVK4224ConvertPS : public FGlobalShader
{
public:
	DECLARE_EXPORTED_GLOBAL_SHADER(FRGBA8toYUVK4224ConvertPS, DELTACASTMEDIAOUTPUT_API);

	SHADER_USE_PARAMETER_STRUCT(FRGBA8toYUVK4224ConvertPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(FRGBAToYUVKConversion, RGBAToYUVKConversion)
		SHADER_PARAMETER(float, PaddingScale)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

public:
	/** Allocates and setup shader parameter in the incoming graph builder */
	DELTACASTMEDIAOUTPUT_API FRGBA8toYUVK4224ConvertPS::FParameters* AllocateAndSetParameters(FRDGBuilder& GraphBuilder, FRDGTextureRef RGBATexture, const FMatrix& ColorTransform, const FVector& YUVOffset, bool bDoLinearToSrgb, FRDGTextureRef OutputTexture);
};

/**
 * Pixel shader to convert RGBA 16 bits to YUVK 4224 10bits
 */
class FRGBA16toYUVK4224ConvertPS : public FGlobalShader
{
public:
	DECLARE_EXPORTED_GLOBAL_SHADER(FRGBA16toYUVK4224ConvertPS, DELTACASTMEDIAOUTPUT_API);

	SHADER_USE_PARAMETER_STRUCT(FRGBA16toYUVK4224ConvertPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT_INCLUDE(FRGBAToYUVKConversion, RGBAToYUVKConversion)
		SHADER_PARAMETER(float, PaddingScale)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

public:
	/** Allocates and setup shader parameter in the incoming graph builder */
	DELTACASTMEDIAOUTPUT_API FRGBA16toYUVK4224ConvertPS::FParameters* AllocateAndSetParameters(FRDGBuilder& GraphBuilder, FRDGTextureRef RGBATexture, const FMatrix& ColorTransform, const FVector& YUVOffset, bool bDoLinearToSrgb, FRDGTextureRef OutputTexture);
};