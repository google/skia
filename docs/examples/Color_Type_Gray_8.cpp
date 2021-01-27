// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=93da0eb0b6722a4f33dc7dae094abf0b
REG_FIDDLE(Color_Type_Gray_8, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(16, 16);
    SkBitmap bitmap;
    SkImageInfo imageInfo = SkImageInfo::Make(2, 2, kGray_8_SkColorType, kOpaque_SkAlphaType);
    bitmap.allocPixels(imageInfo);
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorGREEN);
    canvas->drawImage(bitmap.asImage(), 0, 0);
    uint8_t gray8[] = { 0xFF, 0xBB, 0x77, 0x33 };
    SkPixmap grayPixmap(imageInfo, &gray8, imageInfo.minRowBytes());
    if (bitmap.writePixels(grayPixmap, 0, 0)) {
        canvas->drawImage(bitmap.asImage(), 2, 2);
    }
}
}  // END FIDDLE
