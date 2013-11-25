/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkIntersections.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkReduceOrder.h"
#include "Test.h"

static struct lineCubic {
    SkDCubic cubic;
    SkDLine line;
} lineCubicTests[] = {
    {{{{154,715}, {151.238571,715}, {149,712.761414}, {149,710}}},
            {{{149,675}, {149,710.001465}}}},

    {{{{0,1}, {1,6}, {4,1}, {4,3}}},
            {{{6,1}, {1,4}}}},

    {{{{0,1}, {2,6}, {4,1}, {5,4}}},
            {{{6,2}, {1,4}}}},

    {{{{0,4}, {3,4}, {6,2}, {5,2}}},
            {{{4,3}, {2,6}}}},
#if 0
    {{{{258, 122}, {260.761414, 122}, { 263, 124.238579}, {263, 127}}},
            {{{259.82843, 125.17157}, {261.535522, 123.46447}}}},
#endif
    {{{{1006.6951293945312,291}, {1023.263671875,291}, {1033.8402099609375,304.43145751953125},
            {1030.318359375,321}}},
            {{{979.30487060546875,561}, {1036.695068359375,291}}}},
    {{{{259.30487060546875,561}, {242.73631286621094,561}, {232.15980529785156,547.56854248046875},
            {235.68154907226562,531}}},
            {{{286.69512939453125,291}, {229.30485534667969,561}}}},
    {{{{1, 2}, {2, 6}, {2, 0}, {1, 0}}}, {{{1, 0}, {1, 2}}}},
    {{{{0, 0}, {0, 1}, {0, 1}, {1, 1}}}, {{{0, 1}, {1, 0}}}},
};

static const size_t lineCubicTests_count = SK_ARRAY_COUNT(lineCubicTests);

static void testOne(skiatest::Reporter* reporter, int iIndex) {
    const SkDCubic& cubic = lineCubicTests[iIndex].cubic;
    SkASSERT(ValidCubic(cubic));
    const SkDLine& line = lineCubicTests[iIndex].line;
    SkASSERT(ValidLine(line));
    SkReduceOrder reduce1;
    SkReduceOrder reduce2;
    int order1 = reduce1.reduce(cubic, SkReduceOrder::kNo_Quadratics);
    int order2 = reduce2.reduce(line);
    if (order1 < 4) {
        SkDebugf("[%d] cubic order=%d\n", iIndex, order1);
        REPORTER_ASSERT(reporter, 0);
    }
    if (order2 < 2) {
        SkDebugf("[%d] line order=%d\n", iIndex, order2);
        REPORTER_ASSERT(reporter, 0);
    }
    if (order1 == 4 && order2 == 2) {
        SkIntersections i;
        int roots = i.intersect(cubic, line);
        for (int pt = 0; pt < roots; ++pt) {
            double tt1 = i[0][pt];
            SkDPoint xy1 = cubic.ptAtT(tt1);
            double tt2 = i[1][pt];
            SkDPoint xy2 = line.ptAtT(tt2);
            if (!xy1.approximatelyEqual(xy2)) {
                SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                    __FUNCTION__, iIndex, pt, tt1, xy1.fX, xy1.fY, tt2, xy2.fX, xy2.fY);
            }
            REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
        }
    }
}

static void PathOpsCubicLineIntersectionTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < lineCubicTests_count; ++index) {
        int iIndex = static_cast<int>(index);
        testOne(reporter, iIndex);
        reporter->bumpTestCount();
    }
}

static void PathOpsCubicLineIntersectionOneOffTest(skiatest::Reporter* reporter) {
    int iIndex = 0;
    testOne(reporter, iIndex);
    const SkDCubic& cubic = lineCubicTests[iIndex].cubic;
    const SkDLine& line = lineCubicTests[iIndex].line;
    SkIntersections i;
    i.intersect(cubic, line);
    SkASSERT(i.used() == 1);
#if ONE_OFF_DEBUG
    double cubicT = i[0][0];
    SkDPoint prev = cubic.ptAtT(cubicT * 2 - 1);
    SkDPoint sect = cubic.ptAtT(cubicT);
    double left[3] = { line.isLeft(prev), line.isLeft(sect), line.isLeft(cubic[3]) };
    SkDebugf("cubic=(%1.9g, %1.9g, %1.9g)\n", left[0], left[1], left[2]);
    SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}},\n", prev.fX, prev.fY, sect.fX, sect.fY);
    SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}},\n", sect.fX, sect.fY, cubic[3].fX, cubic[3].fY);
    SkDPoint prevL = line.ptAtT(i[1][0] - 0.0000007);
    SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}},\n", prevL.fX, prevL.fY, i.pt(0).fX, i.pt(0).fY);
    SkDPoint nextL = line.ptAtT(i[1][0] + 0.0000007);
    SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}},\n", i.pt(0).fX, i.pt(0).fY, nextL.fX, nextL.fY);
    SkDebugf("prevD=%1.9g dist=%1.9g nextD=%1.9g\n", prev.distance(nextL),
            sect.distance(i.pt(0)), cubic[3].distance(prevL));
#endif
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsCubicLineIntersectionTest)

DEFINE_TESTCLASS_SHORT(PathOpsCubicLineIntersectionOneOffTest)
