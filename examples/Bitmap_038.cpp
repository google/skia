// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(Bitmap_038, 256, 256, true, 0);
// HASH=f1d1880d38e0aea4cefd3e11745e8a09
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    if (!bitmap.tryAllocPixelsFlags(SkImageInfo::MakeN32(10000, 10000, kOpaque_SkAlphaType),
                                    SkBitmap::kZeroPixels_AllocFlag)) {
        SkDebugf("bitmap allocation failed!\n");
    } else {
        SkDebugf("bitmap allocation succeeded!\n");
    }
}
}
