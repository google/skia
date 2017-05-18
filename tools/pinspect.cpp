/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkOSFile.h"
#include "SkPicture.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkDumpCanvas.h"

static sk_sp<SkPicture> inspect(const char path[]) {
    SkFILEStream stream(path);
    if (!stream.isValid()) {
        printf("-- Can't open '%s'\n", path);
        return nullptr;
    }

    printf("Opening '%s'...\n", path);

    {
        int32_t header[3];
        if (stream.read(header, sizeof(header)) != sizeof(header)) {
            printf("-- Failed to read header (12 bytes)\n");
            return nullptr;
        }
        printf("version:%d width:%d height:%d\n", header[0], header[1], header[2]);
    }

    stream.rewind();
    auto pic = SkPicture::MakeFromStream(&stream);
    if (nullptr == pic) {
        SkDebugf("Could not create SkPicture: %s\n", path);
        return nullptr;
    }
    printf("picture cullRect: [%f %f %f %f]\n",
           pic->cullRect().fLeft, pic->cullRect().fTop,
           pic->cullRect().fRight, pic->cullRect().fBottom);
    return pic;
}

static void dumpOps(SkPicture* pic) {
#ifdef SK_DEBUG
    SkDebugfDumper dumper;
    SkDumpCanvas canvas(&dumper);
    canvas.drawPicture(pic);
#else
    printf("SK_DEBUG mode not enabled\n");
#endif
}

int main(int argc, char** argv) {
    SkAutoGraphics ag;
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
        auto pic(inspect(argv[index]));
        if (doDumpOps) {
            dumpOps(pic.get());
        }
        if (index < argc - 1) {
            printf("\n");
        }
    }
    return 0;
}
