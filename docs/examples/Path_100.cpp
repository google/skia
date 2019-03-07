// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=1d1892196ba5bda257df4f3351abd084
REG_FIDDLE(Path_100, 256, 60, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath pattern;
    pattern.moveTo(20, 20);
    pattern.lineTo(20, 40);
    pattern.lineTo(40, 20);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 10; i++) {
        SkPath path;
        pattern.offset(20 * i, 0, &path);
        canvas->drawPath(path, paint);
    }
}
}  // END FIDDLE
