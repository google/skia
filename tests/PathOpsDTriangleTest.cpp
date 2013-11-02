/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkPathOpsTriangle.h"
#include "Test.h"

static const SkDTriangle tests[] = {
    {{{2, 0}, {3, 1}, {2, 2}}},
    {{{3, 1}, {2, 2}, {1, 1}}},
    {{{3, 0}, {2, 1}, {3, 2}}},
};

static const SkDPoint inPoint[] = {
    {2.5, 1},
    {2, 1.5},
    {2.5, 1},
};

static const SkDPoint outPoint[] = {
    {3, 0},
    {2.5, 2},
    {2.5, 2},
};

static const size_t tests_count = SK_ARRAY_COUNT(tests);

static void PathOpsTriangleUtilitiesTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < tests_count; ++index) {
        const SkDTriangle& triangle = tests[index];
        SkASSERT(ValidTriangle(triangle));
        bool result = triangle.contains(inPoint[index]);
        if (!result) {
            SkDebugf("%s [%d] expected point in triangle\n", __FUNCTION__, index);
            REPORTER_ASSERT(reporter, 0);
        }
        result = triangle.contains(outPoint[index]);
        if (result) {
            SkDebugf("%s [%d] expected point outside triangle\n", __FUNCTION__, index);
            REPORTER_ASSERT(reporter, 0);
        }
    }
}

static const SkDTriangle oneOff[] = {
    {{{271.03291625750461, 5.0402503630087025e-05}, {275.21652430019037, 3.6997300650817753},
      {279.25839233398438, 7.7416000366210938}}},

    {{{271.03291625750461, 5.0402503617874572e-05}, {275.21652430019037, 3.6997300650817877},
      {279.25839233398438, 7.7416000366210938}}}
};

static const size_t oneOff_count = SK_ARRAY_COUNT(oneOff);

static void PathOpsTriangleOneOffTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < oneOff_count; ++index) {
        const SkDTriangle& triangle = oneOff[index];
        SkASSERT(ValidTriangle(triangle));
        for (int inner = 0; inner < 3; ++inner) {
            bool result = triangle.contains(triangle.fPts[inner]);
            if (result) {
                SkDebugf("%s [%d][%d] point on triangle is not in\n", __FUNCTION__, index, inner);
                REPORTER_ASSERT(reporter, 0);
            }
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsTriangleUtilitiesTest)

DEFINE_TESTCLASS_SHORT(PathOpsTriangleOneOffTest)
