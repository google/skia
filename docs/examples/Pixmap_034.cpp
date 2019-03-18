// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=809284db136748208b3efc31cd89de29
REG_FIDDLE(Pixmap_writable_addr8, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint8_t storage[][5] = {{ 0,   0,  64,   0,  0},
                            { 0, 128, 255, 128,  0},
                            {64, 255, 255, 255, 64},
                            { 0, 128, 255, 128,  0},
                            { 0,   0,  64,   0,  0}};
    SkImageInfo imageInfo = SkImageInfo::Make(5, 5, kGray_8_SkColorType, kPremul_SkAlphaType);
    SkPixmap pixmap(imageInfo, storage[0], 5);
    SkBitmap bitmap;
    bitmap.installPixels(pixmap);
    canvas->scale(10, 10);
    canvas->drawBitmap(bitmap, 0, 0);
    *pixmap.writable_addr8(2, 2) = 0;
//  bitmap.installPixels(pixmap);      // uncomment to fix on GPU
    canvas->drawBitmap(bitmap, 10, 0);
}
}  // END FIDDLE
