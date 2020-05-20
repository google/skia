// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(checker_board, 256, 256, false, 0) {
void checkerboard(SkCanvas* canvas) {
    SkColor color1 = SK_ColorLTGRAY;
    SkColor color2 = SK_ColorCYAN;
    SkScalar scale = 10.0f;
    SkPath path;
    path.addRect(0, 0, scale, scale);
    SkMatrix matrix = SkMatrix::Scale(2 * scale, scale);
    matrix.preSkew(0.5f, 0);
    SkPaint paint;
    paint.setPathEffect(SkPath2DPathEffect::Make(matrix, path));
    paint.setAntiAlias(true);
    paint.setColor(color2);
    canvas->clear(color1);
    SkRect bounds = SkRect::MakeWH(256, 256);
    bounds.outset(scale, scale);
    canvas->drawRect(bounds, paint);
}

void draw(SkCanvas* canvas) {
    canvas->drawColor(SK_ColorWHITE);
    checkerboard(canvas);
}
}  // END FIDDLE
