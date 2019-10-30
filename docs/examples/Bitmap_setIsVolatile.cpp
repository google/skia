// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e8627a4df659b896402f89a91326618f
REG_FIDDLE(Bitmap_setIsVolatile, 256, 20, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kOpaque_SkAlphaType));
    bitmap.allocPixels();
    bitmap.eraseColor(SK_ColorRED);
    canvas->scale(16, 16);
    canvas->drawBitmap(bitmap, 0, 0);
    *(SkPMColor*) bitmap.getPixels() = SkPreMultiplyColor(SK_ColorBLUE);
    canvas->drawBitmap(bitmap, 2, 0);
    bitmap.setIsVolatile(true);
    *(SkPMColor*) bitmap.getPixels() = SkPreMultiplyColor(SK_ColorGREEN);
    canvas->drawBitmap(bitmap, 4, 0);
}
}  // END FIDDLE
