/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkPaint.h"
#include "gm.h"
#include "SkPath.h"
#include "SkRRect.h"

// Exposed a bug in ellipse rendering where the radii were wrong under 90 degree rotation.
DEF_SIMPLE_GM(crbug_946965, canvas, 50, 175) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeWidth(10);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->translate(25, 80);
    canvas->rotate(90.f);
    canvas->scale(3, 1);
    canvas->drawLine(-20, 0, 20, 0, paint);
}
