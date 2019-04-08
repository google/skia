/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGradientShader.h"
#include "SkSurface.h"
#include "gm.h"

static void make_transparency(SkCanvas* canvas, SkScalar width, SkScalar height) {
    SkPoint pts[2];
    pts[0] = SkPoint::Make(0, 0);
    pts[1] = SkPoint::Make(width, 0);
    const SkColor kColors[] = {
            SK_ColorBLACK,
            SK_ColorGRAY,
            SK_ColorWHITE,
            SK_ColorRED,
            SK_ColorYELLOW,
            SK_ColorGREEN,
            SK_ColorCYAN,
            SK_ColorBLUE,
            SK_ColorMAGENTA,
    };
    const SkScalar kRowHeight = height / SK_ARRAY_COUNT(kColors);
    for (size_t i = 0; i < SK_ARRAY_COUNT(kColors); ++i) {
        SkColor shaderColors[2];
        shaderColors[0] = SK_AlphaTRANSPARENT;
        shaderColors[1] = kColors[i];
        SkPaint p;
        p.setShader(SkGradientShader::MakeLinear(pts, shaderColors, nullptr, 2, SkTileMode::kClamp));
        canvas->drawRect(SkRect::MakeXYWH(0, i * kRowHeight, width, kRowHeight), p);
    }
}

// http://crrev.com/834303005
static sk_sp<SkShader> create_checkerboard_shader(SkColor c1, SkColor c2, int size) {
    SkBitmap bm;
    bm.allocN32Pixels(2 * size, 2 * size);
    bm.eraseColor(c1);
    bm.eraseArea(SkIRect::MakeLTRB(0, 0, size, size), c2);
    bm.eraseArea(SkIRect::MakeLTRB(size, size, 2 * size, 2 * size), c2);
    return bm.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat);
}

// http://crrev.com/834303005
static void checkerboard(SkCanvas* canvas, SkColor c1, SkColor c2, int size) {
    SkPaint paint;
    paint.setShader(create_checkerboard_shader(c1, c2, size));
    canvas->drawPaint(paint);
}

/**
 *  This GM verifies that a transparent bitmap drawn over a
 *  checkerboard pattern looks correct.
 */
DEF_SIMPLE_GM(transparency_check, canvas, 1792, 1080) {
    checkerboard(canvas, 0xFF999999, 0xFF666666, 8);
    {
        SkAutoCanvasRestore autoCanvasRestore(canvas, true);
        auto surface(SkSurface::MakeRasterN32Premul(256, 9));
        make_transparency(surface->getCanvas(), 256.0f, 9.0f);
        canvas->scale(7.0f, 120.0f);
        surface->draw(canvas, 0, 0, nullptr);
    }
}
