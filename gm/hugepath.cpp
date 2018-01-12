/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

DEF_SIMPLE_GM(path_huge_crbug_800804, canvas, 30, 600) {
    SkPaint paint;
    paint.setAntiAlias(true);

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    SkPath path;
    path.moveTo(-1000,12345678901234567890.f);
    path.lineTo(10.5f,200);
    canvas->drawPath(path, paint);

    path.reset();
    path.moveTo(20.5f,400);
    path.lineTo(1000,-9.8765432109876543210e+19f);
    canvas->drawPath(path, paint);
}

