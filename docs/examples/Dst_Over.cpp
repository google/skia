// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=10dbb4d97902956ef5f5f8562f65119e
REG_FIDDLE(Dst_Over, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor colors[] = { SK_ColorRED, SK_ColorBLUE };
    SkPoint horz[] = { { 0, 0 }, { 256, 0 } };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(horz, colors, nullptr, SK_ARRAY_COUNT(colors),
            SkTileMode::kClamp));
    canvas->drawPaint(paint);
    paint.setBlendMode(SkBlendMode::kDstIn);
    SkColor alphas[] = { SK_ColorBLACK, SK_ColorTRANSPARENT };
    SkPoint vert[] = { { 0, 0 }, { 0, 256 } };
    paint.setShader(SkGradientShader::MakeLinear(vert, alphas, nullptr, SK_ARRAY_COUNT(alphas),
            SkTileMode::kClamp));
    canvas->drawPaint(paint);
    canvas->clipRect( { 30, 30, 226, 226 } );
    canvas->drawColor(SkColorSetA(SK_ColorGREEN, 128), SkBlendMode::kDstOver);
}
}  // END FIDDLE
