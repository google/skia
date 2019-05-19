/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "src/utils/SkMetaData.h"
#include "tests/Test.h"

static void test_ptrs(skiatest::Reporter* reporter) {
    SkRefCnt ref;
    REPORTER_ASSERT(reporter, ref.unique());

    {
        SkMetaData md0, md1;
        const char name[] = "refcnt";

        md0.setRefCnt(name, &ref);
        REPORTER_ASSERT(reporter, md0.findRefCnt(name));
        REPORTER_ASSERT(reporter, md0.hasRefCnt(name, &ref));
        REPORTER_ASSERT(reporter, !ref.unique());

        md1 = md0;
        REPORTER_ASSERT(reporter, md1.findRefCnt(name));
        REPORTER_ASSERT(reporter, md1.hasRefCnt(name, &ref));
        REPORTER_ASSERT(reporter, !ref.unique());

        REPORTER_ASSERT(reporter, md0.removeRefCnt(name));
        REPORTER_ASSERT(reporter, !md0.findRefCnt(name));
        REPORTER_ASSERT(reporter, !md0.hasRefCnt(name, &ref));
        REPORTER_ASSERT(reporter, !ref.unique());
    }
    REPORTER_ASSERT(reporter, ref.unique());
}

DEF_TEST(MetaData, reporter) {
    SkMetaData  m1;

    REPORTER_ASSERT(reporter, !m1.findS32("int"));
    REPORTER_ASSERT(reporter, !m1.findScalar("scalar"));
    REPORTER_ASSERT(reporter, !m1.findString("hello"));
    REPORTER_ASSERT(reporter, !m1.removeS32("int"));
    REPORTER_ASSERT(reporter, !m1.removeScalar("scalar"));
    REPORTER_ASSERT(reporter, !m1.removeString("hello"));
    REPORTER_ASSERT(reporter, !m1.removeString("true"));
    REPORTER_ASSERT(reporter, !m1.removeString("false"));

    m1.setS32("int", 12345);
    m1.setScalar("scalar", SK_Scalar1 * 42);
    m1.setString("hello", "world");
    m1.setPtr("ptr", &m1);
    m1.setBool("true", true);
    m1.setBool("false", false);

    int32_t     n;
    SkScalar    s;

    m1.setScalar("scalar", SK_Scalar1/2);

    REPORTER_ASSERT(reporter, m1.findS32("int", &n) && n == 12345);
    REPORTER_ASSERT(reporter, m1.findScalar("scalar", &s) && s == SK_Scalar1/2);
    REPORTER_ASSERT(reporter, !strcmp(m1.findString("hello"), "world"));
    REPORTER_ASSERT(reporter, m1.hasBool("true", true));
    REPORTER_ASSERT(reporter, m1.hasBool("false", false));

    SkMetaData::Iter iter(m1);
    const char* name;

    static const struct {
        const char*         fName;
        SkMetaData::Type    fType;
        int                 fCount;
    } gElems[] = {
        { "int",    SkMetaData::kS32_Type,      1 },
        { "scalar", SkMetaData::kScalar_Type,   1 },
        { "ptr",    SkMetaData::kPtr_Type,      1 },
        { "hello",  SkMetaData::kString_Type,   sizeof("world") },
        { "true",   SkMetaData::kBool_Type,     1 },
        { "false",  SkMetaData::kBool_Type,     1 }
    };

    int                 loop = 0;
    int count;
    SkMetaData::Type    t;
    while ((name = iter.next(&t, &count)) != nullptr)
    {
        int match = 0;
        for (unsigned i = 0; i < SK_ARRAY_COUNT(gElems); i++)
        {
            if (!strcmp(name, gElems[i].fName))
            {
                match += 1;
                REPORTER_ASSERT(reporter, gElems[i].fType == t);
                REPORTER_ASSERT(reporter, gElems[i].fCount == count);
            }
        }
        REPORTER_ASSERT(reporter, match == 1);
        loop += 1;
    }
    REPORTER_ASSERT(reporter, loop == SK_ARRAY_COUNT(gElems));

    REPORTER_ASSERT(reporter, m1.removeS32("int"));
    REPORTER_ASSERT(reporter, m1.removeScalar("scalar"));
    REPORTER_ASSERT(reporter, m1.removeString("hello"));
    REPORTER_ASSERT(reporter, m1.removeBool("true"));
    REPORTER_ASSERT(reporter, m1.removeBool("false"));

    REPORTER_ASSERT(reporter, !m1.findS32("int"));
    REPORTER_ASSERT(reporter, !m1.findScalar("scalar"));
    REPORTER_ASSERT(reporter, !m1.findString("hello"));
    REPORTER_ASSERT(reporter, !m1.findBool("true"));
    REPORTER_ASSERT(reporter, !m1.findBool("false"));

    test_ptrs(reporter);
}
