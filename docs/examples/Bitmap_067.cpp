// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=cb9a08e8ff779b6a1cf8bb54f3883aaf
REG_FIDDLE(Bitmap_067, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    const int width = 8;
    const int height = 8;
    uint8_t pixels[height][width];
    SkImageInfo info = SkImageInfo::Make(width, height, kGray_8_SkColorType, kOpaque_SkAlphaType);
    if (bitmap.installPixels(info, pixels, info.minRowBytes())) {
        SkDebugf("&pixels[4][2] %c= bitmap.getAddr8(2, 4)\n",
                  &pixels[4][2]  == bitmap.getAddr8(2, 4) ? '=' : '!');
    }
}
}  // END FIDDLE
