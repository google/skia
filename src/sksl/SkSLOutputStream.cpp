/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLOutputStream.h"

#include <stdio.h>
#include <memory>

namespace SkSL {

void OutputStream::writeString(const std::string& s) {
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
    va_list copy;
    va_copy(copy, args);
    int length = vsnprintf(buffer, kBufferSize, format, args);
    if (length > (int) kBufferSize) {
        std::unique_ptr<char[]> bigBuffer(new char[length + 1]);
        vsnprintf(bigBuffer.get(), length + 1, format, copy);
        this->write(bigBuffer.get(), length);
    } else {
        this->write(buffer, length);
    }
    va_end(copy);
}

}  // namespace SkSL
