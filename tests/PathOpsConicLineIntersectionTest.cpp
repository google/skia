/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsTestCommon.h"
#include "SkIntersections.h"
#include "SkPathOpsConic.h"
#include "SkPathOpsLine.h"
#include "SkReduceOrder.h"
#include "Test.h"

static struct lineConic {
    SkDConic conic;
    SkDLine line;
    int result;
    SkDPoint expected[2];
} lineConicTests[] = {
    {
     {{{{30.6499996,25.6499996}, {30.6499996,20.6499996}, {25.6499996,20.6499996}}}, 0.707107008f},
      {{{25.6499996,20.6499996}, {45.6500015,20.6499996}}},
          1, 
       {{25.6499996,20.6499996}, {0,0}}
    },
};

static size_t lineConicTests_count = SK_ARRAY_COUNT(lineConicTests);

static int doIntersect(SkIntersections& intersections, const SkDConic& conic, const SkDLine& line,
                       bool& flipped) {
    int result;
    flipped = false;
    if (line[0].fX == line[1].fX) {
        double top = line[0].fY;
        double bottom = line[1].fY;
        flipped = top > bottom;
        if (flipped) {
            SkTSwap<double>(top, bottom);
        }
        result = intersections.vertical(conic, top, bottom, line[0].fX, flipped);
    } else if (line[0].fY == line[1].fY) {
        double left = line[0].fX;
        double right = line[1].fX;
        flipped = left > right;
        if (flipped) {
            SkTSwap<double>(left, right);
        }
        result = intersections.horizontal(conic, left, right, line[0].fY, flipped);
    } else {
        intersections.intersect(conic, line);
        result = intersections.used();
    }
    return result;
}

static struct oneLineConic {
    SkDConic conic;
    SkDLine line;
} oneOffs[] = {
    {{{{{30.6499996,25.6499996}, {30.6499996,20.6499996}, {25.6499996,20.6499996}}}, 0.707107008f},
      {{{25.6499996,20.6499996}, {45.6500015,20.6499996}}}}
};

static size_t oneOffs_count = SK_ARRAY_COUNT(oneOffs);

static void testOneOffs(skiatest::Reporter* reporter) {
    bool flipped = false;
    for (size_t index = 0; index < oneOffs_count; ++index) {
        const SkDConic& conic = oneOffs[index].conic;
        SkASSERT(ValidConic(conic));
        const SkDLine& line = oneOffs[index].line;
        SkASSERT(ValidLine(line));
        SkIntersections intersections;
        int result = doIntersect(intersections, conic, line, flipped);
        for (int inner = 0; inner < result; ++inner) {
            double conicT = intersections[0][inner];
            SkDPoint conicXY = conic.ptAtT(conicT);
            double lineT = intersections[1][inner];
            SkDPoint lineXY = line.ptAtT(lineT);
            if (!conicXY.approximatelyEqual(lineXY)) {
                conicXY.approximatelyEqual(lineXY);
                SkDebugf("");
            }
            REPORTER_ASSERT(reporter, conicXY.approximatelyEqual(lineXY));
        }
    }
}

DEF_TEST(PathOpsConicLineIntersectionOneOff, reporter) {
    testOneOffs(reporter);
}

DEF_TEST(PathOpsConicLineIntersection, reporter) {
    for (size_t index = 0; index < lineConicTests_count; ++index) {
        int iIndex = static_cast<int>(index);
        const SkDConic& conic = lineConicTests[index].conic;
        SkASSERT(ValidConic(conic));
        const SkDLine& line = lineConicTests[index].line;
        SkASSERT(ValidLine(line));
        SkReduceOrder reducer;
        SkPoint pts[3] = { conic.fPts.fPts[0].asSkPoint(), conic.fPts.fPts[1].asSkPoint(), 
            conic.fPts.fPts[2].asSkPoint() };
        SkPoint reduced[3];
        SkPath::Verb order1 = SkReduceOrder::Conic(pts, conic.fWeight, reduced);
        if (order1 != SkPath::kConic_Verb) {
            SkDebugf("%s [%d] conic verb=%d\n", __FUNCTION__, iIndex, order1);
            REPORTER_ASSERT(reporter, 0);
        }
        int order2 = reducer.reduce(line);
        if (order2 < 2) {
            SkDebugf("%s [%d] line order=%d\n", __FUNCTION__, iIndex, order2);
            REPORTER_ASSERT(reporter, 0);
        }
        SkIntersections intersections;
        bool flipped = false;
        int result = doIntersect(intersections, conic, line, flipped);
        REPORTER_ASSERT(reporter, result == lineConicTests[index].result);
        if (intersections.used() <= 0) {
            continue;
        }
        for (int pt = 0; pt < result; ++pt) {
            double tt1 = intersections[0][pt];
            REPORTER_ASSERT(reporter, tt1 >= 0 && tt1 <= 1);
            SkDPoint t1 = conic.ptAtT(tt1);
            double tt2 = intersections[1][pt];
            REPORTER_ASSERT(reporter, tt2 >= 0 && tt2 <= 1);
            SkDPoint t2 = line.ptAtT(tt2);
            if (!t1.approximatelyEqual(t2)) {
                SkDebugf("%s [%d,%d] x!= t1=%1.9g (%1.9g,%1.9g) t2=%1.9g (%1.9g,%1.9g)\n",
                    __FUNCTION__, iIndex, pt, tt1, t1.fX, t1.fY, tt2, t2.fX, t2.fY);
                REPORTER_ASSERT(reporter, 0);
            }
            if (!t1.approximatelyEqual(lineConicTests[index].expected[0])
                    && (lineConicTests[index].result == 1
                    || !t1.approximatelyEqual(lineConicTests[index].expected[1]))) {
                SkDebugf("%s t1=(%1.9g,%1.9g)\n", __FUNCTION__, t1.fX, t1.fY);
                REPORTER_ASSERT(reporter, 0);
            }
        }
    }
}
