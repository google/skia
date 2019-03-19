// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=19f0cfc7eeba8937fe19446ec0b5f932
REG_FIDDLE(Path_rCubicTo, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPath path;
    path.moveTo(24, 108);
    for (int i = 0; i < 16; i++) {
       SkScalar sx, sy;
       sx = SkScalarSinCos(i * SK_ScalarPI / 8, &sy);
       path.rCubicTo(40 * sx, 4 * sy, 4 * sx, 40 * sy, 40 * sx, 40 * sy);
    }
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
