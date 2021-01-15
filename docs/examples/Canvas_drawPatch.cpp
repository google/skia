// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=accb545d67984ced168f5be6ab824795
REG_FIDDLE(Canvas_drawPatch, 256, 256, false, 5) {
void draw(SkCanvas* canvas) {
    // SkBitmap source = cmbkygk;
    SkPaint paint;
    paint.setAntiAlias(true);
    SkPoint cubics[] = { { 3, 1 },    { 4, 2 }, { 5, 1 },    { 7, 3 },
                      /* { 7, 3 }, */ { 6, 4 }, { 7, 5 },    { 5, 7 },
                      /* { 5, 7 }, */ { 4, 6 }, { 3, 7 },    { 1, 5 },
                      /* { 1, 5 }, */ { 2, 4 }, { 1, 3 }, /* { 3, 1 } */ };
    SkColor colors[] = { 0xbfff0000, 0xbf0000ff, 0xbfff00ff, 0xbf00ffff };
    SkPoint texCoords[] = { { -30, -30 }, { 162, -30}, { 162, 162}, { -30, 162} };
    paint.setShader(source.makeShader(SkSamplingOptions(SkFilterMode::kLinear)));
    canvas->scale(15, 15);
    for (auto blend : { SkBlendMode::kSrcOver, SkBlendMode::kModulate, SkBlendMode::kXor } ) {
        canvas->drawPatch(cubics, colors, texCoords, blend, paint);
        canvas->translate(4, 4);
    }
}
}  // END FIDDLE
