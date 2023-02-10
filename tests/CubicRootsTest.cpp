/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkCubics.h"
#include "src/base/SkUtils.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "tests/Test.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <string>

static void testCubicRootsReal(skiatest::Reporter* reporter, std::string name,
                               double A, double B, double C, double D,
                               SkSpan<const double> expectedRoots,
                               bool skipPathops = false,
                               bool skipRootValidation = false) {
    skiatest::ReporterContext subtest(reporter, name);
    // Validate test case
    REPORTER_ASSERT(reporter, expectedRoots.size() <= 3,
                    "Invalid test case, up to 3 roots allowed");

    for (size_t i = 0; i < expectedRoots.size(); i++) {
        double x = expectedRoots[i];
        // A*x^3 + B*x^2 + C*x + D should equal 0 (unless floating point error causes issues)
        double y = A * x * x * x + B * x * x + C * x + D;
        if (!skipRootValidation) {
            REPORTER_ASSERT(reporter, sk_double_nearly_zero(y),
                            "Invalid test case root %zu. %.16f != 0", i, y);
        }

        if (i > 0) {
            REPORTER_ASSERT(reporter, expectedRoots[i-1] <= expectedRoots[i],
                    "Invalid test case root %zu. Roots should be sorted in ascending order", i);
        }
    }

    // The old pathops implementation sometimes gives incorrect solutions. We can opt
    // our tests out of checking that older implementation if that causes issues.
    if (!skipPathops) {
        skiatest::ReporterContext subsubtest(reporter, "Pathops Implementation");
        double roots[3] = {0, 0, 0};
        int rootCount = SkDCubic::RootsReal(A, B, C, D, roots);
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
        skiatest::ReporterContext subsubtest(reporter, "SkCubics Analytic Implementation");
        double roots[3] = {0, 0, 0};
        int rootCount = SkCubics::RootsReal(A, B, C, D, roots);
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

DEF_TEST(CubicRootsReal_ActualCubics, reporter) {
    // All answers are given with 16 significant digits (max for a double) or as an integer
    // when the answer is exact.
    testCubicRootsReal(reporter, "one root 1x^3 + 2x^2 + 3x + 4",
                       1, 2, 3, 4,
                       {-1.650629191439388});
                      //-1.650629191439388218880801 from Wolfram Alpha

    // (3x-5)(6x-10)(x+4) = 18x^3 + 12x^2 - 190x + 200
    testCubicRootsReal(reporter, "touches y axis 18x^3 + 12x^2 - 190x + 200",
                       18, 12, -190, 200,
                       {-4.,
                         1.666666666666667, // 5/3
                       });

    testCubicRootsReal(reporter, "three roots 10x^3 - 20x^2 - 30x + 40",
                       10, -20, -30, 40,
                       {-1.561552812808830,
                      //-1.561552812808830274910705 from Wolfram Alpha
                         1.,
                         2.561552812808830,
                      // 2.561552812808830274910705 from Wolfram Alpha
                       });

    testCubicRootsReal(reporter, "three roots -10x^3 + 200x^2 + 300x - 400",
                       -10, 200, 300, -400,
                       {-2.179884793243323,
                      //-2.179884793243323422232630 from Wolfram Alpha
                         0.8607083693981839,
                      // 0.8607083693981838897123320 from Wolfram Alpha
                        21.31917642384514,
                      //21.31917642384513953252030 from Wolfram Alpha
                       });

    testCubicRootsReal(reporter, "one root -x^3 + 0x^2 + 5x - 7",
                       -1, 0, 5, -7,
                       {-2.747346540307211,
                      //-2.747346540307210849971436 from Wolfram Alpha
                       });

    testCubicRootsReal(reporter, "one root 2x^3 - 3x^2 + 0x + 3",
                       2, -3, 0, 3,
                       {-0.806443932358772,
                      //-0.8064439323587723772036250 from Wolfram Alpha
                       });

    testCubicRootsReal(reporter, "one root x^3 + 0x^2 + 0x - 9",
                       1, 0, 0, -9,
                       {2.080083823051904,
                      //2.0800838230519041145300568 from Wolfram Alpha
                       });

    testCubicRootsReal(reporter, "three roots 2x^3 - 3x^2 - 4x + 0",
                       2, -3, -4, 0,
                       {-0.8507810593582122,
                      //-0.8507810593582121716220544 from Wolfram Alpha
                        0.,
                        2.350781059358212
                      //2.350781059358212171622054 from Wolfram Alpha
                       });

    testCubicRootsReal(reporter, "R^2 and Q^3 are near zero",
                       -0.33790159225463867, -0.81997990608215332,
                       -0.66327774524688721, -0.17884063720703125,
                       {-0.7995944894729731});

    // The following three cases fallback to treating the cubic as a quadratic.
    // Otherwise, floating point error mangles the solutions near +- 1
    // This means we don't find all the roots, but usually we only care about roots
    // in the range [0, 1], so that is ok.
    testCubicRootsReal(reporter, "oss-fuzz:55625 Two roots near zero, one big root",
                       sk_bit_cast<double>(0xbf1a8de580000000), // -0.00010129655
                       sk_bit_cast<double>(0x4106c0c680000000), // 186392.8125
                       0.0,
                       sk_bit_cast<double>(0xc104c0ce80000000), // -170009.8125
                       { -0.9550418733785169, // Wolfram Alpha puts the root at X = 0.955042
                          0.9550418733785169, // (~2e7 error)
                         // 1.84007e9 is the other root, which we do not find.
                       },
                       true /* == skipPathops */, true /* == skipRootValidation */);

    testCubicRootsReal(reporter, "oss-fuzz:55625 Two roots near zero, one big root, near linear",
                       sk_bit_cast<double>(0x3c04040400000000), // -1.3563156-19
                       sk_bit_cast<double>(0x4106c0c680000000), // 186392.8125
                       0.0,
                       sk_bit_cast<double>(0xc104c0ce80000000), // -170009.8125
                       { -0.9550418733785169,
                          0.9550418733785169,
                         // 1.84007e9 is the other root, which we do not find.
                       },
                       true /* == skipPathops */);

    testCubicRootsReal(reporter, "oss-fuzz:55680 A nearly zero, C is zero",
                       sk_bit_cast<double>(0x3eb0000000000000), // 9.5367431640625000e-07
                       sk_bit_cast<double>(0x409278a560000000), // 1182.1614990234375
                       0.0,
                       sk_bit_cast<double>(0xc092706160000000), // -1180.0950927734375
                       { -0.9991256228290017,
                      // -0.9991256232316570469050229 according to Wolfram Alpha (~1e-09 error)
                          0.9991256228290017,
                       // 0.9991256224263463476403026 according to Wolfram Alpha (~1e-09 error)
                         // 1.239586176×10^9 is the other root, which we do not find.
                       },
                       true, true /* == skipRootValidation */);
}

DEF_TEST(CubicRootsReal_Quadratics, reporter) {
    testCubicRootsReal(reporter, "two roots -2x^2 + 3x + 4",
                       0, -2, 3, 4,
                       {-0.8507810593582122,
                      //-0.8507810593582121716220544 from Wolfram Alpha
                         2.350781059358212,
                      // 2.350781059358212171622054 from Wolfram Alpha
                       });

    testCubicRootsReal(reporter, "touches y axis -x^2 + 3x + 4",
                       0, -2, 3, 4,
                       {-0.8507810593582122,
                      //-0.8507810593582121716220544 from Wolfram Alpha
                         2.350781059358212,
                      // 2.350781059358212171622054 from Wolfram Alpha
                       });

    testCubicRootsReal(reporter, "no roots x^2 + 2x + 7",
                       0, 1, 2, 7,
                       {});

    // similar to oss-fuzz:55680
    testCubicRootsReal(reporter, "two roots one small one big (and ignored)",
                       0, -0.01, 200000000000000, -120000000000000,
                       { 0.6 },
                        true /* == skipPathops */);
}

DEF_TEST(CubicRootsReal_Linear, reporter) {
    testCubicRootsReal(reporter, "positive slope 3x + 4",
                       0, 0, 3, 4,
                       {-1.333333333333333});

    testCubicRootsReal(reporter, "negative slope -2x - 8",
                       0, 0, -2, -8,
                       {-4.});
}

DEF_TEST(CubicRootsReal_Constant, reporter) {
    testCubicRootsReal(reporter, "No intersections y = 4",
                       0, 0, 0, 4,
                       {});

    testCubicRootsReal(reporter, "Infinite solutions y = 0",
                       0, 0, 0, 0,
                       {0.});
}

DEF_TEST(CubicRootsReal_NonFiniteNumbers, reporter) {
    // The Pathops implementation does not check for infinities nor nans in all cases.
    double roots[3] = {0, 0, 0};
    REPORTER_ASSERT(reporter,
        SkCubics::RootsReal(NAN, 1, 2, 3, roots) == 0,
        "Nan A"
    );
    REPORTER_ASSERT(reporter,
        SkCubics::RootsReal(1, NAN, 2, 3, roots) == 0,
        "Nan B"
    );
    REPORTER_ASSERT(reporter,
        SkCubics::RootsReal(1, 2, NAN, 3, roots) == 0,
        "Nan C"
    );
    REPORTER_ASSERT(reporter,
        SkCubics::RootsReal(1, 2, 3, NAN, roots) == 0,
        "Nan D"
    );

    {
        skiatest::ReporterContext subtest(reporter, "oss-fuzz:55419 C and D are large");
        int numRoots = SkCubics::RootsReal(
                                           -2, 0,
                                           sk_bit_cast<double>(0xd5422020202020ff), //-5.074559e+102
                                           sk_bit_cast<double>(0x600fff202020ff20), // 5.362551e+154
                                           roots);
        REPORTER_ASSERT(reporter, numRoots == 0, "No finite roots expected, got %d", numRoots);
    }
    {
        skiatest::ReporterContext subtest(reporter, "oss-fuzz:55829 A is zero and B is NAN");
        int numRoots = SkCubics::RootsReal(
                                           0,
                                           sk_bit_cast<double>(0xffffffffffff2020), //-nan
                                           sk_bit_cast<double>(0x20202020202020ff), // 6.013470e-154
                                           sk_bit_cast<double>(0xff20202020202020), //-2.211661e+304
                                           roots);
        REPORTER_ASSERT(reporter, numRoots == 0, "No finite roots expected, got %d", numRoots);
    }
}

static void testCubicValidT(skiatest::Reporter* reporter, std::string name,
                            double A, double B, double C, double D,
                            SkSpan<const double> expectedRoots) {
    skiatest::ReporterContext subtest(reporter, name);
    // Validate test case
    REPORTER_ASSERT(reporter, expectedRoots.size() <= 3,
                    "Invalid test case, up to 3 roots allowed");

    for (size_t i = 0; i < expectedRoots.size(); i++) {
        double x = expectedRoots[i];
        REPORTER_ASSERT(reporter, x >= 0 && x <= 1,
                        "Invalid test case root %zu. Roots must be in [0, 1]", i);

        // A*x^3 + B*x^2 + C*x + D should equal 0
        double y = A * x * x * x + B * x * x + C * x + D;
        REPORTER_ASSERT(reporter, sk_double_nearly_zero(y),
                    "Invalid test case root %zu. %.16f != 0", i, y);

        if (i > 0) {
            REPORTER_ASSERT(reporter, expectedRoots[i-1] <= expectedRoots[i],
                    "Invalid test case root %zu. Roots should be sorted in ascending order", i);
        }
    }

    {
        skiatest::ReporterContext subsubtest(reporter, "Pathops Implementation");
        double roots[3] = {0, 0, 0};
        int rootCount = SkDCubic::RootsValidT(A, B, C, D, roots);
        REPORTER_ASSERT(reporter, expectedRoots.size() == size_t(rootCount),
                        "Wrong number of roots returned %zu != %d",
                        expectedRoots.size(), rootCount);

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
        skiatest::ReporterContext subsubtest(reporter, "SkCubics Analytic Implementation");
        double roots[3] = {0, 0, 0};
        int rootCount = SkCubics::RootsValidT(A, B, C, D, roots);
        REPORTER_ASSERT(reporter, expectedRoots.size() == size_t(rootCount),
                        "Wrong number of roots returned %zu != %d",
                        expectedRoots.size(), rootCount);

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
        skiatest::ReporterContext subsubtest(reporter, "SkCubics Binary Search Implementation");
        double roots[3] = {0, 0, 0};
        int rootCount = SkCubics::BinarySearchRootsValidT(A, B, C, D, roots);
        REPORTER_ASSERT(reporter, expectedRoots.size() == size_t(rootCount),
                        "Wrong number of roots returned %zu != %d", expectedRoots.size(),
                        rootCount);

        // We don't care which order the roots are returned from the algorithm.
        // For determinism, we will sort them (and ensure the provided solutions are also sorted).
        std::sort(std::begin(roots), std::begin(roots) + rootCount);
        for (int i = 0; i < rootCount; i++) {
            double delta = std::abs(roots[i] - expectedRoots[i]);
            REPORTER_ASSERT(reporter,
                            // Binary search is not absolutely accurate all the time, but
                            // it should be accurate enough reliably
                            delta < 0.000001,
                            "%.16f != %.16f at index %d", expectedRoots[i], roots[i], i);
        }
    }
}

DEF_TEST(CubicRootsValidT, reporter) {
    // All answers are given with 16 significant digits (max for a double) or as an integer
    // when the answer is exact.
    testCubicValidT(reporter, "three roots 24x^3 - 46x^2 + 29x - 6",
                    24, -46, 29, -6,
                    {0.5,
                     0.6666666666666667,
                     0.75});

    testCubicValidT(reporter, "three roots total, two in range 54x^3 - 117x^2 + 45x + 0",
                    54, -117, 45, 0,
                    {0.0,
                     0.5,
                     // 5/3 is the other root, but not in [0, 1]
                    });

    testCubicValidT(reporter, "one root = 1 10x^3 - 20x^2 - 30x + 40",
                    10, -20, -30, 40,
                    {1.0});

    testCubicValidT(reporter, "one root = 0 2x^3 - 3x^2 - 4x + 0",
                    2, -3, -4, 0,
                    {0.0});

    testCubicValidT(reporter, "three roots total, two in range -2x^3 - 3x^2 + 4x + 0",
                    -2, -3, 4, 0,
                    { 0.0,
                      0.8507810593582122,
                   // 0.8507810593582121716220544 from Wolfram Alpha
                    });

    // x(x-1) = x^2 - x
    testCubicValidT(reporter, "Two roots at exactly 0 and 1",
                    0, 1, -1, 0,
                    {0.0, 1.0});

    testCubicValidT(reporter, "Single point has one root",
                    0, 0, 0, 0,
                    {0.0});
}

DEF_TEST(CubicRootsValidT_ClampToZeroAndOne, reporter) {
    {
        // (x + 0.00001)(x - 1.00005), but the answers will be 0 and 1
        double A = 0.;
        double B = 1.;
        double C = -1.00004;
        double D = -0.0000100005;
        double roots[3] = {0, 0, 0};
        int rootCount = SkDCubic::RootsValidT(A, B, C, D, roots);

        REPORTER_ASSERT(reporter, rootCount == 2);
        std::sort(std::begin(roots), std::begin(roots) + rootCount);
        REPORTER_ASSERT(reporter, sk_double_nearly_zero(roots[0]), "%.16f != 0", roots[0]);
        REPORTER_ASSERT(reporter, sk_doubles_nearly_equal_ulps(roots[1], 1), "%.16f != 1", roots[1]);
    }
    {
        // Three very small roots, all of them are nearly equal zero
        // (1 - 10000000000x)(1 - 20000000000x)(1 - 30000000000x)
        // -6000000000000000000000000000000 x^3 + 1100000000000000000000 x^2 - 60000000000 x + 1
        double A = -6.0e30;
        double B = 1.1e21;
        double C = -6.0e10;
        double D = 1;
        double roots[3] = {0, 0, 0};
        int rootCount = SkDCubic::RootsValidT(A, B, C, D, roots);

        REPORTER_ASSERT(reporter, rootCount == 1);
        std::sort(std::begin(roots), std::begin(roots) + rootCount);
        REPORTER_ASSERT(reporter, sk_double_nearly_zero(roots[0]), "%.16f != 0", roots[0]);
    }
}
