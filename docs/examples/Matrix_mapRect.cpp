// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=dbcf928b035a31ca69c99392e2e2cca9
REG_FIDDLE(Matrix_mapRect, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkMatrix matrix;
    matrix.setRotate(45, 128, 128);
    SkRect rotatedBounds, bounds = {40, 50, 190, 200};
    matrix.mapRect(&rotatedBounds, bounds );
    paint.setColor(SK_ColorGRAY);
    canvas->drawRect(rotatedBounds, paint);
    canvas->concat(matrix);
    paint.setColor(SK_ColorRED);
    canvas->drawRect(bounds, paint);
}
}  // END FIDDLE
