// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=faa5dfa466f6e16c07c124d971f32679
REG_FIDDLE(Bitmap_writePixels_2, 256, 80, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(2, 2));
    bitmap.eraseColor(SK_ColorGREEN);
    SkPMColor color = 0xFF5599BB;
    SkPixmap src(SkImageInfo::MakeN32Premul(1, 1), &color, 4);
    bitmap.writePixels(src);
    canvas->scale(40, 40);
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
