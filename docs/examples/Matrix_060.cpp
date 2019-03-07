// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f5144ef4bd7cea294fad2f756ed335af
REG_FIDDLE(Matrix_060, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkRect rect = {20, 20, 100, 100};
    for (int i = 0; i < 2; ++i ) {
        SkMatrix matrix;
        i == 0 ? matrix.reset(): matrix.setRotate(25, rect.centerX(), 320);
        {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->concat(matrix);
            paint.setColor(SK_ColorGRAY);
            canvas->drawRect(rect, paint);
        }
        paint.setColor(SK_ColorRED);
        for (int j = 0; j < 2; ++j ) {
            SkAutoCanvasRestore acr(canvas, true);
            matrix.postTranslate(40, 40);
            canvas->concat(matrix);
            canvas->drawCircle(0, 0, 3, paint);
        }
    }
}
}  // END FIDDLE
