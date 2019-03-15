// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6b4562c7052da94f3d5b2412dca41946
REG_FIDDLE(Matrix_104, 256, 64, false, 4) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix, matrix2;
    SkPoint bitmapBounds[4], perspect[4] = {{50, 10}, {180, 40}, {236, 176}, {10, 206}};
    SkRect::Make(source.bounds()).toQuad(bitmapBounds);
    matrix.setPolyToPoly(bitmapBounds, perspect, 4);
    matrix2.setPolyToPoly(perspect, bitmapBounds, 4);
    SkMatrix concat = SkMatrix::Concat(matrix, matrix2);
    canvas->concat(concat);
    canvas->drawBitmap(source, 0, 0);
}
}  // END FIDDLE
