// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(skew_x_animated, 256, 256, false, 6, 6) {
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkMatrix matrix;
    matrix.setAll(2, 5 * cos(6.28318548 * frame), 0,
                  0, 2, 128,
                  0, 0, 1);
    SkPaint paint;
    paint.setShader(
        image->makeShader(
            SkTileMode::kRepeat,
            SkTileMode::kRepeat,
            SkSamplingOptions(),
            matrix));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
