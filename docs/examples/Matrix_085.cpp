#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=8bf1518db3f369696cd3065b541a8bd7
REG_FIDDLE(Matrix_085, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    paint.setAntiAlias(true);
    paint.setTextSize(48);
    SkMatrix matrix;
    matrix.setRotate(90);
    SkVector offset = { 7, 7 };
    for (int i = 0; i < 4; ++i) {
        paint.setImageFilter(SkDropShadowImageFilter::Make(offset.fX, offset.fY, 3, 3,
              SK_ColorBLUE, SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode, nullptr));
        offset = matrix.mapVector(offset.fX, offset.fY);
        canvas->translate(0, 60);
        canvas->drawString("Text", 50, 0, paint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
