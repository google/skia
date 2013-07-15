/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkPaint.h"
#include "SkFontStream.h"
#include "SkStream.h"
#include "SkTypeface.h"
#include "SkEndian.h"

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
    {   kFontTableTag_maxp,         32 },
};

// Test that getUnitsPerEm() agrees with a direct lookup in the 'head' table
// (if that table is available).
static void test_unitsPerEm(skiatest::Reporter* reporter, SkTypeface* face) {
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
    } else {
        // not sure this is a bug, but lets report it for now as info.
        SkDebugf("--- typeface returned 0 upem [%X]\n", face->uniqueID());
    }
}

// Test that countGlyphs() agrees with a direct lookup in the 'maxp' table
// (if that table is available).
static void test_countGlyphs(skiatest::Reporter* reporter, SkTypeface* face) {
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
    } else {
        // not sure this is a bug, but lets report it for now as info.
        SkDebugf("--- typeface returned 0 glyphs [%X]\n", face->uniqueID());
    }
}

static void test_fontstream(skiatest::Reporter* reporter,
                            SkStream* stream, int ttcIndex) {
    int n = SkFontStream::GetTableTags(stream, ttcIndex, NULL);
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

static void test_fontstream(skiatest::Reporter* reporter, SkStream* stream) {
    int count = SkFontStream::CountTTCEntries(stream);
#ifdef DUMP_TTC_TABLES
    SkDebugf("CountTTCEntries %d\n", count);
#endif
    for (int i = 0; i < count; ++i) {
        test_fontstream(reporter, stream, i);
    }
}

static void test_fontstream(skiatest::Reporter* reporter) {
    // TODO: replace when we get a tools/resources/fonts/test.ttc
    const char* name = "/AmericanTypewriter.ttc";
    SkFILEStream stream(name);
    if (stream.isValid()) {
        test_fontstream(reporter, &stream);
    }
}

static void test_tables(skiatest::Reporter* reporter, SkTypeface* face) {
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
        NULL,   // default font
        "Arial", "Times", "Times New Roman", "Helvetica", "Courier",
        "Courier New", "Terminal", "MS Sans Serif",
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gNames); ++i) {
        SkTypeface* face = SkTypeface::CreateFromName(gNames[i],
                                                      SkTypeface::kNormal);
        if (face) {
#ifdef DUMP_TABLES
            SkDebugf("%s\n", gNames[i]);
#endif
            test_tables(reporter, face);
            test_unitsPerEm(reporter, face);
            test_countGlyphs(reporter, face);
            face->unref();
        }
    }
}

/*
 * Verifies that the advance values returned by generateAdvance and
 * generateMetrics match.
 */
static void test_advances(skiatest::Reporter* reporter) {
    static const char* const faces[] = {
        NULL,   // default font
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
        SkAutoTUnref<SkTypeface> face(SkTypeface::CreateFromName(faces[i], SkTypeface::kNormal));
        paint.setTypeface(face);

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

static void TestFontHost(skiatest::Reporter* reporter) {
    test_tables(reporter);
    test_fontstream(reporter);
    test_advances(reporter);
}

// need tests for SkStrSearch

#include "TestClassDef.h"
DEFINE_TESTCLASS("FontHost", FontHostTestClass, TestFontHost)
