
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "SkTypes.h"

static const size_t kBufferSize = 2048;

#include <stdarg.h>
#include <stdio.h>

#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/var.h"

extern pp::Instance* gPluginInstance;

namespace {
static const char* kLogPrefix = "SkDebugf:";
}

void SkDebugf(const char format[], ...) {
    if (gPluginInstance) {
        char buffer[kBufferSize + 1];
        va_list args;
        va_start(args, format);
        sprintf(buffer, kLogPrefix);
        vsnprintf(buffer + strlen(kLogPrefix), kBufferSize, format, args);
        va_end(args);
        pp::Var msg = pp::Var(buffer);
        gPluginInstance->PostMessage(msg);
    }
}
