/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_OUTPUTSTREAM
#define SKSL_OUTPUTSTREAM

#include "include/private/SkSLDefines.h"
#include "include/private/SkSLString.h"

namespace SkSL {

class OutputStream {
public:
    virtual bool isValid() const {
        return true;
    }

    virtual void write8(uint8_t b) = 0;

    void write16(uint16_t i) {
        this->write8((uint8_t) i);
        this->write8((uint8_t) (i >> 8));
    }

    void write32(uint32_t i) {
        this->write8((uint8_t) i);
        this->write8((uint8_t) (i >> 8));
        this->write8((uint8_t) (i >> 16));
        this->write8((uint8_t) (i >> 24));
    }

    virtual void writeText(const char* s) = 0;

    virtual void write(const void* s, size_t size) = 0;

    void writeString(const String& s);

    void printf(const char format[], ...) SK_PRINTF_LIKE(2, 3);

    void appendVAList(const char format[], va_list args);

    virtual ~OutputStream() {}

private:
    static const int kBufferSize = 1024;
};

}  // namespace SkSL

#endif
