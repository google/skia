// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=4a521be1f850058541e136a808c65e78
REG_FIDDLE(Canvas_drawBitmap, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint8_t pixels[][8] = { { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00},
                            { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00},
                            { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00},
                            { 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF},
                            { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
                            { 0x00, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x00},
                            { 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00},
                            { 0xFF, 0x00, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF} };
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeA8(8, 8),
            (void*) pixels, sizeof(pixels[0]));
    SkPaint paint;
    canvas->scale(4, 4);
    for (auto color : { SK_ColorRED, SK_ColorBLUE, 0xFF007F00} ) {
        paint.setColor(color);
        canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);
        canvas->translate(12, 0);
    }
}
}  // END FIDDLE
