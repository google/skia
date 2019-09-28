#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
// HASH=34c37c0212cc0aef670d96945d08fe24
REG_FIDDLE(TextBlobBuilder_make, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkTextBlobBuilder builder;
    sk_sp<SkTextBlob> blob = builder.make();
    SkDebugf("blob " "%s" " nullptr\n", blob == nullptr ? "equals" : "does not equal");
    SkPaint paint;
    paint.setTextEncoding(SkTextEncoding::kGlyphID);
    SkFont font;
    paint.textToGlyphs("x", 1, builder.allocRun(font, 1, 20, 20).glyphs);
    blob = builder.make();
    SkDebugf("blob " "%s" " nullptr\n", blob == nullptr ? "equals" : "does not equal");
    blob = builder.make();
    SkDebugf("blob " "%s" " nullptr\n", blob == nullptr ? "equals" : "does not equal");
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
