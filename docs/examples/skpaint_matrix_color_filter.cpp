// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(skpaint_matrix_color_filter, 256, 128, false, 3) {
void f(SkCanvas* c, SkScalar x, SkScalar y, SkScalar colorMatrix[20]) {
    SkPaint paint;
    paint.setColorFilter(SkColorFilters::Matrix(colorMatrix));
    c->drawImage(image, x, y, SkSamplingOptions(), &paint);
}
void draw(SkCanvas* c) {
    c->scale(0.25, 0.25);
    SkScalar colorMatrix1[20] = {
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        1, 0, 0, 0, 0,
        0, 0, 0, 1, 0};
    f(c, 0, 0, colorMatrix1);

    SkScalar grayscale[20] = {
        0.21f, 0.72f, 0.07f, 0.0f, 0.0f,
        0.21f, 0.72f, 0.07f, 0.0f, 0.0f,
        0.21f, 0.72f, 0.07f, 0.0f, 0.0f,
        0.0f,  0.0f,  0.0f,  1.0f, 0.0f};
    f(c, 512, 0, grayscale);
}
}  // END FIDDLE
