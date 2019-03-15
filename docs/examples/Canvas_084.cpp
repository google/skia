// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e91dbe45974489b8962c815017b7914f
REG_FIDDLE(Canvas_084, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(4);
    SkRect oval = { 4, 4, 60, 60};
    float intervals[] = { 5, 5 };
    paint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 2.5f));
    for (auto degrees : { 270, 360, 540, 720 } ) {
        canvas->drawArc(oval, 0, degrees, false, paint);
        canvas->translate(64, 0);
    }
}
}  // END FIDDLE
