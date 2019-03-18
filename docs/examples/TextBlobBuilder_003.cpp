#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=c77ac50f506106fdfef94d20bc1a6934
REG_FIDDLE(TextBlobBuilder_allocRunPosH, 256, 60, false, 0) {
void draw(SkCanvas* canvas) {
    SkTextBlobBuilder builder;
    SkPaint paint;
    SkFont font;
    const SkTextBlobBuilder::RunBuffer& run = builder.allocRunPosH(font, 5, 20);
    paint.textToGlyphs("hello", 5, run.glyphs);
    SkScalar positions[] = {0, 10, 20, 40, 80};
    memcpy(run.pos, positions, sizeof(positions));
    canvas->drawTextBlob(builder.make(), 20, 20, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
