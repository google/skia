/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstdarg>
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFeatures.h"
#include "include/private/base/SkLoadUserConfig.h"
#include "include/private/base/SkLog.h"

#if !defined(SK_BUILD_FOR_WIN) && !defined(SK_BUILD_FOR_ANDROID)

#include <stdarg.h>
#include <stdio.h>

void SkLogVAList(SkLogPriority priority, const char format[], va_list args) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
    vfprintf(stderr, format, args);
#pragma GCC diagnostic pop
}
#endif  // !defined(SK_BUILD_FOR_WIN) && !defined(SK_BUILD_FOR_ANDROID)
