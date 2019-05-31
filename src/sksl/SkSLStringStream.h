/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRINGSTREAM
#define SKSL_STRINGSTREAM

#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLString.h"

#ifdef SKSL_STANDALONE

namespace SkSL {

class StringStream : public OutputStream {
public:
    void write8(uint8_t b) override {
        fBuffer += (char) b;
    }

    void writeText(const char* s) override {
        fBuffer += s;
    }

    void write(const void* s, size_t size) override {
        fBuffer.append((const char*) s, size);
    }

    const String& str() const {
        return fBuffer;
    }

    void reset() {
        fBuffer = "";
    }

private:
    String fBuffer;
};

#else

#include "include/core/SkData.h"
#include "include/core/SkStream.h"

namespace SkSL {

class StringStream : public OutputStream {
public:
    void write8(uint8_t b) override {
        fStream.write8(b);
    }

    void writeText(const char* s) override {
        fStream.writeText(s);
    }

    void write(const void* s, size_t size) override {
        fStream.write(s, size);
    }

    const String& str() const {
        if (!fString.size()) {
            sk_sp<SkData> data = fStream.detachAsData();
            fString = String((const char*) data->data(), data->size());
        }
        return fString;
    }

    void reset() {
        fStream.reset();
        fString = "";
    }

private:
    mutable SkDynamicMemoryWStream fStream;
    mutable String fString;
};

#endif // SKSL_STANDALONE

} // namespace

#endif
