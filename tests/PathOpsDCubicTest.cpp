/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkSpan.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "src/pathops/SkPathOpsPoint.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

#include <string>
#include <cstddef>

static const CubicPts hullTests[] = {
{{{2.6250000819563866, 2.3750000223517418}, {2.833333432674408, 2.3333333432674408}, {3.1111112236976624, 2.3333333134651184}, {3.4074075222015381, 2.3333332538604736}}},
};

static const size_t hullTests_count = std::size(hullTests);

DEF_TEST(PathOpsCubicHull, reporter) {
    for (size_t index = 0; index < hullTests_count; ++index) {
        const CubicPts& c = hullTests[index];
        SkDCubic cubic;
        cubic.debugSet(c.fPts);
        char order[4];
        cubic.convexHull(order);
    }
}

static bool nearly_equal(double expected, double actual) {
    if (sk_double_nearly_zero(expected)) {
        return sk_double_nearly_zero(actual);
    }
    return sk_doubles_nearly_equal_ulps(expected, actual, 64);
}

static void testConvertToPolynomial(skiatest::Reporter* reporter, const std::string& name,
                                    SkSpan<const SkDPoint> curveInputs, bool yValues,
                                    double expectedA, double expectedB,
                                    double expectedC, double expectedD) {
    skiatest::ReporterContext subtest(reporter, name);
    REPORTER_ASSERT(reporter, curveInputs.size() == 4,
                    "Invalid test case. Need 4 points (start, control, control, end)");

    const double* input = &curveInputs[0].fX;
    if (yValues) {
        input = &curveInputs[0].fY;
    }
    double A, B, C, D;
    SkDCubic::Coefficients(input, &A, &B, &C, &D);

    REPORTER_ASSERT(reporter, nearly_equal(expectedA, A), "%f != %f", expectedA, A);
    REPORTER_ASSERT(reporter, nearly_equal(expectedB, B), "%f != %f", expectedB, B);
    REPORTER_ASSERT(reporter, nearly_equal(expectedC, C), "%f != %f", expectedC, C);
    REPORTER_ASSERT(reporter, nearly_equal(expectedD, D), "%f != %f", expectedD, D);
}

DEF_TEST(SkDCubicPolynomialCoefficients, reporter) {
    testConvertToPolynomial(reporter, "Arbitrary control points X direction",
        {{1, 2}, {-3, 4}, {5, -6}, {7, 8}}, false, /*=yValues*/
        -18, 36, -12, 1
    );
    testConvertToPolynomial(reporter, "Arbitrary control points Y direction",
        {{1, 2}, {-3, 4}, {5, -6}, {7, 8}}, true, /*=yValues*/
        36, -36, 6, 2
    );
}
