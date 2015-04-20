/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkGeometry.h"
#include "SkIntersections.h"
#include "Test.h"

/*
manually compute the intersection of a pair of circles and see if the conic intersection matches
  given two circles
    construct a line connecting their centers
    
 */

static const SkDConic testSet[] = {
    {{{{-4,1}, {-4,5}, {0,5}}}, 0.707106769f},
    {{{{-3,4}, {-3,1}, {0,1}}}, 0.707106769f},

    {{{{0, 0}, {0, 1}, {1, 1}}}, 0.5f},
    {{{{1, 0}, {0, 0}, {0, 1}}}, 0.5f},
};

const int testSetCount = (int) SK_ARRAY_COUNT(testSet);

static void oneOff(skiatest::Reporter* reporter, const SkDConic& c1, const SkDConic& c2,
        bool coin) {
    SkASSERT(ValidConic(c1));
    SkASSERT(ValidConic(c2));
    SkIntersections intersections;
    intersections.intersect(c1, c2);
    if (coin && intersections.used() != 2) {
        SkDebugf("");
    }
    REPORTER_ASSERT(reporter, !coin || intersections.used() == 2);
    double tt1, tt2;
    SkDPoint xy1, xy2;
    for (int pt3 = 0; pt3 < intersections.used(); ++pt3) {
        tt1 = intersections[0][pt3];
        xy1 = c1.ptAtT(tt1);
        tt2 = intersections[1][pt3];
        xy2 = c2.ptAtT(tt2);
        const SkDPoint& iPt = intersections.pt(pt3);
        REPORTER_ASSERT(reporter, xy1.approximatelyEqual(iPt));
        REPORTER_ASSERT(reporter, xy2.approximatelyEqual(iPt));
        REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
    }
    reporter->bumpTestCount();
}

static void oneOff(skiatest::Reporter* reporter, int outer, int inner) {
    const SkDConic& c1 = testSet[outer];
    const SkDConic& c2 = testSet[inner];
    oneOff(reporter, c1, c2, false);
}

static void oneOffTests(skiatest::Reporter* reporter) {
    for (int outer = 0; outer < testSetCount - 1; ++outer) {
        for (int inner = outer + 1; inner < testSetCount; ++inner) {
            oneOff(reporter, outer, inner);
        }
    }
}

DEF_TEST(PathOpsConicIntersectionOneOff, reporter) {
    oneOff(reporter, 0, 1);
}

DEF_TEST(PathOpsConicIntersection, reporter) {
    oneOffTests(reporter);
}
