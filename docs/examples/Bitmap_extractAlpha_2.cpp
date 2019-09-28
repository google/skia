// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=092739b4cd5d732a27c07ced8ef45f01
REG_FIDDLE(Bitmap_extractAlpha_2, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    auto radiusToSigma = [](SkScalar radius) -> SkScalar {
         static const SkScalar kBLUR_SIGMA_SCALE = 0.57735f;
         return radius > 0 ? kBLUR_SIGMA_SCALE * radius + 0.5f : 0.0f;
    };
    SkBitmap alpha, bitmap;
    bitmap.allocN32Pixels(100, 100);
    SkCanvas offscreen(bitmap);
    offscreen.clear(0);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLUE);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    offscreen.drawCircle(50, 50, 39, paint);
    offscreen.flush();
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, radiusToSigma(25)));
    SkIPoint offset;
    bitmap.extractAlpha(&alpha, &paint, &offset);
    paint.setColor(SK_ColorRED);
    canvas->drawBitmap(bitmap, 0, -offset.fY, &paint);
    canvas->drawBitmap(alpha, 100 + offset.fX, 0, &paint);
}
}  // END FIDDLE
