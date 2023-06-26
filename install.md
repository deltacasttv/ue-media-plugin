# Installation basics

This document explains how to install Unreal Engine.

## Windows
1. Go to [https://www.unrealengine.com/en-US/download](https://www.unrealengine.com/en-US/download)
2. Click on the download button
	* The download may require to sign in
	* The download may install the Epic Games Launcher and not directly Unreal Engine
3. Run the downloaded installer
	* The installer may ask for specific redist to install (.NET), install them and continue
		* If the installation fails, it could be due to graphical drivers not being up-to-date or not supporting the required APIs
			* Install the lastest graphical drivers and re-try the installation process
	* The installer will automatically run the installed application
4. Follow the installed application "First steps"
	* The first steps may require to sign in
	* The first steps may required an email confirmation if it is a new account
5. _Optional_ Remove/Hide the game part of the Epic Games Launcher
	1. Go to settings
	2. Check "Hide game library"
	3. _Optional_ Uncheck "Run when computer starts"
	4. Apply the changes
6. Go to the Unreal Engine Library
7. Add a new engine version (via the yellow "+" button)
8. Select the engine version to add (e.g.: 5.1.0)
9. Go to the engine option (down arrow next to the launch button)
10. Select at least the following options:
	* Core Components
	* _Optional_ Starter Content
	* _Optional_ Templates and Feature Packs
	* Target Platforms:
		* Linux
10. Click install
	* **Warning**: The installation may take a long time
	* **Warning**: The installation will take around 60GB
12. Launch the engine
	* The first launch will setup the environment and take more time than usual
13. Create a test project
	* Use any default template
14. Run the project (via the green play button on the top toolbar)
	* If the project runs, the installation is successful

### Dev
[Follow the instructions from the UE doc](https://docs.unrealengine.com/5.1/en-US/ProductionPipelines/DevelopmentSetup/)
UE may require minimum Visual Studio version for compatibility with Unreal Engine's version

To improve the integration of UE & Visual Studio, it is highly recommended to install the [plugin `Visual Studio Integration Tool`](https://www.unrealengine.com/marketplace/en-US/product/visual-studio-integration-tool).
Once the plugin is "bought" (for free), click on the "Install to Engine" button to add the tool on the installed engine.

### Cross-Compilation
To allow packaging of the plugin in both `Win64` & `Linux`, a specific cross-compilation toolchain must be installed.

1. Go to the [cross-compilation documentation](https://docs.unrealengine.com/5.1/en-US/linux-development-requirements-for-unreal-engine/)
2. Download the appropriate version of the cross-compilation toolchain
	* **Warning**: Don't use the native toolchain
3. Install the downloaded toolchain
4. Restart the computer to allow all tools to detect the newly installed toolchain.
5. Open a `cmd` and use the following command: `%LINUX_MULTIARCH_ROOT%x86_64-unknown-linux-gnu\bin\clang++ -v`
	* The result should contain:
		- The compiler version: `clang version 13.0.1 [...]` (for UE 5.1)
		- The installed directory: `C:\UnrealToolchains\[...]` (by default)


## Linux
**Important note**: the GPU drivers must be installed!

[Follow the instructions from the UE docs](https://docs.unrealengine.com/5.1/en-US/installing-unreal-engine/)

**or**

1. Download [UE for Linux](https://www.unrealengine.com/en-US/linux)
2. Unzip the files to your chosen location
3. Launch UE Editor: `{install path}\Engine\Binaries\Linux\UnrealEditor`
	* The first launch will setup the environment and take more time than usual
4. Create a test project
	* Use any default template
5. Run the project (via the green play button on the top toolbar)
	* If the project runs, the installation is successful

### Dev
On Linux, the code editor recommended is Visual Studio Code.

1. Create a new project
2. Add a C++ class
	* UE will create a workspace descriptor file for Visual Studio Code.
3. Close UE Editor
4. Open the project via Visual Studio Code
5. Build & Run the project
