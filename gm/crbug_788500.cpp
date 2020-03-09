/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

DEF_SIMPLE_GM(crbug_788500, canvas, 300, 300) {
    SkPath path;
    path.setFillType(SkPathFillType::kEvenOdd);
    path.moveTo(0, 0);
    path.moveTo(245.5f, 98.5f);
    path.cubicTo(245.5f, 98.5f, 242, 78, 260, 75);

    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->drawPath(path, paint);
}
