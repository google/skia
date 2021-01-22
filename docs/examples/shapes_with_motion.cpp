// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(shapes_with_motion, 256, 256, false, 5, 4) {
void draw(SkCanvas* canvas) {
    //float p = 1 - fabs(2 * frame - 1);
    float p = 0.5 * (1 - cos(6.28318548f * frame));
    canvas->drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(4);
    paint.setColor(0xffFE938C);

    SkRect rect = SkRect::MakeXYWH(10, 10, 100, 160);
    canvas->drawRect(rect, paint);

    SkRRect oval;
    oval.setOval(rect);
    oval.offset(40, p * 80);
    paint.setColor(0xffE6B89C);
    canvas->drawRRect(oval, paint);

    paint.setColor(0xff9CAFB7);
    canvas->drawCircle(180 * p, 50, 25, paint);

    rect.offset(80, 50);
    paint.setColor(0xff4281A4);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRoundRect(rect, 10, 10, paint);
}
}  // END FIDDLE
