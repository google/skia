/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Make sure that the PRI format string macros are defined
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>
#include <stdarg.h>

#include "src/utils/SkJSONWriter.h"

void SkJSONWriter::appendS64(int64_t value) {
    this->beginValue();
    this->appendf("%" PRId64, value);
}

void SkJSONWriter::appendU64(uint64_t value) {
    this->beginValue();
    this->appendf("%" PRIu64, value);
}

void SkJSONWriter::appendHexU64(uint64_t value) {
    this->beginValue();
    this->appendf("\"0x%" PRIx64 "\"", value);
}

void SkJSONWriter::appendf(const char* fmt, ...) {
    const int kBufferSize = 1024;
    char buffer[kBufferSize];
    va_list argp;
    va_start(argp, fmt);
#ifdef SK_BUILD_FOR_WIN
    int length = _vsnprintf_s(buffer, kBufferSize, _TRUNCATE, fmt, argp);
#else
    int length = vsnprintf(buffer, kBufferSize, fmt, argp);
#endif
    SkASSERT(length >= 0 && length < kBufferSize);
    va_end(argp);
    this->write(buffer, length);
}
