// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(color_filter_vs_colorFilterImageFilter_saveLayer, 256, 128, false, 3) {
sk_sp<SkColorFilter> saturate() {
    SkScalar colorMatrix[20] = {1.75, 0,    0,    0, 0,
                                0,    1.75, 0,    0, 0,
                                0,    0,    1.75, 0, 0,
                                0,    0,    0,    1, 0};
    return SkColorFilters::Matrix(colorMatrix);
}

void draw(SkCanvas* canvas) {
    SkPaint paint;
    paint.setColorFilter(saturate());
    canvas->drawImageRect(image, {0, 0, 128, 128}, SkSamplingOptions(), &paint);

    SkPaint paint2;
    paint2.setImageFilter(SkImageFilters::ColorFilter(saturate(), nullptr));
    SkAutoCanvasRestore autoCanvasRestore(canvas, false);
    canvas->saveLayer(nullptr, &paint2);
    canvas->drawImageRect(image, {128, 0, 256, 128}, SkSamplingOptions());
}
}  // END FIDDLE
