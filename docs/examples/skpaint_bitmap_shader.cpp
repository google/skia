// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_bitmap_shader, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkMatrix matrix;
    matrix.setScale(0.75f, 0.75f);
    matrix.preRotate(30.0f);
    SkPaint paint;
    paint.setShader(image->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat,
                                      SkSamplingOptions(), matrix));
    canvas->drawPaint(paint);
}
}  // END FIDDLE
