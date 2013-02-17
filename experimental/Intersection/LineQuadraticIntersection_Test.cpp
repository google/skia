/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "CurveUtilities.h"
#include "EdgeWalker_Test.h"
#include "Intersection_Tests.h"
#include "Intersections.h"
#include "TestUtilities.h"

struct lineQuad {
    Quadratic quad;
    _Line line;
    int result;
    _Point expected[2];
} lineQuadTests[] = {
    //        quad                    line                  results
    {{{1, 1}, {2, 1}, {0, 2}},  {{0, 0}, {1, 1}},  1,  {{1, 1}        }},
    {{{0, 0}, {1, 1}, {3, 1}},  {{0, 0}, {3, 1}},  2,  {{0, 0}, {3, 1}}},
    {{{2, 0}, {1, 1}, {2, 2}},  {{0, 0}, {0, 2}},  0                   },
    {{{4, 0}, {0, 1}, {4, 2}},  {{3, 1}, {4, 1}},  0,                  },
    {{{0, 0}, {0, 1}, {1, 1}},  {{0, 1}, {1, 0}},  1,  {{.25, .75}    }},
};

size_t lineQuadTests_count = sizeof(lineQuadTests) / sizeof(lineQuadTests[0]);

const int firstLineQuadIntersectionTest = 0;

static int doIntersect(Intersections& intersections, const Quadratic& quad, const _Line& line, bool& flipped) {
    int result;
    flipped = false;
    if (line[0].x == line[1].x) {
        double top = line[0].y;
        double bottom = line[1].y;
        flipped = top > bottom;
        if (flipped) {
            SkTSwap<double>(top, bottom);
        }
        result = verticalIntersect(quad, top, bottom, line[0].x, flipped, intersections);
    } else if (line[0].y == line[1].y) {
        double left = line[0].x;
        double right = line[1].x;
        flipped = left > right;
        if (flipped) {
            SkTSwap<double>(left, right);
        }
        result = horizontalIntersect(quad, left, right, line[0].y, flipped, intersections);
    } else {
        intersect(quad, line, intersections);
        result = intersections.fUsed;
    }
    return result;
}

static struct oneLineQuad {
    Quadratic quad;
    _Line line;
} oneOffs[] = {
    {{{369.848602,145.680267}, {382.360413,121.298294}, {406.207703,121.298294}},
        {{406.207703,121.298294}, {348.781738,123.864815}}}
    };

static size_t oneOffs_count = sizeof(oneOffs) / sizeof(oneOffs[0]);


static void testOneOffs() {
    Intersections intersections;
    bool flipped = false;
    for (size_t index = 0; index < oneOffs_count; ++index) {
        const Quadratic& quad = oneOffs[index].quad;
        const _Line& line = oneOffs[index].line;
        int result = doIntersect(intersections, quad, line, flipped);
        for (int inner = 0; inner < result; ++inner) {
            double quadT = intersections.fT[0][inner];
            double quadX, quadY;
            xy_at_t(quad, quadT, quadX, quadY);
            double lineT = intersections.fT[1][inner];
            double lineX, lineY;
            xy_at_t(line, lineT, lineX, lineY);
            SkASSERT(AlmostEqualUlps(quadX, lineX)
                    && AlmostEqualUlps(quadY, lineY));
        }
    }
}

void LineQuadraticIntersection_Test() {
    if (1) {
        testOneOffs();
    }
    for (size_t index = firstLineQuadIntersectionTest; index < lineQuadTests_count; ++index) {
        const Quadratic& quad = lineQuadTests[index].quad;
        const _Line& line = lineQuadTests[index].line;
        Quadratic reduce1;
        _Line reduce2;
        int order1 = reduceOrder(quad, reduce1, kReduceOrder_TreatAsFill);
        int order2 = reduceOrder(line, reduce2);
        if (order1 < 3) {
            SkDebugf("%s [%d] quad order=%d\n", __FUNCTION__, (int) index, order1);
            SkASSERT(0);
        }
        if (order2 < 2) {
            SkDebugf("%s [%d] line order=%d\n", __FUNCTION__, (int) index, order2);
            SkASSERT(0);
        }
        Intersections intersections;
        bool flipped = false;
        int result = doIntersect(intersections, quad, line, flipped);
        SkASSERT(result == lineQuadTests[index].result);
        if (!intersections.intersected()) {
            continue;
        }
        for (int pt = 0; pt < result; ++pt) {
            double tt1 = intersections.fT[0][pt];
            SkASSERT(tt1 >= 0 && tt1 <= 1);
            _Point t1, t2;
            xy_at_t(quad, tt1, t1.x, t1.y);
            double tt2 = intersections.fT[1][pt];
            SkASSERT(tt2 >= 0 && tt2 <= 1);
            xy_at_t(line, tt2, t2.x, t2.y);
            if (!AlmostEqualUlps(t1.x, t2.x)) {
                SkDebugf("%s [%d,%d] x!= t1=%1.9g (%1.9g,%1.9g) t2=%1.9g (%1.9g,%1.9g)\n",
                    __FUNCTION__, (int)index, pt, tt1, t1.x, t1.y, tt2, t2.x, t2.y);
                SkASSERT(0);
            }
            if (!AlmostEqualUlps(t1.y, t2.y)) {
                SkDebugf("%s [%d,%d] y!= t1=%1.9g (%1.9g,%1.9g) t2=%1.9g (%1.9g,%1.9g)\n",
                    __FUNCTION__, (int)index, pt, tt1, t1.x, t1.y, tt2, t2.x, t2.y);
                SkASSERT(0);
            }
            if (!t1.approximatelyEqual(lineQuadTests[index].expected[0])
                    && (lineQuadTests[index].result == 1
                    || !t1.approximatelyEqual(lineQuadTests[index].expected[1]))) {
                SkDebugf("%s t1=(%1.9g,%1.9g)\n", __FUNCTION__, t1.x, t1.y);
                SkASSERT(0);
            }
        }
    }
}

static void testLineIntersect(State4& state, const Quadratic& quad, const _Line& line,
        const double x, const double y) {
    char pathStr[1024];
    bzero(pathStr, sizeof(pathStr));
    char* str = pathStr;
    str += sprintf(str, "    path.moveTo(%1.9g, %1.9g);\n", quad[0].x, quad[0].y);
    str += sprintf(str, "    path.quadTo(%1.9g, %1.9g, %1.9g, %1.9g);\n", quad[1].x, quad[1].y, quad[2].x, quad[2].y);
    str += sprintf(str, "    path.moveTo(%1.9g, %1.9g);\n", line[0].x, line[0].y);
    str += sprintf(str, "    path.lineTo(%1.9g, %1.9g);\n", line[1].x, line[1].y);

    Intersections intersections;
    bool flipped = false;
    int result = doIntersect(intersections, quad, line, flipped);
    bool found = false;
    for (int index = 0; index < result; ++index) {
        double quadT = intersections.fT[0][index];
        double quadX, quadY;
        xy_at_t(quad, quadT, quadX, quadY);
        double lineT = intersections.fT[1][index];
        double lineX, lineY;
        xy_at_t(line, lineT, lineX, lineY);
        if (fabs(quadX - lineX) < FLT_EPSILON && fabs(quadY - lineY) < FLT_EPSILON
                && fabs(x - lineX) < FLT_EPSILON && fabs(y - lineY) < FLT_EPSILON) {
            found = true;
        }
    }
    SkASSERT(found);
    state.testsRun++;
}


// find a point on a quad by choosing a t from 0 to 1
// create a vertical span above and below the point
// verify that intersecting the vertical span and the quad returns t
// verify that a vertical span starting at quad[0] intersects at t=0
// verify that a vertical span starting at quad[2] intersects at t=1
static void* testQuadLineIntersectMain(void* data)
{
    SkASSERT(data);
    State4& state = *(State4*) data;
    do {
        int ax = state.a & 0x03;
        int ay = state.a >> 2;
        int bx = state.b & 0x03;
        int by = state.b >> 2;
        int cx = state.c & 0x03;
        int cy = state.c >> 2;
        Quadratic quad = {{ax, ay}, {bx, by}, {cx, cy}};
        Quadratic reduced;
        int order = reduceOrder(quad, reduced, kReduceOrder_TreatAsFill);
        if (order < 3) {
            continue; // skip degenerates
        }
        for (int tIndex = 0; tIndex <= 4; ++tIndex) {
            double x, y;
            xy_at_t(quad, tIndex / 4.0, x, y);
            for (int h = -2; h <= 2; ++h) {
                for (int v = -2; v <= 2; ++v) {
                    if (h == v && abs(h) != 1) {
                        continue;
                    }
                    _Line line = {{x - h, y - v}, {x, y}};
                    testLineIntersect(state, quad, line, x, y);
                    _Line line2 = {{x, y}, {x + h, y + v}};
                    testLineIntersect(state, quad, line2, x, y);
                    _Line line3 = {{x - h, y - v}, {x + h, y + v}};
                    testLineIntersect(state, quad, line3, x, y);
                }
            }
        }
    } while (runNextTestSet(state));
    return NULL;
}

void QuadLineIntersectThreaded_Test(int& testsRun)
{
    SkDebugf("%s\n", __FUNCTION__);
    const char testStr[] = "testQuadLineIntersect";
    initializeTests(testStr, sizeof(testStr));
    int testsStart = testsRun;
    for (int a = 0; a < 16; ++a) {
        for (int b = 0 ; b < 16; ++b) {
            for (int c = 0 ; c < 16; ++c) {
                testsRun += dispatchTest4(testQuadLineIntersectMain,
                        a, b, c, 0);
            }
            if (!gRunTestsInOneThread) SkDebugf(".");
        }
        if (!gRunTestsInOneThread) SkDebugf("%d", a);
    }
    testsRun += waitForCompletion();
    SkDebugf("\n%s tests=%d total=%d\n", __FUNCTION__, testsRun - testsStart, testsRun);
}
