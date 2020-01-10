// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(SkPath_arcTo_example, 512, 512, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(SkColorSetARGB(255, 255, 255, 255));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2.5);

    SkRect oval = {64, 64, 448, 448};
    canvas->drawOval(oval, paint);
    float startAngle = 0;
    float sweepAngle = 60;

    SkPath arc;
    arc.arcTo(oval, startAngle, sweepAngle, false);

    paint.setStrokeWidth(5);
    paint.setColor(SkColorSetARGB(255, 0, 0, 255));
    canvas->drawPath(arc, paint);
}
}  // END FIDDLE
