// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=b1ead09c67a177ab8eace12b061610a7
REG_FIDDLE(Matrix_mapXY_2, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    SkPoint bitmapBounds[4], perspect[4] = {{50, 10}, {180, 40}, {236, 176}, {30, 206}};
    SkRect::Make(source.bounds()).toQuad(bitmapBounds);
    matrix.setPolyToPoly(bitmapBounds, perspect, 4);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStrokeWidth(3);
    for (int x : { 0, source.width() } ) {
        for (int y : { 0, source.height() } ) {
            canvas->drawPoint(matrix.mapXY(x, y), paint);
        }
    }
    canvas->concat(matrix);
    canvas->drawImage(image, 0, 0);
}
}  // END FIDDLE
