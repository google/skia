// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
namespace {
REG_FIDDLE(BlendMode_016, 256, 256, false, 3);
// HASH=23c974d2759f523ca2f4a78ae86855c3
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    SkColor colors[] = { SK_ColorWHITE, SK_ColorBLACK };
    SkPoint horz[] = { { 0, 0 }, { 256, 0 } };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(horz, colors, nullptr, SK_ARRAY_COUNT(colors),
            SkShader::kClamp_TileMode));
    paint.setBlendMode(SkBlendMode::kDarken);
    canvas->drawPaint(paint);
}
}
