// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f2260f2a170a54aef5bafe5b91c121b3
REG_FIDDLE(Path_044, 256, 192, false, 0) {
void draw(SkCanvas* canvas) {
    auto addPoly = [](int sides, SkScalar size, SkPath* path) -> void {
        path->moveTo(size, 0);
        for (int i = 1; i < sides; i++) {
            SkScalar c, s = SkScalarSinCos(SK_ScalarPI * 2 * i / sides, &c);
            path->lineTo(c * size, s * size);
        }
        path->close();
    };
    SkPath path;
    path.incReserve(3 + 4 + 5 + 6 + 7 + 8 + 9);
    for (int sides = 3; sides < 10; ++sides) {
       addPoly(sides, sides, &path);
    }
    SkMatrix matrix;
    matrix.setScale(10, 10, -10, -10);
    path.transform(matrix);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path, paint);
}
}  // END FIDDLE
