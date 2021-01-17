// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=3412c2a16cb529af0e04878d264451f2
REG_FIDDLE(Canvas_drawPatch_2_b, 256, 256, false, 6) {
void draw(SkCanvas* canvas) {
    // SkBitmap source = checkerboard;
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPoint cubics[] = { { 3, 1 },    { 4, 2 }, { 5, 1 },    { 7, 3 },
                      /* { 7, 3 }, */ { 6, 4 }, { 7, 5 },    { 5, 7 },
                      /* { 5, 7 }, */ { 4, 6 }, { 3, 7 },    { 1, 5 },
                      /* { 1, 5 }, */ { 2, 4 }, { 1, 3 }, /* { 3, 1 } */ };
    SkPoint texCoords[] = { { 0, 0 }, { 0, 62}, { 62, 62}, { 62, 0 } };
    paint.setShader(source.makeShader(SkSamplingOptions(SkFilterMode::kLinear)));
    canvas->scale(30, 30);
    canvas->drawPatch(cubics, nullptr, texCoords, paint);
}
}  // END FIDDLE
