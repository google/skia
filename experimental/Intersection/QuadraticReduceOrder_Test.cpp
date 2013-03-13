/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "Intersection_Tests.h"
#include "QuadraticIntersection_TestData.h"
#include "TestUtilities.h"

static const Quadratic testSet[] = {
    {{1, 1}, {2, 2}, {1, 1.000003}},
    {{1, 0}, {2, 6}, {3, 0}}
};

static const size_t testSetCount = sizeof(testSet) / sizeof(testSet[0]);


static void oneOffTest() {
    SkDebugf("%s FLT_EPSILON=%1.9g\n", __FUNCTION__, FLT_EPSILON);
    for (size_t index = 0; index < testSetCount; ++index) {
        const Quadratic& quad = testSet[index];
        Quadratic reduce;
        SkDEBUGCODE(int result = ) reduceOrder(quad, reduce, kReduceOrder_TreatAsFill);
        SkASSERT(result == 3);
    }
}

static void standardTestCases() {
    size_t index;
    Quadratic reduce;
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
    int firstQuadraticLineTest = run == RunAll ? 0 : run == RunQuadraticLines ? firstTestIndex : SK_MaxS32;
    int firstQuadraticModLineTest = run == RunAll ? 0 : run == RunQuadraticModLines ? firstTestIndex : SK_MaxS32;

    for (index = firstQuadraticLineTest; index < quadraticLines_count; ++index) {
        const Quadratic& quad = quadraticLines[index];
        order = reduceOrder(quad, reduce, kReduceOrder_TreatAsFill);
        if (order != 2) {
            printf("[%d] line quad order=%d\n", (int) index, order);
        }
    }
    for (index = firstQuadraticModLineTest; index < quadraticModEpsilonLines_count; ++index) {
        const Quadratic& quad = quadraticModEpsilonLines[index];
        order = reduceOrder(quad, reduce, kReduceOrder_TreatAsFill);
        if (order != 3) {
            printf("[%d] line mod quad order=%d\n", (int) index, order);
        }
    }
}

void QuadraticReduceOrder_Test() {
    oneOffTest();
    standardTestCases();
}
