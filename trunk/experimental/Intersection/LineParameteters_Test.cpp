/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Intersection_Tests.h"
#include "LineParameters.h"


// tests to verify that distance calculations are coded correctly
const Cubic tests[] = {
    {{0, 0}, {1, 1}, {2, 2}, {0, 3}},
    {{0, 0}, {1, 1}, {2, 2}, {3, 0}},
    {{0, 0}, {5, 0}, {-2,4}, {3, 4}},
    {{0, 2}, {1, 0}, {2, 0}, {3, 0}},
    {{0, .2}, {1, 0}, {2, 0}, {3, 0}},
    {{0, .02}, {1, 0}, {2, 0}, {3, 0}},
    {{0, .002}, {1, 0}, {2, 0}, {3, 0}},
    {{0, .0002}, {1, 0}, {2, 0}, {3, 0}},
    {{0, .00002}, {1, 0}, {2, 0}, {3, 0}},
    {{0, PointEpsilon * 2}, {1, 0}, {2, 0}, {3, 0}},
};

const double answers[][2] = {
    {1, 2},
    {1, 2},
    {4, 4},
    {1.1094003924, 0.5547001962},
    {0.133038021, 0.06651901052},
    {0.0133330370, 0.006666518523},
    {0.001333333037, 0.0006666665185},
    {0.000133333333, 6.666666652e-05},
    {1.333333333e-05, 6.666666667e-06},
    {1.333333333e-06, 6.666666667e-07},
};

const size_t tests_count = sizeof(tests) / sizeof(tests[0]);

static size_t firstLineParameterTest = 0;

void LineParameter_Test() {
    for (size_t index = firstLineParameterTest; index < tests_count; ++index) {
        LineParameters lineParameters;
        const Cubic& cubic = tests[index];
        lineParameters.cubicEndPoints(cubic);
        double denormalizedDistance[2];
        lineParameters.controlPtDistance(cubic, denormalizedDistance);
        double normalSquared = lineParameters.normalSquared();
        size_t inner;
        for (inner = 0; inner < 2; ++inner) {
            double distSq = denormalizedDistance[inner];
            distSq *= distSq;
            double answersSq = answers[index][inner];
            answersSq *= answersSq;
            if (approximately_equal(distSq, normalSquared * answersSq)) {
                continue;
            }
            printf("%s [%d,%d] denormalizedDistance:%g != answer:%g"
                    " distSq:%g answerSq:%g normalSquared:%g\n",
                    __FUNCTION__, (int)index, (int)inner,
                    denormalizedDistance[inner], answers[index][inner],
                    distSq, answersSq, normalSquared);
        }
        lineParameters.normalize();
        double normalizedDistance[2];
        lineParameters.controlPtDistance(cubic, normalizedDistance);
        for (inner = 0; inner < 2; ++inner) {
            if (approximately_equal(fabs(normalizedDistance[inner]),
                    answers[index][inner])) {
                continue;
            }
            printf("%s [%d,%d] normalizedDistance:%1.10g != answer:%g\n",
                    __FUNCTION__, (int)index, (int)inner,
                    normalizedDistance[inner], answers[index][inner]);
        }
    }
}
