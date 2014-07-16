
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTypes.h"
#include <stdio.h>

static const size_t kBufferSize = 256;

#define LOG_TAG "skia"
#include <android/log.h>

static bool gSkDebugToStdOut = false;

extern "C" void AndroidSkDebugToStdOut(bool debugToStdOut) {
    gSkDebugToStdOut = debugToStdOut;
}

void SkDebugf(const char format[], ...) {
    va_list args1, args2;
    va_start(args1, format);
    va_copy(args2, args1);
    __android_log_vprint(ANDROID_LOG_DEBUG, LOG_TAG, format, args1);

    // Print debug output to stdout as well.  This is useful for command
    // line applications (e.g. skia_launcher)
    if (gSkDebugToStdOut) {
        vprintf(format, args2);
    }

    va_end(args1);
    va_end(args2);
}
