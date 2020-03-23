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

    for (int i = 0; i < 4; i++) {
        SkScalar blockX = (i % 2) * 100;
        SkScalar blockY = (i / 2) * 100;
        SkPoint pts[] = { {blockX, blockY}, {blockX + 50, blockY + 100} };

        int flags = 0; // interpolate colors in unpremul
        if (i % 2 == 1) {
            // right column will have premul
            flags = SkGradientShader::Flags::kInterpolateColorsInPremul_Flag;
        }

        SkMatrix matr = SkMatrix::I();
        if (i / 2 == 1) {
            // bottom row will be rotated 45 degrees.
            matr.setRotate(45, blockX, blockY);
        }

        auto lgs = SkGradientShader::MakeLinear(
            pts, colors, positions, 3, SkTileMode::kMirror,
            flags, &matr);

        p.setShader(lgs);
        auto r = SkRect::MakeLTRB(blockX, blockY, blockX + 100, blockY + 100);
        canvas->drawRect(r, p);
        canvas->drawRect(r, strokePaint);
    }
}
}  // END FIDDLE
