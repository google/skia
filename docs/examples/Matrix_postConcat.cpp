// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=e4226c55d9bdbc119264bd372b2b9835
REG_FIDDLE(Matrix_postConcat, 256, 64, false, 3) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    SkPoint bitmapBounds[4], perspect[4] = {{50, 10}, {180, 40}, {236, 176}, {10, 206}};
    SkRect::Make(source.bounds()).toQuad(bitmapBounds);
    matrix.setPolyToPoly(bitmapBounds, perspect, 4);
    matrix.postConcat(matrix);
    canvas->concat(matrix);
    canvas->drawBitmap(source, 0, 0);
}
}  // END FIDDLE
