// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=8c34ae3a2b7e2742bb969819737365ec
REG_FIDDLE(Matrix_postSkew, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    SkPoint bitmapBounds[4], perspect[4] = {{50, 10}, {180, 40}, {236, 176}, {10, 206}};
    SkRect::Make(image->bounds()).toQuad(bitmapBounds);
    matrix.setPolyToPoly(bitmapBounds, perspect, 4);
    matrix.postSkew(.5f, 0, image->width() / 2, image->height() / 2);
    canvas->concat(matrix);
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
