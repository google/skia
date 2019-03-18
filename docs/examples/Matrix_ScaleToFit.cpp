#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=17c3070b31b700ea8f52e48af9a66b6e
REG_FIDDLE(Matrix_ScaleToFit, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const char* labels[] = { "Fill", "Start", "Center", "End" };
    SkRect rects[] = {{5, 5, 59, 59}, {5, 74, 59, 108}, {10, 123, 44, 172}, {10, 187, 54, 231}};
    SkRect bounds;
    source.getBounds(&bounds);
    SkPaint paint;
    paint.setAntiAlias(true);
    for (auto fit : { SkMatrix::kFill_ScaleToFit, SkMatrix::kStart_ScaleToFit,
                      SkMatrix::kCenter_ScaleToFit, SkMatrix::kEnd_ScaleToFit } ) {
        for (auto rect : rects ) {
            canvas->drawRect(rect, paint);
            SkMatrix matrix;
            if (!matrix.setRectToRect(bounds, rect, fit)) {
                continue;
            }
            SkAutoCanvasRestore acr(canvas, true);
            canvas->concat(matrix);
            canvas->drawBitmap(source, 0, 0);
        }
        canvas->drawString(labels[fit], 10, 255, paint);
        canvas->translate(64, 0);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
