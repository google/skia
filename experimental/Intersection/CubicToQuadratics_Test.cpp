#include "CubicIntersection_TestData.h"
#include "CubicUtilities.h"
#include "Intersection_Tests.h"
#include "QuadraticIntersection_TestData.h"
#include "TestUtilities.h"
#include "SkGeometry.h"

static void test(const Cubic* cubics, const char* name, int firstTest, size_t testCount) {
    SkTDArray<Quadratic> quads;
    for (size_t index = firstTest; index < testCount; ++index) {
        const Cubic& cubic = cubics[index];
        double precision = calcPrecision(cubic);
        (void) cubic_to_quadratics(cubic, precision, quads);
        if (quads.count() != 1) {
            printf("%s [%d] cubic to quadratics failed count=%d\n", name, (int) index,
                    quads.count());
        }
    }
}

static void test(const Quadratic* quadTests, const char* name, int firstTest, size_t testCount) {
    SkTDArray<Quadratic> quads;
    for (size_t index = firstTest; index < testCount; ++index) {
        const Quadratic& quad = quadTests[index];
        Cubic cubic;
        quad_to_cubic(quad, cubic);
        double precision = calcPrecision(cubic);
        (void) cubic_to_quadratics(cubic, precision, quads);
        if (quads.count() != 1) {
            printf("%s [%d] cubic to quadratics failed count=%d\n", name, (int) index,
                    quads.count());
        }
    }
}

static void testC(const Cubic* cubics, const char* name, int firstTest, size_t testCount) {
    SkTDArray<Quadratic> quads;
    // test if computed line end points are valid
    for (size_t index = firstTest; index < testCount; ++index) {
        const Cubic& cubic = cubics[index];
        double precision = calcPrecision(cubic);
        int order = cubic_to_quadratics(cubic, precision, quads);
        SkASSERT(order != 4);
        if (order < 3) {
            continue;
        }
        if (!AlmostEqualUlps(cubic[0].x, quads[0][0].x)
                || !AlmostEqualUlps(cubic[0].y, quads[0][0].y)) {
            printf("[%d] unmatched start\n", (int) index);
        }
        int last = quads.count() - 1;
        if (!AlmostEqualUlps(cubic[3].x, quads[last][2].x)
                || !AlmostEqualUlps(cubic[3].y, quads[last][2].y)) {
            printf("[%d] unmatched end\n", (int) index);
        }
    }
}

static void testC(const Cubic(* cubics)[2], const char* name, int firstTest, size_t testCount) {
    SkTDArray<Quadratic> quads;
    for (size_t index = firstTest; index < testCount; ++index) {
        for (int idx2 = 0; idx2 < 2; ++idx2) {
            const Cubic& cubic = cubics[index][idx2];
            double precision = calcPrecision(cubic);
            int order = cubic_to_quadratics(cubic, precision, quads);
        SkASSERT(order != 4);
        if (order < 3) {
                continue;
            }
            if (!AlmostEqualUlps(cubic[0].x, quads[0][0].x)
                    || !AlmostEqualUlps(cubic[0].y, quads[0][0].y)) {
                printf("[%d][%d] unmatched start\n", (int) index, idx2);
            }
            int last = quads.count() - 1;
            if (!AlmostEqualUlps(cubic[3].x, quads[last][2].x)
                    || !AlmostEqualUlps(cubic[3].y, quads[last][2].y)) {
                printf("[%d][%d] unmatched end\n", (int) index, idx2);
            }
        }
    }
}

void CubicToQuadratics_Test() {
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
    int firstPointDegeneratesTest = run == RunAll ? 0 : run == RunPointDegenerates ? firstTestIndex : SK_MaxS32;
    int firstNotPointDegeneratesTest = run == RunAll ? 0 : run == RunNotPointDegenerates ? firstTestIndex : SK_MaxS32;
    int firstLinesTest = run == RunAll ? 0 : run == RunLines ? firstTestIndex : SK_MaxS32;
    int firstNotLinesTest = run == RunAll ? 0 : run == RunNotLines ? firstTestIndex : SK_MaxS32;
    int firstModEpsilonTest = run == RunAll ? 0 : run == RunModEpsilonLines ? firstTestIndex : SK_MaxS32;
    int firstLessEpsilonTest = run == RunAll ? 0 : run == RunLessEpsilonLines ? firstTestIndex : SK_MaxS32;
    int firstNegEpsilonTest = run == RunAll ? 0 : run == RunNegEpsilonLines ? firstTestIndex : SK_MaxS32;
    int firstQuadraticLineTest = run == RunAll ? 0 : run == RunQuadraticLines ? firstTestIndex : SK_MaxS32;
    int firstQuadraticModLineTest = run == RunAll ? 0 : run == RunQuadraticModLines ? firstTestIndex : SK_MaxS32;
    int firstComputedLinesTest = run == RunAll ? 0 : run == RunComputedLines ? firstTestIndex : SK_MaxS32;
    int firstComputedCubicsTest = run == RunAll ? 0 : run == RunComputedTests ? firstTestIndex : SK_MaxS32;

    test(pointDegenerates, "pointDegenerates", firstPointDegeneratesTest, pointDegenerates_count);
    test(notPointDegenerates, "notPointDegenerates", firstNotPointDegeneratesTest, notPointDegenerates_count);
    test(lines, "lines", firstLinesTest, lines_count);
    test(notLines, "notLines", firstNotLinesTest, notLines_count);
    test(modEpsilonLines, "modEpsilonLines", firstModEpsilonTest, modEpsilonLines_count);
    test(lessEpsilonLines, "lessEpsilonLines", firstLessEpsilonTest, lessEpsilonLines_count);
    test(negEpsilonLines, "negEpsilonLines", firstNegEpsilonTest, negEpsilonLines_count);
    test(quadraticLines, "quadraticLines", firstQuadraticLineTest, quadraticLines_count);
    test(quadraticModEpsilonLines, "quadraticModEpsilonLines", firstQuadraticModLineTest,
            quadraticModEpsilonLines_count);
    testC(lines, "computed lines", firstComputedLinesTest, lines_count);
    testC(tests, "computed tests", firstComputedCubicsTest, tests_count);
    printf("%s end\n", __FUNCTION__);
}

static Cubic locals[] = {
{{0, 1}, {1.9274705288631189e-19, 1.0000000000000002}, {0.0017190297609673323, 0.99828097023903239},
 {0.0053709083094631276, 0.99505672974365911}},

 {{14.5975863, 41.632436}, {16.3518929, 26.2639684}, {18.5165519, 7.68775139}, {8.03767257, 89.1628526}},
 {{69.7292014, 38.6877352}, {24.7648688, 23.1501713}, {84.9283191, 90.2588441}, {80.392774, 61.3533852}},
 {{
    60.776536520932126,
    71.249307306133829
  }, {
    87.107894191103014,
    22.377669868235323
  }, {
    1.4974754310666936,
    68.069569937917208
  }, {
    45.261946574441133,
    17.536076632112298
  }},
};

static size_t localsCount = sizeof(locals) / sizeof(locals[0]);

#define DEBUG_CRASH 0
#define TEST_AVERAGE_END_POINTS 0 // must take const off to test
extern const bool AVERAGE_END_POINTS;

static void oneOff(size_t x) {
    const Cubic& cubic = locals[x];
    const SkPoint skcubic[4] = {{(float) cubic[0].x, (float) cubic[0].y},
            {(float) cubic[1].x, (float) cubic[1].y}, {(float) cubic[2].x, (float) cubic[2].y},
            {(float) cubic[3].x, (float) cubic[3].y}};
    SkScalar skinflect[2];
    int skin = SkFindCubicInflections(skcubic, skinflect);
    SkDebugf("%s %d %1.9g\n", __FUNCTION__, skin, skinflect[0]);
    SkTDArray<Quadratic> quads;
    double precision = calcPrecision(cubic);
    (void) cubic_to_quadratics(cubic, precision, quads);
    SkDebugf("%s quads=%d\n", __FUNCTION__, quads.count());
}

void CubicsToQuadratics_OneOffTests() {
    for (size_t x = 0; x < localsCount; ++x) {
        oneOff(x);
    }
}

void CubicsToQuadratics_OneOffTest() {
    oneOff(0);
}

void CubicsToQuadratics_RandTest() {
    srand(0);
    const int arrayMax = 8;
    const int sampleMax = 10;
    const int tests = 1000000; // 10000000;
    int quadDist[arrayMax];
    bzero(quadDist, sizeof(quadDist));
    Cubic samples[arrayMax][sampleMax];
    int sampleCount[arrayMax];
    bzero(sampleCount, sizeof(sampleCount));
    for (int x = 0; x < tests; ++x) {
        Cubic cubic;
        for (int i = 0; i < 4; ++i) {
            cubic[i].x = (double) rand() / RAND_MAX * 100;
            cubic[i].y = (double) rand() / RAND_MAX * 100;
        }
    #if DEBUG_CRASH
        char str[1024];
        sprintf(str, "{{%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}},\n",
                cubic[0].x, cubic[0].y,  cubic[1].x, cubic[1].y, cubic[2].x, cubic[2].y,
                cubic[3].x, cubic[3].y);
    #endif
        SkTDArray<Quadratic> quads;
        double precision = calcPrecision(cubic);
        (void) cubic_to_quadratics(cubic, precision, quads);
        int count = quads.count();
        SkASSERT(count > 0);
        SkASSERT(--count < arrayMax);
        quadDist[count]++;
        int sCount = sampleCount[count];
        if (sCount < sampleMax) {
            memcpy(samples[count][sCount], cubic, sizeof(Cubic));
            sampleCount[count]++;
        }
    }
    for (int x = 0; x < arrayMax; ++x) {
        if (!quadDist[x]) {
            continue;
        }
        SkDebugf("%d %1.9g%%\n", x + 1, (double) quadDist[x] / tests * 100);
    }
    SkDebugf("\n");
    for (int x = 0; x < arrayMax; ++x) {
        for (int y = 0; y < sampleCount[x]; ++y) {
#if TEST_AVERAGE_END_POINTS
            for (int w = 0; w < 2; ++w) {
                AVERAGE_END_POINTS = w;
#else
                int w = 0;
#endif
                SkDebugf("<div id=\"cubic%dx%d%s\">\n", x + 1, y, w ? "x" : "");
                const Cubic& cubic = samples[x][y];
                SkDebugf("{{%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}},\n",
                    cubic[0].x, cubic[0].y,  cubic[1].x, cubic[1].y, cubic[2].x, cubic[2].y,
                    cubic[3].x, cubic[3].y);
                SkTDArray<Quadratic> quads;
                double precision = calcPrecision(cubic);
                (void) cubic_to_quadratics(cubic, precision, quads);
                for (int z = 0; z < quads.count(); ++z) {
                    const Quadratic& quad = quads[z];
                    SkDebugf("{{%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}},\n",
                        quad[0].x, quad[0].y,  quad[1].x, quad[1].y, quad[2].x, quad[2].y);
                }
                SkDebugf("</div>\n\n");
#if TEST_AVERAGE_END_POINTS
            }
#endif
        }
    }
    SkDebugf("</div>\n\n");
    SkDebugf("<script type=\"text/javascript\">\n\n");
    SkDebugf("var testDivs = [\n");
    for (int x = 0; x < arrayMax; ++x) {
        for (int y = 0; y < sampleCount[x]; ++y) {
#if TEST_AVERAGE_END_POINTS
            for (int w = 0; w < 2; ++w) {
#else
            int w = 0;
#endif
                SkDebugf("    cubic%dx%d%s,\n", x + 1, y, w ? "x" : "");
#if TEST_AVERAGE_END_POINTS
            }
#endif
        }
    }
    SkDebugf("\n\n\n");
    SkDebugf("%s end\n", __FUNCTION__);
}
