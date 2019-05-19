/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"

// Exposed a bug in ellipse rendering where the radii were wrong under 90 degree rotation.
DEF_SIMPLE_GM(crbug_946965, canvas, 75, 150) {
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->translate(25, 80);
    canvas->rotate(90.f);
    canvas->scale(1.5, 1);
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(-20, -5, 20, 5), 10, 10);
    canvas->drawRRect(rrect, paint);
    canvas->translate(0, -20);
    paint.setStrokeWidth(3.f);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRRect(rrect, paint);
}
