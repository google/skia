// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=91f474a11a2112cd5c88c40a9015048d
REG_FIDDLE(Bitmap_allocPixels_2, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::Make(64, 64, kGray_8_SkColorType, kOpaque_SkAlphaType));
    SkCanvas offscreen(bitmap);
    offscreen.scale(.5f, .5f);
    for (int y : { 0, 64, 128, 192 } ) {
        offscreen.drawBitmap(source, -y, -y);
        canvas->drawBitmap(bitmap, y, y);
    }
}
}  // END FIDDLE
