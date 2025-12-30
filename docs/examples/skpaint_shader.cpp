// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_shader, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPoint points[2] = {SkPoint::Make(0.0f, 0.0f), SkPoint::Make(256.0f, 256.0f)};
    SkColor4f colors[2] = {SkColors::kBlue, SkColors::kYellow};
    SkPaint paint;
    paint.setShader(SkShaders::LinearGradient(points, {{colors, {}, SkTileMode::kClamp}, {}}));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
