// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(Matrix_postRotate, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    SkPoint perspect[4] = {{50, 10}, {180, 40}, {236, 176}, {10, 206}};
    const std::array<SkPoint, 4> bitmapBounds = SkRect::Make(source.bounds()).toQuad();
    matrix.setPolyToPoly(bitmapBounds, perspect);
    matrix.postRotate(45, source.width() / 2, source.height() / 2);
    canvas->concat(matrix);
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
