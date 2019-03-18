// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=db61fdcd382342ee88ea1b4f27c27b95
REG_FIDDLE(Bitmap_isImmutable, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap original;
    SkImageInfo info = SkImageInfo::Make(25, 35, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
    if (original.tryAllocPixels(info)) {
        original.setImmutable();
        SkBitmap copy;
        original.extractSubset(&copy, {5, 10, 15, 20});
        SkDebugf("original is " "%s" "immutable\n", original.isImmutable() ? "" : "not ");
        SkDebugf("copy is " "%s" "immutable\n", copy.isImmutable() ? "" : "not ");
    }
}
}  // END FIDDLE
