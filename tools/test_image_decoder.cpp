/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkForceLinking.h"
#include "SkGraphics.h"
#include "SkImageDecoder.h"

__SK_FORCE_IMAGE_DECODER_LINKING;

/**
   Simple program to test Skia's ability to decode images without
   errors or debug messages. */
int tool_main(int argc, char** argv);
int tool_main(int argc, char** argv) {
    if (argc < 2) {
        SkDebugf("Usage:\n %s imagefile\n\n", argv[0]);
        return 3;
    }
    SkAutoGraphics ag;  // Enable use of SkRTConfig
    SkBitmap bitmap;
    if (!(SkImageDecoder::DecodeFile(argv[1], &bitmap))) {
        return 2;
    }
    if (bitmap.empty()) {
        return 1;
    }
    return 0;
}

#if !defined SK_BUILD_FOR_IOS
int main(int argc, char * const argv[]) {
    return tool_main(argc, (char**) argv);
}
#endif
