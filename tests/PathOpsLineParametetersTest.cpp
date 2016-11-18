/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkLineParameters.h"
#include "Test.h"

// tests to verify that distance calculations are coded correctly
static const CubicPts tests[] = {
    {{{0, 0}, {1, 1}, {2, 2}, {0, 3}}},
    {{{0, 0}, {1, 1}, {2, 2}, {3, 0}}},
    {{{0, 0}, {5, 0}, {-2, 4}, {3, 4}}},
    {{{0, 2}, {1, 0}, {2, 0}, {3, 0}}},
    {{{0, .2}, {1, 0}, {2, 0}, {3, 0}}},
    {{{0, .02}, {1, 0}, {2, 0}, {3, 0}}},
    {{{0, .002}, {1, 0}, {2, 0}, {3, 0}}},
    {{{0, .0002}, {1, 0}, {2, 0}, {3, 0}}},
    {{{0, .00002}, {1, 0}, {2, 0}, {3, 0}}},
    {{{0, FLT_EPSILON * 2}, {1, 0}, {2, 0}, {3, 0}}},
};

static const double answers[][2] = {
    {1, 2},
    {1, 2},
    {4, 4},
    {1.1094003924, 0.5547001962},
    {0.133038021, 0.06651901052},
    {0.0133330370, 0.006666518523},
    {0.001333333037, 0.0006666665185},
    {0.000133333333, 6.666666652e-05},
    {1.333333333e-05, 6.666666667e-06},
    {1.5894571940104115e-07, 7.9472859700520577e-08},
};

static const size_t tests_count = SK_ARRAY_COUNT(tests);

DEF_TEST(PathOpsLineParameters, reporter) {
    for (size_t index = 0; index < tests_count; ++index) {
        SkLineParameters lineParameters;
        const CubicPts& c = tests[index];
        SkDCubic cubic;
        cubic.debugSet(c.fPts);
        SkASSERT(ValidCubic(cubic));
        lineParameters.cubicEndPoints(cubic, 0, 3);
        double denormalizedDistance[2];
        denormalizedDistance[0] = lineParameters.controlPtDistance(cubic, 1);
        denormalizedDistance[1] = lineParameters.controlPtDistance(cubic, 2);
        double normalSquared = lineParameters.normalSquared();
        size_t inner;
        for (inner = 0; inner < 2; ++inner) {
            double distSq = denormalizedDistance[inner];
            distSq *= distSq;
            double answersSq = answers[index][inner];
            answersSq *= answersSq;
            if (AlmostEqualUlps(distSq, normalSquared * answersSq)) {
                continue;
            }
            SkDebugf("%s [%d,%d] denormalizedDistance:%g != answer:%g"
                    " distSq:%g answerSq:%g normalSquared:%g\n",
                    __FUNCTION__, static_cast<int>(index), (int)inner,
                    denormalizedDistance[inner], answers[index][inner],
                    distSq, answersSq, normalSquared);
        }
        lineParameters.normalize();
        double normalizedDistance[2];
        normalizedDistance[0] = lineParameters.controlPtDistance(cubic, 1);
        normalizedDistance[1] = lineParameters.controlPtDistance(cubic, 2);
        for (inner = 0; inner < 2; ++inner) {
            if (AlmostEqualUlps(fabs(normalizedDistance[inner]), answers[index][inner])) {
                continue;
            }
            SkDebugf("%s [%d,%d] normalizedDistance:%1.9g != answer:%g\n",
                    __FUNCTION__, static_cast<int>(index), (int)inner,
                    normalizedDistance[inner], answers[index][inner]);
            REPORTER_ASSERT(reporter, 0);
        }
    }
}
