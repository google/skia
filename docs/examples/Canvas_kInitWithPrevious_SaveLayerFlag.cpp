// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=05db6a937225e8e31ae3481173d25dae
REG_FIDDLE(Canvas_kInitWithPrevious_SaveLayerFlag, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint redPaint, bluePaint, scalePaint;
    redPaint.setColor(SK_ColorRED);
    canvas->drawCircle(21, 21, 8, redPaint);
    bluePaint.setColor(SK_ColorBLUE);
    canvas->drawCircle(31, 21, 8, bluePaint);
    SkMatrix matrix;
    matrix.setScale(4, 4);
    scalePaint.setAlpha(0x40);
    scalePaint.setImageFilter(
            SkImageFilters::MatrixTransform(matrix, SkSamplingOptions(), nullptr));
    SkCanvas::SaveLayerRec saveLayerRec(nullptr, &scalePaint,
            SkCanvas::kInitWithPrevious_SaveLayerFlag);
    canvas->saveLayer(saveLayerRec);
    canvas->restore();
}
}  // END FIDDLE
