// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Path_transform, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath pattern = SkPathBuilder()
                     .moveTo(100, 100)
                     .lineTo(100, 20)
                     .lineTo(20, 100)
                     .detach();
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 10; i++) {
        SkMatrix matrix;
        matrix.setRotate(36 * i, 100, 100);
        SkPath path = pattern.makeTransform(matrix);
        canvas->drawPath(path, paint);
    }
}
}  // END FIDDLE
