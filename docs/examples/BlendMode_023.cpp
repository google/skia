// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=a544ee1c67c7c557a9e54d5e99f94bb6
REG_FIDDLE(BlendMode_023, 256, 256, false, 5) {
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
            SkShader::kClamp_TileMode));
    canvas->drawPaint(paint);
    canvas->clipRect( { 30, 30, 226, 226 } );
    canvas->drawColor(0x80bb9977, SkBlendMode::kExclusion);
}
}  // END FIDDLE
