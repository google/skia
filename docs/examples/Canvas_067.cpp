// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1cd076b9b1a7c976cdca72b93c4f42dd
REG_FIDDLE(Canvas_drawPaint, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkColor     colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE };
    SkScalar    pos[] = { 0, SK_Scalar1/2, SK_Scalar1 };
    SkPaint     paint;
    paint.setShader(SkGradientShader::MakeSweep(256, 256, colors, pos, SK_ARRAY_COUNT(colors)));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
