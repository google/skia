// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_2pt, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor colors[2] = {SK_ColorBLUE, SK_ColorYELLOW};
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeTwoPointConical(
            SkPoint::Make(128.0f, 128.0f), 128.0f, SkPoint::Make(128.0f, 16.0f), 16.0f, colors,
            nullptr, 2, SkTileMode::kClamp, 0, nullptr));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
