// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=84b2d1c0fc29f1b35e855b6fc6672f9e
REG_FIDDLE(Path_addPath_2, 256, 80, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    SkPath dest, path;
    path.addOval({-80, 20, 0, 60}, SkPath::kCW_Direction, 1);
    for (int i = 0; i < 2; i++) {
        dest.addPath(path, SkPath::kExtend_AddPathMode);
        dest.offset(100, 0);
    }
    canvas->drawPath(dest, paint);
}
}  // END FIDDLE
