// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(issue640176, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    float startAngle = -0.5235985, endAngle = -2.439e-4, radius = 120;

    canvas->translate(radius, radius);
    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(radius * cos(startAngle), radius * sin(startAngle));
    SkRect oval = {-radius, -radius, radius, radius};
    path.arcTo(oval, startAngle * 180 / 3.14159265359,
               (endAngle - startAngle) * 180 / 3.14159265359, false);
    canvas->drawPath(path, p);

    p.setStyle(SkPaint::kStroke_Style);
    p.setColor(SK_ColorGREEN);
    canvas->drawPath(path, p);
}
}  // END FIDDLE
