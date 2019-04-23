/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/pathops/SkIntersections.h"
#include "src/pathops/SkPathOpsCubic.h"
#include "src/pathops/SkPathOpsLine.h"
#include "src/pathops/SkReduceOrder.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

#include <utility>

struct lineCubic {
    CubicPts cubic;
    SkDLine line;
};

static lineCubic failLineCubicTests[] = {
    {{{{37.5273438,-1.44140625}, {37.8736992,-1.69921875}, {38.1640625,-2.140625},
            {38.3984375,-2.765625}}},
            {{{40.625,-5.7890625}, {37.7109375,1.3515625}}}},
};

static const size_t failLineCubicTests_count = SK_ARRAY_COUNT(failLineCubicTests);

static void testFail(skiatest::Reporter* reporter, int iIndex) {
    const CubicPts& cuPts = failLineCubicTests[iIndex].cubic;
    SkDCubic cubic;
    cubic.debugSet(cuPts.fPts);
    SkASSERT(ValidCubic(cubic));
    const SkDLine& line = failLineCubicTests[iIndex].line;
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
        REPORTER_ASSERT(reporter, roots == 0);
    }
}

static lineCubic lineCubicTests[] = {
    {{{{0, 6}, {1.0851458311080933, 4.3722810745239258}, {1.5815209150314331, 3.038947582244873}, {1.9683018922805786, 1.9999997615814209}}},
     {{{3,2}, {1,2}}}},

    {{{{0.468027353,4}, {1.06734705,1.33333337}, {1.36700678,0}, {3,0}}},
    {{{2,1}, {0,1}}}},

    {{{{-634.60540771484375, -481.262939453125}, {266.2696533203125, -752.70867919921875},
            {-751.8370361328125, -317.37921142578125}, {-969.7427978515625, 824.7255859375}}},
            {{{-287.9506133720805678, -557.1376476615772617},
            {-285.9506133720805678, -557.1376476615772617}}}},

    {{{{36.7184372,0.888650894}, {36.7184372,0.888650894}, {35.1233864,0.554015458},
            {34.5114098,-0.115255356}}}, {{{35.4531212,0}, {31.9375,0}}}},

    {{{{421, 378}, {421, 380.209137f}, {418.761414f, 382}, {416, 382}}},
            {{{320, 378}, {421, 378.000031f}}}},

    {{{{416, 383}, {418.761414f, 383}, {421, 380.761414f}, {421, 378}}},
            {{{320, 378}, {421, 378.000031f}}}},

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

static int doIntersect(SkIntersections& intersections, const SkDCubic& cubic, const SkDLine& line) {
    int result;
    bool flipped = false;
    if (line[0].fX == line[1].fX) {
        double top = line[0].fY;
        double bottom = line[1].fY;
        flipped = top > bottom;
        if (flipped) {
            using std::swap;
            swap(top, bottom);
        }
        result = intersections.vertical(cubic, top, bottom, line[0].fX, flipped);
    } else if (line[0].fY == line[1].fY) {
        double left = line[0].fX;
        double right = line[1].fX;
        flipped = left > right;
        if (flipped) {
            using std::swap;
            swap(left, right);
        }
        result = intersections.horizontal(cubic, left, right, line[0].fY, flipped);
    } else {
        intersections.intersect(cubic, line);
        result = intersections.used();
    }
    return result;
}

static void testOne(skiatest::Reporter* reporter, int iIndex) {
    const CubicPts& cuPts = lineCubicTests[iIndex].cubic;
    SkDCubic cubic;
    cubic.debugSet(cuPts.fPts);
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
        int roots = doIntersect(i, cubic, line);
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
#if ONE_OFF_DEBUG
        double cubicT = i[0][0];
        SkDPoint prev = cubic.ptAtT(cubicT * 2 - 1);
        SkDPoint sect = cubic.ptAtT(cubicT);
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
}

DEF_TEST(PathOpsFailCubicLineIntersection, reporter) {
    for (size_t index = 0; index < failLineCubicTests_count; ++index) {
        int iIndex = static_cast<int>(index);
        testFail(reporter, iIndex);
        reporter->bumpTestCount();
    }
}

DEF_TEST(PathOpsCubicLineIntersection, reporter) {
    for (size_t index = 0; index < lineCubicTests_count; ++index) {
        int iIndex = static_cast<int>(index);
        testOne(reporter, iIndex);
        reporter->bumpTestCount();
    }
}

DEF_TEST(PathOpsCubicLineIntersectionOneOff, reporter) {
    int iIndex = 0;
    testOne(reporter, iIndex);
    const CubicPts& cuPts = lineCubicTests[iIndex].cubic;
    SkDCubic cubic;
    cubic.debugSet(cuPts.fPts);
    const SkDLine& line = lineCubicTests[iIndex].line;
    SkIntersections i;
    i.intersect(cubic, line);
    SkASSERT(i.used() == 1);
}
