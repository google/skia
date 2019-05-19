// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=d307e7e1237f39fb54d80723e5449857
REG_FIDDLE(Canvas_drawImageRect_5, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint32_t pixels[][2] = { { 0x00000000, 0x55555555},
                             { 0xAAAAAAAA, 0xFFFFFFFF} };
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeN32Premul(2, 2),
            (void*) pixels, sizeof(pixels[0]));
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    SkPaint paint;
    canvas->scale(4, 4);
    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN } ) {
        paint.setColorFilter(SkColorFilters::Blend(color, SkBlendMode::kPlus));
        canvas->drawImageRect(image, SkIRect::MakeWH(2, 2), SkRect::MakeWH(8, 8), &paint);
        canvas->translate(8, 0);
    }
}
}  // END FIDDLE
