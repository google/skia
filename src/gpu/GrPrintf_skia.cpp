
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "GrTypes.h"

#include <stdarg.h>
#include <stdio.h>

#include "SkTypes.h"

void GrPrintf(const char format[], ...) {
    const size_t MAX_BUFFER_SIZE = 2048;

    char buffer[MAX_BUFFER_SIZE + 1];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, MAX_BUFFER_SIZE, format, args);
    va_end(args);

    // skia has already mapped this to do the "right thing"
    SkDebugf("%s", buffer);
}


