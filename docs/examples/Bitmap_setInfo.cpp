// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=599ab64d0aea005498176249bbfb64eb
REG_FIDDLE(Bitmap_setInfo, 256, 96, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeN32(44, 16, kOpaque_SkAlphaType));
    bitmap.allocPixels();
    bitmap.eraseColor(SK_ColorGREEN);
    SkCanvas offscreen(bitmap);
    SkPaint paint;
    SkFont font;
    offscreen.drawString("!@#$%", 0, 12, font, paint);
    canvas->scale(6, 6);
    canvas->drawImage(bitmap.asImage(), 0, 0);
}
}  // END FIDDLE
