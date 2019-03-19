// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=23c4543ac6cdd0e8fe762816a0dc2e03
REG_FIDDLE(Bitmap_isVolatile, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap original;
    SkImageInfo info = SkImageInfo::Make(25, 35, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
    if (original.tryAllocPixels(info)) {
        original.setIsVolatile(true);
        SkBitmap copy;
        original.extractSubset(&copy, {5, 10, 15, 20});
        SkDebugf("original is " "%s" "volatile\n", original.isVolatile() ? "" : "not ");
        SkDebugf("copy is " "%s" "volatile\n", copy.isImmutable() ? "" : "not ");
    }
}
}  // END FIDDLE
