/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
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
#include "src/base/SkRandom.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

static constexpr int kBoxSize     = 100;
static constexpr int kPadding     = 5;
static constexpr int kLabelHeight = 15;

static void next_column(SkCanvas* canvas) {
    canvas->translate(kBoxSize + kPadding, 0);
}

static void next_row(SkCanvas* canvas) {
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

static void draw_label(SkCanvas* canvas, const char* label) {
    SkFont font = ToolUtils::DefaultPortableFont();
    SkPaint p(SkColors::kBlack);
    SkRect bounds;
    font.measureText(label, strlen(label), SkTextEncoding::kUTF8, &bounds);

    canvas->drawSimpleText(label, strlen(label), SkTextEncoding::kUTF8,
                           (kBoxSize - bounds.width()) * 0.5f,
                           (kLabelHeight + bounds.height()) * 0.5f, font, p);
    canvas->translate(0, kLabelHeight);
}

static SkBitmap draw_shader(SkCanvas* canvas, sk_sp<SkShader> shader,
                            bool allowRasterFallback = true) {
    SkPaint paint;
    paint.setShader(std::move(shader));

    SkBitmap bitmap;
    SkImageInfo info = SkImageInfo::MakeN32Premul({kBoxSize, kBoxSize});
    auto surface = canvas->makeSurface(info);
    if (allowRasterFallback && !surface) {
        surface = SkSurfaces::Raster(info);
    }

    if (surface) {
        surface->getCanvas()->clear(SK_ColorWHITE);
        surface->getCanvas()->scale(kBoxSize, kBoxSize);
        surface->getCanvas()->drawRect({0, 0, 1, 1}, paint);

        bitmap.allocPixels(info);
        surface->readPixels(bitmap, 0, 0);

        canvas->drawImage(bitmap.asImage(), 0, 0);
    }
    return bitmap;
}

/*
  Test cases are inserted into the middle of this shader. The pasted expression is expected to
  produce a single float. It can reference:

    'x'  : float  in [xMin, xMax]
    'xi' : int    in [xMin, xMax]
    'p'  : float2 in [xMin, xMax]  Lerps from (xMax, xMin) to (xMin, xMax)
    'pi' : int2   in [xMin, xMax]  Lerps from (xMax, xMin) to (xMin, xMax)
                                   (helpful for intrinsics with a mix of scalar/vector params)
    'v1' : float2(1)
    'v2' : float2(2)
*/
static SkString make_unary_sksl_1d(const char* fn, bool requireES3) {
    return SkStringPrintf(
            "#version %s\n"
            "uniform float xScale; uniform float xBias;"
            "uniform float yScale; uniform float yBias;"
            "half4 main(float2 p) {"
            "    const float2 v1 = float2(1);"
            "    const float2 v2 = float2(2);"
            "    p = float2(p.x, 1 - p.x) * xScale + xBias;"
            "    float x = p.x;"
            "    int2  pi = int2(floor(p));"
            "    int   xi = pi.x;"
            "    float y = float(%s) * yScale + yBias;"
            "    return y.xxx1;"
            "}",
            requireES3 ? "300" : "100", fn);
}

// Draws one row of boxes, then advances the canvas translation vertically
static void plot(SkCanvas* canvas,
                 const char* fn,
                 float xMin,
                 float xMax,
                 float yMin,
                 float yMax,
                 const char* label = nullptr,
                 bool requireES3 = false) {
    canvas->save();

    draw_label(canvas, label ? label : fn);

    auto [effect, error] = SkRuntimeEffect::MakeForShader(make_unary_sksl_1d(fn, requireES3));
    if (!effect) {
        SkDebugf("Error: %s\n", error.c_str());
        return;
    }

    SkRuntimeShaderBuilder builder(effect);
    builder.uniform("xScale") = xMax - xMin;
    builder.uniform("xBias")  = xMin;
    builder.uniform("yScale") = 1.0f  / (yMax - yMin);
    builder.uniform("yBias")  = -yMin / (yMax - yMin);

    SkBitmap bitmap =
            draw_shader(canvas, builder.makeShader(), /*allowRasterFallback=*/!requireES3);
    if (!bitmap.empty()) {
        // Plot.
        SkPaint plotPaint({ 0.0f, 0.5f, 0.0f, 1.0f });
        SkPoint pts[kBoxSize];
        for (int x = 0; x < kBoxSize; ++x) {
            SkColor c = bitmap.getColor(x, 0);
            SkScalar y = (1 - (SkColorGetR(c) / 255.0f)) * kBoxSize;
            pts[x].set(x + 0.5f, y);
        }
        plotPaint.setAntiAlias(true);
        canvas->drawPoints(SkCanvas::kPolygon_PointMode, kBoxSize, pts, plotPaint);
    }

    canvas->restore();
    next_column(canvas);
}

static void plot_es3(SkCanvas* canvas,
                     const char* fn,
                     float xMin,
                     float xMax,
                     float yMin,
                     float yMax,
                     const char* label = nullptr) {
    plot(canvas, fn, xMin, xMax, yMin, yMax, label, /*requireES3=*/true);
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.1
DEF_SIMPLE_GM(runtime_intrinsics_trig,
              canvas,
              columns_to_width(3),
              rows_to_height(5)) {
    const float kPI = SK_FloatPI, kTwoPI = 2 * SK_FloatPI, kPIOverTwo = SK_FloatPI / 2;

    canvas->translate(kPadding, kPadding);
    canvas->save();

    plot(canvas, "radians(x)", 0.0f, 360.0f, 0.0f, kTwoPI);
    plot(canvas, "degrees(x)", 0.0f, kTwoPI, 0.0f, 360.0f);
    next_row(canvas);

    plot(canvas, "sin(x)", 0.0f, kTwoPI,  -1.0f,  1.0f);
    plot(canvas, "cos(x)", 0.0f, kTwoPI,  -1.0f,  1.0f);
    plot(canvas, "tan(x)", 0.0f,    kPI, -10.0f, 10.0f);
    next_row(canvas);

    plot(canvas, "asin(x)",  -1.0f,  1.0f, -kPIOverTwo, kPIOverTwo);
    plot(canvas, "acos(x)",  -1.0f,  1.0f,        0.0f,        kPI);
    plot(canvas, "atan(x)", -10.0f, 10.0f, -kPIOverTwo, kPIOverTwo);
    next_row(canvas);

    plot(canvas, "atan(0.1,  x)", -1.0f, 1.0f,        0.0f,        kPI);
    plot(canvas, "atan(-0.1, x)", -1.0f, 1.0f,        -kPI,       0.0f);
    next_row(canvas);

    plot(canvas, "atan(x,  0.1)", -1.0f, 1.0f, -kPIOverTwo, kPIOverTwo);
    plot(canvas, "atan(x, -0.1)", -1.0f, 1.0f,        -kPI,        kPI);
    next_row(canvas);
}

// The OpenGL ES Shading Language, Version 3.00, Section 8.1
DEF_SIMPLE_GPU_GM_CAN_FAIL(runtime_intrinsics_trig_es3,
                           ctx, canvas, errorMsg,
                           columns_to_width(3),
                           rows_to_height(2)) {
    if (ctx->priv().caps()->shaderCaps()->supportedSkSLVerion() < SkSL::Version::k300) {
        *errorMsg = "SkSL 300 is not supported.";
        return skiagm::DrawResult::kSkip;
    }

    canvas->translate(kPadding, kPadding);
    canvas->save();

    plot_es3(canvas, "sinh(x)", -2.0f,  2.0f, -4.0f, 4.0f);
    plot_es3(canvas, "cosh(x)", -2.0f,  2.0f,  0.0f, 4.0f);
    plot_es3(canvas, "tanh(x)", -2.0f,  2.0f, -1.0f, 1.0f);
    next_row(canvas);

    if (ctx->priv().caps()->shaderCaps()->fInverseHyperbolicSupport) {
        plot_es3(canvas, "asinh(x)", -2.0f, 2.0f, -2.0f, 2.0f);
        plot_es3(canvas, "acosh(x)",  0.0f, 5.0f,  0.0f, 3.0f);
        plot_es3(canvas, "atanh(x)", -1.0f, 1.0f, -4.0f, 4.0f);
    }
    next_row(canvas);

    return skiagm::DrawResult::kOk;
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.2
DEF_SIMPLE_GM(runtime_intrinsics_exponential,
              canvas,
              columns_to_width(2),
              rows_to_height(5)) {
    canvas->translate(kPadding, kPadding);
    canvas->save();

    plot(canvas, "pow(x, 3)",  0.0f, 8.0f, 0.0f, 500.0f);
    plot(canvas, "pow(x, -3)", 0.0f, 4.0f, 0.0f,  10.0f);
    next_row(canvas);

    plot(canvas, "pow(0.9, x)", -10.0f, 10.0f, 0.0f, 3.0f);
    plot(canvas, "pow(1.1, x)", -10.0f, 10.0f, 0.0f, 3.0f);
    next_row(canvas);

    plot(canvas, "exp(x)", -1.0f, 7.0f,  0.0f, 1000.0f);
    plot(canvas, "log(x)",  0.0f, 2.5f, -4.0f,    1.0f);
    next_row(canvas);

    plot(canvas, "exp2(x)", -1.0f, 7.0f,  0.0f, 130.0f);
    plot(canvas, "log2(x)",  0.0f, 4.0f, -4.0f,   2.0f);
    next_row(canvas);

    plot(canvas,        "sqrt(x)", 0.0f, 25.0f, 0.0f, 5.0f);
    plot(canvas, "inversesqrt(x)", 0.0f, 25.0f, 0.2f, 4.0f);
    next_row(canvas);
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.3
DEF_SIMPLE_GM(runtime_intrinsics_common,
              canvas,
              columns_to_width(6),
              rows_to_height(7)) {
    canvas->translate(kPadding, kPadding);
    canvas->save();

    plot(canvas, "abs(x)",  -10.0f, 10.0f, 0.0f, 10.0f);
    plot(canvas, "sign(x)",  -1.0f,  1.0f, -1.5f, 1.5f);
    next_row(canvas);

    plot(canvas, "floor(x)",     -3.0f, 3.0f, -4.0f, 4.0f);
    plot(canvas, "ceil(x)",      -3.0f, 3.0f, -4.0f, 4.0f);
    plot(canvas, "fract(x)",     -3.0f, 3.0f,  0.0f, 1.0f);
    plot(canvas, "mod(x, 2)",    -4.0f, 4.0f, -2.0f, 2.0f, "mod(scalar)");
    plot(canvas, "mod(p, -2).x", -4.0f, 4.0f, -2.0f, 2.0f, "mod(mixed)" );
    plot(canvas, "mod(p, v2).x", -4.0f, 4.0f, -2.0f, 2.0f, "mod(vector)");
    next_row(canvas);

    plot(canvas, "min(x, 1)",    0.0f, 2.0f, 0.0f, 2.0f, "min(scalar)");
    plot(canvas, "min(p, 1).x",  0.0f, 2.0f, 0.0f, 2.0f, "min(mixed)" );
    plot(canvas, "min(p, v1).x", 0.0f, 2.0f, 0.0f, 2.0f, "min(vector)");
    plot(canvas, "max(x, 1)",    0.0f, 2.0f, 0.0f, 2.0f, "max(scalar)");
    plot(canvas, "max(p, 1).x",  0.0f, 2.0f, 0.0f, 2.0f, "max(mixed)" );
    plot(canvas, "max(p, v1).x", 0.0f, 2.0f, 0.0f, 2.0f, "max(vector)");
    next_row(canvas);

    plot(canvas, "clamp(x, 1, 2)",     0.0f, 3.0f, 0.0f, 3.0f, "clamp(scalar)");
    plot(canvas, "clamp(p, 1, 2).x",   0.0f, 3.0f, 0.0f, 3.0f, "clamp(mixed)" );
    plot(canvas, "clamp(p, v1, v2).x", 0.0f, 3.0f, 0.0f, 3.0f, "clamp(vector)");
    plot(canvas, "saturate(x)", -1.0f, 2.0f, -0.5f, 1.5f);
    next_row(canvas);

    plot(canvas, "mix(1, 2, x)",     -1.0f, 2.0f, 0.0f, 3.0f, "mix(scalar)");
    plot(canvas, "mix(v1, v2, x).x", -1.0f, 2.0f, 0.0f, 3.0f, "mix(mixed)" );
    plot(canvas, "mix(v1, v2, p).x", -1.0f, 2.0f, 0.0f, 3.0f, "mix(vector)");
    next_row(canvas);

    plot(canvas, "step(1, x)",    0.0f, 2.0f, -0.5f, 1.5f, "step(scalar)");
    plot(canvas, "step(1, p).x",  0.0f, 2.0f, -0.5f, 1.5f, "step(mixed)" );
    plot(canvas, "step(v1, p).x", 0.0f, 2.0f, -0.5f, 1.5f, "step(vector)");
    plot(canvas, "smoothstep(1, 2, x)",     0.5f, 2.5f, -0.5f, 1.5f, "smooth(scalar)");
    plot(canvas, "smoothstep(1, 2, p).x",   0.5f, 2.5f, -0.5f, 1.5f, "smooth(mixed)" );
    plot(canvas, "smoothstep(v1, v2, p).x", 0.5f, 2.5f, -0.5f, 1.5f, "smooth(vector)");
    next_row(canvas);

    plot(canvas, "floor(p).x", -3.0f, 3.0f, -4.0f, 4.0f);
    plot(canvas, "ceil(p).x",  -3.0f, 3.0f, -4.0f, 4.0f);
    plot(canvas, "floor(p).y", -3.0f, 3.0f, -4.0f, 4.0f);
    plot(canvas, "ceil(p).y",  -3.0f, 3.0f, -4.0f, 4.0f);
    next_row(canvas);
}

// The OpenGL ES Shading Language, Version 3.00, Section 8.1
DEF_SIMPLE_GPU_GM_CAN_FAIL(runtime_intrinsics_common_es3,
                           ctx, canvas, errorMsg,
                           columns_to_width(6),
                           rows_to_height(5)) {
    if (ctx->priv().caps()->shaderCaps()->supportedSkSLVerion() < SkSL::Version::k300) {
        *errorMsg = "SkSL 300 is not supported.";
        return skiagm::DrawResult::kSkip;
    }

    canvas->translate(kPadding, kPadding);
    canvas->save();

    plot_es3(canvas, "floatBitsToInt(x)",    -2, 2, -2'000'000'000, 2'000'000'000,
                                             "floatBitsToInt(s)");
    plot_es3(canvas, "floatBitsToInt(p).x",  -2, 2, -2'000'000'000, 2'000'000'000,
                                             "floatBitsToInt(v)");
    plot_es3(canvas, "floatBitsToUint(x)",   -2, 2, 0, 4'000'000'000,
                                             "floatBitsToUint(s)");
    plot_es3(canvas, "floatBitsToUint(p).x", -2, 2, 0, 4'000'000'000,
                                             "floatBitsToUint(v)");
    next_row(canvas);

    plot_es3(canvas, "intBitsToFloat(xi)",           -2'000'000'000, 2'000'000'000, -2, 2,
                                                     "intBitsToFloat(s)");
    plot_es3(canvas, "intBitsToFloat(pi).x",         -2'000'000'000, 2'000'000'000, -2, 2,
                                                     "intBitsToFloat(v)");
    plot_es3(canvas, "uintBitsToFloat(uint(xi))",    0, 4'000'000'000, -2, 2,
                                                     "uintBitsToFloat(s)");
    plot_es3(canvas, "uintBitsToFloat(uint2(pi)).x", 0, 4'000'000'000, -2, 2,
                                                     "uintBitsToFloat(v)");
    next_row(canvas);

    plot_es3(canvas, "trunc(x)",           -2, 2, -3, 3);
    plot_es3(canvas, "trunc(p).x",         -2, 2, -3, 3);
    plot_es3(canvas, "round(x)",           -2, 2, -3, 3);
    plot_es3(canvas, "round(p).x",         -2, 2, -3, 3);
    plot_es3(canvas, "roundEven(x)",       -2, 2, -3, 3);
    plot_es3(canvas, "roundEven(p).x",     -2, 2, -3, 3);
    next_row(canvas);

    plot_es3(canvas, "min(xi, 1)",         -2, 5, -3, 5, "min(int-scalar)");
    plot_es3(canvas, "min(pi, 1).x",       -2, 5, -3, 5, "min(int-mixed)" );
    plot_es3(canvas, "min(pi, int2(1)).x", -2, 5, -3, 5, "min(int-vector)");
    plot_es3(canvas, "max(xi, 1)",         -2, 5, -3, 5, "max(int-scalar)");
    plot_es3(canvas, "max(pi, 1).x",       -2, 5, -3, 5, "max(int-mixed)" );
    plot_es3(canvas, "max(pi, int2(1)).x", -2, 5, -3, 5, "max(int-vector)");
    next_row(canvas);

    plot_es3(canvas, "clamp(xi, 1, 3)",               -1, 5, -1, 5, "clamp(int-scalar)");
    plot_es3(canvas, "clamp(pi, 1, 3).x",             -1, 5, -1, 5, "clamp(int-mixed)" );
    plot_es3(canvas, "clamp(pi, int2(1), int2(3)).x", -1, 5, -1, 5, "clamp(int-vector)");
    plot_es3(canvas, "mix(p.x,  p.y, (x>0)   )",      -1, 2, 0, 3,  "mix(scalar, bool)");
    plot_es3(canvas, "mix(p.yx, p,   (x>0).xx).x",    -1, 2, 0, 3,  "mix(vector, bool)");
    next_row(canvas);

    return skiagm::DrawResult::kOk;
}


// The OpenGL ES Shading Language, Version 1.00, Section 8.4
DEF_SIMPLE_GM(runtime_intrinsics_geometric,
              canvas,
              columns_to_width(4),
              rows_to_height(5)) {
    canvas->translate(kPadding, kPadding);
    canvas->save();

    plot(canvas, "length(x)",       -1.0f, 1.0f, -0.5f, 1.5f);
    plot(canvas, "length(p)",        0.0f, 1.0f,  0.5f, 1.5f);
    plot(canvas, "distance(x, 0)",  -1.0f, 1.0f, -0.5f, 1.5f);
    plot(canvas, "distance(p, v1)",  0.0f, 1.0f,  0.5f, 1.5f);
    next_row(canvas);

    plot(canvas, "dot(x, 2)",    -1.0f, 1.0f, -2.5f, 2.5f);
    plot(canvas, "dot(p, p.y1)", -1.0f, 1.0f, -2.5f, 0.5f);
    next_row(canvas);

    plot(canvas, "cross(p.xy1, p.y1x).x", 0.0f, 1.0f, -1.0f, 1.0f);
    plot(canvas, "cross(p.xy1, p.y1x).y", 0.0f, 1.0f, -1.0f, 1.0f);
    plot(canvas, "cross(p.xy1, p.y1x).z", 0.0f, 1.0f, -1.0f, 1.0f);
    next_row(canvas);

    plot(canvas, "normalize(x)",   -2.0f, 2.0f, -1.5f, 1.5f);
    plot(canvas, "normalize(p).x",  0.0f, 2.0f,  0.0f, 1.0f);
    plot(canvas, "normalize(p).y",  0.0f, 2.0f,  0.0f, 1.0f);
    plot(canvas, "faceforward(v1, p.x0, v1.x0).x", -1.0f, 1.0f, -1.5f, 1.5f, "faceforward");
    next_row(canvas);

    plot(canvas, "reflect(p.x1, v1.0x).x",         -1.0f, 1.0f, -1.0f, 1.0f, "reflect(horiz)");
    plot(canvas, "reflect(p.x1, normalize(v1)).y", -1.0f, 1.0f, -1.0f, 1.0f, "reflect(diag)" );
    plot(canvas, "refract(v1.x0, v1.0x, x).x",      0.0f, 1.0f, -1.0f, 1.0f, "refract().x");
    plot(canvas, "refract(v1.x0, v1.0x, x).y",      0.0f, 1.0f, -1.0f, 1.0f, "refract().y");
    next_row(canvas);
}

#define SKSL_MATRIX_SELECTORS               \
    "inline float2 sel2(float x) {"         \
    "    return float2("                    \
    "      x <  0.5 ? 1 : 0,"               \
    "      x >= 0.5 ? 1 : 0);"              \
    "}"                                     \
    "inline float3 sel3(float x) {"         \
    "    return float3("                    \
    "      x <  0.33             ? 1 : 0,"  \
    "      x >= 0.33 && x < 0.66 ? 1 : 0,"  \
    "      x >= 0.66             ? 1 : 0);" \
    "}"                                     \
    "inline float4 sel4(float x) {"         \
    "    return float4("                    \
    "      x <  0.25             ? 1 : 0,"  \
    "      x >= 0.25 && x < 0.5  ? 1 : 0,"  \
    "      x >= 0.5  && x < 0.75 ? 1 : 0,"  \
    "      x >= 0.75             ? 1 : 0);" \
    "}"

// Shader for testing matrixCompMult intrinsic
static SkString make_matrix_comp_mult_sksl(int dim) {
    return SkStringPrintf(
            "uniform float%dx%d m1;"                              // dim, dim
            "uniform float%dx%d m2;"                              // dim, dim
            SKSL_MATRIX_SELECTORS
            "half4 main(float2 p) {"
            "    float%d colSel = sel%d(p.x);"                    // dim, dim
            "    float%d rowSel = sel%d(p.y);"                    // dim, dim
            "    float%d col = matrixCompMult(m1, m2) * colSel;"  // dim
            "    float  v = dot(col, rowSel);"
            "    return v.xxx1;"
            "}", dim, dim, dim, dim, dim, dim, dim, dim, dim);
}

template <int N>
static void plot_matrix_comp_mult(SkCanvas* canvas,
                                  std::array<float, N*N> mtx1,
                                  std::array<float, N*N> mtx2,
                                  const char* label) {
    canvas->save();

    draw_label(canvas, label);

    auto [effect, error] = SkRuntimeEffect::MakeForShader(make_matrix_comp_mult_sksl(N));
    if (!effect) {
        SkDebugf("Error: %s\n", error.c_str());
        return;
    }

    SkRuntimeShaderBuilder builder(effect);
    builder.uniform("m1") = mtx1;
    builder.uniform("m2") = mtx2;

    draw_shader(canvas, builder.makeShader());

    canvas->restore();
    next_column(canvas);
}

// Shader for testing inverse() intrinsic
static SkString make_matrix_inverse_sksl(int dim) {
    return SkStringPrintf(
            "uniform float scale; uniform float bias;"
            "uniform float%dx%d m;"                    // dim, dim
            SKSL_MATRIX_SELECTORS
            "half4 main(float2 p) {"
            "    float%d colSel = sel%d(p.x);"         // dim, dim
            "    float%d rowSel = sel%d(p.y);"         // dim, dim
            "    float%d col = inverse(m) * colSel;"   // dim
            "    float  v = dot(col, rowSel) * scale + bias;"
            "    return v.xxx1;"
            "}", dim, dim, dim, dim, dim, dim, dim);
}

template <int N>
static void plot_matrix_inverse(SkCanvas* canvas, std::array<float, N*N> mtx, const char* label) {
    canvas->save();

    draw_label(canvas, label);

    auto [effect, error] = SkRuntimeEffect::MakeForShader(make_matrix_inverse_sksl(N));
    if (!effect) {
        SkDebugf("Error: %s\n", error.c_str());
        return;
    }

    SkRuntimeShaderBuilder builder(effect);
    builder.uniform("scale") = 0.5f;
    builder.uniform("bias")  = 0.5f;
    builder.uniform("m")     = mtx;

    draw_shader(canvas, builder.makeShader());

    canvas->restore();
    next_column(canvas);
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.5
DEF_SIMPLE_GM(runtime_intrinsics_matrix,
              canvas,
              columns_to_width(3),
              rows_to_height(2)) {
    canvas->translate(kPadding, kPadding);
    canvas->save();

    // Random pairs of matrices where the elements of matrixCompMult(m1, m2) lie in [0, 1]
    plot_matrix_comp_mult<2>(canvas,
                             {1.00f, 0.0f, 2.0f, 0.5f},
                             {0.75f, 2.0f, 0.2f, 1.2f},
                             "compMult(2x2)");

    plot_matrix_comp_mult<3>(canvas,
                             {1.00f, 0.0f, 2.0f, 0.5f, -1.0f, -2.0f, -0.5f, 4.00f, 0.25f},
                             {0.75f, 2.0f, 0.2f, 1.2f, -0.8f, -0.1f, -1.8f, 0.25f, 2.00f},
                             "compMult(3x3)");

    plot_matrix_comp_mult<4>(canvas,
                             {1.00f, 0.0f, 2.0f, 0.5f, -1.0f, -2.0f, -0.5f, 4.00f, 0.25f, 0.05f,
                              10.00f, -0.66f, -1.0f, -0.5f, 0.5f, 0.66f},
                             {0.75f, 2.0f, 0.2f, 1.2f, -0.8f, -0.1f, -1.8f, 0.25f, 2.00f, 2.00f,
                              0.03f, -1.00f, -1.0f, -0.5f, 1.7f, 0.66f},
                             "compMult(4x4)");
    next_row(canvas);

    // Random, invertible matrices where the elements of inverse(m) lie in [-1, 1]
    plot_matrix_inverse<2>(canvas,
                           { 1.20f,  0.68f,
                            -0.27f, -1.55f},
                           "inverse(2x2)");

    plot_matrix_inverse<3>(canvas,
                           {-1.13f, -2.96f, -0.14f,
                             1.45f, -1.88f, -1.02f,
                            -2.54f, -2.58f, -1.17f},
                           "inverse(3x3)");

    plot_matrix_inverse<4>(canvas,
                           {-1.51f, -3.95f, -0.19f,  1.93f,
                            -2.51f, -1.35f, -3.39f, -3.45f,
                            -1.56f,  1.61f, -0.22f, -1.08f,
                            -2.81f, -2.14f, -0.09f,  3.00f},
                           "inverse(4x4)");
    next_row(canvas);
}

/*
  Specialized shader for testing relational operators.
*/
static SkString make_bvec_sksl(const char* type, const char* fn) {
    // We use negative floats, to ensure that the integer variants are working with the correct
    // interpretation of the data.
    return SkStringPrintf(
            "uniform %s2 v1;"
            "half4 main(float2 p) {"
            "    p.x = p.x < 0.33 ? -3.0 : (p.x < 0.66 ? -2.0 : -1.0);"
            "    p.y = p.y < 0.33 ? -3.0 : (p.y < 0.66 ? -2.0 : -1.0);"
            "    bool2 cmp = %s;"
            "    return half4(cmp.x ? 1.0 : 0.0, cmp.y ? 1.0 : 0.0, 0, 1);"
            "}",
            type, fn);
}

template <typename T = float>
static void plot_bvec(SkCanvas* canvas, const char* fn, const char* label) {
    canvas->save();

    draw_label(canvas, label);

    const char* type = std::is_integral<T>::value ? "int" : "float";
    auto [effect, error] = SkRuntimeEffect::MakeForShader(make_bvec_sksl(type, fn));
    if (!effect) {
        SkDebugf("Error: %s\n", error.c_str());
        return;
    }

    T uniformData[2] = { -2, -2 };
    sk_sp<SkData> uniforms = SkData::MakeWithCopy(uniformData, sizeof(uniformData));

    draw_shader(canvas, effect->makeShader(uniforms, /*children=*/{}));

    canvas->restore();
    next_column(canvas);
}

// The OpenGL ES Shading Language, Version 1.00, Section 8.6
DEF_SIMPLE_GM(runtime_intrinsics_relational,
              canvas,
              columns_to_width(4),
              rows_to_height(6)) {
    canvas->translate(kPadding, kPadding);
    canvas->save();

    plot_bvec<float>(canvas, "lessThan(p, v1)",            "lessThan");
    plot_bvec<int>  (canvas, "lessThan(int2(p), v1)",      "lessThan(int)");
    plot_bvec<float>(canvas, "lessThanEqual(p, v1)",       "lessThanEqual");
    plot_bvec<int>  (canvas, "lessThanEqual(int2(p), v1)", "lessThanEqual(int)");
    next_row(canvas);

    plot_bvec<float>(canvas, "greaterThan(p, v1)",            "greaterThan");
    plot_bvec<int>  (canvas, "greaterThan(int2(p), v1)",      "greaterThan(int)");
    plot_bvec<float>(canvas, "greaterThanEqual(p, v1)",       "greaterThanEqual");
    plot_bvec<int>  (canvas, "greaterThanEqual(int2(p), v1)", "greaterThanEqual(int)");
    next_row(canvas);

    plot_bvec<float>(canvas, "equal(p, v1)",          "equal");
    plot_bvec<int>  (canvas, "equal(int2(p), v1)",    "equal(int)");
    plot_bvec<float>(canvas, "notEqual(p, v1)",       "notEqual");
    plot_bvec<int>  (canvas, "notEqual(int2(p), v1)", "notEqual(int)");
    next_row(canvas);

    plot_bvec(canvas, "equal(   lessThanEqual(p, v1), greaterThanEqual(p, v1))", "equal(bvec)");
    plot_bvec(canvas, "notEqual(lessThanEqual(p, v1), greaterThanEqual(p, v1))", "notequal(bvec)");
    next_row(canvas);

    plot_bvec(canvas, "not(notEqual(p, v1))", "not(notEqual)");
    plot_bvec(canvas, "not(equal(p, v1))",    "not(equal)");
    next_row(canvas);

    plot_bvec(canvas, "bool2(any(equal(p, v1)))", "any(equal)");
    plot_bvec(canvas, "bool2(all(equal(p, v1)))", "all(equal)");
    next_row(canvas);
}
