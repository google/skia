// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(sweep_gradient_talk_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const float s = 1.0f / 6;
    SkColor colors[] = {0xFF0000FF, 0xFF0000FF, 0xFF00FF00, 0xFF00FF00,
                        0xFFFF0000, 0xFFFF0000, 0xFF0000FF, 0xFF0000FF};
    float pos[] = {0, s, s, 3 * s, 3 * s, 5 * s, 5 * s, 1};
    float cx = 128, cy = 128;
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeSweep(cx, cy, colors, pos, 8));
    paint.setAntiAlias(true);

    canvas->drawPaint(paint);
}
}  // END FIDDLE
