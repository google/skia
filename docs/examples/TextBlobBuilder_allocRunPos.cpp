#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=da4fcb4a972b500996be9aff6c6c40e1
REG_FIDDLE(TextBlobBuilder_allocRunPos, 256, 90, false, 0) {
void draw(SkCanvas* canvas) {
    SkTextBlobBuilder builder;
    SkPaint paint;
    SkFont font;
    const SkTextBlobBuilder::RunBuffer& run = builder.allocRunPos(font, 5);
    paint.textToGlyphs("hello", 5, run.glyphs);
    SkPoint positions[] = {{0, 0}, {10, 10}, {20, 20}, {40, 40}, {80, 80}};
    memcpy(run.pos, positions, sizeof(positions));
    canvas->drawTextBlob(builder.make(), 20, 20, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
