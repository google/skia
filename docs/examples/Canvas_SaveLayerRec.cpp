// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=ee8c0b120234e27364f8c9a786cf8f89
REG_FIDDLE(Canvas_SaveLayerRec, 256, 160, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint redPaint, bluePaint;
    redPaint.setAntiAlias(true);
    redPaint.setColor(SK_ColorRED);
    canvas->drawCircle(21, 21, 8, redPaint);
    bluePaint.setColor(SK_ColorBLUE);
    canvas->drawCircle(31, 21, 8, bluePaint);
    SkMatrix matrix;
    matrix.setScale(4, 4);
    auto scaler = SkImageFilters::MatrixTransform(matrix, SkSamplingOptions(), nullptr);
    SkCanvas::SaveLayerRec saveLayerRec(nullptr, nullptr, scaler.get(), 0);
    canvas->saveLayer(saveLayerRec);
    canvas->drawCircle(125, 85, 8, redPaint);
    canvas->restore();
}
}  // END FIDDLE
