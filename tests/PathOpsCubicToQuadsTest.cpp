/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsCubicIntersectionTestData.h"
#include "PathOpsQuadIntersectionTestData.h"
#include "PathOpsTestCommon.h"
#include "SkGeometry.h"
#include "SkIntersections.h"
#include "SkPathOpsRect.h"
#include "SkReduceOrder.h"
#include "Test.h"

static void test(skiatest::Reporter* reporter, const SkDCubic* cubics, const char* name,
                 int firstTest, size_t testCount) {
    for (size_t index = firstTest; index < testCount; ++index) {
        const SkDCubic& cubic = cubics[index];
        SkASSERT(ValidCubic(cubic));
        double precision = cubic.calcPrecision();
        SkTArray<SkDQuad, true> quads;
        CubicToQuads(cubic, precision, quads);
        if (quads.count() != 1 && quads.count() != 2) {
            SkDebugf("%s [%d] cubic to quadratics failed count=%d\n", name, static_cast<int>(index),
                    quads.count());
        }
        REPORTER_ASSERT(reporter, quads.count() == 1);
    }
}

static void test(skiatest::Reporter* reporter, const SkDQuad* quadTests, const char* name,
                 int firstTest, size_t testCount) {
    for (size_t index = firstTest; index < testCount; ++index) {
        const SkDQuad& quad = quadTests[index];
        SkASSERT(ValidQuad(quad));
        SkDCubic cubic = quad.toCubic();
        double precision = cubic.calcPrecision();
        SkTArray<SkDQuad, true> quads;
        CubicToQuads(cubic, precision, quads);
        if (quads.count() != 1 && quads.count() != 2) {
            SkDebugf("%s [%d] cubic to quadratics failed count=%d\n", name, static_cast<int>(index),
                    quads.count());
        }
        REPORTER_ASSERT(reporter, quads.count() <= 2);
    }
}

static void testC(skiatest::Reporter* reporter, const SkDCubic* cubics, const char* name,
                  int firstTest, size_t testCount) {
    // test if computed line end points are valid
    for (size_t index = firstTest; index < testCount; ++index) {
        const SkDCubic& cubic = cubics[index];
        SkASSERT(ValidCubic(cubic));
        double precision = cubic.calcPrecision();
        SkTArray<SkDQuad, true> quads;
        CubicToQuads(cubic, precision, quads);
        if (!AlmostEqualUlps(cubic[0].fX, quads[0][0].fX)
                || !AlmostEqualUlps(cubic[0].fY, quads[0][0].fY)) {
            SkDebugf("[%d] unmatched start\n", static_cast<int>(index));
            REPORTER_ASSERT(reporter, 0);
        }
        int last = quads.count() - 1;
        if (!AlmostEqualUlps(cubic[3].fX, quads[last][2].fX)
                || !AlmostEqualUlps(cubic[3].fY, quads[last][2].fY)) {
            SkDebugf("[%d] unmatched end\n", static_cast<int>(index));
            REPORTER_ASSERT(reporter, 0);
        }
    }
}

static void testC(skiatest::Reporter* reporter, const SkDCubic(* cubics)[2], const char* name,
                  int firstTest, size_t testCount) {
    for (size_t index = firstTest; index < testCount; ++index) {
        for (int idx2 = 0; idx2 < 2; ++idx2) {
            const SkDCubic& cubic = cubics[index][idx2];
            SkASSERT(ValidCubic(cubic));
            double precision = cubic.calcPrecision();
            SkTArray<SkDQuad, true> quads;
            CubicToQuads(cubic, precision, quads);
            if (!AlmostEqualUlps(cubic[0].fX, quads[0][0].fX)
                    || !AlmostEqualUlps(cubic[0].fY, quads[0][0].fY)) {
                SkDebugf("[%d][%d] unmatched start\n", static_cast<int>(index), idx2);
                REPORTER_ASSERT(reporter, 0);
            }
            int last = quads.count() - 1;
            if (!AlmostEqualUlps(cubic[3].fX, quads[last][2].fX)
                    || !AlmostEqualUlps(cubic[3].fY, quads[last][2].fY)) {
                SkDebugf("[%d][%d] unmatched end\n", static_cast<int>(index), idx2);
                REPORTER_ASSERT(reporter, 0);
            }
        }
    }
}

DEF_TEST(CubicToQuads, reporter) {
    enum {
        RunAll,
        RunPointDegenerates,
        RunNotPointDegenerates,
        RunLines,
        RunNotLines,
        RunModEpsilonLines,
        RunLessEpsilonLines,
        RunNegEpsilonLines,
        RunQuadraticLines,
        RunQuadraticModLines,
        RunComputedLines,
        RunComputedTests,
        RunNone
    } run = RunAll;
    int firstTestIndex = 0;
#if 0
    run = RunComputedLines;
    firstTestIndex = 18;
#endif
    int firstPointDegeneratesTest = run == RunAll ? 0 : run == RunPointDegenerates
            ? firstTestIndex : SK_MaxS32;
    int firstNotPointDegeneratesTest = run == RunAll ? 0 : run == RunNotPointDegenerates
            ? firstTestIndex : SK_MaxS32;
    int firstLinesTest = run == RunAll ? 0 : run == RunLines ? firstTestIndex : SK_MaxS32;
    int firstNotLinesTest = run == RunAll ? 0 : run == RunNotLines ? firstTestIndex : SK_MaxS32;
    int firstModEpsilonTest = run == RunAll ? 0 : run == RunModEpsilonLines
            ? firstTestIndex : SK_MaxS32;
    int firstLessEpsilonTest = run == RunAll ? 0 : run == RunLessEpsilonLines
            ? firstTestIndex : SK_MaxS32;
    int firstNegEpsilonTest = run == RunAll ? 0 : run == RunNegEpsilonLines
            ? firstTestIndex : SK_MaxS32;
    int firstQuadraticLineTest = run == RunAll ? 0 : run == RunQuadraticLines
            ? firstTestIndex : SK_MaxS32;
    int firstQuadraticModLineTest = run == RunAll ? 0 : run == RunQuadraticModLines
            ? firstTestIndex : SK_MaxS32;
    int firstComputedLinesTest = run == RunAll ? 0 : run == RunComputedLines
            ? firstTestIndex : SK_MaxS32;
    int firstComputedCubicsTest = run == RunAll ? 0 : run == RunComputedTests
            ? firstTestIndex : SK_MaxS32;

    test(reporter, pointDegenerates, "pointDegenerates", firstPointDegeneratesTest,
            pointDegenerates_count);
    testC(reporter, notPointDegenerates, "notPointDegenerates", firstNotPointDegeneratesTest,
            notPointDegenerates_count);
    test(reporter, lines, "lines", firstLinesTest, lines_count);
    testC(reporter, notLines, "notLines", firstNotLinesTest, notLines_count);
    testC(reporter, modEpsilonLines, "modEpsilonLines", firstModEpsilonTest, modEpsilonLines_count);
    test(reporter, lessEpsilonLines, "lessEpsilonLines", firstLessEpsilonTest,
            lessEpsilonLines_count);
    test(reporter, negEpsilonLines, "negEpsilonLines", firstNegEpsilonTest, negEpsilonLines_count);
    test(reporter, quadraticLines, "quadraticLines", firstQuadraticLineTest, quadraticLines_count);
    test(reporter, quadraticModEpsilonLines, "quadraticModEpsilonLines", firstQuadraticModLineTest,
            quadraticModEpsilonLines_count);
    testC(reporter, lines, "computed lines", firstComputedLinesTest, lines_count);
    testC(reporter, tests, "computed tests", firstComputedCubicsTest, tests_count);
}

static SkDCubic locals[] = {
    {{{0, 1}, {1.9274705288631189e-19, 1.0000000000000002},
            {0.0017190297609673323, 0.99828097023903239},
            {0.0053709083094631276, 0.99505672974365911}}},
    {{{14.5975863, 41.632436}, {16.3518929, 26.2639684}, {18.5165519, 7.68775139},
            {8.03767257, 89.1628526}}},
    {{{69.7292014, 38.6877352}, {24.7648688, 23.1501713}, {84.9283191, 90.2588441},
            {80.392774, 61.3533852}}},
    {{{60.776536520932126, 71.249307306133829}, {87.107894191103014, 22.377669868235323},
            {1.4974754310666936, 68.069569937917208}, {45.261946574441133, 17.536076632112298}}},
};

static size_t localsCount = SK_ARRAY_COUNT(locals);

#define DEBUG_CRASH 0
#define TEST_AVERAGE_END_POINTS 0  // must take const off to test
extern const bool AVERAGE_END_POINTS;

static void oneOff(skiatest::Reporter* reporter, size_t x) {
    const SkDCubic& cubic = locals[x];
    SkASSERT(ValidCubic(cubic));
    const SkPoint skcubic[4] = {
            {static_cast<float>(cubic[0].fX), static_cast<float>(cubic[0].fY)},
            {static_cast<float>(cubic[1].fX), static_cast<float>(cubic[1].fY)},
            {static_cast<float>(cubic[2].fX), static_cast<float>(cubic[2].fY)},
            {static_cast<float>(cubic[3].fX), static_cast<float>(cubic[3].fY)}};
    SkScalar skinflect[2];
    int skin = SkFindCubicInflections(skcubic, skinflect);
    if (false) SkDebugf("%s %d %1.9g\n", __FUNCTION__, skin, skinflect[0]);
    SkTArray<SkDQuad, true> quads;
    double precision = cubic.calcPrecision();
    CubicToQuads(cubic, precision, quads);
    if (false) SkDebugf("%s quads=%d\n", __FUNCTION__, quads.count());
}

DEF_TEST(CubicsToQuadratics_OneOff_Loop, reporter) {
    for (size_t x = 0; x < localsCount; ++x) {
        oneOff(reporter, x);
    }
}

DEF_TEST(CubicsToQuadratics_OneOff_Single, reporter) {
    oneOff(reporter, 0);
}
