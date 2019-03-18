// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=3a47ef94cb70144455f80333d8653e6c
REG_FIDDLE(Canvas_drawImageRect_6, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint32_t pixels[][2] = { { 0x00000000, 0x55550000},
                             { 0xAAAA0000, 0xFFFF0000} };
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeN32Premul(2, 2),
            (void*) pixels, sizeof(pixels[0]));
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    SkPaint paint;
    canvas->scale(4, 4);
    for (auto color : { SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN } ) {
        paint.setColorFilter(SkColorFilter::MakeModeFilter(color, SkBlendMode::kPlus));
        canvas->drawImageRect(image, SkRect::MakeWH(8, 8), &paint);
        canvas->translate(8, 0);
    }
}
}  // END FIDDLE
