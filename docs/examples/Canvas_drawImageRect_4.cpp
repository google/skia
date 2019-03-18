// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d4b35a9d24c32c042bd1f529b8de3c0d
REG_FIDDLE(Canvas_drawImageRect_4, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    uint32_t pixels[][2] = { { SK_ColorBLACK, SK_ColorWHITE },
                             { SK_ColorWHITE, SK_ColorBLACK } };
    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::MakeN32Premul(2, 2),
            (void*) pixels, sizeof(pixels[0]));
    sk_sp<SkImage> image = SkImage::MakeFromBitmap(bitmap);
    SkPaint paint;
    canvas->scale(4, 4);
    for (auto alpha : { 50, 100, 150, 255 } ) {
        paint.setAlpha(alpha);
        canvas->drawImageRect(image, SkRect::MakeWH(2, 2), SkRect::MakeWH(8, 8), &paint);
        canvas->translate(8, 0);
    }
}
}  // END FIDDLE
