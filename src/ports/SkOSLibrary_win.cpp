/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_WIN)

#include "src/core/SkLeanWindows.h"
#include "src/ports/SkOSLibrary.h"

void* DynamicLoadLibrary(const char* libraryName) {
#ifdef SK_BUILD_FOR_WINRT
    int str_len = ::MultiByteToWideChar(CP_UTF8, 0, libraryName, -1, nullptr, 0);
    wchar_t *wideLibraryName = new wchar_t[str_len];
    ::MultiByteToWideChar(CP_UTF8, 0, libraryName, -1, wideLibraryName, str_len);

    return LoadPackagedLibrary(wideLibraryName, 0);
#else // SK_BUILD_FOR_WINRT
    return LoadLibraryA(libraryName);
#endif // SK_BUILD_FOR_WINRT
}

void* GetProcedureAddress(void* library, const char* functionName) {
    return reinterpret_cast<void*>(::GetProcAddress((HMODULE)library, functionName));
}

#endif//defined(SK_BUILD_FOR_WIN)
