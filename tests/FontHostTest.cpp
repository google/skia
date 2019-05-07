/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypeface.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkEndian.h"
#include "src/core/SkFontStream.h"
#include "src/core/SkOSFile.h"
#include "tests/Test.h"
#include "tools/Resources.h"

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
    std::unique_ptr<SkStreamAsset> stream(GetResourceAsStream("fonts/test.ttc"));
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

// Exercise this rare cmap format (platform 3, encoding 0)
static void test_symbolfont(skiatest::Reporter* reporter) {
    auto tf = MakeResourceAsTypeface("fonts/SpiderSymbol.ttf");
    if (tf) {
        SkUnichar c = 0xf021;
        uint16_t g = SkFont(tf).unicharToGlyph(c);
        REPORTER_ASSERT(reporter, g == 3);
    } else {
        // not all platforms support data fonts, so we just note that failure
        SkDebugf("Skipping FontHostTest::test_symbolfont\n");
    }
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
        SkFontHinting   hinting;
        bool            linear;
        bool            subpixel;
    } settings[] = {
        { kNo_SkFontHinting,     false, false },
        { kNo_SkFontHinting,     true,  false },
        { kNo_SkFontHinting,     false, true  },
        { kSlight_SkFontHinting, false, false },
        { kSlight_SkFontHinting, true,  false },
        { kSlight_SkFontHinting, false, true  },
        { kNormal_SkFontHinting, false, false },
        { kNormal_SkFontHinting, true,  false },
        { kNormal_SkFontHinting, false, true  },
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

    SkFont font;
    char const * const txt = "long.text.with.lots.of.dots.";
    size_t textLen = strlen(txt);

    for (size_t i = 0; i < SK_ARRAY_COUNT(faces); i++) {
        font.setTypeface(SkTypeface::MakeFromName(faces[i], SkFontStyle()));

        for (size_t j = 0; j  < SK_ARRAY_COUNT(settings); j++) {
            font.setHinting(settings[j].hinting);
            font.setLinearMetrics(settings[j].linear);
            font.setSubpixel(settings[j].subpixel);

            for (size_t k = 0; k < SK_ARRAY_COUNT(gScaleRec); ++k) {
                font.setScaleX(gScaleRec[k].fScaleX);
                font.setSkewX(gScaleRec[k].fSkewX);

                SkRect bounds;

                // For no hinting and light hinting this should take the
                // optimized generateAdvance path.
                SkScalar width1 = font.measureText(txt, textLen, SkTextEncoding::kUTF8);

                // Requesting the bounds forces a generateMetrics call.
                SkScalar width2 = font.measureText(txt, textLen, SkTextEncoding::kUTF8, &bounds);

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
