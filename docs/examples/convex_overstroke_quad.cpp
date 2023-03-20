// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(convex_overstroke_quad, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const float SCALE = 1;
    const int WIDTH = 100;

    canvas->scale(SCALE, SCALE);
    canvas->translate(30, 30);

    SkPoint p1 = SkPoint::Make(50, 50);
    SkPoint p2 = SkPoint::Make(80, 50);

    SkPoint p3 = SkPoint::Make(65, 30);

    SkPath path;
    path.moveTo(p1);
    path.lineTo(p2);
    path.quadTo(p3, p1);
    // path.close();

    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(WIDTH);

    SkPath fillpath;
    skpathutils::FillPathWithPaint(path, p, &fillpath);

    SkPaint fillp;
    fillp.setColor(SK_ColorBLACK);
    fillp.setAntiAlias(true);
    fillp.setStyle(SkPaint::kStroke_Style);

    canvas->drawPath(path, p);
    canvas->drawPath(fillpath, fillp);
}
}  // END FIDDLE
