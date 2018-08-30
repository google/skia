/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"

DEF_SIMPLE_GM(bug869172, canvas, 250, 250) {
    SkMatrix m;
    m.setAll(1.1250, 0.0000, 28.2500,
             0.0000, 1.1250, -19.2500,
             0.0000, 0.0000, 1.0000);
    canvas->concat(m);

    SkPaint paint;
    paint.setStrokeWidth(1);
    paint.setStrokeMiter(4);
    paint.setStrokeJoin(SkPaint::kBevel_Join);
    paint.setStrokeCap(SkPaint::kButt_Cap);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    paint.setAlpha(0x80);  // transparent to see overdraw

    // This is the original path from the bug translated by (10, -26.72020).
    SkPath path;
    path.setFillType(SkPath::kWinding_FillType);
    path.moveTo(10.f, 0.f);
    path.lineTo(38.f, 0.f);
    path.lineTo(66.f, 0.f);
    path.lineTo(94.f, 0.f);
    path.lineTo(122.f, 0.f);
    path.lineTo(150.f, 0.f);
    path.lineTo(150.f, 0.f);
    path.lineTo(122.f, 0.f);
    path.lineTo(94.f, 0.f);
    path.lineTo(66.f, 0.f);
    path.lineTo(38.f, 0.f);
    path.lineTo(10.f, 0.f);
    path.close();
    canvas->translate(0, 20.f);
    canvas->drawPath(path, paint);
}
