/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/base/SkBezierCurves.h"
#include "tests/Test.h"

#include <string>

// Grouping the test inputs into DoublePoints makes the test cases easier to read.
struct DoublePoint {
    double x;
    double y;
};

static bool nearly_equal(double expected, double actual) {
    if (sk_double_nearly_zero(expected)) {
        return sk_double_nearly_zero(actual);
    }
    return sk_doubles_nearly_equal_ulps(expected, actual, 64);
}

static void testCubicEvalAtT(skiatest::Reporter* reporter, std::string name,
                             SkSpan<const DoublePoint> curveInputs, double t,
                             const DoublePoint& expectedXY) {
    skiatest::ReporterContext subtest(reporter, name);
    REPORTER_ASSERT(reporter, curveInputs.size() == 4,
                    "Invalid test case. Should have 4 input points.");
    REPORTER_ASSERT(reporter, t >= 0.0 && t <= 1.0,
                    "Invalid test case. t %f should be in [0, 1]", t);

    auto [x, y] = SkBezierCubic::EvalAt(reinterpret_cast<const double*>(curveInputs.data()), t);

    REPORTER_ASSERT(reporter, nearly_equal(expectedXY.x, x),
                    "X wrong %1.16f != %1.16f", expectedXY.x, x);
    REPORTER_ASSERT(reporter, nearly_equal(expectedXY.y, y),
                    "Y wrong %1.16f != %1.16f", expectedXY.y, y);
}

DEF_TEST(BezierCubicEvalAt, reporter) {
    testCubicEvalAtT(reporter, "linear curve @0.1234",
                     {{ 0, 0 }, { 0, 0 }, { 10, 10 }, { 10, 10 }},
                     0.1234,
                     { 0.4192451819200000, 0.4192451819200000 });

    testCubicEvalAtT(reporter, "linear curve @0.2345",
                     {{ 0, 0 }, { 5, 5 }, { 5, 5 }, { 10, 10 }},
                     0.2345,
                     { 2.8215983862500000, 2.8215983862500000 });

    testCubicEvalAtT(reporter, "Arbitrary Cubic, t=0.0",
                     {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
                     0.0,
                     { -10, -20 });

    testCubicEvalAtT(reporter, "Arbitrary Cubic, t=0.3456",
                     {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
                     0.3456,
                     { -2.503786700800000, -3.31715344793600 });

    testCubicEvalAtT(reporter, "Arbitrary Cubic, t=0.5",
                     {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
                     0.5,
                     { 1.75, 0.25 });

    testCubicEvalAtT(reporter, "Arbitrary Cubic, t=0.7891",
                     {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
                     0.7891,
                     { 6.158763291450000, 5.938550084434000 });

    testCubicEvalAtT(reporter, "Arbitrary Cubic, t=1.0",
                     {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
                     1.0,
                     { 3, 13 });
}

static void testCubicConvertToPolynomial(skiatest::Reporter* reporter, std::string name,
                                         SkSpan<const DoublePoint> curveInputs, bool yValues,
                                         double expectedA, double expectedB,
                                         double expectedC, double expectedD) {
    skiatest::ReporterContext subtest(reporter, name);
    REPORTER_ASSERT(reporter, curveInputs.size() == 4,
                    "Invalid test case. Need 4 points (start, control, control, end)");

    skiatest::ReporterContext subsubtest(reporter, "SkBezierCurve Implementation");
    const double* input = &curveInputs[0].x;
    auto [A, B, C, D] = SkBezierCubic::ConvertToPolynomial(input, yValues);

    REPORTER_ASSERT(reporter, nearly_equal(expectedA, A), "%f != %f", expectedA, A);
    REPORTER_ASSERT(reporter, nearly_equal(expectedB, B), "%f != %f", expectedB, B);
    REPORTER_ASSERT(reporter, nearly_equal(expectedC, C), "%f != %f", expectedC, C);
    REPORTER_ASSERT(reporter, nearly_equal(expectedD, D), "%f != %f", expectedD, D);
}

DEF_TEST(BezierCubicToPolynomials, reporter) {
    // See also tests/PathOpsDCubicTest.cpp->SkDCubicPolynomialCoefficients
    testCubicConvertToPolynomial(reporter, "Arbitrary control points X direction",
        {{1, 2}, {-3, 4}, {5, -6}, {7, 8}}, false, /*=yValues*/
        -18, 36, -12, 1
    );
    testCubicConvertToPolynomial(reporter, "Arbitrary control points Y direction",
        {{1, 2}, {-3, 4}, {5, -6}, {7, 8}}, true, /*=yValues*/
        36, -36, 6, 2
    );
}
