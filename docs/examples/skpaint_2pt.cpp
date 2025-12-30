// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_2pt, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor4f colors[2] = {SkColors::kBlue, SkColors::kYellow};
    SkPaint paint;
    paint.setShader(SkShaders::TwoPointConicalGradient(
            SkPoint::Make(128.0f, 128.0f), 128.0f, SkPoint::Make(128.0f, 16.0f), 16.0f,
            {{colors, {}, SkTileMode::kClamp}, {}}));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
