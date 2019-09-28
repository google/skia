// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c717491f9251604724c9cbde7088ec20
REG_FIDDLE(Bitmap_allocN32Pixels, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkRandom random;
    SkBitmap bitmap;
    bitmap.allocN32Pixels(64, 64);
    bitmap.eraseColor(SK_ColorTRANSPARENT);
    for (int y = 0; y < 256; y += 64) {
        for (int x = 0; x < 256; x += 64) {
            SkColor color = random.nextU();
            uint32_t w = random.nextRangeU(4, 32);
            uint32_t cx = random.nextRangeU(0, 64 - w);
            uint32_t h = random.nextRangeU(4, 32);
            uint32_t cy = random.nextRangeU(0, 64 - h);
            bitmap.erase(color, SkIRect::MakeXYWH(cx, cy, w, h));
            canvas->drawBitmap(bitmap, x, y);
        }
    }
}
}  // END FIDDLE
