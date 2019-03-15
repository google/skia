// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=42318b18d403e17e07a541652da91ee2
REG_FIDDLE(Canvas_029, 256, 128, false, 0) {
#include "SkBlurImageFilter.h"

void draw(SkCanvas* canvas) {
    SkPaint paint, blur;
    blur.setImageFilter(SkBlurImageFilter::Make(3, 3, nullptr));
    canvas->saveLayer(nullptr, &blur);
    SkRect rect = { 25, 25, 50, 50};
    canvas->drawRect(rect, paint);
    canvas->translate(50, 50);
    paint.setColor(SK_ColorRED);
    canvas->drawRect(rect, paint);
    canvas->restore();
}
}  // END FIDDLE
