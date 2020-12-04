/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSLibrary_DEFINED
#define SkOSLibrary_DEFINED

void* SkLoadDynamicLibrary(const char* libraryName);
void* SkGetProcedureAddress(void* library, const char* functionName);
bool  SkFreeDynamicLibrary(void* library);

#endif
