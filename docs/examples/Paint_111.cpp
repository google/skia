#if 0  // Disabled until updated to use current API.
// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "fiddle/examples.h"
// HASH=6a68cb3c8b81a5976c81ee004f559247
REG_FIDDLE(Paint_containsText, 256, 256, true, 0) {
void draw(SkCanvas* canvas) {
    SkPaint paint;
    const uint16_t goodGlyph = 511;
    const uint16_t zeroGlyph = 0;
    const uint16_t badGlyph = 65535; // larger than glyph count in font
    paint.setTextEncoding(kGlyphID_SkTextEncoding);
    SkDebugf("0x%04x %c= has glyph\n", goodGlyph,
            paint.containsText(&goodGlyph, 2) ? '=' : '!');
    SkDebugf("0x%04x %c= has glyph\n", zeroGlyph,
            paint.containsText(&zeroGlyph, 2) ? '=' : '!');
    SkDebugf("0x%04x %c= has glyph\n", badGlyph,
            paint.containsText(&badGlyph, 2) ? '=' : '!');
}
}  // END FIDDLE
#endif  // Disabled until updated to use current API.
