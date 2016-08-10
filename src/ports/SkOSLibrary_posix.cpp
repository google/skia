/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkTypes.h"
#if !defined(SK_BUILD_FOR_WIN32)

#include "SkOSLibrary.h"

#include <dlfcn.h>

void* DynamicLoadLibrary(const char* libraryName) {
    void* result = dlopen(libraryName, RTLD_LAZY);
    if (!result) {
        SkDebugf("Error loading %s {\n %s\n}\n", libraryName, dlerror());
    }
    return result;
}

void* GetProcedureAddress(void* library, const char* functionName) {
    return dlsym(library, functionName);
}
#endif//!defined(SK_BUILD_FOR_WIN32)
