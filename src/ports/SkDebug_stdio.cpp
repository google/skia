/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_IOS)

#include <stdarg.h>
#include <CoreFoundation/CFString.h>

struct NSString;
extern "C" void NSLogv(NSString *format, va_list args);

void SkDebugf(const char* format, ...) {
    va_list args, argsCopy;
    va_start(args, format);
    va_copy(argsCopy, args);
    vfprintf(stderr, format, argsCopy);
    va_end(argsCopy);
    CFStringRef fmt = CFStringCreateWithCString(nullptr, format, kCFStringEncodingUTF8);
    NSLogv((NSString*)fmt, args);
    CFRelease(fmt);
    va_end(args);
}
#elif !defined(SK_BUILD_FOR_WIN) && !defined(SK_BUILD_FOR_ANDROID)

#include <stdarg.h>
#include <stdio.h>

void SkDebugf(const char format[], ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}
#endif//!defined(SK_BUILD_FOR_WIN) && !defined(SK_BUILD_FOR_ANDROID)
