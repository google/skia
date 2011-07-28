
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTypes.h"

static const size_t kBufferSize = 2048;

#include <stdarg.h>
#include <stdio.h>

void SkDebugf(const char format[], ...) {
    char    buffer[kBufferSize + 1];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, kBufferSize, format, args);
    va_end(args);
    fprintf(stderr, "%s", buffer);
}

