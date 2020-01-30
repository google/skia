// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_shader, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPoint points[2] = {SkPoint::Make(0.0f, 0.0f), SkPoint::Make(256.0f, 256.0f)};
    SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(points, colors, nullptr, 2, SkTileMode::kClamp,
                                                 0, nullptr));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
