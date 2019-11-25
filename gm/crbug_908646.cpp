/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

DEF_SIMPLE_GM(crbug_908646, canvas, 300, 300) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(50,  50);
    path.lineTo(50,  300);
    path.lineTo(250, 300);
    path.lineTo(250, 50);
    path.moveTo(200, 100);
    path.lineTo(100, 100);
    path.lineTo(150, 200);
    path.moveTo(100, 250);
    path.lineTo(150, 150);
    path.lineTo(200, 250);
    canvas->drawPath(path, paint);
}
