// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_sweep, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor colors[4] = {SK_ColorCYAN, SK_ColorMAGENTA, SK_ColorYELLOW, SK_ColorCYAN};
    SkPaint paint;
    paint.setShader(
            SkGradientShader::MakeSweep(128.0f, 128.0f, colors, nullptr, 4, 0, nullptr));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
