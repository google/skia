/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#if !defined(SK_BUILD_FOR_WIN)

#include "src/ports/SkOSLibrary.h"

#include <dlfcn.h>

void* SkLoadDynamicLibrary(const char* libraryName) {
    return dlopen(libraryName, RTLD_LAZY);
}

void* SkGetProcedureAddress(void* library, const char* functionName) {
    return dlsym(library, functionName);
}

bool SkFreeDynamicLibrary(void* library) {
    return dlclose(library) == 0;
}

#endif//!defined(SK_BUILD_FOR_WIN)
