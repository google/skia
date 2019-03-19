// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6e0be0766b8ca320da51640326e608b3
REG_FIDDLE(Path_rLineTo, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkPath path;
    path.moveTo(10, 98);
    SkScalar x = 0, y = 0;
    for (int i = 10; i < 100; i += 5) {
        x += i * ((i & 2) - 1);
        y += i * (((i + 1) & 2) - 1);
        path.rLineTo(x, y);
    }
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
