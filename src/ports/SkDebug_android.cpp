
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
    va_list args;
    va_start(args, format);
    __android_log_vprint(ANDROID_LOG_DEBUG, LOG_TAG, format, args);

    // Print debug output to stdout as well.  This is useful for command
    // line applications (e.g. skia_launcher)
    if (gSkDebugToStdOut) {
        vprintf(format, args);
    }

    va_end(args);
}
