// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=418928dbfffa9eb00c8225530f44baf5
REG_FIDDLE(Bitmap_eraseColor, 256, 20, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32(1, 1, kOpaque_SkAlphaType));
    bitmap.eraseColor(SK_ColorRED);
    canvas->scale(16, 16);
    canvas->drawBitmap(bitmap, 0, 0);
}
}  // END FIDDLE
