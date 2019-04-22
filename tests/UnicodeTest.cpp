/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "src/utils/SkUTF.h"
#include "tests/Test.h"

// Simple test to ensure that when we call textToGlyphs, we get the same
// result (for the same text) when using UTF8, UTF16, UTF32.
// TODO: make the text more complex (i.e. incorporate chars>7bits)
DEF_TEST(Unicode_textencodings, reporter) {
    const char text8[] = "ABCDEFGabcdefg0123456789";
    uint16_t text16[sizeof(text8)];
    int32_t  text32[sizeof(text8)];
    size_t len8 = strlen(text8);
    size_t len16 = len8 * 2;
    size_t len32 = len8 * 4;

    // expand our 8bit chars to 16 and 32
    for (size_t i = 0; i < len8; ++i) {
        text32[i] = text16[i] = text8[i];
    }

    uint16_t glyphs8[sizeof(text8)];
    uint16_t glyphs16[sizeof(text8)];
    uint16_t glyphs32[sizeof(text8)];

    SkFont font;

    int count8  = font.textToGlyphs(text8,  len8,  kUTF8_SkTextEncoding,  glyphs8,  SK_ARRAY_COUNT(glyphs8));
    int count16 = font.textToGlyphs(text16, len16, kUTF16_SkTextEncoding, glyphs16, SK_ARRAY_COUNT(glyphs16));
    int count32 = font.textToGlyphs(text32, len32, kUTF32_SkTextEncoding, glyphs32, SK_ARRAY_COUNT(glyphs32));

    REPORTER_ASSERT(reporter, (int)len8 == count8);
    REPORTER_ASSERT(reporter, (int)len8 == count16);
    REPORTER_ASSERT(reporter, (int)len8 == count32);

    REPORTER_ASSERT(reporter, !memcmp(glyphs8, glyphs16, count8 * sizeof(uint16_t)));
    REPORTER_ASSERT(reporter, !memcmp(glyphs8, glyphs32, count8 * sizeof(uint16_t)));
}

#include "include/core/SkFont.h"
#include "src/core/SkFontPriv.h"

DEF_TEST(glyphs_to_unichars, reporter) {
    SkFont font;

    const int N = 52;
    SkUnichar uni[N];
    for (int i = 0; i < 26; ++i) {
        uni[i +  0] = i + 'A';
        uni[i + 26] = i + 'a';
    }
    uint16_t glyphs[N];
    font.textToGlyphs(uni, sizeof(uni), kUTF32_SkTextEncoding, glyphs, N);

    SkUnichar uni2[N];
    SkFontPriv::GlyphsToUnichars(font, glyphs, N, uni2);
    REPORTER_ASSERT(reporter, memcmp(uni, uni2, sizeof(uni)) == 0);
}

