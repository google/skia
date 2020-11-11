/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FILEOUTPUTSTREAM
#define SKSL_FILEOUTPUTSTREAM

#include "src/sksl/SkSLOutputStream.h"
#include "src/sksl/SkSLUtil.h"
#include <stdio.h>

namespace SkSL {

class FileOutputStream : public OutputStream {
public:
    FileOutputStream(const SkSL::String& name) {
        fFile = fopen(name.c_str(), "wb");
    }

    ~FileOutputStream() override {
        if (fOpen) {
            close();
        }
    }

    bool isValid() const override {
        return nullptr != fFile;
    }

    void write8(uint8_t b) override {
        SkASSERT(fOpen);
        if (isValid()) {
            if (EOF == fputc(b, fFile)) {
                fFile = nullptr;
            }
        }
    }

    void writeText(const char* s) override {
        SkASSERT(fOpen);
        if (isValid()) {
            if (EOF == fputs(s, fFile)) {
                fFile = nullptr;
            }
        }
    }

    void write(const void* s, size_t size) override {
        if (isValid()) {
            size_t written = fwrite(s, 1, size, fFile);
            if (written != size) {
                fFile = nullptr;
            }
        }
    }

    bool close() {
        fOpen = false;
        if (isValid() && fclose(fFile)) {
            fFile = nullptr;
            return false;
        }
        return true;
    }

private:
    bool fOpen = true;
    FILE *fFile;

    using INHERITED = OutputStream;
};

} // namespace

#endif
