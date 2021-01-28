// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=2531f8d1e05d7b6dc22f3efcd2fb84e4
REG_FIDDLE(Matrix_preScale, 256, 256, false, 3) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    SkPoint bitmapBounds[4], perspect[4] = {{50, 10}, {180, 40}, {236, 176}, {10, 206}};
    SkRect::Make(image->bounds()).toQuad(bitmapBounds);
    matrix.setPolyToPoly(bitmapBounds, perspect, 4);
    matrix.preScale(.75f, 1.5f, image->width() / 2, image->height() / 2);
    canvas->concat(matrix);
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
