/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRINGSTREAM
#define SKSL_STRINGSTREAM

#include "SkSLOutputStream.h"

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

    const char* data() const {
        return fBuffer.c_str();
    }

    size_t size() const {
        return fBuffer.size();
    }

    void reset() {
        fBuffer = "";
    }

private:
    String fBuffer;
};

#else

#include "SkData.h"
#include "SkStream.h"

namespace SkSL {

class StringStream : public OutputStream {
public:
    void write8(uint8_t b) override {
        SkASSERT(!fData);
        fStream.write8(b);
    }

    void writeText(const char* s) override {
        SkASSERT(!fData);
        fStream.writeText(s);
    }

    void write(const void* s, size_t size) override {
        SkASSERT(!fData);
        fStream.write(s, size);
    }

    const char* data() const {
        if (!fData) {
            fData = fStream.detachAsData();
        }
        return (const char*) fData->data();
    }

    size_t size() const {
        if (!fData) {
            fData = fStream.detachAsData();
        }
        return fData->size();
    }

    void reset() {
        fStream.reset();
        fData = nullptr;
    }

private:
    mutable SkDynamicMemoryWStream fStream;
    mutable sk_sp<SkData> fData;
};

#endif // SKSL_STANDALONE

} // namespace

#endif
