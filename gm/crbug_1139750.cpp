/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"

DEF_SIMPLE_GM_BG(crbug_1139750, canvas, 50, 50, SK_ColorWHITE) {
    // Draw a round-rect with a (slightly) non-square scale. This forces the GPU backend to use
    // the elliptical round-rect op. We set the stroke width to exactly double the radii, which
    // makes the inner radii exactly zero. The shader uses the inverse inner radii to compute the
    // coverage ramp, so this would end up producing infinity, and the geometry would disappear.
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(2);

    SkRect r = SkRect::MakeXYWH(1, 1, 19, 19);
    SkRRect rr = SkRRect::MakeRectXY(r, 1, 1);

    canvas->translate(10, 10);
    canvas->scale(1.47619f, 1.52381f);
    canvas->drawRRect(rr, p);
}
