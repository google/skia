/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "src/pathops/SkPathOpsPoint.h"
#include "tests/Test.h"

#include <cstring>
#include <string>

struct DoublePoint{
    double x;
    double y;
};

static bool nearly_equal(double expected, double actual) {
    if (sk_double_nearly_zero(expected)) {
        return sk_double_nearly_zero(actual);
    }
    return sk_doubles_nearly_equal_ulps(expected, actual, 64);
}

static void testChopCubicAtT(skiatest::Reporter* reporter, std::string name,
                             SkSpan<const DoublePoint> curveInputs, double t,
                             SkSpan<const DoublePoint> expectedOutputs) {
    skiatest::ReporterContext subtest(reporter, name);
    REPORTER_ASSERT(reporter, curveInputs.size() == 4,
                    "Invalid test case. Should have 4 input points.");
    REPORTER_ASSERT(reporter, t >= 0.0 && t <= 1.0,
                    "Invalid test case. t %f should be in [0, 1]", t);
    // Two cubic curves, sharing an input point
    REPORTER_ASSERT(reporter, expectedOutputs.size() == 7,
                    "Invalid test case. Should have 7 output points.");


    SkDCubic input;
    std::memcpy(&input.fPts[0], curveInputs.begin(), 8 * sizeof(double));
    SkDCubicPair output = input.chopAt(t);

    for (int i = 0; i < 7; ++i) {
        REPORTER_ASSERT(reporter,
                        nearly_equal(expectedOutputs[i].x, output.pts[i].fX) &&
                        nearly_equal(expectedOutputs[i].y, output.pts[i].fY),
                        "(%.16f, %.16f) != (%.16f, %.16f) at index %d",
                        expectedOutputs[i].x, expectedOutputs[i].y,
                        output.pts[i].fX, output.pts[i].fY, i);
    }
}

DEF_TEST(ChopCubicAtT_NormalizedCubic, reporter) {
    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.0,
        {{  0.000000,  0.000000 }, {  0.000000,  0.000000 }, {  0.000000,  0.000000 },
         {  0.000000,  0.000000 },
         {  0.120000,  1.800000 }, {  0.920000, -1.650000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.1",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.1,
        {{  0.000000,  0.000000 }, {  0.012000,  0.180000 }, {  0.030800,  0.307500 },
         {  0.055000,  0.393850 },
         {  0.272800,  1.171000 }, {  0.928000, -1.385000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.2",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.2,
        {{  0.000000,  0.000000 }, {  0.024000,  0.360000 }, {  0.075200,  0.510000 },
         {  0.142400,  0.540800 },
         {  0.411200,  0.664000 }, {  0.936000, -1.120000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.3",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.3,
        {{  0.000000,  0.000000 }, {  0.036000,  0.540000 }, {  0.133200,  0.607500 },
         {  0.253800,  0.508950 },
         {  0.535200,  0.279000 }, {  0.944000, -0.855000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.4",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.4,
        {{  0.000000,  0.000000 }, {  0.048000,  0.720000 }, {  0.204800,  0.600000 },
         {  0.380800,  0.366400 },
         {  0.644800,  0.016000 }, {  0.952000, -0.590000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.5",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.5,
        {{  0.000000,  0.000000 }, {  0.060000,  0.900000 }, {  0.290000,  0.487500 },
         {  0.515000,  0.181250 },
         {  0.740000, -0.125000 }, {  0.960000, -0.325000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.6",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.6,
        {{  0.000000,  0.000000 }, {  0.072000,  1.080000 }, {  0.388800,  0.270000 },
         {  0.648000,  0.021600 },
         {  0.820800, -0.144000 }, {  0.968000, -0.060000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.7",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.7,
        {{  0.000000,  0.000000 }, {  0.084000,  1.260000 }, {  0.501200, -0.052500 },
         {  0.771400, -0.044450 },
         {  0.887200, -0.041000 }, {  0.976000,  0.205000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.8",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.8,
        {{  0.000000,  0.000000 }, {  0.096000,  1.440000 }, {  0.627200, -0.480000 },
         {  0.876800,  0.051200 },
         {  0.939200,  0.184000 }, {  0.984000,  0.470000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=0.9",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        0.9,
        {{  0.000000,  0.000000 }, {  0.108000,  1.620000 }, {  0.766800, -1.012500 },
         {  0.955800,  0.376650 },
         {  0.976800,  0.531000 }, {  0.992000,  0.735000 }, {  1.000000,  1.000000 }}
    );

    testChopCubicAtT(reporter, "Cubic between 0,0, and 1,1, t=1.0",
        {{ 0, 0 }, { .12, 1.8 }, { .92, -1.65 }, { 1.0, 1.0 }},
        1.0,
        {{  0.000000,  0.000000 }, {  0.120000,  1.800000 }, {  0.920000, -1.650000 },
         {  1.000000,  1.000000 },
         {  1.000000,  1.000000 }, {  1.000000,  1.000000 }, {  1.000000,  1.000000 }}
    );
}

DEF_TEST(ChopCubicAtT_ArbitraryCubic, reporter) {
    testChopCubicAtT(reporter, "Arbitrary Cubic, t=0.1234",
        {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
        0.1234,
        {{-10.00000000000000, -20.00000000000000 },
         { -9.62980000000000, -16.91500000000000 },
         { -8.98550392000000, -14.31728192000000 },
         { -8.16106580520000, -12.10537539118400 },
         { -2.30448192000000,   3.60740632000000 },
         { 12.64260000000000,  -0.14900000000000 },
         {  3.00000000000000,  13.00000000000000 }}
    );

    testChopCubicAtT(reporter, "Arbitrary Cubic, t=0.3456",
        {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
        0.3456,
        {{-10.00000000000000, -20.00000000000000 },
         { -8.96320000000000, -11.36000000000000 },
         { -5.77649152000000,  -6.54205952000000 },
         { -2.50378670080000,  -3.31715344793600 },
         {  3.69314048000000,   2.78926592000000 },
         { 10.19840000000000,   3.18400000000000 },
         {  3.00000000000000,  13.00000000000000 }}
    );

    testChopCubicAtT(reporter, "Arbitrary Cubic, t=0.8765",
        {{ -10, -20 }, { -7, 5 }, { 14, -2 }, { 3, 13 }},
        0.8765,
        {{-10.00000000000000, -20.00000000000000 },
         { -7.37050000000000,   1.91250000000000 },
         {  9.08754050000000,  -0.75907200000000 },
         {  5.70546664375000,   8.34743124475000 },
         {  5.22892800000000,   9.63054950000000 },
         {  4.35850000000000,  11.14750000000000 },
         {  3.00000000000000,  13.00000000000000 }}
    );
}
