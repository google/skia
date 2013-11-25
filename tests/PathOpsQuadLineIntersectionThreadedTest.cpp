/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsExtendedTest.h"
#include "PathOpsThreadedCommon.h"
#include "SkIntersections.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"
#include "SkReduceOrder.h"

static int doIntersect(SkIntersections& intersections, const SkDQuad& quad, const SkDLine& line,
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
        result = intersections.vertical(quad, top, bottom, line[0].fX, flipped);
    } else if (line[0].fY == line[1].fY) {
        double left = line[0].fX;
        double right = line[1].fX;
        flipped = left > right;
        if (flipped) {
            SkTSwap<double>(left, right);
        }
        result = intersections.horizontal(quad, left, right, line[0].fY, flipped);
    } else {
        intersections.intersect(quad, line);
        result = intersections.used();
    }
    return result;
}

static void testLineIntersect(skiatest::Reporter* reporter, const SkDQuad& quad,
                              const SkDLine& line, const double x, const double y) {
    char pathStr[1024];
    sk_bzero(pathStr, sizeof(pathStr));
    char* str = pathStr;
    str += sprintf(str, "    path.moveTo(%1.9g, %1.9g);\n", quad[0].fX, quad[0].fY);
    str += sprintf(str, "    path.quadTo(%1.9g, %1.9g, %1.9g, %1.9g);\n", quad[1].fX,
            quad[1].fY, quad[2].fX, quad[2].fY);
    str += sprintf(str, "    path.moveTo(%1.9g, %1.9g);\n", line[0].fX, line[0].fY);
    str += sprintf(str, "    path.lineTo(%1.9g, %1.9g);\n", line[1].fX, line[1].fY);

    SkIntersections intersections;
    bool flipped = false;
    int result = doIntersect(intersections, quad, line, flipped);
    bool found = false;
    for (int index = 0; index < result; ++index) {
        double quadT = intersections[0][index];
        SkDPoint quadXY = quad.ptAtT(quadT);
        double lineT = intersections[1][index];
        SkDPoint lineXY = line.ptAtT(lineT);
        if (quadXY.approximatelyEqual(lineXY)) {
            found = true;
        }
    }
    REPORTER_ASSERT(reporter, found);
}


// find a point on a quad by choosing a t from 0 to 1
// create a vertical span above and below the point
// verify that intersecting the vertical span and the quad returns t
// verify that a vertical span starting at quad[0] intersects at t=0
// verify that a vertical span starting at quad[2] intersects at t=1
static void testQuadLineIntersectMain(PathOpsThreadState* data)
{
    PathOpsThreadState& state = *data;
    REPORTER_ASSERT(state.fReporter, data);
    int ax = state.fA & 0x03;
    int ay = state.fA >> 2;
    int bx = state.fB & 0x03;
    int by = state.fB >> 2;
    int cx = state.fC & 0x03;
    int cy = state.fC >> 2;
    SkDQuad quad = {{{(double) ax, (double) ay}, {(double) bx, (double) by},
            {(double) cx, (double) cy}}};
    SkReduceOrder reducer;
    int order = reducer.reduce(quad);
    if (order < 3) {
        return;
    }
    for (int tIndex = 0; tIndex <= 4; ++tIndex) {
        SkDPoint xy = quad.ptAtT(tIndex / 4.0);
        for (int h = -2; h <= 2; ++h) {
            for (int v = -2; v <= 2; ++v) {
                if (h == v && abs(h) != 1) {
                    continue;
                }
                double x = xy.fX;
                double y = xy.fY;
                SkDLine line = {{{x - h, y - v}, {x, y}}};
                testLineIntersect(state.fReporter, quad, line, x, y);
                state.fReporter->bumpTestCount();
                SkDLine line2 = {{{x, y}, {x + h, y + v}}};
                testLineIntersect(state.fReporter, quad, line2, x, y);
                state.fReporter->bumpTestCount();
                SkDLine line3 = {{{x - h, y - v}, {x + h, y + v}}};
                testLineIntersect(state.fReporter, quad, line3, x, y);
                state.fReporter->bumpTestCount();
            }
        }
    }
}

static void PathOpsQuadLineIntersectionThreadedTest(skiatest::Reporter* reporter)
{
    int threadCount = initializeTests(reporter, "testQuadLineIntersect");
    PathOpsThreadedTestRunner testRunner(reporter, threadCount);
    for (int a = 0; a < 16; ++a) {
        for (int b = 0 ; b < 16; ++b) {
            for (int c = 0 ; c < 16; ++c) {
                    *testRunner.fRunnables.append() = SkNEW_ARGS(PathOpsThreadedRunnable,
                            (&testQuadLineIntersectMain, a, b, c, 0, &testRunner));
            }
            if (!reporter->allowExtendedTest()) goto finish;
        }
    }
finish:
    testRunner.render();
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsQuadLineIntersectionThreadedTest)
