/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if !defined(SK_BUILD_FOR_WIN) && !defined(SK_BUILD_FOR_ANDROID)

#include <stdarg.h>
#include <stdio.h>

#if defined(SK_INCLUDE_LIBFUZZER_ENTRYPOINTS)
  // libfuzzer output is hard to read with excess logging.
  #define SkDebugf(...) (void)
#else
void SkDebugf(const char format[], ...) {

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}
#endif//defined(SK_INCLUDE_LIBFUZZER_ENTRYPOINTS)

#endif//!defined(SK_BUILD_FOR_WIN) && !defined(SK_BUILD_FOR_ANDROID)
