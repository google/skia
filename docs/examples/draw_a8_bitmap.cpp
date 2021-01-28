// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(draw_a8_bitmap, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeA8(256, 256));
    for (int y = 0; y < bitmap.height(); ++y) {
        for (int x = 0; x < bitmap.width(); ++x) {
            *bitmap.getAddr8(x, y) = (2 * x + 2 * y) & 0xFF;
        }
    }
    SkPaint paint;
    paint.setColor(0xFF00FF00);
    canvas->clear(0xFF000000);
    canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);
}
}  // END FIDDLE
