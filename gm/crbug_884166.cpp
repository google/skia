/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

DEF_SIMPLE_GM(crbug_884166, canvas, 300, 300) {
    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);

    SkPath path;
    path.moveTo(153.25, 280.75);
    path.lineTo(161.75, 281.75);
    path.lineTo(164.25, 282.00);
    path.lineTo(  0.00, 276.00);
    path.lineTo(161.50,   0.00);
    path.lineTo(286.25, 231.25);
    path.lineTo(163.75, 282.00);
    path.lineTo(150.00, 280.00);
    canvas->drawPath(path, paint);
}
