/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFeatures.h"
#include "include/private/base/SkLog.h"

#if defined(SK_BUILD_FOR_ANDROID)

#include <stdio.h>
#include <cstdarg>

#ifdef LOG_TAG
  #undef LOG_TAG
#endif
#define LOG_TAG "skia"
#include <android/log.h>

// Print debug output to stdout as well.  This is useful for command line
// applications
bool gSkDebugToStdOut = false;

void SkLogVAList(SkLogPriority priority, const char format[], va_list args) {
    va_list args_copy;

    if (gSkDebugToStdOut) {
        va_copy(args_copy, args);
        vprintf(format, args_copy);
        va_end(args_copy);
        fflush(stdout);
    }

    int android_priority;
    switch (priority) {
        case SkLogPriority::kFatal:   android_priority = ANDROID_LOG_FATAL;   break;
        case SkLogPriority::kError:   android_priority = ANDROID_LOG_ERROR;   break;
        case SkLogPriority::kWarning: android_priority = ANDROID_LOG_WARN;    break;
        case SkLogPriority::kInfo:    android_priority = ANDROID_LOG_INFO;    break;
        case SkLogPriority::kDebug:   android_priority = ANDROID_LOG_DEBUG;   break;
        default:                      android_priority = ANDROID_LOG_DEBUG;   break;
    }

// Forces all Render Engine logs to show as warnings. This hack should be dropped as we update
// SkDebugf call sites to have more precise priority.
#if defined(SK_IN_RENDERENGINE)
    android_priority = ANDROID_LOG_WARN;
#endif

    __android_log_vprint(android_priority, LOG_TAG, format, args);
}

#endif  // defined(SK_BUILD_FOR_ANDROID)
