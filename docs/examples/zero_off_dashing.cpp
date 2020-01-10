// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(zero_off_dashing, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint p;
    canvas->drawCircle(128, 128, 60, p);

    p.setColor(0x88FF0000);
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeCap(SkPaint::kSquare_Cap);
    p.setStrokeWidth(80);
    SkScalar interv[] = { 120*M_PI/6 - 0.05, 0.0000 };
    p.setPathEffect(SkDashPathEffect::Make(interv, 2, 0.5));

    SkPath path, path2;
    path.addCircle(128, 128, 60);
    canvas->drawPath(path, p);

p.setColor(0x8800FF00);
SkScalar interv2[] = { 120*M_PI/6 - 0.05, 10000.0000 };
    p.setPathEffect(SkDashPathEffect::Make(interv2, 2, 0));
canvas->drawPath(path, p);

    p.getFillPath(path, &path2);
    p.setColor(0xFF000000);
    p.setStrokeWidth(0);
    p.setPathEffect(nullptr);
    canvas->drawPath(path2, p);
}
}  // END FIDDLE
