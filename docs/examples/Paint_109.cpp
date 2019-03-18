#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=d11136d8a74f63009da2a7f550710823
REG_FIDDLE(Paint_textToGlyphs, 256, 64, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    const uint8_t utf8[] = { 0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0xC2, 0xA5, 0xC2, 0xA3 };
    std::vector<SkGlyphID> glyphs;
    int count = paint.textToGlyphs(utf8, sizeof(utf8), nullptr);
    glyphs.resize(count);
    (void) paint.textToGlyphs(utf8, sizeof(utf8), &glyphs.front());
    paint.setTextEncoding(kGlyphID_SkTextEncoding);
    paint.setTextSize(32);
    canvas->drawText(&glyphs.front(), glyphs.size() * sizeof(SkGlyphID), 10, 40, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
