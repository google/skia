/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FILEOUTPUTSTREAM
#define SKSL_FILEOUTPUTSTREAM

#include "SkSLOutputStream.h"
#include <stdio.h>

namespace SkSL {

class FileOutputStream : public OutputStream {
public:
    FileOutputStream(const char* name) {
        file = fopen(name, "w");
    }

    ~FileOutputStream() {
        fclose(file);
    }

    bool isValid() const override {
        return nullptr != file;
    }

    void write8(int8_t b) override {
        if (isValid()) {
            fputc(b, file);
        }
    }

    void write(const char* s) override {
        if (isValid()) {
            fputs(s, file);
        }
    }

    void write(const void* s, size_t size) override {
        if (isValid()) {
            size_t written = fwrite(s, size, 1, file);
            if (written != size) {
                fclose(file);
                file = nullptr;
            }
        }
    }

private:
    FILE *file;

    typedef OutputStream INHERITED;
};

} // namespace

#endif
