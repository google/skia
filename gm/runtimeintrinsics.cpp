/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/utils/SkRandom.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

static constexpr int kBoxSize     = 100;
static constexpr int kPadding     = 5;
static constexpr int kLabelHeight = 15;

static SkString make_unary_sksl_1d(const char* fn) {
    return SkStringPrintf(
            "uniform float xScale; uniform float xBias;"
            "uniform float yScale; uniform float yBias;"
            "half4 main(float2 p) {"
            "    float x = p.x * xScale + xBias;"
            "    float y = %s  * yScale + yBias;"
            "    return y.xxx1;"
            "}",
            fn);
}

// Draws one row of boxes, then advances the canvas translation vertically
static void do_unary(
        SkCanvas* canvas, const char* fn, float xMin, float xMax, float yMin, float yMax) {
    canvas->save();

    SkFont font(ToolUtils::create_portable_typeface());
    SkPaint p(SkColors::kBlack);
    SkRect bounds;
    font.measureText(fn, strlen(fn), SkTextEncoding::kUTF8, &bounds);

    canvas->drawSimpleText(fn, strlen(fn), SkTextEncoding::kUTF8,
                           (kBoxSize - bounds.width()) * 0.5f,
                           (kLabelHeight + bounds.height()) * 0.5f, font, p);
    canvas->translate(0, kLabelHeight);

    const float xScale = xMax - xMin,
                xBias  = xMin,
                yScale = 1.0f  / (yMax - yMin),
                yBias  = -yMin / (yMax - yMin);

    {
        auto [effect, error] = SkRuntimeEffect::Make(make_unary_sksl_1d(fn));
        if (!effect) {
            SkDebugf("Error: %s\n", error.c_str());
            return;
        }

        SkRuntimeShaderBuilder builder(effect);
        builder.uniform("xScale") = xScale;
        builder.uniform("xBias")  = xBias;
        builder.uniform("yScale") = yScale;
        builder.uniform("yBias")  = yBias;

        SkPaint paint;
        paint.setShader(builder.makeShader(nullptr, false));

        SkImageInfo info = SkImageInfo::MakeN32Premul({ kBoxSize, kBoxSize});
        auto surface = canvas->makeSurface(info);
        if (!surface) {
            surface = SkSurface::MakeRaster(info);
        }

        surface->getCanvas()->clear(SK_ColorWHITE);
        surface->getCanvas()->scale(kBoxSize, kBoxSize);
        surface->getCanvas()->drawRect({0, 0, 1, 1}, paint);

        SkBitmap bitmap;
        bitmap.allocPixels(info);
        surface->readPixels(bitmap, 0, 0);

        canvas->drawBitmap(bitmap, 0, 0);

        // Plot...
        SkPaint plotPaint({ 0.0f, 0.5f, 0.0f, 1.0f });
        SkPoint pts[kBoxSize];
        for (int x = 0; x < kBoxSize; ++x) {
            SkColor c = bitmap.getColor(x, 0);
            SkScalar y = (1 - (SkColorGetR(c) / 255.0f)) * kBoxSize;
            pts[x].set(x + 0.5f, y);
        }
        canvas->drawPoints(SkCanvas::kPoints_PointMode, kBoxSize, pts, plotPaint);
    }

    canvas->restore();
}

static void col(SkCanvas* canvas) {
    canvas->translate(kBoxSize + kPadding, 0);
}

static void row(SkCanvas* canvas) {
    canvas->restore();
    canvas->translate(0, kBoxSize + kPadding + kLabelHeight);
    canvas->save();
}

static constexpr int columns_to_width(int columns) {
    return (kPadding + kBoxSize) * columns + kPadding;
}

static constexpr int rows_to_height(int rows) {
    return (kPadding + kLabelHeight + kBoxSize) * rows + kPadding;
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.1
DEF_SIMPLE_GM_BG(
        runtime_intrinsics_trig, canvas, columns_to_width(3), rows_to_height(5), SK_ColorWHITE) {
    const float kPI = SK_FloatPI, kTwoPI = 2 * SK_FloatPI, kPIOverTwo = SK_FloatPI / 2;

    canvas->translate(kPadding, kPadding);
    canvas->save();

    do_unary(canvas, "radians(x)", 0.0f, 360.0f, 0.0f, kTwoPI); col(canvas);
    do_unary(canvas, "degrees(x)", 0.0f, kTwoPI, 0.0f, 360.0f); row(canvas);

    do_unary(canvas, "sin(x)", 0.0f, kTwoPI,  -1.0f,  1.0f); col(canvas);
    do_unary(canvas, "cos(x)", 0.0f, kTwoPI,  -1.0f,  1.0f); col(canvas);
    do_unary(canvas, "tan(x)", 0.0f,    kPI, -10.0f, 10.0f); row(canvas);

    do_unary(canvas, "asin(x)",  -1.0f,  1.0f, -kPIOverTwo, kPIOverTwo); col(canvas);
    do_unary(canvas, "acos(x)",  -1.0f,  1.0f,        0.0f,        kPI); col(canvas);
    do_unary(canvas, "atan(x)", -10.0f, 10.0f, -kPIOverTwo, kPIOverTwo); row(canvas);

    do_unary(canvas, "atan(0.1,  x)", -1.0f, 1.0f,        0.0f,        kPI); col(canvas);
    do_unary(canvas, "atan(-0.1, x)", -1.0f, 1.0f,        -kPI,       0.0f); row(canvas);

    do_unary(canvas, "atan(x,  0.1)", -1.0f, 1.0f, -kPIOverTwo, kPIOverTwo); col(canvas);
    do_unary(canvas, "atan(x, -0.1)", -1.0f, 1.0f,        -kPI,        kPI); row(canvas);
}
