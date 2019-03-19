// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=99761add116ce3b0730557224c1b0105
REG_FIDDLE(Path_transform, 256, 200, false, 0) {
void draw(SkCanvas* canvas) {
    SkPath pattern;
    pattern.moveTo(100, 100);
    pattern.lineTo(100, 20);
    pattern.lineTo(20, 100);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 10; i++) {
        SkPath path;
        SkMatrix matrix;
        matrix.setRotate(36 * i, 100, 100);
        pattern.transform(matrix, &path);
        canvas->drawPath(path, paint);
    }
}
}  // END FIDDLE
