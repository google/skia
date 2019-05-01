/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"

// This draws a hard stop gradient applied to a rectangle. The hard stops fall at half pixel
// boundaries in y. On some GPUs we've found that the winding of the two triangles that make up the
// rectangle can affect whether the interpolants for local coords wind up agreeing across a row.
// When they disagree the hard stop gradient appears jagged. We draw the rectangle four times:
// no-mirroring, mirror in x, mirror in y, and mirror in x and y.
DEF_SIMPLE_GM(crbug_938592, canvas, 500, 300) {
    static constexpr SkPoint pts[] = {{0, 0}, {0, 30}};
    static constexpr SkScalar pos[] = {0.f, 9.f / 20, 9.f / 20, 11.f / 20, 11.f / 20, 20.f / 20};
    static constexpr SkColor c0 = SK_ColorBLUE;
    static constexpr SkColor c1 = SK_ColorRED;
    static constexpr SkColor c2 = SK_ColorGREEN;
    static constexpr SkColor colors[] = {c0, c0, c1, c1, c2, c2};
    auto grad = SkGradientShader::MakeLinear(pts, colors, pos, 6, SkTileMode::kClamp);
    SkPaint paint;
    paint.setShader(grad);
    static constexpr int kMirrorX = 400;
    static constexpr int kMirrorY = 200;
    canvas->translate(50, 50);
    for (int i = 0; i < 4; ++i) {
        canvas->save();
        if (i & 0b01) {
            canvas->translate(0, kMirrorY);
            canvas->scale(1.f, -1.f);
        }
        if (i & 0b10) {
            canvas->translate(kMirrorX, 0);
            canvas->scale(-1.f, 1.f);
        }
        canvas->drawRect({0, 0, 150, 30}, paint);
        canvas->restore();
    }
}
