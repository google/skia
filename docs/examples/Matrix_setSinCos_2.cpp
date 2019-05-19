// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e37a94a53c959951b059fcd624639ef6
REG_FIDDLE(Matrix_setSinCos_2, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorGRAY);
    paint.setAntiAlias(true);
    SkRect rect = {20, 20, 100, 100};
    canvas->drawRect(rect, paint);
    paint.setColor(SK_ColorRED);
    SkMatrix matrix;
    matrix.setSinCos(.25f, .85f);
    matrix.postTranslate(rect.centerX(), rect.centerY());
    canvas->concat(matrix);
    canvas->translate(-rect.centerX(), -rect.centerY());
    canvas->drawRect(rect, paint);
}
}  // END FIDDLE
