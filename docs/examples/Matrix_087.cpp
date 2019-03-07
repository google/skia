// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=5fafd0bd23d1ed37425b970b4a3c6cc9
REG_FIDDLE(Matrix_087, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkMatrix matrix;
    matrix.setRotate(45, 128, 128);
    SkRect bounds = {40, 50, 190, 200};
    matrix.mapRect(&bounds);
    paint.setColor(SK_ColorGRAY);
    canvas->drawRect(bounds, paint);
    canvas->concat(matrix);
    paint.setColor(SK_ColorRED);
    canvas->drawRect({40, 50, 190, 200}, paint);
}
}  // END FIDDLE
