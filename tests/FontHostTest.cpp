/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkPaint.h"
#include "SkTypeface.h"

//#define DUMP_TABLES

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

static void test_tables(skiatest::Reporter* reporter, SkTypeface* face) {
    SkFontID fontID = face->uniqueID();

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
        "Courier New",
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gNames); ++i) {
        SkTypeface* face = SkTypeface::CreateFromName(gNames[i],
                                                      SkTypeface::kNormal);
        if (face) {
#ifdef DUMP_TABLES
            SkDebugf("%s\n", gNames[i]);
#endif
            test_tables(reporter, face);
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

    SkPaint paint;
    char txt[] = "long.text.with.lots.of.dots.";

    for (size_t i = 0; i < SK_ARRAY_COUNT(faces); i++) {
        SkTypeface* face = SkTypeface::CreateFromName(faces[i], SkTypeface::kNormal);
        paint.setTypeface(face);

        for (size_t j = 0; j  < SK_ARRAY_COUNT(settings); j++) {
             paint.setHinting(settings[j].hinting);
             paint.setLinearText((settings[j].flags & SkPaint::kLinearText_Flag) != 0);
             paint.setSubpixelText((settings[j].flags & SkPaint::kSubpixelText_Flag) != 0);

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

static void TestFontHost(skiatest::Reporter* reporter) {
    test_tables(reporter);
    test_advances(reporter);
}

// need tests for SkStrSearch

#include "TestClassDef.h"
DEFINE_TESTCLASS("FontHost", FontHostTestClass, TestFontHost)
