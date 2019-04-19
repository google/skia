#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6e12cceca981ddabc0fc18c380543f34
REG_FIDDLE(TextBlob_uniqueID, 256, 256, false, 0) {
void draw(SkCanvas* canvas) {
    for (int index = 0; index < 2; ++index) {
        SkTextBlobBuilder textBlobBuilder;
        const char bunny[] = "/(^x^)\\";
        const int len = sizeof(bunny) - 1;
        uint16_t glyphs[len];
        SkPaint paint;
        paint.textToGlyphs(bunny, len, glyphs);
        paint.setTextEncoding(kGlyphID_SkTextEncoding);
        paint.setTextScaleX(0.5);
        SkFont font;
        font.setScaleX(0.5);
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
        paint.reset();
        canvas->drawTextBlob(blob.get(), 0, 0, paint);
        std::string id = "unique ID:" + std::to_string(blob->uniqueID());
        canvas->drawString(id.c_str(), 30, blob->bounds().fBottom + 15, paint);
        canvas->translate(blob->bounds().fRight + 10, 0);
    }
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
