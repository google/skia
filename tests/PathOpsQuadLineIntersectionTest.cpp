/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/pathops/SkIntersections.h"
#include "src/pathops/SkPathOpsLine.h"
#include "src/pathops/SkPathOpsQuad.h"
#include "src/pathops/SkReduceOrder.h"
#include "tests/PathOpsExtendedTest.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

#include <utility>

static struct lineQuad {
    QuadPts quad;
    SkDLine line;
    int result;
    SkDPoint expected[2];
} lineQuadTests[] = {
    //        quad                    line                  results
    {{{{1, 1}, {2, 1}, {0, 2}}}, {{{0, 0}, {1, 1}}},  1,  {{1, 1}, {0, 0}} },
    {{{{0, 0}, {1, 1}, {3, 1}}}, {{{0, 0}, {3, 1}}},  2,  {{0, 0}, {3, 1}} },
    {{{{2, 0}, {1, 1}, {2, 2}}}, {{{0, 0}, {0, 2}}},  0,  {{0, 0}, {0, 0}} },
    {{{{4, 0}, {0, 1}, {4, 2}}}, {{{3, 1}, {4, 1}}},  0,  {{0, 0}, {0, 0}} },
    {{{{0, 0}, {0, 1}, {1, 1}}}, {{{0, 1}, {1, 0}}},  1,  {{.25, .75}, {0, 0}} },
};

static size_t lineQuadTests_count = SK_ARRAY_COUNT(lineQuadTests);

static int doIntersect(SkIntersections& intersections, const SkDQuad& quad, const SkDLine& line,
                       bool& flipped) {
    int result;
    flipped = false;
    if (line[0].fX == line[1].fX) {
        double top = line[0].fY;
        double bottom = line[1].fY;
        flipped = top > bottom;
        if (flipped) {
            using std::swap;
            swap(top, bottom);
        }
        result = intersections.vertical(quad, top, bottom, line[0].fX, flipped);
    } else if (line[0].fY == line[1].fY) {
        double left = line[0].fX;
        double right = line[1].fX;
        flipped = left > right;
        if (flipped) {
            using std::swap;
            swap(left, right);
        }
        result = intersections.horizontal(quad, left, right, line[0].fY, flipped);
    } else {
        intersections.intersect(quad, line);
        result = intersections.used();
    }
    return result;
}

static struct oneLineQuad {
    QuadPts quad;
    SkDLine line;
} oneOffs[] = {
    {{{{97.9337616,100}, {88,112.94265}, {88,130}}},
     {{{88.919838,120}, {107.058823,120}}}},
    {{{{447.96701049804687, 894.4381103515625}, {448.007080078125, 894.4239501953125},
       {448.0140380859375, 894.4215087890625}}},
     {{{490.43548583984375, 879.40740966796875}, {405.59262084960937, 909.435546875}}}},
    {{{{142.589081, 102.283646}, {149.821579, 100}, {158, 100}}},
        {{{90, 230}, {160, 60}}}},
    {{{{1101, 10}, {1101, 8.3431453704833984}, {1099.828857421875, 7.1711997985839844}}},
        {{{1099.828857421875,7.1711711883544922}, {1099.121337890625,7.8786783218383789}}}},
    {{{{973, 507}, {973, 508.24264526367187}, {972.12158203125, 509.12161254882812}}},
        {{{930, 467}, {973, 510}}}},
    {{{{369.848602, 145.680267}, {382.360413, 121.298294}, {406.207703, 121.298294}}},
        {{{406.207703, 121.298294}, {348.781738, 123.864815}}}},
};

static size_t oneOffs_count = SK_ARRAY_COUNT(oneOffs);

static void testOneOffs(skiatest::Reporter* reporter) {
    bool flipped = false;
    for (size_t index = 0; index < oneOffs_count; ++index) {
        const QuadPts& q = oneOffs[index].quad;
        SkDQuad quad;
        quad.debugSet(q.fPts);
        SkASSERT(ValidQuad(quad));
        const SkDLine& line = oneOffs[index].line;
        SkASSERT(ValidLine(line));
        SkIntersections intersections;
        int result = doIntersect(intersections, quad, line, flipped);
        for (int inner = 0; inner < result; ++inner) {
            double quadT = intersections[0][inner];
            SkDPoint quadXY = quad.ptAtT(quadT);
            double lineT = intersections[1][inner];
            SkDPoint lineXY = line.ptAtT(lineT);
            if (!quadXY.approximatelyEqual(lineXY)) {
                quadXY.approximatelyEqual(lineXY);
                SkDebugf("");
            }
            REPORTER_ASSERT(reporter, quadXY.approximatelyEqual(lineXY));
        }
    }
}

DEF_TEST(PathOpsQuadLineIntersectionOneOff, reporter) {
    testOneOffs(reporter);
}

DEF_TEST(PathOpsQuadLineIntersection, reporter) {
    for (size_t index = 0; index < lineQuadTests_count; ++index) {
        int iIndex = static_cast<int>(index);
        const QuadPts& q = lineQuadTests[index].quad;
        SkDQuad quad;
        quad.debugSet(q.fPts);
        SkASSERT(ValidQuad(quad));
        const SkDLine& line = lineQuadTests[index].line;
        SkASSERT(ValidLine(line));
        SkReduceOrder reducer1, reducer2;
        int order1 = reducer1.reduce(quad);
        int order2 = reducer2.reduce(line);
        if (order1 < 3) {
            SkDebugf("%s [%d] quad order=%d\n", __FUNCTION__, iIndex, order1);
            REPORTER_ASSERT(reporter, 0);
        }
        if (order2 < 2) {
            SkDebugf("%s [%d] line order=%d\n", __FUNCTION__, iIndex, order2);
            REPORTER_ASSERT(reporter, 0);
        }
        SkIntersections intersections;
        bool flipped = false;
        int result = doIntersect(intersections, quad, line, flipped);
        REPORTER_ASSERT(reporter, result == lineQuadTests[index].result);
        if (intersections.used() <= 0) {
            continue;
        }
        for (int pt = 0; pt < result; ++pt) {
            double tt1 = intersections[0][pt];
            REPORTER_ASSERT(reporter, tt1 >= 0 && tt1 <= 1);
            SkDPoint t1 = quad.ptAtT(tt1);
            double tt2 = intersections[1][pt];
            REPORTER_ASSERT(reporter, tt2 >= 0 && tt2 <= 1);
            SkDPoint t2 = line.ptAtT(tt2);
            if (!t1.approximatelyEqual(t2)) {
                SkDebugf("%s [%d,%d] x!= t1=%1.9g (%1.9g,%1.9g) t2=%1.9g (%1.9g,%1.9g)\n",
                    __FUNCTION__, iIndex, pt, tt1, t1.fX, t1.fY, tt2, t2.fX, t2.fY);
                REPORTER_ASSERT(reporter, 0);
            }
            if (!t1.approximatelyEqual(lineQuadTests[index].expected[0])
                    && (lineQuadTests[index].result == 1
                    || !t1.approximatelyEqual(lineQuadTests[index].expected[1]))) {
                SkDebugf("%s t1=(%1.9g,%1.9g)\n", __FUNCTION__, t1.fX, t1.fY);
                REPORTER_ASSERT(reporter, 0);
            }
        }
    }
}
