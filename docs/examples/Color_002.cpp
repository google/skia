// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=896ce0316b489608a95af5439ca2aab1
REG_FIDDLE(Color_002, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    for (int alpha = 255; alpha >= 0; alpha -= 17) {
        paint.setAlpha(alpha);
        canvas->drawRect({5, 5, 100, 20}, paint);
        SkAlpha alphaInPaint = SkColorGetA(paint.getColor());
        canvas->drawString(std::to_string(alphaInPaint).c_str(), 110, 18, SkFont(), paint);
        canvas->translate(0, 15);
    }
}
}  // END FIDDLE
