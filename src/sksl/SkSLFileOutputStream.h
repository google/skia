/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FILEOUTPUTSTREAM
#define SKSL_FILEOUTPUTSTREAM

#include "SkSLOutputStream.h"
#include "SkSLUtil.h"
#include <stdio.h>

namespace SkSL {

class FileOutputStream : public OutputStream {
public:
    FileOutputStream(const char* name) {
        fFile = fopen(name, "w");
    }

    ~FileOutputStream() override {
        ASSERT(!fOpen);
    }

    bool isValid() const override {
        return nullptr != fFile;
    }

    void write8(uint8_t b) override {
        ASSERT(fOpen);
        if (isValid()) {
            if (EOF == fputc(b, fFile)) {
                fFile = nullptr;
            }
        }
    }

    void writeText(const char* s) override {
        ASSERT(fOpen);
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

    typedef OutputStream INHERITED;
};

} // namespace

#endif
