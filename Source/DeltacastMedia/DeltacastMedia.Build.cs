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

using UnrealBuildTool;

public class DeltacastMedia : ModuleRules
{
   public DeltacastMedia(ReadOnlyTargetRules Target) : base(Target)
   {
      PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

      bUseUnity = false;

      PublicIncludePaths.AddRange(
         new string[]
         {
            // ... add public include paths required here ...
         }
      );


      PrivateIncludePaths.AddRange(
         new string[]
         {
            // ... add other private include paths required here ...
            "DeltacastMedia/Private",
            "DeltacastMedia/Private/Deltacast",
            "DeltacastMedia/Private/Timecode",
            "DeltacastMedia/Private/TimeStep",
         }
      );


      PublicDependencyModuleNames.AddRange(
         new string[]
         {
            // ... add other public dependencies that you statically link with here ...
            "Core",
            "MediaIOCore",
            "TimeManagement",
         }
      );


      PrivateDependencyModuleNames.AddRange(
         new string[]
         {
            // ... add private dependencies that you statically link with here ...
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore",
         }
      );


      DynamicallyLoadedModuleNames.AddRange(
         new string[]
         {
            // ... add any modules that your module loads dynamically here ...
         }
      );

      if (Target.Type == TargetRules.TargetType.Editor)
      {
         DynamicallyLoadedModuleNames.Add("Settings");
         PrivateIncludePathModuleNames.Add("Settings");
      }
   }
}