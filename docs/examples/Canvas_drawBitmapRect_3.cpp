// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=bdbeac3c97f60a63987b1cc8e1f1e91e
REG_FIDDLE(Canvas_drawBitmapRect_3, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint32_t pixels[][2] = { { 0x00000000, 0x55550000},
                             { 0xAAAA0000, 0xFFFF0000} };
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeN32Premul(2, 2),
            (void*) pixels, sizeof(pixels[0]));
    SkPaint paint;
    canvas->scale(4, 4);
    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN } ) {
        paint.setColorFilter(SkColorFilters::Blend(color, SkBlendMode::kPlus));
        canvas->drawImageRect(bitmap.asImage().get(), SkRect::MakeWH(8, 8),
                              SkSamplingOptions(), &paint);
        canvas->translate(8, 0);
    }
}
}  // END FIDDLE
