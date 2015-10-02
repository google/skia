
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOSLibrary_DEFINED
#define SkOSLibrary_DEFINED

void* DynamicLoadLibrary(const char* libraryName);
void* GetProcedureAddress(void* library, const char* functionName);

#endif

