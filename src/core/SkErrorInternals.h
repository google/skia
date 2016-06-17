/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkErrorInternals_DEFINED
#define SkErrorInternals_DEFINED

#include "SkError.h"

class SkErrorInternals {

public:
    static void ClearError();
    static void SetError(SkError code, const char *fmt, ...);
    static SkError GetLastError();
    static const char *GetLastErrorString();
    static void SetErrorCallback(SkErrorCallbackFunction cb, void *context);
    static void DefaultErrorCallback(SkError code, void *context);
};



#endif /* SkErrorInternals_DEFINED */
