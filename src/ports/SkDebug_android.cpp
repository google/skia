
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTypes.h"

static const size_t kBufferSize = 256;

#define LOG_TAG "skia"
#include <utils/Log.h>

void Android_SkDebugf(const char* file, int line, const char* function, 
    const char* format, ...)
{
    if (format[0] == '\n' && format[1] == '\0')
        return;
    va_list args;
    va_start(args, format);
#ifdef HAVE_ANDROID_OS
    char buffer[kBufferSize + 1];
    vsnprintf(buffer, kBufferSize, format, args);
    if (buffer[0] != 0)
        __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "%s", buffer);
#else
    android_vprintLog(ANDROID_LOG_DEBUG, NULL, LOG_TAG, format, args);
#endif
    va_end(args);
}


