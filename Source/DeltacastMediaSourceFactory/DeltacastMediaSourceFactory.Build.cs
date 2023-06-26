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

namespace UnrealBuildTool.Rules
{
   public class DeltacastMediaSourceFactory : ModuleRules
   {
      public DeltacastMediaSourceFactory(ReadOnlyTargetRules Target) : base(Target)
      {
         DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
               "DeltacastMediaSource",
               "Media",
            });

         PublicDependencyModuleNames.AddRange(
            new string[]
            {
               "Core",
               "CoreUObject",
            });

         PrivateDependencyModuleNames.AddRange(
            new string[]
            {
               "DeltacastMedia",
               "MediaAssets",
               "MediaIOCore",
               "Projects",
            });

         PrivateIncludePathModuleNames.AddRange(
            new string[]
            {
               "DeltacastMedia",
               "DeltacastMediaSource",
               "Media",
            });

         PrivateIncludePaths.AddRange(
            new string[]
            {
               "DeltacastMediaSourceFactory/Private",
               "DeltacastMedia/Private/Deltacast",
            });
      }
   }
}