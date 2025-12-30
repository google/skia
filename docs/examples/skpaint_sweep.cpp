// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_sweep, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor4f colors[4] = {SkColors::kCyan, SkColors::kMagenta, SkColors::kYellow, SkColors::kCyan};
    SkPaint paint;
    paint.setShader(
            SkShaders::SweepGradient({128.0f, 128.0f}, {{colors, {}, SkTileMode::kClamp}, {}}));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
