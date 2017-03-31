/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_OUTPUTSTREAM
#define SKSL_OUTPUTSTREAM

#include "SkSLString.h"

namespace SkSL {

class OutputStream {
public:
    virtual bool isValid() const {
        return true;
    }

    virtual void write8(uint8_t b) = 0;

    virtual void writeText(const char* s) = 0;

    virtual void write(const void* s, size_t size) = 0;

    void writeString(String s) {
        this->write(s.c_str(), s.size());
    }

    virtual ~OutputStream() {}
};

} // namespace

#endif
