// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=0cc2c6a0dffa61a88711534bd3d43b40
REG_FIDDLE(Bitmap_peekPixels, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeN32Premul(6, 11));
    SkCanvas offscreen(bitmap);
    offscreen.clear(SK_ColorWHITE);
    SkPaint paint;
    SkFont font;
    offscreen.drawString("?", 0, 10, font, paint);
    SkPixmap pixmap;
    if (bitmap.peekPixels(&pixmap)) {
        const SkPMColor* pixels = pixmap.addr32();
        SkPMColor pmWhite = pixels[0];
        for (int y = 0; y < bitmap.height(); ++y) {
            for (int x = 0; x < bitmap.width(); ++x) {
                SkDebugf("%c", *pixels++ == pmWhite ? '-' : 'x');
            }
            SkDebugf("\n");
        }
    }
}
}  // END FIDDLE
