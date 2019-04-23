/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkStream.h"
#include "include/utils/SkLua.h"
#include "src/core/SkOSFile.h"

#include <stdlib.h>

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

static sk_sp<SkData> read_into_data(const char file[]) {
    sk_sp<SkData> data(SkData::MakeFromFileName(file));
    if (!data) {
        data = SkData::MakeEmpty();
    }
    return data;
}

int main(int argc, char** argv) {
    SkAutoGraphics ag;
    SkLua L;

    for (int i = 1; i < argc; ++i) {
        sk_sp<SkData> data;
        const void* ptr;
        size_t len;

        if (!strcmp(argv[i], "--lua") && i < argc-1) {
            ptr = argv[i + 1];
            len = strlen(argv[i + 1]);
            i += 1;
        } else {
            data = read_into_data(argv[i]);
            ptr = data->data();
            len = data->size();
        }
        if (!L.runCode(ptr, len)) {
            SkDebugf("failed to load %s\n", argv[i]);
            exit(-1);
        }
    }
    return 0;
}
