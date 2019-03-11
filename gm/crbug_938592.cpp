/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPaint.h"
#include "gm.h"

// This draws a hard stop gradient applied to a rectangle. The hard stops fall at half pixel
// boundaries in y. On some GPUs we've found that the winding of the two triangles that make up the
// rectangle can affect whether the interpolants for local coords wind up agreeing across a row.
// When they disagree the hard stop gradient appears jagged. We draw the rectangle four times:
// no-mirroring, mirror in x, mirror in y, and mirror in x and y.
DEF_SIMPLE_GM(crbug_938592, canvas, 500, 300) {
    SkPoint pts[] = {{0, 0}, {0, 30}};
    SkScalar pos[] = {0.f, 9.f/20, 9.f/20, 11.f/20, 11.f/20, 20.f/20};
    SkColor c0 = SK_ColorBLUE;
    SkColor c1 = SK_ColorRED;
    SkColor c2 = SK_ColorGREEN;
    SkColor colors[] = {c0, c0, c1, c1, c2, c2};
    auto grad = SkGradientShader::MakeLinear(pts, colors, pos, 6, SkShader::kClamp_TileMode);
    SkPaint paint;
    paint.setShader(grad);
    static constexpr int kW = 400;
    static constexpr int kH = 200;
    canvas->translate(50, 50);
    for (int i = 0; i < 4; ++i) {
        canvas->save();
        if (i & 0b01) {
            canvas->translate(0, kH);
            canvas->scale(1.f, -1.f);
        }
        if (i & 0b10) {
            canvas->translate(kW, 0);
            canvas->scale(-1.f, 1.f);
        }
        canvas->drawRect({0, 0, 150, 30}, paint);
        canvas->restore();
    }
}
