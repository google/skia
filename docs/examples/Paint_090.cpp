#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6679d6e4ec632715ee03e68391bd7f9a
REG_FIDDLE(Paint_090, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    SkCanvas offscreen(bitmap);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorWHITE);
    paint.setTextSize(96);
    offscreen.clear(0);
    offscreen.drawString("e", 20, 70, paint);
    paint.setImageFilter(
           SkLightingImageFilter::MakePointLitDiffuse(SkPoint3::Make(80, 100, 10),
           SK_ColorWHITE, 1, 2, nullptr, nullptr));
    canvas->drawBitmap(bitmap, 0, 0, &paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
