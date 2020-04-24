// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=c40979a3b92a30cfb7bae36abcc1d805
REG_FIDDLE(Path_transform_2, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(100, 100);
    path.quadTo(100, 20, 20, 100);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 10; i++) {
        SkMatrix matrix;
        matrix.setRotate(36, 100, 100);
        path.transform(matrix);
        canvas->drawPath(path, paint);
    }
}
}  // END FIDDLE
