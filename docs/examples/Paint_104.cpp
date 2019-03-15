#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=767fa4e7b6300e16a419f9881f0f9d3d
REG_FIDDLE(Paint_104, 256, 128, false, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    const char hello8[] = "Hello" "\xE2" "\x98" "\xBA";
    const uint16_t hello16[] = { 'H', 'e', 'l', 'l', 'o', 0x263A };
    const uint32_t hello32[] = { 'H', 'e', 'l', 'l', 'o', 0x263A };
    paint.setTextSize(24);
    canvas->drawText(hello8, sizeof(hello8) - 1, 10, 30, paint);
    paint.setTextEncoding(kUTF16_SkTextEncoding);
    canvas->drawText(hello16, sizeof(hello16), 10, 60, paint);
    paint.setTextEncoding(kUTF32_SkTextEncoding);
    canvas->drawText(hello32, sizeof(hello32), 10, 90, paint);
    uint16_t glyphs[SK_ARRAY_COUNT(hello32)];
    paint.textToGlyphs(hello32, sizeof(hello32), glyphs);
    paint.setTextEncoding(kGlyphID_SkTextEncoding);
    canvas->drawText(glyphs, sizeof(glyphs), 10, 120, paint);
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
