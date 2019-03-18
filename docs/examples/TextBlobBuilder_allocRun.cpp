#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=f0e584aec20eaee7a5bfed62aa885eee
REG_FIDDLE(TextBlobBuilder_allocRun, 256, 60, false, 0) {
void draw(SkCanvas* canvas) {
    SkTextBlobBuilder builder;
    SkFont font;
    SkPaint paint;
    const SkTextBlobBuilder::RunBuffer& run = builder.allocRun(font, 5, 20, 20);
    paint.textToGlyphs("hello", 5, run.glyphs);
    canvas->drawRect({20, 20, 30, 30}, paint);
    canvas->drawTextBlob(builder.make(), 20, 20, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
