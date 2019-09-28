// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3a90a91030f7289d5df0671d342dbbad
REG_FIDDLE(Path_addPath_3, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    SkPath dest, path;
    path.addOval({20, 20, 200, 120}, SkPath::kCW_Direction, 1);
    for (int i = 0; i < 6; i++) {
        SkMatrix matrix;
        matrix.reset();
        matrix.setPerspX(i / 400.f);
        dest.addPath(path, matrix);
    }
    canvas->drawPath(dest, paint);
}
}  // END FIDDLE
