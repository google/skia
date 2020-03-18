// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(GradientShader_MakeLinear, 201, 201, false, 0) {
void draw(SkCanvas* canvas) {
    // This fiddle draws 4 instances of a LinearGradient, demonstrating
    // how the local matrix affects the gradient as well as the flag
    // which controls unpremul vs premul color interpolation.

    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setColor(SK_ColorBLACK);

    SkPaint p;
    p.setStyle(SkPaint::kFill_Style);

    SkColor transparentGreen = SkColorSetARGB(0, 0, 255, 255);
    SkColor colors[] = { transparentGreen, SK_ColorBLUE, SK_ColorRED };
    SkScalar positions[] = { 0.0, 0.65, 1.0 };

    SkPoint pts[] = { {0, 0}, {50, 100} };
    auto lgs = SkGradientShader::MakeLinear(
        pts,
        colors, positions, 3, // num colors
        SkTileMode::kMirror,
        0, // flags - interpolate colors in unpremul
        nullptr);

    p.setShader(lgs);
    auto r = SkRect::MakeLTRB(0, 0, 100, 100);
    canvas->drawRect(r, p);
    canvas->drawRect(r, strokePaint);

    pts[0] = {100, 0};
    pts[1] = {150, 100};
    auto lgsPremul = SkGradientShader::MakeLinear(
        pts,
        colors, positions, 3, // num colors
        SkTileMode::kMirror,
        SkGradientShader::Flags::kInterpolateColorsInPremul_Flag,
        nullptr);

    p.setShader(lgsPremul);
    r = SkRect::MakeLTRB(100, 0, 200, 100);
    canvas->drawRect(r, p);
    canvas->drawRect(r, strokePaint);

    pts[0] = {0, 100};
    pts[1] = {50, 200};
    SkMatrix m;
    m.setRotate(45, 0, 100);
    auto lgs45 = SkGradientShader::MakeLinear(
        pts,
        colors, positions, 3, // num colors
        SkTileMode::kMirror,
        0, // flags - interpolate colors in unpremul
        &m);

    p.setShader(lgs45);
    r = SkRect::MakeLTRB(0, 100, 100, 200);
    canvas->drawRect(r, p);
    canvas->drawRect(r, strokePaint);

    pts[0] = {100, 100};
    pts[1] = {150, 200};
    m.setRotate(45, 100, 100);
    auto lgs45AndPremul = SkGradientShader::MakeLinear(
        pts,
        colors, positions, 3, // num colors
        SkTileMode::kMirror,
        SkGradientShader::Flags::kInterpolateColorsInPremul_Flag,
        &m);

    p.setShader(lgs45AndPremul);
    r = SkRect::MakeLTRB(100, 100, 200, 200);
    canvas->drawRect(r, p);
    canvas->drawRect(r, strokePaint);
}
}  // END FIDDLE
