/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLua.h"
#include "SkGraphics.h"
#include "SkStream.h"
#include "SkData.h"
#include "SkOSFile.h"

#include <stdlib.h>

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

static SkData* read_into_data(const char file[]) {
    SkData* data = SkData::NewFromFileName(file);
    if (!data) {
        data = SkData::NewEmpty();
    }
    return data;
}

int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    SkAutoGraphics ag;
    SkLua L;

    for (int i = 1; i < argc; ++i) {
        SkData* data = nullptr;
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
        SkSafeUnref(data);
    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
