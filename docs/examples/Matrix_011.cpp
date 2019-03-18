#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=688123908c733169bbbfaf11f41ecff6
REG_FIDDLE(Matrix_hasPerspective, 256, 256, false, 4) {
void draw(SkCanvas* canvas) {
    SkMatrix matrix;
    SkPoint bitmapBounds[4], perspect[4] = {{50, 10}, {180, 40}, {236, 176}, {10, 206}};
    SkRect::Make(source.bounds()).toQuad(bitmapBounds);
    matrix.setPolyToPoly(bitmapBounds, perspect, 4);
    canvas->concat(matrix);
    SkString string;
    string.printf("hasPerspective %s", matrix.hasPerspective() ? "true" : "false");
    canvas->drawBitmap(source, 0, 0);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(48);
    canvas->drawString(string, 0, source.bounds().height() + 48, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
