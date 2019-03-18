// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=404f11c5c9c9ca8a64822d484552a473
REG_FIDDLE(Path_interpolate, 256, 60, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPath path, path2;
    path.moveTo(20, 20);
    path.lineTo(40, 40);
    path.lineTo(20, 40);
    path.lineTo(40, 20);
    path.close();
    path2.addRect({20, 20, 40, 40});
    for (SkScalar i = 0; i <= 1; i += 1.f / 6) {
      SkPath interp;
      path.interpolate(path2, i, &interp);
      canvas->drawPath(interp, paint);
      canvas->translate(30, 0);
    }
}
}  // END FIDDLE
