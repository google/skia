// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=5754501a00a1323e76353fb53153e939
REG_FIDDLE(Matrix_mapVectors_2, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkMatrix matrix;
    matrix.setScale(2, 3);
    SkVector radii[] = {{7, 7}, {3, 3}, {2, 2}, {4, 0}};
    for (int i = 0; i < 4; ++i) {
        SkRRect rrect;
        rrect.setRectRadii({20, 20, 180, 70}, radii);
        canvas->drawRRect(rrect, paint);
        canvas->translate(0, 60);
        matrix.mapVectors(radii, SK_ARRAY_COUNT(radii));
    }
}
}  // END FIDDLE
