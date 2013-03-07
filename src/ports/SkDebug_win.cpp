
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkTypes.h"

static const size_t kBufferSize = 2048;

#include <stdarg.h>
#include <stdio.h>
#include <Windows.h>

void SkDebugf(const char format[], ...) {
    char    buffer[kBufferSize + 1];
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    va_start(args, format);
    vsnprintf(buffer, kBufferSize, format, args);
    va_end(args);

    OutputDebugStringA(buffer);
}
