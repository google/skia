// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=7d04932f2a259cc70d6e45cd25a6feb6
REG_FIDDLE(Canvas_drawBitmapRect, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint8_t pixels[][8] = { { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00},
                            { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00},
                            { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00},
                            { 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF},
                            { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                            { 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00},
                            { 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00},
                            { 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00} };
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeA8(8, 8),
            (void*) pixels, sizeof(pixels[0]));
    SkPaint paint;
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kSolid_SkBlurStyle, 6));
    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xFF007F00} ) {
        paint.setColor(color);
        canvas->drawBitmapRect(bitmap, SkRect::MakeWH(8, 8), SkRect::MakeWH(32, 32), &paint);
        canvas->translate(48, 0);
    }
}
}  // END FIDDLE
