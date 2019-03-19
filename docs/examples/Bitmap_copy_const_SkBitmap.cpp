// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=bbbae7a181bfd128a4484e8e9f454db1
REG_FIDDLE(Bitmap_copy_const_SkBitmap, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap original;
    if (original.tryAllocPixels(
            SkImageInfo::Make(25, 35, kRGBA_8888_SkColorType, kOpaque_SkAlphaType))) {
        SkDebugf("original has pixels before copy: %s\n", original.getPixels() ? "true" : "false");
        SkBitmap copy(original);
        SkDebugf("original has pixels after copy: %s\n", original.getPixels() ? "true" : "false");
        SkDebugf("copy has pixels: %s\n", copy.getPixels() ? "true" : "false");
    }
}
}  // END FIDDLE
