// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=34479d5aa23ce9f5e334b0786c9edb22
REG_FIDDLE(Bitmap_tryAllocPixels, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    SkImageInfo info = SkImageInfo::Make(64, 256, kGray_8_SkColorType, kOpaque_SkAlphaType);
    if (bitmap.tryAllocPixels(info, 0)) {
        SkCanvas offscreen(bitmap);
        offscreen.scale(.5f, .5f);
        for (int x : { 0, 64, 128, 192 } ) {
            offscreen.drawBitmap(source, -x, 0);
            canvas->drawBitmap(bitmap, x, 0);
        }
    }
}
}  // END FIDDLE
