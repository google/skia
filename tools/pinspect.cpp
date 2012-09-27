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
#include "SkDumpCanvas.h"

static SkPicture* inspect(const char path[]) {
    SkFILEStream stream(path);
    if (!stream.isValid()) {
        printf("-- Can't open '%s'\n", path);
        return NULL;
    }

    printf("Opening '%s'...\n", path);

    {
        int32_t header[3];
        if (stream.read(header, sizeof(header)) != sizeof(header)) {
            printf("-- Failed to read header (12 bytes)\n");
            return NULL;
        }
        printf("version:%d width:%d height:%d\n", header[0], header[1], header[2]);
    }

    stream.rewind();
    SkPicture* pic = SkNEW_ARGS(SkPicture, (&stream));
    printf("picture size:[%d %d]\n", pic->width(), pic->height());
    return pic;
}

static void dumpOps(SkPicture* pic) {
    SkDebugfDumper dumper;
    SkDumpCanvas canvas(&dumper);
    canvas.drawPicture(*pic);
}

int main(int argc, char* const argv[]) {
    if (argc < 2) {
        printf("Usage: pinspect [--dump-ops] filename [filename ...]\n");
        return 1;
    }

    bool doDumpOps = false;

    int index = 1;
    if (!strcmp(argv[index], "--dump-ops")) {
        index += 1;
        doDumpOps = true;
    }

    for (; index < argc; ++index) {
        SkAutoTUnref<SkPicture> pic(inspect(argv[index]));
        if (doDumpOps) {
            dumpOps(pic);
        }
        if (index < argc - 1) {
            printf("\n");
        }
    }
    return 0;
}
