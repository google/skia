// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f0db16e06c9a1436917c8179f8c1718f
REG_FIDDLE(Bitmap_049, 256, 50, false, 0) {
void draw(SkCanvas* canvas) {
    uint8_t set1[5] = { 0xCA, 0xDA, 0xCA, 0xC9, 0xA3 };
    uint8_t set2[5] = { 0xAC, 0xA8, 0x89, 0x47, 0x87 };
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::Make(5, 1, kGray_8_SkColorType, kOpaque_SkAlphaType), set1, 5);
    canvas->scale(10, 50);
    canvas->drawBitmap(bitmap, 0, 0);
    bitmap.setPixels(set2);
    canvas->drawBitmap(bitmap, 10, 0);
}
}  // END FIDDLE
