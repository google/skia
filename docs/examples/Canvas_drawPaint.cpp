// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Canvas_drawPaint, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor4f   colors[] = { SkColors::kRed, SkColors::kGreen, SkColors::kBlue };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;
    paint.setShader(SkShaders::SweepGradient({256, 256}, {{colors, pos, SkTileMode::kClamp}, {}}));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
