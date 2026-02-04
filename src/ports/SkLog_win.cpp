/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstdarg>
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFeatures.h"
#include "include/private/base/SkLog.h"

#if defined(SK_BUILD_FOR_WIN)

#include "src/base/SkLeanWindows.h"

#include <stdarg.h>
#include <stdio.h>

static const size_t kBufferSize = 2048;

void SkLogVAList(SkLogPriority priority, const char format[], va_list args) {
    char buffer[kBufferSize + 1];
    va_list args_copy;

    va_copy(args_copy, args);
    vfprintf(stderr, format, args_copy);
    va_end(args_copy);
    fflush(stderr);  // stderr seems to be buffered on Windows.

    vsnprintf(buffer, kBufferSize, format, args);

    OutputDebugStringA(buffer);
}
#endif  // defined(SK_BUILD_FOR_WIN)
