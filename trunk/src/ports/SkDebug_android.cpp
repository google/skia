
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTypes.h"

static const size_t kBufferSize = 256;

#define LOG_TAG "skia"
#include <android/log.h>

void SkDebugf(const char format[], ...) {
    va_list args;
    va_start(args, format);
    __android_log_vprint(ANDROID_LOG_DEBUG, LOG_TAG, format, args);
    va_end(args);
}
