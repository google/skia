// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=95cb08b8c8db3af3b2c9ad56ae7d6bc1
REG_FIDDLE(BlendMode_017, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    SkColor colors[] = { SK_ColorBLACK, SK_ColorWHITE };
    SkPoint horz[] = { { 0, 0 }, { 256, 0 } };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(horz, colors, nullptr, SK_ARRAY_COUNT(colors),
            SkShader::kClamp_TileMode));
    paint.setBlendMode(SkBlendMode::kLighten);
    canvas->drawPaint(paint);
}
}  // END FIDDLE
