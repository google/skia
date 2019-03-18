// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=e09194ee48a81e7b375ade473d340f0d
REG_FIDDLE(Matrix_postRotate, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    SkPoint bitmapBounds[4], perspect[4] = {{50, 10}, {180, 40}, {236, 176}, {10, 206}};
    SkRect::Make(source.bounds()).toQuad(bitmapBounds);
    matrix.setPolyToPoly(bitmapBounds, perspect, 4);
    matrix.postRotate(45, source.width() / 2, source.height() / 2);
    canvas->concat(matrix);
    canvas->drawBitmap(source, 0, 0);
}
}  // END FIDDLE
