#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c851d1313e8909aaea4f0591699fdb7b
REG_FIDDLE(Matrix_072, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    const SkPoint src[] = { { 0, 0}, {30,   0}, {30, -30}, { 0, -30} };
    const SkPoint dst[] = { {50, 0}, {80, -10}, {90, -30}, {60, -40} };
    SkPaint blackPaint;
    blackPaint.setAntiAlias(true);
    blackPaint.setTextSize(42);
    SkPaint redPaint = blackPaint;
    redPaint.setColor(SK_ColorRED);
    for (int count : { 1, 2, 3, 4 } ) {
        canvas->translate(35, 55);
        for (int index = 0; index < count; ++index) {
            canvas->drawCircle(src[index], 3, blackPaint);
            canvas->drawCircle(dst[index], 3, blackPaint);
            if (index > 0) {
                canvas->drawLine(src[index], src[index - 1], blackPaint);
                canvas->drawLine(dst[index], dst[index - 1], blackPaint);
            }
        }
        SkMatrix matrix;
        matrix.setPolyToPoly(src, dst, count);
        canvas->drawString("A", src[0].fX, src[0].fY, redPaint);
        SkAutoCanvasRestore acr(canvas, true);
        canvas->concat(matrix);
        canvas->drawString("A", src[0].fX, src[0].fY, redPaint);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
