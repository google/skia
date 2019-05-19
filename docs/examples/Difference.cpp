// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=52d2c8d1b9b428de4477b4caa1543a3d
REG_FIDDLE(Difference, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    canvas->drawImage(image, 128, 0);
    canvas->drawImage(image, 0, 128);
    canvas->drawImage(image, 128, 128);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kDstATop);
    SkColor alphas[] = { SK_ColorBLACK, SK_ColorTRANSPARENT };
    SkPoint vert[] = { { 0, 0 }, { 0, 256 } };
    paint.setShader(SkGradientShader::MakeLinear(vert, alphas, nullptr, SK_ARRAY_COUNT(alphas),
            SkTileMode::kClamp));
    canvas->drawPaint(paint);
    canvas->clipRect( { 30, 30, 226, 226 } );
    canvas->drawColor(0x80bb9977, SkBlendMode::kDifference);
}
}  // END FIDDLE
