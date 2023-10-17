/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkQuads.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/pathops/SkPathOpsQuad.h"
#include "tests/Test.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <string>

static void testQuadRootsReal(skiatest::Reporter* reporter, const std::string& name,
                               double A, double B, double C,
                               SkSpan<const double> expectedRoots) {
    skiatest::ReporterContext subtest(reporter, name);
    // Validate test case
    REPORTER_ASSERT(reporter, expectedRoots.size() <= 2,
                    "Invalid test case, up to 2 roots allowed");

    for (size_t i = 0; i < expectedRoots.size(); i++) {
        double x = expectedRoots[i];
        // A*x^2 + B*x + C should equal 0
        double y = A * x * x + B * x + C;
        REPORTER_ASSERT(reporter, sk_double_nearly_zero(y),
                    "Invalid test case root %zu. %.16f != 0", i, y);

        if (i > 0) {
            REPORTER_ASSERT(reporter, expectedRoots[i-1] <= expectedRoots[i],
                    "Invalid test case root %zu. Roots should be sorted in ascending order", i);
        }
    }

    {
        skiatest::ReporterContext subsubtest(reporter, "Pathops Implementation");
        double roots[2] = {0, 0};
        int rootCount = SkDQuad::RootsReal(A, B, C, roots);
        REPORTER_ASSERT(reporter, expectedRoots.size() == size_t(rootCount),
                        "Wrong number of roots returned %zu != %d", expectedRoots.size(),
                        rootCount);

        // We don't care which order the roots are returned from the algorithm.
        // For determinism, we will sort them (and ensure the provided solutions are also sorted).
        std::sort(std::begin(roots), std::begin(roots) + rootCount);
        for (int i = 0; i < rootCount; i++) {
            if (sk_double_nearly_zero(expectedRoots[i])) {
                REPORTER_ASSERT(reporter, sk_double_nearly_zero(roots[i]),
                                "0 != %.16f at index %d", roots[i], i);
            } else {
                REPORTER_ASSERT(reporter,
                                sk_doubles_nearly_equal_ulps(expectedRoots[i], roots[i], 64),
                                "%.16f != %.16f at index %d", expectedRoots[i], roots[i], i);
            }
        }
    }
    {
        skiatest::ReporterContext subsubtest(reporter, "SkQuads Implementation");
        double roots[2] = {0, 0};
        int rootCount = SkQuads::RootsReal(A, B, C, roots);
        REPORTER_ASSERT(reporter, expectedRoots.size() == size_t(rootCount),
                        "Wrong number of roots returned %zu != %d", expectedRoots.size(),
                        rootCount);

        // We don't care which order the roots are returned from the algorithm.
        // For determinism, we will sort them (and ensure the provided solutions are also sorted).
        std::sort(std::begin(roots), std::begin(roots) + rootCount);
        for (int i = 0; i < rootCount; i++) {
            if (sk_double_nearly_zero(expectedRoots[i])) {
                REPORTER_ASSERT(reporter, sk_double_nearly_zero(roots[i]),
                                "0 != %.16f at index %d", roots[i], i);
            } else {
                REPORTER_ASSERT(reporter,
                                sk_doubles_nearly_equal_ulps(expectedRoots[i], roots[i], 64),
                                "%.16f != %.16f at index %d", expectedRoots[i], roots[i], i);
            }
        }
    }
}

DEF_TEST(QuadRootsReal_ActualQuadratics, reporter) {
    // All answers are given with 16 significant digits (max for a double) or as an integer
    // when the answer is exact.
    testQuadRootsReal(reporter, "two roots 3x^2 - 20x - 40",
                       3, -20, -40,
                       {-1.610798991397109,
                      //-1.610798991397108632474265 from Wolfram Alpha
                         8.277465658063775,
                      // 8.277465658063775299140932 from Wolfram Alpha
                       });

    // (2x - 4)(x + 17)
    testQuadRootsReal(reporter, "two roots 2x^2 + 30x - 68",
                       2, 30, -68,
                       {-17, 2});

    testQuadRootsReal(reporter, "two roots x^2 - 5",
                       1, 0, -5,
                       {-2.236067977499790,
                      //-2.236067977499789696409174 from Wolfram Alpha
                         2.236067977499790,
                      // 2.236067977499789696409174 from Wolfram Alpha
                       });

    testQuadRootsReal(reporter, "one root x^2 - 2x + 1",
                       1, -2, 1,
                       {1});

    testQuadRootsReal(reporter, "no roots 5x^2 + 6x + 7",
                       5, 6, 7,
                       {});

    testQuadRootsReal(reporter, "no roots 4x^2 + 1",
                       4, 0, 1,
                       {});

    testQuadRootsReal(reporter, "one root is zero, another is big",
                       14, -13, 0,
                       {0,
                        0.9285714285714286
                      //0.9285714285714285714285714 from Wolfram Alpha
                        });

    // Values from a failing test case observed during testing.
    testQuadRootsReal(reporter, "one root is zero, another is small",
                       0.2929016490705016, 0.0000030451558069, 0,
                       {-0.00001039651301576329, 0});

    testQuadRootsReal(reporter, "b and c are zero, a is positive 4x^2",
                       4, 0, 0,
                       {0});

    testQuadRootsReal(reporter, "b and c are zero, a is negative -4x^2",
                       -4, 0, 0,
                       {0});

    testQuadRootsReal(reporter, "a and b are huge, c is zero",
                       4.3719914983870202e+291, 1.0269509510194551e+152, 0,
                       // One solution is 0, the other is so close to zero it returns
                       // true for sk_double_nearly_zero, so it is collapsed into one.
                       {0});

    testQuadRootsReal(reporter, "Very small A B, very large C",
                      0x1p-1055, 0x1.3000006p-1044, -0x1.c000008p+1009,
                      // The roots are not in the range of doubles.
                      {});
}

DEF_TEST(QuadRootsReal_Linear, reporter) {
    testQuadRootsReal(reporter, "positive slope 5x + 6",
                       0, 5, 6,
                       {-1.2});

    testQuadRootsReal(reporter, "negative slope -3x - 9",
                       0, -3, -9,
                       {-3.});
}

DEF_TEST(QuadRootsReal_Constant, reporter) {
    testQuadRootsReal(reporter, "No intersections y = -10",
                       0, 0, -10,
                       {});

    testQuadRootsReal(reporter, "Infinite solutions y = 0",
                       0, 0, 0,
                       {0.});
}

DEF_TEST(QuadRootsReal_NonFiniteNumbers, reporter) {
    // The Pathops implementation does not check for infinities nor nans in all cases.
    double roots[2];
    REPORTER_ASSERT(reporter,
        SkQuads::RootsReal(DBL_MAX, 0, DBL_MAX, roots) == 0,
        "Discriminant is negative infinity"
    );
    REPORTER_ASSERT(reporter,
        SkQuads::RootsReal(DBL_MAX, DBL_MAX, DBL_MAX, roots) == 0,
        "Double Overflow"
    );

    REPORTER_ASSERT(reporter,
        SkQuads::RootsReal(1, NAN, -3, roots) == 0,
        "Nan quadratic"
    );
    REPORTER_ASSERT(reporter,
        SkQuads::RootsReal(0, NAN, 3, roots) == 0,
        "Nan linear"
    );
    REPORTER_ASSERT(reporter,
        SkQuads::RootsReal(0, 0, NAN, roots) == 0,
        "Nan constant"
    );
}

// Test the discriminant using
// Use quadratics of the form F_n * x^2 - 2 * F_(n-1) * x + F_(n-2).
//   This has a discriminant of F_(n-1)^2 - F_n * F_(n-2) = 1 if n is even else -1.
DEF_TEST(QuadDiscriminant_Fibonacci, reporter) {
    //            n,  n-1, n-2
    int64_t F[] = {1,   1,   0};
    // F_79 just fits in the 53 significant bits of a double.
    for (int i = 2; i < 79; ++i) {
        F[0] = F[1] + F[2];

        const int expectedDiscriminant = i % 2 == 0 ? 1 : -1;
        REPORTER_ASSERT(reporter, SkQuads::Discriminant(F[0], F[1], F[2]) == expectedDiscriminant);

        F[2] = F[1];
        F[1] = F[0];
    }
}

DEF_TEST(QuadRoots_Basic, reporter) {
    {
        // (x - 1) (x - 1) normal quadratic form A = 1, B = 2, C =1.
        auto [discriminant, r0, r1] = SkQuads::Roots(1, -0.5 * -2, 1);
        REPORTER_ASSERT(reporter, discriminant == 0);
        REPORTER_ASSERT(reporter, r0 == 1 && r1 == 1);
    }

    {
        // (x + 2) (x + 2) normal quadratic form A = 1, B = 4, C = 4.
        auto [discriminant, r0, r1] = SkQuads::Roots(1, -0.5 * 4, 4);
        REPORTER_ASSERT(reporter, discriminant == 0);
        REPORTER_ASSERT(reporter, r0 == -2 && r1 == -2);
    }
}

// Test the roots using
// Use quadratics of the form F_n * x^2 - 2 * F_(n-1) * x + F_(n-2).
// The roots are (F_(n–1) ± 1)/F_n if n is even otherwise there are no roots.
DEF_TEST(QuadRoots_Fibonacci, reporter) {
    //            n,  n-1, n-2
    int64_t F[] = {1,   1,   0};
    // F_79 just fits in the 53 significant bits of a double.
    for (int i = 2; i < 79; ++i) {
        F[0] = F[1] + F[2];

        const int expectedDiscriminant = i % 2 == 0 ? 1 : -1;
        auto [discriminant, r0, r1] = SkQuads::Roots(F[0], F[1], F[2]);
        REPORTER_ASSERT(reporter, discriminant == expectedDiscriminant);

        // There are only real roots when i is even.
        if (i % 2 == 0) {
        const double expectedLittle = ((double)F[1] - 1) / F[0];
        const double expectedBig = ((double)F[1] + 1) / F[0];
            if (r0 <= r1) {
                REPORTER_ASSERT(reporter, r0 == expectedLittle);
                REPORTER_ASSERT(reporter, r1 == expectedBig);
            } else {
                REPORTER_ASSERT(reporter, r1 == expectedLittle);
                REPORTER_ASSERT(reporter, r0 == expectedBig);
            }
        } else {
            REPORTER_ASSERT(reporter, std::isnan(r0));
            REPORTER_ASSERT(reporter, std::isnan(r1));
        }

        F[2] = F[1];
        F[1] = F[0];
    }
}

// These are test cases used in the paper "The Ins and Outs of Solving Quadratic Equations with
// Floating-Point Arithmetic" located at
// https://github.com/goualard-f/QuadraticEquation.jl/blob/main/test/tests.jl

struct TestCase {
    const double A;
    const double B;
    const double C;
    const double answerLo;
    const double answerHi;
};

DEF_TEST(QuadRoots_Hard, reporter) {
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();

    auto specialEqual = [] (double actual, double test) {
        if (std::isnan(actual)) {
            return std::isnan(test);
        }

        if (std::isinf(actual)) {
            return std::isinf(test);
        }

        // Comparison function from the paper "The Ins and Outs ...."
        const double errorFactor = std::sqrt(std::numeric_limits<double>::epsilon());
        return std::abs(test - actual) <= errorFactor * std::max(std::abs(test), std::abs(actual));
    };

    auto p2 = [](double a) {
        return std::exp2(a);
    };

    TestCase cases[] = {
        // no real solutions
        {2, 0, 3, nan, nan},
        {1, 1, 1, nan, nan},
        {2.0 * p2(600), 0, 2.0 * p2(600), nan, nan},
        {-2.0 * p2(600), 0, -2.0 * p2(600), nan, nan},

        // degenerate cases
        {0, 0, 0, infinity, infinity},
        {0, 1, 0, 0, 0},
        {0, 1, 2, -2, -2},
        {0, 3, 4, -4.0/3.0, -4.0/3.0},
        {0, p2(600), -p2(600), 1, 1},
        {0, p2(600), p2(600), -1, -1},
        {0, p2(-600), p2(600), -infinity, -infinity},
        {0, p2(600), p2(-600), 0, 0},
        {0, 2, -1.0e-323, 5.0e-324, 5.0e-324},
        {3, 0, 0, 0, 0},
        {p2(600), 0, 0, 0, 0},
        {2, 0, -3, -sqrt(3.0/2.0), sqrt(3.0/2.0)},
        // {p2(600), 0, -p2(600), -1, 1}, determinant is infinity
        {3, 2, 0, -2.0/3.0, 0},
        // {p2(600), p2(700), 0, -p2(100), 0},
        {p2(-600), p2(700), 0, -infinity, 0},
        {p2(600), p2(-700), 0, 0, 0},

        // two solutions tests
        {1, -1, -1, -0.6180339887498948, 1.618033988749895},
        {1, 1 + p2(-52), 0.25 + p2(-53), (-1 - p2(-51)) / 2.0, -0.5},
        {1, p2(-511) + p2(-563), std::exp2(-1024), -7.458340888372987e-155,-7.458340574027429e-155},
        {1, p2(27), 0.75, -134217728.0, -5.587935447692871e-09},
        {1, -1e9, 1, 1e-09, 1000000000.0},
        // {1.3407807929942596e154, -1.3407807929942596e154, -1.3407807929942596e154, -0.6180339887498948, 1.618033988749895},
        {p2(600), 0.5, -p2(-600), -3.086568504549085e-181, 1.8816085719976428e-181},
        // {p2(600), 0.5, -p2(600), -1.0, 1.0},
        // {8.0, p2(800),-p2(500), -8.335018041099818e+239, 4.909093465297727e-91},
        {1, p2(26), -0.125, -67108864.0, 1.862645149230957e-09},
        // {p2(-1073), -p2(-1073), -p2(-1073), -0.6180339887498948,1.618033988749895},
        {p2(600), -p2(-600), -p2(-600), -2.409919865102884e-181, 2.409919865102884e-181},

        // Tests in Nivergelt paper
        {-158114166017, 316227766017, -158113600000, 0.99999642020057874, 1},
        {-312499999999.0, 707106781186.0, -400000000000.0, 1.131369396027, 1.131372303775},
        {-67, 134, -65, 0.82722631488372798, 1.17277368511627202},
        {0.247260273973, 0.994520547945, -0.138627953316, -4.157030027041105, 0.1348693622211607},
        {1, -2300000, 2.0e11, 90518.994979145, 2209481.005020854},
        {1.5*p2(-1026), 0, -p2(1022), -1.4678102981723264e308, 1.4678102981723264e308},

        // one solution tests
        {1.5*p2(-1026), 0, -p2(1022), -1.4678102981723264e308, 1.4678102981723264e308},
    };

    for (auto testCase : cases) {
        double A = testCase.A,
               B = testCase.B,
               C = testCase.C,
               answerLo = testCase.answerLo,
               answerHi = testCase.answerHi;
        if (std::isfinite(answerLo) && std::isfinite(answerHi)) {
            SkASSERT(answerLo <= answerHi);
        }
        auto [discriminate, r0, r1] = SkQuads::Roots(A, -0.5*B, C);
        double rLo = std::min(r0, r1),
               rHi = std::max(r0, r1);
        REPORTER_ASSERT(reporter, specialEqual(rLo, answerLo));
        REPORTER_ASSERT(reporter, specialEqual(rHi, answerHi));
    }
}

