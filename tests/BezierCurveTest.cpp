/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/base/SkBezierCurves.h"
#include "src/base/SkQuads.h"
#include "tests/Test.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <limits>
#include <set>
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

static void testCubicEvalAtT(skiatest::Reporter* reporter, const std::string& name,
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

static void testCubicConvertToPolynomial(skiatest::Reporter* reporter, const std::string& name,
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

// Since, Roots and EvalAt are separately unit tested, make sure that the parametric pramater t
// is correctly in range, and checked.
DEF_TEST(QuadRoots_CheckTRange, reporter) {
    // Pick interesting numbers around 0 and 1.
    const double interestingRoots[] =
            {-1000, -10, -1, -0.1, -0.0001, 0, 0.0001, 0.1, 0.9, 0.9999, 1, 1.0001, 1.1, 10, 1000};

    // Interesting scales to make the quadratic.
    const double interestingScales[] =
            {-1000, -10, -1, -0.1, -0.0001, 0.0001, 0.1, 1, 10, 1000};

    auto outsideTRange = [](double r) {
        return r < 0 || 1 < r;
    };

    auto insideTRange = [&] (double r) {
        return !outsideTRange(r);
    };

    // The original test for AddValidTs (which quad intersect was based on) used 1 float ulp of
    // leeway for comparison. Tighten this up to half a float ulp.
    auto equalAsFloat = [] (double a, double b) {
        // When converted to float, a double will be rounded up to half a float ulp for a double
        // value between two float values.
        return sk_double_to_float(a) == sk_double_to_float(b);
    };

    for (double r1 : interestingRoots) {
        for (double r0 : interestingRoots) {
            for (double s : interestingScales) {
                // Create a quadratic using the roots r0 and r1.
                // s(x-r0)(x-r1) = s(x^2 - r0*x - r1*x + r0*r1)
                const double A = s;
                // Normally B = -(r0 + r1) but this needs the modified B' = (r0 + r1) / 2.
                const double B = s * 0.5 * (r0 + r1);
                const double C = s*r0*r1;

                float storage[2];
                // The X coefficients are set to return t's generated by root intersection.
                // The offset is set to 0, because an arbitrary offset is essentially encoded in C.
                auto intersections = SkBezierQuad::Intersect(0, -0.5, 0, A, B, C, 0, storage);

                if (intersections.empty()) {
                    // Either imaginary or both roots are outside [0, 1].
                    REPORTER_ASSERT(reporter,
                                    SkQuads::Discriminant(A, B, C) < 0
                                    || (outsideTRange(r0) && outsideTRange(r1)));
                } else if (intersections.size() == 1) {
                    // One of the roots is outside [0, 1]
                    REPORTER_ASSERT(reporter, insideTRange(r0) || insideTRange(r1));
                    const double insideRoot = insideTRange(r0) ? r0 : r1;
                    REPORTER_ASSERT(reporter, equalAsFloat(insideRoot, intersections[0]));
                } else {
                    REPORTER_ASSERT(reporter, intersections.size() == 2);
                    REPORTER_ASSERT(reporter, insideTRange(r0) && insideTRange(r1));
                    auto [smaller, bigger] = std::minmax(intersections[0], intersections[1]);
                    auto [smallerRoot, biggerRoot] = std::minmax(r0, r1);
                    REPORTER_ASSERT(reporter, equalAsFloat(smaller, smallerRoot));
                    REPORTER_ASSERT(reporter, equalAsFloat(bigger, biggerRoot));
                }
            }
        }
    }

    // Check when A == 0.
    {
        const double A = 0;

        // We need M = 4, so that will be a Kahan style B of -0.5 * M = -2.
        const double B = -2;
        const double C = -1;
        float storage[2];

        // Assume the offset is already encoded in C.
        auto intersections = SkBezierQuad::Intersect(0, -0.5, 0, A, B, C, 0, storage);
        REPORTER_ASSERT(reporter, intersections.size() == 1);
        REPORTER_ASSERT(reporter, intersections[0] == 0.25);
    }
}

// Since, Roots and EvalAt are separately unit tested, make sure that the parametric pramater t
// is correctly in range, and checked.
DEF_TEST(SkBezierCubic_CheckTRange, reporter) {
    // Pick interesting numbers around 0 and 1.
    const double interestingRoots[] =
            {-10, -5, -2, -1, 0, 0.5, 1, 2, 5, 10};

    // Interesting scales to make the quadratic.
    const double interestingScales[] =
            {-1000, -10, -1, -0.1, -0.0001, 0.0001, 0.1, 1, 10, 1000};

    auto outsideTRange = [](double r) {
        return r < 0 || 1 < r;
    };

    auto insideTRange = [&] (double r) {
        return !outsideTRange(r);
    };

    auto specialEqual = [] (double actual, double test) {
        // At least a floats worth of digits are correct.
        const double errorFactor = std::numeric_limits<float>::epsilon();
        return std::abs(test - actual) <= errorFactor * std::max(std::abs(test), std::abs(actual));
    };

    for (double r2 : interestingRoots) {
        for (double r1 : interestingRoots) {
            for (double r0 : interestingRoots) {
                for (double s : interestingScales) {
                    // Create a cubic using the roots r0, r1, and r2.
                    // s(x-r0)(x-r1)(x-r2) = s(x^3 - (r0+r1+r2)x^2 + (r0r1+r1r2+r0r2)x - r0r1r2)
                    const double A =  s,
                                 B = -s * (r0+r1+r2),
                                 C =  s * (r0*r1 + r1*r2 + r0*r2),
                                 D = -s * r0 * r1 * r2;

                    // Accumulate all the valid t's.
                    std::set<double> inRangeRoots;
                    for (auto r : {r0, r1, r2}) {
                        if (insideTRange(r)) {
                            inRangeRoots.insert(r);
                        }
                    }

                    float storage[3];
                    // The X coefficients are set to return t's generated by root intersection.
                    // The offset is set to 0, because an arbitrary offset is essentially encoded
                    // in C.
                    auto intersections =
                            SkBezierCubic::Intersect(0, 0, 1, 0, A, B, C, D, 0, storage);

                    size_t correct = 0;
                    for (auto candidate : intersections) {
                        for (auto answer : inRangeRoots) {
                            if (specialEqual(candidate, answer)) {
                                correct += 1;
                                break;
                            }
                        }
                    }
                    REPORTER_ASSERT(reporter, correct == intersections.size());
                }
            }
        }
    }
}

