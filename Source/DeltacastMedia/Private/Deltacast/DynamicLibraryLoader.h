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

#ifndef DYNAMIC_LIBRARY_LOADER_HEADER
#define DYNAMIC_LIBRARY_LOADER_HEADER

#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <memory>



namespace Deltacast
{
	namespace DynamicLibrary
	{
		enum class DynamicLibraryStatus
		{
			ok = 0,
			library_load_failed,
			get_function_address_failed,
			not_loaded,
		};


		class DynamicLibraryLoaderBase
		{
		public:
			DynamicLibraryLoaderBase() = default;
			virtual ~DynamicLibraryLoaderBase();

		public:
			virtual DynamicLibraryStatus Load(const wchar_t *LibraryName);
			virtual DynamicLibraryStatus Load(const char *LibraryName);

			void Unload();

		public:
			[[nodiscard]] bool IsLoaded() const;

		public:
			template <typename FunctionPointer>
			[[nodiscard]] FunctionPointer GetFunctionPointer(const wchar_t *FunctionName, DynamicLibraryStatus *Status = nullptr) const;

			template <typename FunctionPointer>
			[[nodiscard]] FunctionPointer GetFunctionPointer(const char *FunctionName) const;

		private:
			[[nodiscard]] void *GetFunctionPointer_Impl(const char *FunctionName) const;

		private:
			void *m_LibHandle = nullptr;
		};


		class DynamicLibraryLoader : public DynamicLibraryLoaderBase
		{
		public:
			DynamicLibraryLoader()                   = default;
			virtual ~DynamicLibraryLoader() override = default;

			virtual DynamicLibraryStatus Load(const wchar_t*LibraryName) override;
			virtual DynamicLibraryStatus LoadAllFunctionAddress() = 0;
		};

		template <typename FunctionPointer>
		FunctionPointer DynamicLibraryLoaderBase::GetFunctionPointer(const wchar_t *FunctionName, DynamicLibraryStatus *Status) const
		{
			const auto FunctionNameSize      = std::wcslen(FunctionName);
			const auto ConvertedFunctionName = std::make_unique<char[]>(FunctionNameSize + 1);

			std::memset(ConvertedFunctionName.get(), '\0', FunctionNameSize + 1);

#ifdef WIN32
			size_t ConvertedSize = 0;
			wcstombs_s(&ConvertedSize, ConvertedFunctionName.get(), FunctionNameSize + 1, FunctionName, FunctionNameSize);
#else
			wcstombs(ConvertedFunctionName.get(), FunctionName, FunctionNameSize);
#endif

			FunctionPointer Function = reinterpret_cast<FunctionPointer>(GetFunctionPointer_Impl(ConvertedFunctionName.get()));

			if (Status)
				*Status = Function == nullptr ? DynamicLibraryStatus::get_function_address_failed : DynamicLibraryStatus::ok;

			return Function;
		}

		template <typename FunctionPointer>
		FunctionPointer DynamicLibraryLoaderBase::GetFunctionPointer(const char* FunctionName) const
		{
			if (!IsLoaded())
			{
				return nullptr;
			}

			return static_cast<FunctionPointer>(GetFunctionPointer_Impl(FunctionName));
		}		
	}
}

#endif
