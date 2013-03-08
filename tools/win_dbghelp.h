/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef win_dbghelp_DEFINED
#define win_dbghelp_DEFINED

#ifdef SK_BUILD_FOR_WIN32

#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>

void setAppName(const char* app_name);
const char* getAppName();

void setBinariesPath(const char* binaries_path);
const char* getBinariesPath();

void setAppVersion(const char* version);
const char* getAppVersion();

void setCdbPath(const char* path);
const char* getCdbPath();

void setUpDebuggingFromArgs(const char* vargs0);

int GenerateDumpAndPrintCallstack(EXCEPTION_POINTERS* pExceptionPointers);

#endif  // SK_BUILD_FOR_WIN32

#endif  // win_dbghelp_DEFINED
