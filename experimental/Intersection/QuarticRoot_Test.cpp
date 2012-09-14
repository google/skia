#include <assert.h>
#include <math.h>
#include "CubicUtilities.h"
#include "Intersection_Tests.h"

namespace QuarticRootTest {

#include "QuarticRoot.cpp"

}

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

static void quadraticTest() {
    // (x - a)(x - b) == x^2 - (a + b)x + ab
    for (size_t aIndex = 0; aIndex < mulACount; ++aIndex) {
        for (size_t bIndex = 0; bIndex < rootBCount; ++bIndex) {
            for (size_t cIndex = 0; cIndex < rootCCount; ++cIndex) {
                const double A = mulA[aIndex];
                const double B = rootB[bIndex];
                const double C = rootC[cIndex];
                const double b = A * (B + C);
                const double c = A * B * C;
                double roots[2];
                const int rootCount = QuarticRootTest::quadraticRootsX(A, b, c, roots);
                const int expected = 1 + (B != C);
                assert(rootCount == expected);
                assert(approximately_equal(roots[0], -B)
                        || approximately_equal(roots[0], -C));
                if (B != C) {
                    assert(!approximately_equal(roots[0], roots[1]));
                    assert(approximately_equal(roots[1], -B)
                            || approximately_equal(roots[1], -C));
                }
            }
        }
    }
}

static void cubicTest() {
    // (x - a)(x - b)(x - c) == x^3 - (a + b + c)x^2 + (ab + bc + ac)x - abc
    for (size_t aIndex = 0; aIndex < mulACount; ++aIndex) {
        for (size_t bIndex = 0; bIndex < rootBCount; ++bIndex) {
            for (size_t cIndex = 0; cIndex < rootCCount; ++cIndex) {
                for (size_t dIndex = 0; dIndex < rootDCount; ++dIndex) {
                    const double A = mulA[aIndex];
                    const double B = rootB[bIndex];
                    const double C = rootC[cIndex];
                    const double D = rootD[dIndex];
                    const double b = A * (B + C + D);
                    const double c = A * (B * C + C * D + B * D);
                    const double d = A * B * C * D;
                    double roots[3];
                    const int rootCount = QuarticRootTest::cubicRootsX(A, b, c, d, roots);
                    const int expected = 1 + (B != C) + (B != D && C != D);
                    assert(rootCount == expected);
                    assert(approximately_equal(roots[0], -B)
                            || approximately_equal(roots[0], -C)
                            || approximately_equal(roots[0], -D));
                    if (expected > 1) {
                        assert(!approximately_equal(roots[0], roots[1]));
                        assert(approximately_equal(roots[1], -B)
                                || approximately_equal(roots[1], -C)
                                || approximately_equal(roots[1], -D));
                        if (expected > 2) {
                            assert(!approximately_equal(roots[0], roots[2])
                                    && !approximately_equal(roots[1], roots[2]));
                            assert(approximately_equal(roots[2], -B)
                                    || approximately_equal(roots[2], -C)
                                    || approximately_equal(roots[2], -D));
                        }
                    }
                }
            }
        }
    }
}

static void quarticTest() {
    // (x - a)(x - b)(x - c)(x - d) == x^4 - (a + b + c + d)x^3
    //   + (ab + bc + cd + ac + bd + cd)x^2 - (abc + bcd + abd + acd) * x + abcd
    for (size_t aIndex = 0; aIndex < mulACount; ++aIndex) {
        for (size_t bIndex = 0; bIndex < rootBCount; ++bIndex) {
            for (size_t cIndex = 0; cIndex < rootCCount; ++cIndex) {
                for (size_t dIndex = 0; dIndex < rootDCount; ++dIndex) {
                    for (size_t eIndex = 0; eIndex < rootECount; ++eIndex) {
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
                        const int rootCount = QuarticRootTest::quarticRoots(A, b, c, d, e, roots);
                        const int expected = 1 + (B != C) + (B != D && C != D) + (B != E && C != E && D != E);
                        assert(rootCount == expected);
                        assert(approximately_equal(roots[0], -B)
                                || approximately_equal(roots[0], -C)
                                || approximately_equal(roots[0], -D)
                                || approximately_equal(roots[0], -E));
                        if (expected > 1) {
                            assert(!approximately_equal(roots[0], roots[1]));
                            assert(approximately_equal(roots[1], -B)
                                    || approximately_equal(roots[1], -C)
                                    || approximately_equal(roots[1], -D)
                                    || approximately_equal(roots[1], -E));
                            if (expected > 2) {
                                assert(!approximately_equal(roots[0], roots[2])
                                        && !approximately_equal(roots[1], roots[2]));
                                assert(approximately_equal(roots[2], -B)
                                        || approximately_equal(roots[2], -C)
                                        || approximately_equal(roots[2], -D)
                                        || approximately_equal(roots[2], -E));
                                if (expected > 3) {
                                    assert(!approximately_equal(roots[0], roots[3])
                                            && !approximately_equal(roots[1], roots[3])
                                            && !approximately_equal(roots[2], roots[3]));
                                    assert(approximately_equal(roots[3], -B)
                                            || approximately_equal(roots[3], -C)
                                            || approximately_equal(roots[3], -D)
                                            || approximately_equal(roots[3], -E));
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void QuarticRoot_Test() {
    quadraticTest();
    cubicTest();
    quarticTest();
}
