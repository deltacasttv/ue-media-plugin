# Package creation

## Windows

1. Open a project with the plugin to package included in the project files.
	- Typically found in `{.uproject directory}/Plugins/DeltacastMedia`.
2. Open the _"Plugins"_ window
3. Search for the plugin in the list
4. Click _"Package"_

## Linux

This is done via the exact steps used for Windows but the Linux target platform should be installed as a pre-requisite
as well as the [cross-compilation toolchain](https://dev.epicgames.com/documentation/en-us/unreal-engine/linux-development-requirements-for-unreal-engine#version-history).

This can be installed from the Epic game launcher by doing:

1. Go to the "Library" tab
2. For the engine version, click "Options" in the drop-down
3. Select the target platform "Linux" and apply

_Note: this must be installed for each version of the engine_