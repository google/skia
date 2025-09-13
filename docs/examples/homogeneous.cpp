// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(homogeneous, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkPoint3 src[] = {{3, 3, 1}, {8, 2, 2}, {5, 0, 4}, {0, 1, 3},
                      {3, 7, 1}, {8, 6, 2}, {5, 4, 4}, {0, 5, 3}};
    int lines[] = {0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};
    auto debugster = [=](SkPoint3 src[]) -> void {
        for (size_t i = 0; i < std::size(lines); i += 2) {
            const SkPoint3& s = src[lines[i]];
            const SkPoint3& e = src[lines[i + 1]];
            SkPaint paint;
            paint.setARGB(77, 23, 99, 154);
            canvas->drawLine(s.fX / s.fZ, s.fY / s.fZ, e.fX / e.fZ, e.fY / e.fZ, paint);
        }
    };
    canvas->save();
    canvas->translate(5, 5);
    canvas->scale(15, 15);
    debugster(src);
    canvas->restore();
    canvas->translate(128, 128);
    SkMatrix matrix;
    matrix.setAll(15, 0, 0, 0, 15, 0, -0.08, 0.04, 1);
    matrix.mapHomogeneousPoints(src, src);
    debugster(src);
}
}  // END FIDDLE
