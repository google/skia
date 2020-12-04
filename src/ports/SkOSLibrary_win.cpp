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

void* SkLoadDynamicLibrary(const char* libraryName) {
    return LoadLibraryA(libraryName);
}

void* SkGetProcedureAddress(void* library, const char* functionName) {
    return reinterpret_cast<void*>(::GetProcAddress((HMODULE)library, functionName));
}

bool SkFreeDynamicLibrary(void* library) {
    return FreeLibrary((HMODULE)library);
}

#endif//defined(SK_BUILD_FOR_WIN)
