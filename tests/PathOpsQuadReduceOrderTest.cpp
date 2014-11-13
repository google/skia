/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsQuadIntersectionTestData.h"
#include "SkIntersections.h"
#include "SkPathOpsRect.h"
#include "SkReduceOrder.h"
#include "Test.h"

static const SkDQuad testSet[] = {
    {{{1, 1}, {2, 2}, {1, 1.000003}}},
    {{{1, 0}, {2, 6}, {3, 0}}}
};

static const size_t testSetCount = SK_ARRAY_COUNT(testSet);

static void oneOffTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < testSetCount; ++index) {
        const SkDQuad& quad = testSet[index];
        SkReduceOrder reducer;
        SkDEBUGCODE(int result = ) reducer.reduce(quad);
        SkASSERT(result == 3);
    }
}

static void standardTestCases(skiatest::Reporter* reporter) {
    size_t index;
    SkReduceOrder reducer;
    int order;
    enum {
        RunAll,
        RunQuadraticLines,
        RunQuadraticModLines,
        RunNone
    } run = RunAll;
    int firstTestIndex = 0;
#if 0
    run = RunQuadraticLines;
    firstTestIndex = 1;
#endif
    int firstQuadraticLineTest = run == RunAll ? 0 : run == RunQuadraticLines ? firstTestIndex
            : SK_MaxS32;
    int firstQuadraticModLineTest = run == RunAll ? 0 : run == RunQuadraticModLines ? firstTestIndex
            : SK_MaxS32;

    for (index = firstQuadraticLineTest; index < quadraticLines_count; ++index) {
        const SkDQuad& quad = quadraticLines[index];
        order = reducer.reduce(quad);
        if (order != 2) {
            SkDebugf("[%d] line quad order=%d\n", (int) index, order);
        }
    }
    for (index = firstQuadraticModLineTest; index < quadraticModEpsilonLines_count; ++index) {
        const SkDQuad& quad = quadraticModEpsilonLines[index];
        order = reducer.reduce(quad);
        if (order != 2 && order != 3) {  // FIXME: data probably is not good
            SkDebugf("[%d] line mod quad order=%d\n", (int) index, order);
        }
    }
}

DEF_TEST(PathOpsReduceOrderQuad, reporter) {
    oneOffTest(reporter);
    standardTestCases(reporter);
}
