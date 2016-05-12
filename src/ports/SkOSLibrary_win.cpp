
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"
#if defined(SK_BUILD_FOR_WIN32)

#include "SkOSLibrary.h"
#include <windows.h>

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
    return ::GetProcAddress((HMODULE)library, functionName);
}

#endif//defined(SK_BUILD_FOR_WIN32)
