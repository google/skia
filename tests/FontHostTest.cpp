/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkTypeface.h"
#include "SkFontHost.h"

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

    int count = SkFontHost::CountTables(fontID);

    SkAutoTMalloc<SkFontTableTag> storage(count);
    SkFontTableTag* tags = storage.get();
    SkFontHost::GetTableTags(fontID, tags);

    for (int i = 0; i < count; ++i) {
        size_t size = SkFontHost::GetTableSize(fontID, tags[i]);
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

static void TestFontHost(skiatest::Reporter* reporter) {
    test_tables(reporter);
}

// need tests for SkStrSearch

#include "TestClassDef.h"
DEFINE_TESTCLASS("FontHost", FontHostTestClass, TestFontHost)
