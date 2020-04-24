// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=720e4c053fae9e929ab6518b47e49370
REG_FIDDLE(Bitmap_tryAllocPixels_3, 256, 50, false, 0) {
void draw(SkCanvas* canvas) {
    uint8_t set1[5] = { 0xCA, 0xDA, 0xCA, 0xC9, 0xA3 };
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::Make(5, 1, kGray_8_SkColorType, kOpaque_SkAlphaType), set1, 5);
    canvas->scale(10, 50);
    canvas->drawBitmap(bitmap, 0, 0);
    if (bitmap.tryAllocPixels()) {
        bitmap.eraseColor(SK_ColorBLACK);
        canvas->drawBitmap(bitmap, 8, 0);
        bitmap.setPixels(set1);
        canvas->drawBitmap(bitmap, 16, 0);
    }
}
}  // END FIDDLE
