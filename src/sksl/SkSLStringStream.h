/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STRINGSTREAM
#define SKSL_STRINGSTREAM

#include "include/core/SkData.h"
#include "include/core/SkStream.h"
#include "src/sksl/SkSLOutputStream.h"

namespace SkSL {

class StringStream : public OutputStream {
public:
    void write8(uint8_t b) override {
        SkASSERT(fString.empty());
        fStream.write8(b);
    }

    void writeText(const char* s) override {
        SkASSERT(fString.empty());
        fStream.writeText(s);
    }

    void write(const void* s, size_t size) override {
        SkASSERT(fString.empty());
        fStream.write(s, size);
    }

    size_t bytesWritten() const {
        return fStream.bytesWritten();
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

}  // namespace SkSL

#endif
