// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=918a9778c3d7d5cb306692784399f6dc
REG_FIDDLE(Matrix_mapVectors, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    SkMatrix matrix;
    matrix.reset();
    const SkVector radii[] = {{8, 4}, {9, 1}, {6, 2}, {7, 3}};
    for (int i = 0; i < 4; ++i) {
        SkVector rScaled[4];
        matrix.preScale(1.5f, 2.f);
        matrix.mapVectors(rScaled, radii, SK_ARRAY_COUNT(radii));
        SkRRect rrect;
        rrect.setRectRadii({20, 20, 180, 70}, rScaled);
        canvas->drawRRect(rrect, paint);
        canvas->translate(0, 60);
    }
}
}  // END FIDDLE
