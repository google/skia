/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if defined(SK_BUILD_FOR_ANDROID)

#include "SkString.h"
#include "SkTArray.h"
#include "SkTemplates.h"
#include <stdio.h>

#define LOG_TAG "skia"
#include <android/log.h>

// Print debug output to stdout as well.  This is useful for command line
// applications (e.g. skia_launcher).
bool gSkDebugToStdOut = false;

void SkDebugf(const char format[], ...) {
    va_list args1, args2;
    va_start(args1, format);

    if (gSkDebugToStdOut) {
        va_copy(args2, args1);
        vprintf(format, args2);
        va_end(args2);
    }

    static constexpr int kStrStackSize = 1024;
    SkAutoSTMalloc<kStrStackSize, char> msg;
    int length = vsnprintf(msg, kStrStackSize, format, args1);
    va_end(args1);
    if (length <= 0) {
        return;
    }
    if (length >= kStrStackSize) {
        va_start(args1, format);
        vsnprintf(msg.reset(length+1), length+1, format, args1);
        va_end(args1);
    }

    SkSTArray<16, SkString> lines;
    SkStrSplit(msg, "\n", kStrict_SkStrSplitMode, &lines);

    // Print the message one line at a time so it doesn't get truncated by the android log.
    for (const SkString& line : lines) {
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", line.c_str());
    }

    va_end(args1);
}

#endif//defined(SK_BUILD_FOR_ANDROID)
