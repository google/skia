/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_OUTPUTSTREAM
#define SKSL_OUTPUTSTREAM

namespace SkSL {

class OutputStream {
public:
    virtual bool isValid() const {
        return true;
    }

    virtual void write8(int8_t b) = 0;

    virtual void write(const char* s) = 0;

    virtual void write(const void* s, size_t size) = 0;

    virtual void write(String s) {
        this->write(s.c_str(), s.size());
    }

    virtual ~OutputStream() {}
};

} // namespace

#endif
