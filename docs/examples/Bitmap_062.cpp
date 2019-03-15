// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=2c5c4230ccd2861a5d15b7cd2764ab6e
REG_FIDDLE(Bitmap_062, 256, 70, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32(2, 2, kPremul_SkAlphaType));
    bitmap.erase(0x7fff7f3f, SkIRect::MakeWH(1, 1));
    bitmap.erase(0x7f7f3fff, SkIRect::MakeXYWH(0, 1, 1, 1));
    bitmap.erase(0x7f3fff7f, SkIRect::MakeXYWH(1, 0, 1, 1));
    bitmap.erase(0x7f1fbf5f, SkIRect::MakeXYWH(1, 1, 1, 1));
    canvas->scale(25, 25);
    canvas->drawBitmap(bitmap, 0, 0);
    canvas->drawBitmap(bitmap, .5f, .5f);
}
}  // END FIDDLE
