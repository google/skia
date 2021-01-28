// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(setimagefilter, 256, 100, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    SkCanvas offscreen(bitmap);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    SkFont font(nullptr, 96);
    offscreen.clear(0);
    offscreen.drawString("e", 20, 70, font, paint);
    paint.setImageFilter(SkImageFilters::PointLitDiffuse(
            SkPoint3::Make(80, 100, 10), SK_ColorWHITE, 1, 2, nullptr, nullptr));
    canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);
}

}  // END FIDDLE
