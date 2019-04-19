// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5112c7209a19e035c61cef33a624a652
REG_FIDDLE(Stroke_Width, 256, 170, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    for (bool antialias : { false, true }) {
        paint.setAntiAlias(antialias);
        for (int width = 0; width <= 4; ++width) {
            SkScalar offset = antialias * 100 + width * 20;
            paint.setStrokeWidth(width * 0.25f);
            canvas->drawLine(10 + offset,  10, 20 + offset,  60, paint);
            canvas->drawLine(10 + offset, 110, 60 + offset, 160, paint);
        }
    }
}
}  // END FIDDLE
