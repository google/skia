// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=fe79a9c1ec350264eb9c7b2509dd3638
REG_FIDDLE(Bitmap_000, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeN32(16, 16, kPremul_SkAlphaType));
    SkDebugf("pixel address = %p\n", bitmap.getPixels());
    SkBitmap::HeapAllocator stdalloc;
    if (!stdalloc.allocPixelRef(&bitmap)) {
        SkDebugf("pixel allocation failed\n");
    } else {
        SkDebugf("pixel address = %p\n", bitmap.getPixels());
    }
}
}  // END FIDDLE
