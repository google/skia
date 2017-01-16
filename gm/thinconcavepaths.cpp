/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

namespace {
// Test thin stroked rect (stroked "by hand", not by stroking).
void draw_thin_stroked_rect(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(10 + width, 10 + width);
    path.lineTo(40,         10 + width);
    path.lineTo(40,         20);
    path.lineTo(10 + width, 20);
    path.moveTo(10,         10);
    path.lineTo(10,         20 + width);
    path.lineTo(40 + width, 20 + width);
    path.lineTo(40 + width, 10);
    canvas->drawPath(path, paint);
}

void draw_thin_right_angle(SkCanvas* canvas, const SkPaint& paint, SkScalar width) {
    SkPath path;
    path.moveTo(10 + width, 10 + width);
    path.lineTo(40,         10 + width);
    path.lineTo(40,         20);
    path.lineTo(40 + width, 20 + width);
    path.lineTo(40 + width, 10);
    path.lineTo(10,         10);
    canvas->drawPath(path, paint);
}

};

DEF_SIMPLE_GM(thinconcavepaths, canvas, 400, 400) {
    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);

    canvas->save();
    for (SkScalar width = 1.0; width < 2.05; width += 0.25) {
        draw_thin_stroked_rect(canvas, paint, width);
        canvas->translate(0, 25);
    }
    canvas->restore();
    canvas->translate(50, 0);
    canvas->save();
    for (SkScalar width = 0.5; width < 2.05; width += 0.25) {
        draw_thin_right_angle(canvas, paint, width);
        canvas->translate(0, 25);
    }
    canvas->restore();
    canvas->translate(100, 0);
}
