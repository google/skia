/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"
#include "SkUTF.h"
#include "Test.h"

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

    SkPaint paint;

    paint.setTextEncoding(SkPaint::kUTF8_TextEncoding);
    int count8  = paint.textToGlyphs(text8,  len8,  glyphs8);

    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    int count16 = paint.textToGlyphs(text16, len16, glyphs16);

    paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);
    int count32 = paint.textToGlyphs(text32, len32, glyphs32);

    REPORTER_ASSERT(reporter, (int)len8 == count8);
    REPORTER_ASSERT(reporter, (int)len8 == count16);
    REPORTER_ASSERT(reporter, (int)len8 == count32);

    REPORTER_ASSERT(reporter, !memcmp(glyphs8, glyphs16, count8 * sizeof(uint16_t)));
    REPORTER_ASSERT(reporter, !memcmp(glyphs8, glyphs32, count8 * sizeof(uint16_t)));
}
