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

#include "DeltacastMediaShaders.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "RenderGraphBuilder.h"
#include "RHIStaticStates.h"

/** Setup YUV Offset in matrix */
FMatrix CombineColorTransformAndOffset(const FMatrix& InMatrix, const FVector& InYUVOffset)
{
	FMatrix Result = InMatrix;
	// Offset in last column:
	// 1) to allow for 4x4 matrix multiplication optimization when going from RGB to YUV (hence the 1.0 in the [3][3] matrix location)
	// 2) stored in empty space when going from YUV to RGB
	Result.M[0][3] = InYUVOffset.X;
	Result.M[1][3] = InYUVOffset.Y;
	Result.M[2][3] = InYUVOffset.Z;
	Result.M[3][3] = 1.0f;
	return Result;
}
 
/* FRGBA8toYUVK4224ConvertPS shader
 *****************************************************************************/

IMPLEMENT_GLOBAL_SHADER(FRGBA8toYUVK4224ConvertPS, "/Plugin/DeltacastMedia/Private/DeltacastMediaShaders.usf", "RGBA8toYUVK8ConvertPS", SF_Pixel);

FRGBA8toYUVK4224ConvertPS::FParameters* FRGBA8toYUVK4224ConvertPS::AllocateAndSetParameters(FRDGBuilder& GraphBuilder, FRDGTextureRef RGBATexture, const FMatrix& ColorTransform, const FVector& YUVOffset, bool bDoLinearToSrgb, FRDGTextureRef OutputTexture)
{
	FRGBA8toYUVK4224ConvertPS::FParameters* Parameters = GraphBuilder.AllocParameters<FRGBA8toYUVK4224ConvertPS::FParameters>();

	Parameters->RGBAToYUVKConversion.InputTexture = RGBATexture;
	Parameters->RGBAToYUVKConversion.InputSampler = TStaticSamplerState<SF_Point>::GetRHI();
	Parameters->RGBAToYUVKConversion.ColorTransform = (FMatrix44f)CombineColorTransformAndOffset(ColorTransform, YUVOffset);
	Parameters->RGBAToYUVKConversion.DoLinearToSrgb = bDoLinearToSrgb;
	Parameters->RGBAToYUVKConversion.OnePixelDeltaX = 1.0f / (float)RGBATexture->Desc.Extent.X;

	//Output texture will be based on a size dividable by 48 (i.e 1280 -> 1296) and divided by 6 (i.e 1296 / 6 = 216)
	//To map output texture UVs, we get a scale from the source texture original size to the mapped output size
	//And use the source texture size to get the pixel delta
	const float PaddedResolution = float(uint32((RGBATexture->Desc.Extent.X + 47) / 48) * 48);
	Parameters->PaddingScale = PaddedResolution / (float)RGBATexture->Desc.Extent.X;

	Parameters->RenderTargets[0] = FRenderTargetBinding{ OutputTexture, ERenderTargetLoadAction::ENoAction };

	return Parameters;
}
 
/* FRGBA10toYUVK4224ConvertPS shader
 *****************************************************************************/

IMPLEMENT_GLOBAL_SHADER(FRGBA16toYUVK4224ConvertPS, "/Plugin/DeltacastMedia/Private/DeltacastMediaShaders.usf", "RGBA16toYUVK10ConvertPS", SF_Pixel);

FRGBA16toYUVK4224ConvertPS::FParameters* FRGBA16toYUVK4224ConvertPS::AllocateAndSetParameters(FRDGBuilder& GraphBuilder, FRDGTextureRef RGBATexture, const FMatrix& ColorTransform, const FVector& YUVOffset, bool bDoLinearToSrgb, FRDGTextureRef OutputTexture)
{
	FRGBA16toYUVK4224ConvertPS::FParameters* Parameters = GraphBuilder.AllocParameters<FRGBA16toYUVK4224ConvertPS::FParameters>();

	Parameters->RGBAToYUVKConversion.InputTexture = RGBATexture;
	Parameters->RGBAToYUVKConversion.InputSampler = TStaticSamplerState<SF_Point>::GetRHI();
	Parameters->RGBAToYUVKConversion.ColorTransform = (FMatrix44f)CombineColorTransformAndOffset(ColorTransform, YUVOffset);
	Parameters->RGBAToYUVKConversion.DoLinearToSrgb = bDoLinearToSrgb;
	Parameters->RGBAToYUVKConversion.OnePixelDeltaX = 1.0f / (float)RGBATexture->Desc.Extent.X;

	//Output texture will be based on a size dividable by 48 (i.e 1280 -> 1296) and divided by 6 (i.e 1296 / 6 = 216)
	//To map output texture UVs, we get a scale from the source texture original size to the mapped output size
	//And use the source texture size to get the pixel delta
	const float PaddedResolution = float(uint32((RGBATexture->Desc.Extent.X + 47) / 48) * 48);
	Parameters->PaddingScale = PaddedResolution / (float)RGBATexture->Desc.Extent.X;

	Parameters->RenderTargets[0] = FRenderTargetBinding{ OutputTexture, ERenderTargetLoadAction::ENoAction };

	return Parameters;
}