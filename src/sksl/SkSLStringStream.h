/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRINGSTREAM
#define SKSL_STRINGSTREAM

#include "SkSLOutputStream.h"

namespace SkSL {

class StringStream : public OutputStream {
public:
    void write8(int8_t b) override {
        fBuffer += (char) b;
    }
    
    void write(const char* s) override {
        fBuffer += s;
    }

    void write(const void* s, size_t size) override {
        fBuffer.append((const char*) s, size);
    }

    const void* data() const {
        return fBuffer.c_str();
    }

    size_t size() const {
        return fBuffer.size();
    }

private:
    String fBuffer;
};

} // namespace

#endif
