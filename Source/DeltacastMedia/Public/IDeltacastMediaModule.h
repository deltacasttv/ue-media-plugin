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

#include "Logging/LogMacros.h"
#include "Modules/ModuleInterface.h"
#include "Stats/Stats.h"


DECLARE_LOG_CATEGORY_EXTERN(LogDeltacastMedia, Log, All);
DECLARE_STATS_GROUP(TEXT("Deltacast"), STATGROUP_Deltacast, STATCAT_Advanced);


class IDeltacastMediaModule : public IModuleInterface
{
public:
	/** @return true if the Deltacast module and its dependencies could be loaded */
	virtual bool IsInitialized() const = 0;

	/** @return true if the Deltacast card can be used */
	virtual bool CanBeUsed() const = 0;
};
