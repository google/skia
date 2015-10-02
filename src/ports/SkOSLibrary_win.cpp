
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkOSLibrary.h"
#include <windows.h>

void* DynamicLoadLibrary(const char* libraryName) {
    return LoadLibrary(libraryName);
}

void* GetProcedureAddress(void* library, const char* functionName) {
    return ::GetProcAddress((HMODULE)library, functionName);
}
