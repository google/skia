/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkAutoMalloc.h"
#include "SkEndian.h"
#include "SkFontStream.h"
#include "SkOSFile.h"
#include "SkPaint.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "Test.h"

//#define DUMP_TABLES
//#define DUMP_TTC_TABLES

#define kFontTableTag_head          SkSetFourByteTag('h', 'e', 'a', 'd')
#define kFontTableTag_hhea          SkSetFourByteTag('h', 'h', 'e', 'a')
#define kFontTableTag_maxp          SkSetFourByteTag('m', 'a', 'x', 'p')

static const struct TagSize {
    SkFontTableTag  fTag;
    size_t          fSize;
} gKnownTableSizes[] = {
    {   kFontTableTag_head,         54 },
    {   kFontTableTag_hhea,         36 },
};

// Test that getUnitsPerEm() agrees with a direct lookup in the 'head' table
// (if that table is available).
static void test_unitsPerEm(skiatest::Reporter* reporter, const sk_sp<SkTypeface>& face) {
    int nativeUPEM = face->getUnitsPerEm();

    int tableUPEM = -1;
    size_t size = face->getTableSize(kFontTableTag_head);
    if (size) {
        // unitsPerEm is at offset 18 into the 'head' table.
        uint16_t rawUPEM;
        face->getTableData(kFontTableTag_head, 18, sizeof(rawUPEM), &rawUPEM);
        tableUPEM = SkEndian_SwapBE16(rawUPEM);
    }

    if (tableUPEM >= 0) {
        REPORTER_ASSERT(reporter, tableUPEM == nativeUPEM);
    }
}

// Test that countGlyphs() agrees with a direct lookup in the 'maxp' table
// (if that table is available).
static void test_countGlyphs(skiatest::Reporter* reporter, const sk_sp<SkTypeface>& face) {
    int nativeGlyphs = face->countGlyphs();

    int tableGlyphs = -1;
    size_t size = face->getTableSize(kFontTableTag_maxp);
    if (size) {
        // glyphs is at offset 4 into the 'maxp' table.
        uint16_t rawGlyphs;
        face->getTableData(kFontTableTag_maxp, 4, sizeof(rawGlyphs), &rawGlyphs);
        tableGlyphs = SkEndian_SwapBE16(rawGlyphs);
    }

    if (tableGlyphs >= 0) {
        REPORTER_ASSERT(reporter, tableGlyphs == nativeGlyphs);
    }
}

// The following three are all the same code points in various encodings.
// a中Яיו𝄞a𠮟
static uint8_t utf8Chars[] = { 0x61, 0xE4,0xB8,0xAD, 0xD0,0xAF, 0xD7,0x99, 0xD7,0x95, 0xF0,0x9D,0x84,0x9E, 0x61, 0xF0,0xA0,0xAE,0x9F };
static uint16_t utf16Chars[] = { 0x0061, 0x4E2D, 0x042F, 0x05D9, 0x05D5, 0xD834,0xDD1E, 0x0061, 0xD842,0xDF9F };
static uint32_t utf32Chars[] = { 0x00000061, 0x00004E2D, 0x0000042F, 0x000005D9, 0x000005D5, 0x0001D11E, 0x00000061, 0x00020B9F };

struct CharsToGlyphs_TestData {
    const void* chars;
    int charCount;
    size_t charsByteLength;
    SkTypeface::Encoding typefaceEncoding;
    const char* name;
} static charsToGlyphs_TestData[] = {
    { utf8Chars, 8, sizeof(utf8Chars), SkTypeface::kUTF8_Encoding, "Simple UTF-8" },
    { utf16Chars, 8, sizeof(utf16Chars), SkTypeface::kUTF16_Encoding, "Simple UTF-16" },
    { utf32Chars, 8, sizeof(utf32Chars), SkTypeface::kUTF32_Encoding, "Simple UTF-32" },
};

// Test that SkPaint::textToGlyphs agrees with SkTypeface::charsToGlyphs.
static void test_charsToGlyphs(skiatest::Reporter* reporter, const sk_sp<SkTypeface>& face) {
    uint16_t paintGlyphIds[256];
    uint16_t faceGlyphIds[256];

    for (size_t testIndex = 0; testIndex < SK_ARRAY_COUNT(charsToGlyphs_TestData); ++testIndex) {
        CharsToGlyphs_TestData& test = charsToGlyphs_TestData[testIndex];

        SkPaint paint;
        paint.setTypeface(face);
        paint.setTextEncoding((SkPaint::TextEncoding)test.typefaceEncoding);
        paint.textToGlyphs(test.chars, test.charsByteLength, paintGlyphIds);

        face->charsToGlyphs(test.chars, test.typefaceEncoding, faceGlyphIds, test.charCount);

        for (int i = 0; i < test.charCount; ++i) {
            SkString name;
            face->getFamilyName(&name);
            SkString a;
            a.appendf("%s, paintGlyphIds[%d] = %d, faceGlyphIds[%d] = %d, face = %s",
                      test.name, i, (int)paintGlyphIds[i], i, (int)faceGlyphIds[i], name.c_str());
            REPORTER_ASSERT_MESSAGE(reporter, paintGlyphIds[i] == faceGlyphIds[i], a.c_str());
        }
    }
}

static void test_fontstream(skiatest::Reporter* reporter, SkStream* stream, int ttcIndex) {
    int n = SkFontStream::GetTableTags(stream, ttcIndex, nullptr);
    SkAutoTArray<SkFontTableTag> array(n);

    int n2 = SkFontStream::GetTableTags(stream, ttcIndex, array.get());
    REPORTER_ASSERT(reporter, n == n2);

    for (int i = 0; i < n; ++i) {
#ifdef DUMP_TTC_TABLES
        SkString str;
        SkFontTableTag t = array[i];
        str.appendUnichar((t >> 24) & 0xFF);
        str.appendUnichar((t >> 16) & 0xFF);
        str.appendUnichar((t >>  8) & 0xFF);
        str.appendUnichar((t >>  0) & 0xFF);
        SkDebugf("[%d:%d] '%s'\n", ttcIndex, i, str.c_str());
#endif
        size_t size = SkFontStream::GetTableSize(stream, ttcIndex, array[i]);
        for (size_t j = 0; j < SK_ARRAY_COUNT(gKnownTableSizes); ++j) {
            if (gKnownTableSizes[j].fTag == array[i]) {
                REPORTER_ASSERT(reporter, gKnownTableSizes[j].fSize == size);
            }
        }
    }
}

static void test_fontstream(skiatest::Reporter* reporter) {
    std::unique_ptr<SkStreamAsset> stream(GetResourceAsStream("/fonts/test.ttc"));
    if (!stream) {
        SkDebugf("Skipping FontHostTest::test_fontstream\n");
        return;
    }

    int count = SkFontStream::CountTTCEntries(stream.get());
#ifdef DUMP_TTC_TABLES
    SkDebugf("CountTTCEntries %d\n", count);
#endif
    for (int i = 0; i < count; ++i) {
        test_fontstream(reporter, stream.get(), i);
    }
}

static void test_symbolfont(skiatest::Reporter* reporter) {
    SkUnichar c = 0xf021;
    uint16_t g;
    SkPaint paint;
    paint.setTypeface(MakeResourceAsTypeface("/fonts/SpiderSymbol.ttf"));
    paint.setTextEncoding(SkPaint::kUTF32_TextEncoding);
    paint.textToGlyphs(&c, 4, &g);

    if (!paint.getTypeface()) {
        SkDebugf("Skipping FontHostTest::test_symbolfont\n");
        return;
    }

    REPORTER_ASSERT(reporter, g == 3);
}

static void test_tables(skiatest::Reporter* reporter, const sk_sp<SkTypeface>& face) {
    if (false) { // avoid bit rot, suppress warning
        SkFontID fontID = face->uniqueID();
        REPORTER_ASSERT(reporter, fontID);
    }

    int count = face->countTables();

    SkAutoTMalloc<SkFontTableTag> storage(count);
    SkFontTableTag* tags = storage.get();

    int count2 = face->getTableTags(tags);
    REPORTER_ASSERT(reporter, count2 == count);

    for (int i = 0; i < count; ++i) {
        size_t size = face->getTableSize(tags[i]);
        REPORTER_ASSERT(reporter, size > 0);

#ifdef DUMP_TABLES
        char name[5];
        name[0] = (tags[i] >> 24) & 0xFF;
        name[1] = (tags[i] >> 16) & 0xFF;
        name[2] = (tags[i] >>  8) & 0xFF;
        name[3] = (tags[i] >>  0) & 0xFF;
        name[4] = 0;
        SkDebugf("%s %d\n", name, size);
#endif

        for (size_t j = 0; j < SK_ARRAY_COUNT(gKnownTableSizes); ++j) {
            if (gKnownTableSizes[j].fTag == tags[i]) {
                REPORTER_ASSERT(reporter, gKnownTableSizes[j].fSize == size);
            }
        }

        // do we get the same size from GetTableData and GetTableSize
        {
            SkAutoMalloc data(size);
            size_t size2 = face->getTableData(tags[i], 0, size, data.get());
            REPORTER_ASSERT(reporter, size2 == size);
        }
    }
}

static void test_tables(skiatest::Reporter* reporter) {
    static const char* const gNames[] = {
        nullptr,   // default font
        "Helvetica", "Arial",
        "Times", "Times New Roman",
        "Courier", "Courier New",
        "Terminal", "MS Sans Serif",
        "Hiragino Mincho ProN", "MS PGothic",
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gNames); ++i) {
        sk_sp<SkTypeface> face(SkTypeface::MakeFromName(gNames[i], SkFontStyle()));
        if (face) {
#ifdef DUMP_TABLES
            SkDebugf("%s\n", gNames[i]);
#endif
            test_tables(reporter, face);
            test_unitsPerEm(reporter, face);
            test_countGlyphs(reporter, face);
            test_charsToGlyphs(reporter, face);
        }
    }
}

/*
 * Verifies that the advance values returned by generateAdvance and
 * generateMetrics match.
 */
static void test_advances(skiatest::Reporter* reporter) {
    static const char* const faces[] = {
        nullptr,   // default font
        "Arial", "Times", "Times New Roman", "Helvetica", "Courier",
        "Courier New", "Verdana", "monospace",
    };

    static const struct {
        SkPaint::Hinting    hinting;
        unsigned            flags;
    } settings[] = {
        { SkPaint::kNo_Hinting,     0                               },
        { SkPaint::kNo_Hinting,     SkPaint::kLinearText_Flag       },
        { SkPaint::kNo_Hinting,     SkPaint::kSubpixelText_Flag     },
        { SkPaint::kSlight_Hinting, 0                               },
        { SkPaint::kSlight_Hinting, SkPaint::kLinearText_Flag       },
        { SkPaint::kSlight_Hinting, SkPaint::kSubpixelText_Flag     },
        { SkPaint::kNormal_Hinting, 0                               },
        { SkPaint::kNormal_Hinting, SkPaint::kLinearText_Flag       },
        { SkPaint::kNormal_Hinting, SkPaint::kSubpixelText_Flag     },
    };

    static const struct {
        SkScalar    fScaleX;
        SkScalar    fSkewX;
    } gScaleRec[] = {
        { SK_Scalar1, 0 },
        { SK_Scalar1/2, 0 },
        // these two exercise obliquing (skew)
        { SK_Scalar1, -SK_Scalar1/4 },
        { SK_Scalar1/2, -SK_Scalar1/4 },
    };

    SkPaint paint;
    char txt[] = "long.text.with.lots.of.dots.";

    for (size_t i = 0; i < SK_ARRAY_COUNT(faces); i++) {
        paint.setTypeface(SkTypeface::MakeFromName(faces[i], SkFontStyle()));

        for (size_t j = 0; j  < SK_ARRAY_COUNT(settings); j++) {
            paint.setHinting(settings[j].hinting);
            paint.setLinearText((settings[j].flags & SkPaint::kLinearText_Flag) != 0);
            paint.setSubpixelText((settings[j].flags & SkPaint::kSubpixelText_Flag) != 0);

            for (size_t k = 0; k < SK_ARRAY_COUNT(gScaleRec); ++k) {
                paint.setTextScaleX(gScaleRec[k].fScaleX);
                paint.setTextSkewX(gScaleRec[k].fSkewX);

                SkRect bounds;

                // For no hinting and light hinting this should take the
                // optimized generateAdvance path.
                SkScalar width1 = paint.measureText(txt, strlen(txt));

                // Requesting the bounds forces a generateMetrics call.
                SkScalar width2 = paint.measureText(txt, strlen(txt), &bounds);

                // SkDebugf("Font: %s, generateAdvance: %f, generateMetrics: %f\n",
                //    faces[i], SkScalarToFloat(width1), SkScalarToFloat(width2));

                REPORTER_ASSERT(reporter, width1 == width2);
            }
        }
    }
}

DEF_TEST(FontHost, reporter) {
    test_tables(reporter);
    test_fontstream(reporter);
    test_advances(reporter);
    test_symbolfont(reporter);
}

// need tests for SkStrSearch
