#include "CubicUtilities.h"
#include "Intersection_Tests.h"
#include "QuadraticUtilities.h"
#include "QuarticRoot.h"

double mulA[] = {-3, -1, 1, 3};
size_t mulACount = sizeof(mulA) / sizeof(mulA[0]);
double rootB[] = {-9, -6, -3, -1, 0, 1, 3, 6, 9};
size_t rootBCount = sizeof(rootB) / sizeof(rootB[0]);
double rootC[] = {-8, -6, -2, -1, 0, 1, 2, 6, 8};
size_t rootCCount = sizeof(rootC) / sizeof(rootC[0]);
double rootD[] = {-7, -4, -1, 0, 1, 2, 5};
size_t rootDCount = sizeof(rootD) / sizeof(rootD[0]);
double rootE[] = {-5, -1, 0, 1, 7};
size_t rootECount = sizeof(rootE) / sizeof(rootE[0]);


static void quadraticTest(bool limit) {
    // (x - a)(x - b) == x^2 - (a + b)x + ab
    for (size_t aIndex = 0; aIndex < mulACount; ++aIndex) {
        for (size_t bIndex = 0; bIndex < rootBCount; ++bIndex) {
            for (size_t cIndex = 0; cIndex < rootCCount; ++cIndex) {
                const double A = mulA[aIndex];
                double B = rootB[bIndex];
                double C = rootC[cIndex];
                if (limit) {
                    B = (B - 6) / 12;
                    C = (C - 6) / 12;
                }
                const double b = A * (B + C);
                const double c = A * B * C;
                double roots[2];
                const int rootCount = limit ? quadraticRootsValidT(A, b, c, roots)
                    : quadraticRootsReal(A, b, c, roots);
                int expected;
                if (limit) {
                    expected = B <= 0 && B >= -1;
                    expected += B != C && C <= 0 && C >= -1;
                } else {
                    expected = 1 + (B != C);
                }
                SkASSERT(rootCount == expected);
                if (!rootCount) {
                    continue;
                }
                SkASSERT(approximately_equal(roots[0], -B)
                        || approximately_equal(roots[0], -C));
                if (expected > 1) {
                    SkASSERT(!approximately_equal(roots[0], roots[1]));
                    SkASSERT(approximately_equal(roots[1], -B)
                            || approximately_equal(roots[1], -C));
                }
            }
        }
    }
}

static void testOneCubic(bool limit, size_t aIndex, size_t bIndex, size_t cIndex, size_t dIndex) {
    const double A = mulA[aIndex];
    double B = rootB[bIndex];
    double C = rootC[cIndex];
    double D = rootD[dIndex];
    if (limit) {
        B = (B - 6) / 12;
        C = (C - 6) / 12;
        D = (C - 2) / 6;
    }
    const double b = A * (B + C + D);
    const double c = A * (B * C + C * D + B * D);
    const double d = A * B * C * D;
    double roots[3];
    const int rootCount = limit ? cubicRootsValidT(A, b, c, d, roots)
            : cubicRootsReal(A, b, c, d, roots);
    int expected;
    if (limit) {
        expected = B <= 0 && B >= -1;
        expected += B != C && C <= 0 && C >= -1;
        expected += B != D && C != D && D <= 0 && D >= -1;
    } else {
        expected = 1 + (B != C) + (B != D && C != D);
    }
    SkASSERT(rootCount == expected);
    if (!rootCount) {
        return;
    }
    SkASSERT(approximately_equal(roots[0], -B)
            || approximately_equal(roots[0], -C)
            || approximately_equal(roots[0], -D));
    if (expected <= 1) {
        return;
    }
    SkASSERT(!approximately_equal(roots[0], roots[1]));
    SkASSERT(approximately_equal(roots[1], -B)
            || approximately_equal(roots[1], -C)
            || approximately_equal(roots[1], -D));
    if (expected <= 2) {
        return;
    }
    SkASSERT(!approximately_equal(roots[0], roots[2])
            && !approximately_equal(roots[1], roots[2]));
    SkASSERT(approximately_equal(roots[2], -B)
            || approximately_equal(roots[2], -C)
            || approximately_equal(roots[2], -D));
}

static void cubicTest(bool limit) {
    // (x - a)(x - b)(x - c) == x^3 - (a + b + c)x^2 + (ab + bc + ac)x - abc
    for (size_t aIndex = 0; aIndex < mulACount; ++aIndex) {
        for (size_t bIndex = 0; bIndex < rootBCount; ++bIndex) {
            for (size_t cIndex = 0; cIndex < rootCCount; ++cIndex) {
                for (size_t dIndex = 0; dIndex < rootDCount; ++dIndex) {
                    testOneCubic(limit, aIndex, bIndex, cIndex, dIndex);
                }
            }
        }
    }
}

static void testOneQuartic(size_t aIndex, size_t bIndex, size_t cIndex, size_t dIndex,
        size_t eIndex) {
    const double A = mulA[aIndex];
    const double B = rootB[bIndex];
    const double C = rootC[cIndex];
    const double D = rootD[dIndex];
    const double E = rootE[eIndex];
    const double b = A * (B + C + D + E);
    const double c = A * (B * C + C * D + B * D + B * E + C * E + D * E);
    const double d = A * (B * C * D + B * C * E + B * D * E + C * D * E);
    const double e = A * B * C * D * E;
    double roots[4];
    bool oneHint = approximately_zero(A + b + c + d + e);
    int rootCount = reducedQuarticRoots(A, b, c, d, e, oneHint, roots);
    if (rootCount < 0) {
        rootCount = quarticRootsReal(0, A, b, c, d, e, roots);
    }
    const int expected = 1 + (B != C) + (B != D && C != D) + (B != E && C != E && D != E);
    SkASSERT(rootCount == expected);
    SkASSERT(AlmostEqualUlps(roots[0], -B)
            || AlmostEqualUlps(roots[0], -C)
            || AlmostEqualUlps(roots[0], -D)
            || AlmostEqualUlps(roots[0], -E));
    if (expected <= 1) {
        return;
    }
    SkASSERT(!AlmostEqualUlps(roots[0], roots[1]));
    SkASSERT(AlmostEqualUlps(roots[1], -B)
            || AlmostEqualUlps(roots[1], -C)
            || AlmostEqualUlps(roots[1], -D)
            || AlmostEqualUlps(roots[1], -E));
    if (expected <= 2) {
        return;
    }
    SkASSERT(!AlmostEqualUlps(roots[0], roots[2])
            && !AlmostEqualUlps(roots[1], roots[2]));
    SkASSERT(AlmostEqualUlps(roots[2], -B)
            || AlmostEqualUlps(roots[2], -C)
            || AlmostEqualUlps(roots[2], -D)
            || AlmostEqualUlps(roots[2], -E));
    if (expected <= 3) {
        return;
    }
    SkASSERT(!AlmostEqualUlps(roots[0], roots[3])
            && !AlmostEqualUlps(roots[1], roots[3])
            && !AlmostEqualUlps(roots[2], roots[3]));
    SkASSERT(AlmostEqualUlps(roots[3], -B)
            || AlmostEqualUlps(roots[3], -C)
            || AlmostEqualUlps(roots[3], -D)
            || AlmostEqualUlps(roots[3], -E));
}

static void quarticTest() {
    // (x - a)(x - b)(x - c)(x - d) == x^4 - (a + b + c + d)x^3
    //   + (ab + bc + cd + ac + bd + cd)x^2 - (abc + bcd + abd + acd) * x + abcd
    for (size_t aIndex = 0; aIndex < mulACount; ++aIndex) {
        for (size_t bIndex = 0; bIndex < rootBCount; ++bIndex) {
            for (size_t cIndex = 0; cIndex < rootCCount; ++cIndex) {
                for (size_t dIndex = 0; dIndex < rootDCount; ++dIndex) {
                    for (size_t eIndex = 0; eIndex < rootECount; ++eIndex) {
                        testOneQuartic(aIndex, bIndex, cIndex, dIndex, eIndex);
                    }
                }
            }
        }
    }
}

void QuarticRoot_Test() {
    testOneCubic(false, 0, 5, 5, 4);
    testOneQuartic(0, 0, 2, 4, 3);
    quadraticTest(true);
    quadraticTest(false);
    cubicTest(true);
    cubicTest(false);
    quarticTest();
}
