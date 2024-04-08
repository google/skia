/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFeatures.h"

#if defined(SK_BUILD_FOR_ANDROID)

#include <stdio.h>

#ifdef LOG_TAG
  #undef LOG_TAG
#endif
#define LOG_TAG "skia"
#include <android/log.h>

// Print debug output to stdout as well.  This is useful for command line
// applications
bool gSkDebugToStdOut = false;

void SkDebugf(const char format[], ...) {
    va_list args1, args2;
    va_start(args1, format);

    if (gSkDebugToStdOut) {
        va_copy(args2, args1);
        vprintf(format, args2);
        va_end(args2);
        fflush(stdout);
    }

    __android_log_vprint(ANDROID_LOG_DEBUG, LOG_TAG, format, args1);

    va_end(args1);
}

#endif  // defined(SK_BUILD_FOR_ANDROID)
