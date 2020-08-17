/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"

// See crbug.com/1086705. The convex linearizing path renderer would collapse too many of the
// very-near duplicate vertices and turn the path into a triangle. Since the stroke width is larger
// than the radius of the circle, there's the separate issue of what to do when stroke
// self-intersects
DEF_SIMPLE_GM(crbug_1086705, canvas, 200, 200) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(5.f);
    paint.setAntiAlias(true);

    SkPoint circleVertices[700];
    for (int i = 0; i < 700; ++i) {
        SkScalar angleRads = 2 * SK_ScalarPI * i / 700.f;
        circleVertices[i] = {100.f + 2.f * SkScalarCos(angleRads),
                             100.f + 2.f * SkScalarSin(angleRads)};
    }

    SkPathBuilder circle;
    circle.moveTo(circleVertices[0]);
    for (int i = 1; i < 700; ++i) {
        circle.lineTo(circleVertices[i]);
    }
    circle.close();

    canvas->drawPath(circle.detach(), paint);
}
