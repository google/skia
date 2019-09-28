// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=cd7543fa8c9f3cede46dc2d72eb8c4bd
REG_FIDDLE(Bitmap_extractAlpha_3, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
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
    paint.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle, 3));
    SkIPoint offset;
    bitmap.extractAlpha(&alpha, &paint, nullptr, &offset);
    paint.setColor(SK_ColorRED);
    canvas->drawBitmap(bitmap, 0, -offset.fY, &paint);
    canvas->drawBitmap(alpha, 100 + offset.fX, 0, &paint);
}
}  // END FIDDLE
