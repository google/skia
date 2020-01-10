// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(cubics_are_horrible, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->scale(0.4, 0.4);
    canvas->translate(175, 175);

    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(400);

    SkPoint p1 = SkPoint::Make(60, -40);
    SkPoint p2 = SkPoint::Make(120, 150);
    SkPoint p3 = SkPoint::Make(180, 60);

    SkPath path;
    path.moveTo(20, 60);
    path.cubicTo(p1, p2, p3);
    // path.close();

    SkPath fillpath;
    p.getFillPath(path, &fillpath);
    SkPaint fillp;
    fillp.setColor(SK_ColorMAGENTA);
    fillp.setAntiAlias(true);
    fillp.setStyle(SkPaint::kStroke_Style);
    fillp.setStrokeWidth(0);

    canvas->drawPath(path, p);
    canvas->drawPath(fillpath, fillp);
}
}  // END FIDDLE
