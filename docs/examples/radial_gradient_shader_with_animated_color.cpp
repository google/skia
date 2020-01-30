// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(radial_gradient_shader_with_animated_color, 256, 256, false, 0, 2) {
void draw(SkCanvas* canvas) {
    float p = 0.5 * (1 - cos(6.28318548f * frame));
    SkColor blue = SkColor(0xff * p) | 0xFF000000;
    SkColor colors[2] = {blue, SK_ColorYELLOW};
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeRadial(
                SkPoint::Make(128.0f, 128.0f), 180.0f,
                colors, nullptr, 2, SkTileMode::kClamp, 0, nullptr));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
