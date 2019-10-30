/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

DEF_SIMPLE_GM(crbug_887103, canvas, 520, 520) {
    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);

    SkPath path;
    path.moveTo(510,  20);
    path.lineTo(500,  20);
    path.lineTo(510, 500);

    path.moveTo(500,  20);
    path.lineTo(510, 500);
    path.lineTo(500, 510);

    path.moveTo(500,  30);
    path.lineTo(510,  10);
    path.lineTo( 10,  30);
    canvas->drawPath(path, paint);
}
