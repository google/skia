#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=fb8b2502bbe52d2029aecdf569dd9fdb
REG_FIDDLE(TextBlob_000, 256, 70, false, 0) {
void draw(SkCanvas* canvas) {
    SkTextBlobBuilder textBlobBuilder;
    const char bunny[] = "/(^x^)\\";
    const int len = sizeof(bunny) - 1;
    uint16_t glyphs[len];
    SkPaint paint;
    paint.textToGlyphs(bunny, len, glyphs);
    paint.setTextEncoding(kGlyphID_SkTextEncoding);
    SkFont font;
    int runs[] = { 3, 1, 3 };
    SkPoint textPos = { 20, 50 };
    int glyphIndex = 0;
    for (auto runLen : runs) {
        font.setSize(1 == runLen ? 20 : 50);
        paint.setTextSize(1 == runLen ? 20 : 50);
        const SkTextBlobBuilder::RunBuffer& run =
                textBlobBuilder.allocRun(font, runLen, textPos.fX, textPos.fY);
        memcpy(run.glyphs, &glyphs[glyphIndex], sizeof(glyphs[0]) * runLen);
        textPos.fX += paint.measureText(&glyphs[glyphIndex], sizeof(glyphs[0]) * runLen, nullptr);
        glyphIndex += runLen;
    }
    sk_sp<const SkTextBlob> blob = textBlobBuilder.make();
    canvas->drawTextBlob(blob.get(), 0, 0, paint);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRect(blob->bounds(), paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
