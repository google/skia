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

/*
  Test cases are inserted into the middle of this shader. The pasted expression is expected to
  produce a single float. It can reference:

    'x'  : float  in [xMin, xMax]
    'p'  : float2 in [xMin, xMax]  Lerps from (xMax, xMin) to (xMin, xMax)
                                   (helpful for intrinsics with a mix of scalar/vector params)
    'v1' : float2(1)
    'v2' : float2(2)
*/
static SkString make_unary_sksl_1d(const char* fn) {
    return SkStringPrintf(
            "uniform float xScale; uniform float xBias;"
            "uniform float yScale; uniform float yBias;"
            "half4 main(float2 p) {"
            "    float2 v1 = float2(1);"
            "    float2 v2 = float2(2);"
            "    p = float2(p.x, 1 - p.x) * xScale + xBias;"
            "    float x = p.x;"
            "    float y = %s  * yScale + yBias;"
            "    return y.xxx1;"
            "}",
            fn);
}

// Draws one row of boxes, then advances the canvas translation vertically
static void plot(SkCanvas* canvas,
                 const char* fn,
                 float xMin,
                 float xMax,
                 float yMin,
                 float yMax,
                 const char* label = nullptr) {
    canvas->save();

    SkFont font(ToolUtils::create_portable_typeface());
    SkPaint p(SkColors::kBlack);
    SkRect bounds;
    if (!label) {
        label = fn;
    }
    font.measureText(label, strlen(label), SkTextEncoding::kUTF8, &bounds);

    canvas->drawSimpleText(label, strlen(label), SkTextEncoding::kUTF8,
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

    plot(canvas, "radians(x)", 0.0f, 360.0f, 0.0f, kTwoPI); col(canvas);
    plot(canvas, "degrees(x)", 0.0f, kTwoPI, 0.0f, 360.0f); row(canvas);

    plot(canvas, "sin(x)", 0.0f, kTwoPI,  -1.0f,  1.0f); col(canvas);
    plot(canvas, "cos(x)", 0.0f, kTwoPI,  -1.0f,  1.0f); col(canvas);
    plot(canvas, "tan(x)", 0.0f,    kPI, -10.0f, 10.0f); row(canvas);

    plot(canvas, "asin(x)",  -1.0f,  1.0f, -kPIOverTwo, kPIOverTwo); col(canvas);
    plot(canvas, "acos(x)",  -1.0f,  1.0f,        0.0f,        kPI); col(canvas);
    plot(canvas, "atan(x)", -10.0f, 10.0f, -kPIOverTwo, kPIOverTwo); row(canvas);

    plot(canvas, "atan(0.1,  x)", -1.0f, 1.0f,        0.0f,        kPI); col(canvas);
    plot(canvas, "atan(-0.1, x)", -1.0f, 1.0f,        -kPI,       0.0f); row(canvas);

    plot(canvas, "atan(x,  0.1)", -1.0f, 1.0f, -kPIOverTwo, kPIOverTwo); col(canvas);
    plot(canvas, "atan(x, -0.1)", -1.0f, 1.0f,        -kPI,        kPI); row(canvas);
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.2
DEF_SIMPLE_GM_BG(runtime_intrinsics_exponential,
                 canvas,
                 columns_to_width(2),
                 rows_to_height(5),
                 SK_ColorWHITE) {
    canvas->translate(kPadding, kPadding);
    canvas->save();

    plot(canvas, "pow(x, 3)",  0.0f, 8.0f, 0.0f, 500.0f); col(canvas);
    plot(canvas, "pow(x, -3)", 0.0f, 4.0f, 0.0f,  10.0f); row(canvas);

    plot(canvas, "pow(0.9, x)", -10.0f, 10.0f, 0.0f, 3.0f); col(canvas);
    plot(canvas, "pow(1.1, x)", -10.0f, 10.0f, 0.0f, 3.0f); row(canvas);

    plot(canvas, "exp(x)", -1.0f, 7.0f,  0.0f, 1000.0f); col(canvas);
    plot(canvas, "log(x)",  0.0f, 2.5f, -4.0f,    1.0f); row(canvas);

    plot(canvas, "exp2(x)", -1.0f, 7.0f,  0.0f, 130.0f); col(canvas);
    plot(canvas, "log2(x)",  0.0f, 4.0f, -4.0f,   2.0f); row(canvas);

    plot(canvas,        "sqrt(x)", 0.0f, 25.0f, 0.0f, 5.0f); col(canvas);
    plot(canvas, "inversesqrt(x)", 0.0f, 25.0f, 0.2f, 4.0f); row(canvas);
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.3
DEF_SIMPLE_GM_BG(runtime_intrinsics_common,
                 canvas,
                 columns_to_width(6),
                 rows_to_height(6),
                 SK_ColorWHITE) {
    canvas->translate(kPadding, kPadding);
    canvas->save();

    plot(canvas, "abs(x)",  -10.0f, 10.0f, 0.0f, 10.0f); col(canvas);
    plot(canvas, "sign(x)",  -1.0f,  1.0f, -1.5f, 1.5f); row(canvas);

    plot(canvas, "floor(x)", -3.0f, 3.0f, -4.0f, 4.0f); col(canvas);
    plot(canvas, "ceil(x)",  -3.0f, 3.0f, -4.0f, 4.0f); col(canvas);
    plot(canvas, "fract(x)", -3.0f, 3.0f,  0.0f, 1.0f); col(canvas);
    plot(canvas, "mod(x, 2)",    -4.0f, 4.0f, -2.0f, 2.0f, "mod(scalar)"); col(canvas);
    plot(canvas, "mod(p, -2).x", -4.0f, 4.0f, -2.0f, 2.0f, "mod(mixed)" ); col(canvas);
    plot(canvas, "mod(p, v2).x", -4.0f, 4.0f, -2.0f, 2.0f, "mod(vector)"); row(canvas);

    plot(canvas, "min(x, 1)",    0.0f, 2.0f, 0.0f, 2.0f, "min(scalar)"); col(canvas);
    plot(canvas, "min(p, 1).x",  0.0f, 2.0f, 0.0f, 2.0f, "min(mixed)" ); col(canvas);
    plot(canvas, "min(p, v1).x", 0.0f, 2.0f, 0.0f, 2.0f, "min(vector)"); col(canvas);
    plot(canvas, "max(x, 1)",    0.0f, 2.0f, 0.0f, 2.0f, "max(scalar)"); col(canvas);
    plot(canvas, "max(p, 1).x",  0.0f, 2.0f, 0.0f, 2.0f, "max(mixed)" ); col(canvas);
    plot(canvas, "max(p, v1).x", 0.0f, 2.0f, 0.0f, 2.0f, "max(vector)"); row(canvas);

    plot(canvas, "clamp(x, 1, 2)",     0.0f, 3.0f, 0.0f, 3.0f, "clamp(scalar)"); col(canvas);
    plot(canvas, "clamp(p, 1, 2).x",   0.0f, 3.0f, 0.0f, 3.0f, "clamp(mixed)" ); col(canvas);
    plot(canvas, "clamp(p, v1, v2).x", 0.0f, 3.0f, 0.0f, 3.0f, "clamp(vector)"); col(canvas);
    plot(canvas, "saturate(x)", -1.0f, 2.0f, -0.5f, 1.5f); row(canvas);

    plot(canvas, "mix(1, 2, x)",     -1.0f, 2.0f, 0.0f, 3.0f, "mix(scalar)"); col(canvas);
    plot(canvas, "mix(v1, v2, x).x", -1.0f, 2.0f, 0.0f, 3.0f, "mix(mixed)" ); col(canvas);
    plot(canvas, "mix(v1, v2, p).x", -1.0f, 2.0f, 0.0f, 3.0f, "mix(vector)"); row(canvas);

    plot(canvas, "step(1, x)",    0.0f, 2.0f, -0.5f, 1.5f, "step(scalar)"); col(canvas);
    plot(canvas, "step(1, p).x",  0.0f, 2.0f, -0.5f, 1.5f, "step(mixed)" ); col(canvas);
    plot(canvas, "step(v1, p).x", 0.0f, 2.0f, -0.5f, 1.5f, "step(vector)"); col(canvas);
    plot(canvas, "smoothstep(1, 2, x)",     0.5f, 2.5f, -0.5f, 1.5f, "smooth(scalar)"); col(canvas);
    plot(canvas, "smoothstep(1, 2, p).x",   0.5f, 2.5f, -0.5f, 1.5f, "smooth(mixed)" ); col(canvas);
    plot(canvas, "smoothstep(v1, v2, p).x", 0.5f, 2.5f, -0.5f, 1.5f, "smooth(vector)"); row(canvas);
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.4
DEF_SIMPLE_GM_BG(runtime_intrinsics_geometric,
                 canvas,
                 columns_to_width(4),
                 rows_to_height(5),
                 SK_ColorWHITE) {
    canvas->translate(kPadding, kPadding);
    canvas->save();

    plot(canvas, "length(x)",  -1.0f, 1.0f, -0.5f, 1.5f); col(canvas);
    plot(canvas, "length(p)",   0.0f, 1.0f,  0.5f, 1.5f); col(canvas);

    plot(canvas, "distance(x, 0)",  -1.0f, 1.0f, -0.5f, 1.5f); col(canvas);
    plot(canvas, "distance(p, v1)",  0.0f, 1.0f,  0.5f, 1.5f); row(canvas);

    plot(canvas, "dot(x, 2)",    -1.0f, 1.0f, -2.5f, 2.5f); col(canvas);
    plot(canvas, "dot(p, p.y1)", -1.0f, 1.0f, -2.5f, 0.5f); row(canvas);

    plot(canvas, "cross(p.xy1, p.y1x).x", 0.0f, 1.0f, -1.0f, 1.0f); col(canvas);
    plot(canvas, "cross(p.xy1, p.y1x).y", 0.0f, 1.0f, -1.0f, 1.0f); col(canvas);
    plot(canvas, "cross(p.xy1, p.y1x).z", 0.0f, 1.0f, -1.0f, 1.0f); row(canvas);

    plot(canvas, "normalize(x)",   -2.0f, 2.0f, -1.5f, 1.5f); col(canvas);
    plot(canvas, "normalize(p).x",  0.0f, 2.0f,  0.0f, 1.0f); col(canvas);
    plot(canvas, "normalize(p).y",  0.0f, 2.0f,  0.0f, 1.0f); col(canvas);

    plot(canvas, "faceforward(v1, p.x0, v1.x0).x", -1.0f, 1.0f, -1.5f, 1.5f, "faceforward"); row(canvas);

    plot(canvas, "reflect(p.x1, v1.0x).x",         -1.0f, 1.0f, -1.0f, 1.0f, "reflect(horiz)"); col(canvas);
    plot(canvas, "reflect(p.x1, normalize(v1)).y", -1.0f, 1.0f, -1.0f, 1.0f, "reflect(diag)" ); col(canvas);

    plot(canvas, "refract(v1.x0, v1.0x, x).x", 0.0f, 1.0f, -1.0f, 1.0f, "refract().x"); col(canvas);
    plot(canvas, "refract(v1.x0, v1.0x, x).y", 0.0f, 1.0f, -1.0f, 1.0f, "refract().y"); row(canvas);
}

/*
  Specialized shader for testing relational operators.
*/
static SkString make_bvec_sksl(const char* fn) {
    return SkStringPrintf(
            "half4 main(float2 p) {"
            "    float2 v1 = float2(1.0);"
            "    p.x = p.x < 0.33 ? 0.0 : (p.x < 0.66 ? 1.0 : 2.0);"
            "    p.y = p.y < 0.33 ? 0.0 : (p.y < 0.66 ? 1.0 : 2.0);"
            "    bool2 cmp = %s;"
            "    return half4(cmp.x ? 1.0 : 0.0, cmp.y ? 1.0 : 0.0, 0, 1);"
            "}",
            fn);
}

static void plot_bvec(SkCanvas* canvas, const char* fn, const char* label = nullptr) {
    canvas->save();

    SkFont font(ToolUtils::create_portable_typeface());
    SkPaint p(SkColors::kBlack);
    SkRect bounds;
    if (!label) {
        label = fn;
    }
    font.measureText(label, strlen(label), SkTextEncoding::kUTF8, &bounds);

    canvas->drawSimpleText(label, strlen(label), SkTextEncoding::kUTF8,
                           (kBoxSize - bounds.width()) * 0.5f,
                           (kLabelHeight + bounds.height()) * 0.5f, font, p);
    canvas->translate(0, kLabelHeight);

    {
        auto [effect, error] = SkRuntimeEffect::Make(make_bvec_sksl(fn));
        if (!effect) {
            SkDebugf("Error: %s\n", error.c_str());
            return;
        }

        SkRuntimeShaderBuilder builder(effect);

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
    }

    canvas->restore();
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.6
DEF_SIMPLE_GM_BG(runtime_intrinsics_relational,
                 canvas,
                 columns_to_width(2),
                 rows_to_height(6),
                 SK_ColorWHITE) {
    canvas->translate(kPadding, kPadding);
    canvas->save();

    // TODO: ivec versions of these. (Not declared in sksl_public.sksl yet).

    plot_bvec(canvas, "lessThan(p, v1)",      "lessThan");      col(canvas);
    plot_bvec(canvas, "lessThanEqual(p, v1)", "lessThanEqual"); row(canvas);

    plot_bvec(canvas, "greaterThan(p, v1)",      "greaterThan");      col(canvas);
    plot_bvec(canvas, "greaterThanEqual(p, v1)", "greaterThanEqual"); row(canvas);

    plot_bvec(canvas, "equal(p, v1)",    "equal");    col(canvas);
    plot_bvec(canvas, "notEqual(p, v1)", "notEqual"); row(canvas);

    plot_bvec(canvas, "equal(lessThanEqual(p, v1), greaterThanEqual(p, v1))",
                      "equal(bvec)"); col(canvas);
    plot_bvec(canvas, "notEqual(lessThanEqual(p, v1), greaterThanEqual(p, v1))",
                      "notequal(bvec)"); row(canvas);

    plot_bvec(canvas, "not(notEqual(p, v1))", "not(notEqual)"); col(canvas);
    plot_bvec(canvas, "not(equal(p, v1))",    "not(equal)");    row(canvas);

    plot_bvec(canvas, "bool2(any(equal(p, v1)))", "any(equal)"); col(canvas);
    plot_bvec(canvas, "bool2(all(equal(p, v1)))", "all(equal)"); row(canvas);
}
