// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(text_shadow, 128, 64, false, 0) {
void draw(SkCanvas* canvas) {
    const SkScalar sigma = 1.65f;
    const SkScalar xDrop = 2.0f;
    const SkScalar yDrop = 2.0f;
    const SkScalar x = 8.0f;
    const SkScalar y = 52.0f;
    const SkScalar textSize = 48.0f;
    const uint8_t blurAlpha = 127;
    auto blob = SkTextBlob::MakeFromString("Skia", SkFont(nullptr, textSize));
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPaint blur(paint);
    blur.setAlpha(blurAlpha);
    blur.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma, 0));
    canvas->drawColor(SK_ColorWHITE);
    canvas->drawTextBlob(blob.get(), x + xDrop, y + yDrop, blur);
    canvas->drawTextBlob(blob.get(), x, y, paint);
}
}  // END FIDDLE
