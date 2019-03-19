// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=9210060d1f4ca46e1375496237902ef3
REG_FIDDLE(Bitmap_setImmutable, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeN32(4, 4, kPremul_SkAlphaType));
    bitmap.allocPixels();
    SkCanvas offscreen(bitmap);
    SkDebugf("draw white\n");
    offscreen.clear(SK_ColorWHITE);
    bitmap.setImmutable();
    SkDebugf("draw black\n");
    // Triggers assert if SK_DEBUG is true, runs fine otherwise.
    // offscreen.clear(SK_ColorBLACK);
}
}  // END FIDDLE
