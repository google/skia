// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8b128e067881f9251357653692fa28da
REG_FIDDLE(Canvas_writePixels_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkImageInfo imageInfo = SkImageInfo::MakeN32Premul(2, 2);
    SkBitmap bitmap;
    bitmap.setInfo(imageInfo);
    uint32_t pixels[4];
    bitmap.setPixels(pixels);
    for (int y = 0; y < 256; y += 2) {
        for (int x = 0; x < 256;  x += 2) {
            pixels[0] = SkColorSetRGB(x, y, x | y);
            pixels[1] = SkColorSetRGB(x ^ y, y, x);
            pixels[2] = SkColorSetRGB(x, x & y, y);
            pixels[3] = SkColorSetRGB((~x) & 0xFF, (~y) & 0xFF, x);
            canvas->writePixels(bitmap, x, y);
        }
    }
}
}  // END FIDDLE
