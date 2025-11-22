// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(convex_overstroke_linear_approx_curve, 256, 256, false, 0) {
#include <math.h>

void draw(SkCanvas* canvas) {
    const float SCALE = 1;
    const int WIDTH = 150;

    const float PI = 3.1415926;

    canvas->scale(SCALE, SCALE);
    canvas->translate(50, 50);

    SkPoint p1 = SkPoint::Make(50, 50);
    SkPoint p2 = SkPoint::Make(80, 50);

    SkPoint points[10];

    for (int i = 0; i < 10; i++) {
        points[i] = SkPoint::Make(65 + 15 * cos(i * PI / 10), 50 - 15 * sin(i * PI / 10));
    }

    SkPathBuilder builder;
    builder.moveTo(p1);
    builder.lineTo(p2);

    for (int i = 0; i < 10; i++) {
        builder.lineTo(points[i]);
    }
    builder.lineTo(p1);
    // builder.close();
    SkPath path = builder.detach();

    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(WIDTH);

    canvas->drawPath(path, p);

    SkPath fillpath = skpathutils::FillPathWithPaint(path, p);

    SkPaint fillp;
    fillp.setColor(SK_ColorBLACK);
    fillp.setAntiAlias(true);
    fillp.setStyle(SkPaint::kStroke_Style);
    fillp.setStrokeWidth(1);

    canvas->drawPath(fillpath, fillp);
}
}  // END FIDDLE
