// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=b07e62298e7b0ab5683db199faffceb2
REG_FIDDLE(Matrix_preConcat, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix, matrix2;
    SkPoint bitmapBounds[4], perspect[4] = {{50, 10}, {180, 40}, {236, 176}, {10, 206}};
    SkRect::Make(source.bounds()).toQuad(bitmapBounds);
    matrix.setPolyToPoly(bitmapBounds, perspect, 4);
    matrix2.setPolyToPoly(perspect, bitmapBounds, 4);
    matrix.preConcat(matrix2);
    canvas->concat(matrix);
    canvas->drawBitmap(source, 0, 0);
}
}  // END FIDDLE
