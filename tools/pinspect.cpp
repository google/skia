/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkString.h"

static void inspect(const char path[]) {
    SkFILEStream stream(path);
    if (!stream.isValid()) {
        printf("-- Can't open '%s'\n", path);
        return;
    }

    printf("Opening '%s'...\n", path);

    {
        int32_t header[3];
        if (stream.read(header, sizeof(header)) != sizeof(header)) {
            printf("-- Failed to read header (12 bytes)\n");
            return;
        }
        printf("version:%d width:%d height:%d\n", header[0], header[1], header[2]);
    }

    stream.rewind();
    SkPicture pic(&stream);
    printf("picture size:[%d %d]\n", pic.width(), pic.height());
}

int main(int argc, char* const argv[]) {
    if (argc < 2) {
        printf("Usage: pinspect filename [filename ...]\n");
    }
    for (int i = 1; i < argc; ++i) {
        inspect(argv[i]);
        if (i < argc - 1) {
            printf("\n");
        }
    }
    return 0;
}
