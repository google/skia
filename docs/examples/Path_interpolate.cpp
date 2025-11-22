// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_interpolate, 256, 60, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPath path, path2;
    path = SkPathBuilder()
           .moveTo(20, 20)
           .lineTo(40, 40)
           .lineTo(20, 40)
           .lineTo(40, 20)
           .close()
           .detach();
    path2 = SkPath::Rect({20, 20, 40, 40});
    for (SkScalar i = 0; i <= 1; i += 1.f / 6) {
      SkPath interp;
      path.interpolate(path2, i, &interp);
      canvas->drawPath(interp, paint);
      canvas->translate(30, 0);
    }
}
}  // END FIDDLE
