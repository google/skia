// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c7bb6248e4735b8d1a32d02fba40d344
REG_FIDDLE(Paint_setStyle, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStrokeWidth(5);
    SkRegion region;
    region.op(140, 10, 160, 30, SkRegion::kUnion_Op);
    region.op(170, 40, 190, 60, SkRegion::kUnion_Op);
    SkBitmap bitmap;
    bitmap.setInfo(SkImageInfo::MakeA8(50, 50), 50);
    uint8_t pixels[50][50];
    for (int x = 0; x < 50; ++x) {
        for (int y = 0; y < 50; ++y) {
            pixels[y][x] = (x + y) % 5 ? 0xFF : 0x00;
        }
    }
    bitmap.setPixels(pixels);
    for (auto style : { SkPaint::kFill_Style,
                        SkPaint::kStroke_Style,
                        SkPaint::kStrokeAndFill_Style }) {
        paint.setStyle(style);
        canvas->drawLine(10, 10, 60, 60, paint);
        canvas->drawRect({80, 10, 130, 60}, paint);
        canvas->drawRegion(region, paint);
        canvas->drawBitmap(bitmap, 200, 10, &paint);
        canvas->translate(0, 80);
    }
}
}  // END FIDDLE
