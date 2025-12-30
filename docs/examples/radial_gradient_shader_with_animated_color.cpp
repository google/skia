// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(radial_gradient_shader_with_animated_color, 256, 256, false, 0, 2) {
void draw(SkCanvas* canvas) {
    float p = 0.5 * (1 - cos(6.28318548f * frame));
    SkColor4f blue = {0, 0, p, 1};
    SkColor4f colors[2] = {blue, SkColors::kYellow};
    SkPaint paint;
    paint.setShader(SkShaders::RadialGradient(
                {128.0f, 128.0f}, 180.0f, {{colors, {}, SkTileMode::kClamp}, {}}));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
