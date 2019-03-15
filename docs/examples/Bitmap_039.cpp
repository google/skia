// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=737e721c7d9e0f367d25521a1b823b9d
REG_FIDDLE(Bitmap_039, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixelsFlags(SkImageInfo::MakeN32(44, 16, kPremul_SkAlphaType),
                            SkBitmap::kZeroPixels_AllocFlag);
    SkCanvas offscreen(bitmap);
    SkPaint paint;
    SkFont font;
    offscreen.drawString("!@#$%", 0, 12, font, paint);
    canvas->scale(6, 6);
    canvas->drawBitmap(bitmap, 0, 0);
    canvas->drawBitmap(bitmap, 8, 8);
}
}  // END FIDDLE
