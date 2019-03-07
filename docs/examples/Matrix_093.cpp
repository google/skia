// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=fad6b92b21b1e1deeae61978cec2d232
REG_FIDDLE(Matrix_093, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    const SkPoint center = { 128, 128 };
    matrix.setScale(20, 25, center.fX, center.fY);
    matrix.postRotate(75, center.fX, center.fY);
    {
       SkAutoCanvasRestore acr(canvas, true);
       canvas->concat(matrix);
       canvas->drawBitmap(source, 0, 0);
    }
    if (matrix.isFixedStepInX()) {
       SkPaint paint;
       paint.setAntiAlias(true);
       SkVector step = matrix.fixedStepInX(128);
       SkVector end = center + step;
       canvas->drawLine(center, end, paint);
       SkVector arrow = { step.fX + step.fY, step.fY - step.fX};
       arrow = arrow * .25f;
       canvas->drawLine(end, end - arrow, paint);
       canvas->drawLine(end, {end.fX + arrow.fY, end.fY - arrow.fX}, paint);
    }
}
}  // END FIDDLE
