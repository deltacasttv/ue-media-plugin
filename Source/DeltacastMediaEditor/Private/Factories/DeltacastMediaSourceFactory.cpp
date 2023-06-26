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

#include "DeltacastMediaSourceFactory.h"

#include "AssetTypeCategories.h"
#include "DeltacastMediaSource.h"


UDeltacastMediaSourceFactory::UDeltacastMediaSourceFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UDeltacastMediaSource::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}


UObject* UDeltacastMediaSourceFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UDeltacastMediaSource>(InParent, InClass, InName, Flags);
}

uint32 UDeltacastMediaSourceFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Media;
}


bool UDeltacastMediaSourceFactory::ShouldShowInNewMenu() const
{
	return true;
}
