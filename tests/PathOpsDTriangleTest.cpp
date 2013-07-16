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

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsTriangleUtilitiesTest)
