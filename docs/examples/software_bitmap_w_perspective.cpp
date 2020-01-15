// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE_ANIMATED(software_bitmap_w_perspective, 256, 256, false, 6, 5) {
void draw(SkCanvas* canvas) {
    const float width = image->width();
    const float height = image->height();

    const SkPoint src[] = {{ 0, 0}, {width, 0}, {width * (float)frame, height * (float)frame}, { 0, height}};
    const SkPoint dst[] = { {0, 0}, {width, 0}, {width, height}, {0, height} };

    SkMatrix matrix;
    matrix.setPolyToPoly(src, dst, 4);

    SkAutoCanvasRestore acr(canvas, true);
    canvas->concat(matrix);
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
