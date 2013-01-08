#include "CubicIntersection_TestData.h"
#include "CubicUtilities.h"
#include "Intersection_Tests.h"
#include "QuadraticIntersection_TestData.h"
#include "TestUtilities.h"

static double calcPrecision(const Cubic& cubic) {
    _Rect dRect;
    dRect.setBounds(cubic);
    double width = dRect.right - dRect.left;
    double height = dRect.bottom - dRect.top;
    return (width > height ? width : height) / 256;
}

static void test(const Cubic(& cubics)[], const char* name, int firstTest, size_t testCount) {
    SkTDArray<Quadratic> quads;
    for (size_t index = firstTest; index < testCount; ++index) {
        const Cubic& cubic = cubics[index];
        double precision = calcPrecision(cubic);
        cubic_to_quadratics(cubic, precision, quads);
        if (quads.count() != 1) {
            printf("%s [%d] cubic to quadratics failed count=%d\n", name, (int) index,
                    quads.count());
        }
    }
}

static void test(const Quadratic(& quadTests)[], const char* name, int firstTest, size_t testCount) {
    SkTDArray<Quadratic> quads;
    for (size_t index = firstTest; index < testCount; ++index) {
        const Quadratic& quad = quadTests[index];
        Cubic cubic;
        quad_to_cubic(quad, cubic);
        double precision = calcPrecision(cubic);
        cubic_to_quadratics(cubic, precision, quads);
        if (quads.count() != 1) {
            printf("%s [%d] cubic to quadratics failed count=%d\n", name, (int) index,
                    quads.count());
        }
    }
}

static void testC(const Cubic(& cubics)[], const char* name, int firstTest, size_t testCount) {
    SkTDArray<Quadratic> quads;
    // test if computed line end points are valid
    for (size_t index = firstTest; index < testCount; ++index) {
        const Cubic& cubic = cubics[index];
        double precision = calcPrecision(cubic);
        int order = cubic_to_quadratics(cubic, precision, quads);
        assert(order != 4);
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

static void testC(const Cubic(& cubics)[][2], const char* name, int firstTest, size_t testCount) {
    SkTDArray<Quadratic> quads;
    for (size_t index = firstTest; index < testCount; ++index) {
        for (int idx2 = 0; idx2 < 2; ++idx2) {
            const Cubic& cubic = cubics[index][idx2];
            double precision = calcPrecision(cubic);
            int order = cubic_to_quadratics(cubic, precision, quads);
            assert(order != 4);
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
    int firstPointDegeneratesTest = run == RunAll ? 0 : run == RunPointDegenerates ? firstTestIndex : INT_MAX;
    int firstNotPointDegeneratesTest = run == RunAll ? 0 : run == RunNotPointDegenerates ? firstTestIndex : INT_MAX;
    int firstLinesTest = run == RunAll ? 0 : run == RunLines ? firstTestIndex : INT_MAX;
    int firstNotLinesTest = run == RunAll ? 0 : run == RunNotLines ? firstTestIndex : INT_MAX;
    int firstModEpsilonTest = run == RunAll ? 0 : run == RunModEpsilonLines ? firstTestIndex : INT_MAX;
    int firstLessEpsilonTest = run == RunAll ? 0 : run == RunLessEpsilonLines ? firstTestIndex : INT_MAX;
    int firstNegEpsilonTest = run == RunAll ? 0 : run == RunNegEpsilonLines ? firstTestIndex : INT_MAX;
    int firstQuadraticLineTest = run == RunAll ? 0 : run == RunQuadraticLines ? firstTestIndex : INT_MAX;
    int firstQuadraticModLineTest = run == RunAll ? 0 : run == RunQuadraticModLines ? firstTestIndex : INT_MAX;
    int firstComputedLinesTest = run == RunAll ? 0 : run == RunComputedLines ? firstTestIndex : INT_MAX;
    int firstComputedCubicsTest = run == RunAll ? 0 : run == RunComputedTests ? firstTestIndex : INT_MAX;

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
  }}
};

static size_t localsCount = sizeof(locals) / sizeof(locals[0]);

void CubicsToQuadratics_RandTest() {
    for (size_t x = 0; x < localsCount; ++x) {
        const Cubic& cubic = locals[x];
        SkTDArray<Quadratic> quads;
        double precision = calcPrecision(cubic);
        cubic_to_quadratics(cubic, precision, quads);
    }
    srand(0);
    const int arrayMax = 1000;
    const int tests = 1000000;
    int quadDist[arrayMax];
    bzero(quadDist, sizeof(quadDist));
    for (int x = 0; x < tests; ++x) {
        Cubic cubic;
        for (int i = 0; i < 4; ++i) {
            cubic[i].x = (double) rand() / RAND_MAX * 100;
            cubic[i].y = (double) rand() / RAND_MAX * 100;
        }
        SkTDArray<Quadratic> quads;
        double precision = calcPrecision(cubic);
        cubic_to_quadratics(cubic, precision, quads);
        assert(quads.count() < arrayMax);
        quadDist[quads.count()]++;
    }
    for (int x = 0; x < arrayMax; ++x) {
        if (!quadDist[x]) {
            continue;
        }
        printf("%d 1.9%g%%\n", x, (double) quadDist[x] / tests * 100);
    }
}
