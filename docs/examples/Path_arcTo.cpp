// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5f02890edaa10cb5e1a4243a82b6a382
REG_FIDDLE(Path_arcTo, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    SkPath path;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(4);
    path.moveTo(0, 0);
    path.arcTo({20, 20, 120, 120}, -90, 90, false);
    canvas->drawPath(path, paint);
    path.rewind();
    path.arcTo({120, 20, 220, 120}, -90, 90, false);
    canvas->drawPath(path, paint);
    path.rewind();
    path.moveTo(0, 0);
    path.arcTo({20, 120, 120, 220}, -90, 90, true);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
