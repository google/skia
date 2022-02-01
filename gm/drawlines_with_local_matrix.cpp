/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/effects/SkGradientShader.h"

DEF_SIMPLE_GM(drawlines_with_local_matrix, canvas, 500, 500) {
    canvas->clipRect({0,0,500,500});
    SkPaint grad;
    grad.setAntiAlias(true);
    grad.setStrokeCap(SkPaint::kSquare_Cap);
    float pos[6] = {0, 2/6.f, 3/6.f, 4/6.f, 5/6.f, 1};
    constexpr SkColor indigo = SkColorSetARGB(0xFF, 0x4b, 0x00, 0x82);
    constexpr SkColor violet = SkColorSetARGB(0xFF, 0xee, 0x82, 0xee);
    SkColor colors[6] = {SK_ColorRED, SK_ColorYELLOW, SK_ColorGREEN, SK_ColorBLUE, indigo, violet};
    grad.setShader(SkGradientShader::MakeRadial({250,250}, 280, colors, pos, 6,SkTileMode::kClamp));
    canvas->drawPaint(grad);

    SkPaint white;
    white.setAntiAlias(true);
    white.setStrokeCap(SkPaint::kSquare_Cap);
    white.setColor(SK_ColorWHITE);

    auto drawLine = [&](float x0, float y0, float x1, float y1, float w) {
        SkPoint p[2] = {{x0, y0}, {x1, y1}};
        white.setStrokeWidth(w);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 2, p, white);
        grad.setStrokeWidth(w - 4);
        canvas->drawPoints(SkCanvas::kLines_PointMode, 2, p, grad);
    };

    drawLine(20, 20, 200, 120, 20);
    drawLine(20, 200, 20, 100, 20);
    drawLine(480, 20, 400, 400, 20);
    drawLine(50, 480, 260, 100, 20);
    drawLine(270, 20, 380, 210, 20);
    drawLine(280, 280, 400, 480, 20);
    drawLine(160, 375, 280, 375, 20);
    drawLine(220, 410, 220, 470, 20);
    drawLine(250, 250, 250, 250, 20);
}
