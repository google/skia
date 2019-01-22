/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLOutputStream.h"

namespace SkSL {

void OutputStream::writeString(String s) {
    this->write(s.c_str(), s.size());
}

void OutputStream::printf(const char format[], ...) {
   va_list args;
   va_start(args, format);
   this->appendVAList(format, args);
   va_end(args);
}

void OutputStream::appendVAList(const char format[], va_list args) {
    char buffer[kBufferSize];
    int length = vsnprintf(buffer, kBufferSize, format, args);
    SkASSERT(length >= 0 && length < (int) kBufferSize);
    this->write(buffer, length);
}

}
