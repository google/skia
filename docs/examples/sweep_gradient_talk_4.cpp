// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_SRGB(sweep_gradient_talk_4, 256, 256, false, 0, 0, false) {
void draw(SkCanvas* canvas) {
    SkColor colors[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFF0000FF };
    float cx = 128, cy = 128;
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeSweep(cx, cy, colors, nullptr, 4));
    paint.setAntiAlias(true);

    canvas->drawPaint(paint);
}
}  // END FIDDLE
