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

#include "DynamicLibraryLoader.h"

#ifdef WIN32
#include <direct.h>
#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#include <windows.h>
#undef NOMINMAX
#else
#include <windows.h>
#endif

#else
#include <dlfcn.h>
#include <cstdio>
#endif

#include <array>

#ifndef MAX_PATH
#define MAX_PATH  260 // Max filename length (Windows and Linux)
#endif

namespace Deltacast::DynamicLibrary
{
	DynamicLibraryLoaderBase::~DynamicLibraryLoaderBase()
	{
		Unload();
	}


	DynamicLibraryStatus DynamicLibraryLoaderBase::Load(const wchar_t* LibraryName)
	{
#ifdef WIN32
		m_LibHandle = LoadLibraryW(LibraryName);

		if (m_LibHandle == nullptr)
		{
			auto Message = std::array<wchar_t, MAX_PATH + 20>();
			swprintf(Message.data(), Message.size(), L"Cannot load %ls", LibraryName);
			OutputDebugStringW(Message.data());

			const DWORD Error = GetLastError();
			swprintf(Message.data(), Message.size(), L"GetLastError -> 0x%08x - %u", static_cast<unsigned int>(Error), static_cast<unsigned int>(Error));
			OutputDebugStringW(Message.data());

			return DynamicLibraryStatus::library_load_failed;
		}
#else
		const auto LibraryNameSize = std::min(size_t{ MAX_PATH }, wcslen(LibraryName));

		std::array<char, MAX_PATH> ConvertedLibraryName;
		ConvertedLibraryName.fill('\0');

		// Convert from `wchar_t` -> `char`
#ifdef WIN32
		size_t ConvertedSize = 0;
		wcstombs_s(&ConvertedSize, ConvertedLibraryName.data(), ConvertedLibraryName.size(), LibraryName, LibraryNameSize);
#else
		wcstombs(ConvertedLibraryName.data(), LibraryName, LibraryNameSize);
#endif

		m_LibHandle = dlopen(ConvertedLibraryName.data(), RTLD_LAZY | RTLD_GLOBAL);

		if (m_LibHandle == nullptr)
		{
			printf("Cannot load %s\n%s\n", ConvertedLibraryName.data(), dlerror());

			return DynamicLibraryStatus::library_load_failed;
		}
#endif

		return DynamicLibraryStatus::ok;
	}

	DynamicLibraryStatus DynamicLibraryLoaderBase::Load(const char* LibraryName)
	{
		const auto LibraryNameSize = strlen(LibraryName);
		const auto ConvertedLibraryName = std::make_unique<wchar_t[]>(LibraryNameSize + 1);

		// Convert from `char` -> `wchar_t`
#ifdef WIN32
		size_t ConvertedSize = 0;
		mbstowcs_s(&ConvertedSize, ConvertedLibraryName.get(), LibraryNameSize + 1, LibraryName, LibraryNameSize);
#else
		mbstowcs(ConvertedLibraryName.get(), LibraryName, LibraryNameSize);
#endif

		return DynamicLibraryLoaderBase::Load(ConvertedLibraryName.get());
	}


	void DynamicLibraryLoaderBase::Unload()
	{
		if (m_LibHandle != nullptr)
		{
#ifdef WIN32
			FreeLibrary((HMODULE)m_LibHandle);
#else
			dlclose(m_LibHandle);
#endif

			m_LibHandle = nullptr;
		}
	}


	bool DynamicLibraryLoaderBase::IsLoaded() const
	{
		return m_LibHandle != nullptr;
	}


	void *DynamicLibraryLoaderBase::GetFunctionPointer_Impl(const char *FunctionName) const
	{
#ifdef WIN32
		void *Function = GetProcAddress((HMODULE)m_LibHandle, FunctionName);

		if (Function == nullptr)
		{
			std::string Message = "GetFunctionPointer failed: ";
			Message += FunctionName;
			OutputDebugStringA(Message.c_str());
		}
#else
		dlerror(); // clear if an error happen before
		void* Function = dlsym(m_LibHandle, FunctionName);

		if (Function == nullptr)
		{
			printf("GetFunctionPointer failed: %s\n\t%s\n", FunctionName, dlerror());
		}
#endif

		return Function;
	}


	DynamicLibraryStatus DynamicLibraryLoader::Load(const wchar_t* LibraryName)
	{
		if (IsLoaded())
			return DynamicLibraryStatus::ok;

		DynamicLibraryStatus LibraryStatus = DynamicLibraryLoaderBase::Load(LibraryName);

		if (LibraryStatus == DynamicLibraryStatus::ok)
			LibraryStatus = LoadAllFunctionAddress();

		return LibraryStatus;
	}
}
