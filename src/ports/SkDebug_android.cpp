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

    // Calls to SkDebugf in Android's SurfaceFligner / RenderEngine are often critical for
    // investigating high-priority issues, even for production builds. This temporary hack forces
    // all Skia logging in RenderEngine to the WARN level, which ensures easier filtering in bug
    // reports, and anecdotally may also reduce the chance of dropping logs (TBD). HWUI and other
    // non-framework builds of Skia are currently left at DEBUG to de-risk potential log spam.
    //
    // TODO: consider printing to logcat's "system" log buffer instead of the app-available "main"
    // buffer when Skia is used in SurfaceFlinger. This would require turning SkDebugf (or its
    // replacement) into a variadic templated function to call __android_log_buf_print, or for
    // Android's liblog to expose its internal __android_log_buf_vprint, which accepts a va_list.
    // The "system" buffer would only be available when RenderEngine is built into SurfaceFlinger,
    // and could not be used in e.g. tests, benchmarks, etc.
#if defined(SK_IN_RENDERENGINE)
    __android_log_vprint(ANDROID_LOG_WARN, LOG_TAG, format, args1);
#else
    __android_log_vprint(ANDROID_LOG_DEBUG, LOG_TAG, format, args1);
#endif  // defined(SK_IN_RENDERENGINE)

    va_end(args1);
}

#endif  // defined(SK_BUILD_FOR_ANDROID)
