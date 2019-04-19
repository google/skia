// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=862cc026601a41a58df49c0b9f0d7777
REG_FIDDLE(Clip, 256, 90, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint redPaint, scalePaint;
    redPaint.setAntiAlias(true);
    redPaint.setColor(SK_ColorRED);
    canvas->save();
    for (bool antialias : { false, true } ) {
        canvas->save();
        canvas->clipRect(SkRect::MakeWH(19.5f, 11.5f), antialias);
        canvas->drawCircle(17, 11, 8, redPaint);
        canvas->restore();
        canvas->translate(16, 0);
    }
    canvas->restore();
    SkMatrix matrix;
    matrix.setScale(6, 6);
    scalePaint.setImageFilter(
            SkImageFilter::MakeMatrixFilter(matrix, kNone_SkFilterQuality, nullptr));
    SkCanvas::SaveLayerRec saveLayerRec(
            nullptr, &scalePaint, SkCanvas::kInitWithPrevious_SaveLayerFlag);
    canvas->saveLayer(saveLayerRec);
    canvas->restore();
}
}  // END FIDDLE
