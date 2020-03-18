// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(GradientShader_MakeSweep, 201, 201, false, 0) {
void draw(SkCanvas* canvas) {
    // This fiddle draws 4 instances of a SweepGradient, demonstrating
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
        SkScalar cx = blockX + 50;
        SkScalar cy = blockY + 50;

        int flags = 0; // interpolate colors in unpremul
        if (i % 2 == 1) {
            // right column will have premul
            flags = SkGradientShader::Flags::kInterpolateColorsInPremul_Flag;
        }

        SkMatrix* matr = nullptr;
        if (i / 2 == 1) {
            // bottom row will be translated.
            SkMatrix m;
            m.setTranslate(20, 40);
            matr = &m;
        }

        auto cgs = SkGradientShader::MakeSweep(
            cx, cy, colors, positions, 3, SkTileMode::kClamp,
            90, 350, // start and stop angles
            flags, matr);

        p.setShader(cgs);
        auto r = SkRect::MakeLTRB(blockX, blockY, blockX + 100, blockY + 100);
        canvas->drawRect(r, p);
        canvas->drawRect(r, strokePaint);
    }
}
}  // END FIDDLE
