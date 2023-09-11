// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_turb, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setShader(SkShaders::MakeTurbulence(0.05f, 0.05f, 4, 0.0f, nullptr));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
