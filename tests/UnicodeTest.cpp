/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkPaint.h"
#include "SkUtils.h"

// Unicode Variation Selector ranges: inclusive
#define UVS_MIN0    0x180B
#define UVS_MAX0    0x180D
#define UVS_MIN1    0xFE00
#define UVS_MAX1    0xFE0F
#define UVS_MIN2    0xE0100
#define UVS_MAX2    0xE01EF

static bool isUVS(SkUnichar uni) {
    return (uni >= UVS_MIN0 && uni <= UVS_MAX0) ||
           (uni >= UVS_MIN1 && uni <= UVS_MAX1) ||
           (uni >= UVS_MIN2 && uni <= UVS_MAX2);
}

static void test_uvs(skiatest::Reporter* reporter) {
    // [min, max], [min, max] ... inclusive
    static const SkUnichar gRanges[] = {
        UVS_MIN0, UVS_MAX0, UVS_MIN1, UVS_MAX1, UVS_MIN2, UVS_MAX2
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRanges); i += 2) {
        for (SkUnichar uni = gRanges[i] - 8; uni <= gRanges[i+1] + 8; ++uni) {
            bool uvs0 = isUVS(uni);
            bool uvs1 = SkUnichar_IsVariationSelector(uni);
            REPORTER_ASSERT(reporter, uvs0 == uvs1);
        }
    }
}

// Simple test to ensure that when we call textToGlyphs, we get the same
// result (for the same text) when using UTF8, UTF16, UTF32.
// TODO: make the text more complex (i.e. incorporate chars>7bits)
static void test_textencodings(skiatest::Reporter* reporter) {
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

DEF_TEST(Unicode, reporter) {
    test_uvs(reporter);
    test_textencodings(reporter);
}
