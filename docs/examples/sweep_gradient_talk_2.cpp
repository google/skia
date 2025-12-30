// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(sweep_gradient_talk_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const float s = 1.0f / 6;
    SkColor4f colors[] = {
        {0,0,1,1}, {0,0,1,1}, {0,1,0,1}, {0,1,0,1},
        {1,0,0,1}, {1,0,0,1}, {0,0,1,1}, {0,0,1,1},
    };
    float pos[] = {0, s, s, 3 * s, 3 * s, 5 * s, 5 * s, 1};
    float cx = 128, cy = 128;
    SkPaint paint;
    paint.setShader(SkShaders::SweepGradient({cx, cy}, {{colors, pos, SkTileMode::kClamp}, {}}));
    paint.setAntiAlias(true);

    canvas->drawPaint(paint);
}
}  // END FIDDLE
