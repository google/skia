// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_arcTo, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(4);
    SkPath path = SkPathBuilder()
                  .moveTo(0, 0)
                  .arcTo({20, 20, 120, 120}, -90, 90, false)
                  .detach();
    canvas->drawPath(path, paint);
    path = SkPathBuilder()
           .arcTo({120, 20, 220, 120}, -90, 90, false)
           .detach();
    canvas->drawPath(path, paint);
    path = SkPathBuilder()
           .moveTo(0, 0)
           .arcTo({20, 120, 120, 220}, -90, 90, true)
           .detach();
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
