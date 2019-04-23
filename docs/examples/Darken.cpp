// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=23c974d2759f523ca2f4a78ae86855c3
REG_FIDDLE(Darken, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    canvas->drawImage(image, 0, 0);
    SkColor colors[] = { SK_ColorWHITE, SK_ColorBLACK };
    SkPoint horz[] = { { 0, 0 }, { 256, 0 } };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(horz, colors, nullptr, SK_ARRAY_COUNT(colors),
            SkTileMode::kClamp));
    paint.setBlendMode(SkBlendMode::kDarken);
    canvas->drawPaint(paint);
}
}  // END FIDDLE
