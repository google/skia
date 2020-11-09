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

static constexpr int kBoxSize    = 50;
static constexpr int kPadding    = 5;
static constexpr int kLabelWidth = 50;

static SkString make_unary_sksl_1d(const char* fn) {
    return SkStringPrintf(
            "uniform float xScale; uniform float xBias;"
            "uniform float yScale; uniform float yBias;"
            "half4 main(float2 p) {"
            "    float x = p.x   * xScale + xBias;"
            "    float y = %s(x) * yScale + yBias;"
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

    canvas->drawSimpleText(fn, strlen(fn), SkTextEncoding::kUTF8, 0.0f,
                           (kBoxSize + bounds.height()) * 0.5f, font, p);
    canvas->translate(kLabelWidth, 0);

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

        canvas->translate(kBoxSize + kPadding, 0);
    }

    canvas->restore();
    canvas->translate(0, kBoxSize + kPadding);
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.1
DEF_SIMPLE_GM_BG(runtime_intrinsics_trig,
                 canvas,
                 /*width=*/ kPadding + kLabelWidth + kBoxSize + kPadding,
                 /*height=*/ 8 * (kPadding + kBoxSize) + kPadding,
                 SK_ColorWHITE) {
    const float kPI = SK_FloatPI, kTwoPI = 2 * SK_FloatPI, kPIOverTwo = SK_FloatPI / 2;

    canvas->translate(kPadding, kPadding);

    do_unary(canvas, "radians", 0.0f, 360.0f, 0.0f, kTwoPI);
    do_unary(canvas, "degrees", 0.0f, kTwoPI, 0.0f, 360.0f);

    do_unary(canvas, "sin", 0.0f, kTwoPI,  -1.0f,  1.0f);
    do_unary(canvas, "cos", 0.0f, kTwoPI,  -1.0f,  1.0f);
    do_unary(canvas, "tan", 0.0f,    kPI, -10.0f, 10.0f);

    do_unary(canvas, "asin",  -1.0f,  1.0f, -kPIOverTwo, kPIOverTwo);
    do_unary(canvas, "acos",  -1.0f,  1.0f,        0.0f,        kPI);
    do_unary(canvas, "atan", -10.0f, 10.0f, -kPIOverTwo, kPIOverTwo);
}
