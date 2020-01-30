// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_compose_shader, 256, 256, false, 0) {
// skpaint_compose_shader
void draw(SkCanvas* canvas) {
    SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
    SkPaint paint;
    paint.setShader(SkShaders::Blend(
            SkBlendMode::kDifference,
            SkGradientShader::MakeRadial(SkPoint::Make(128.0f, 128.0f), 180.0f, colors, nullptr,
                                         2, SkTileMode::kClamp, 0, nullptr),
            SkPerlinNoiseShader::MakeTurbulence(0.025f, 0.025f, 2, 0.0f, nullptr)));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
