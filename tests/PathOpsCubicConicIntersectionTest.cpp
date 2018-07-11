/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkIntersections.h"
#include "SkPathOpsConic.h"
#include "SkPathOpsCubic.h"
#include "SkReduceOrder.h"
#include "Test.h"

static struct cubicConic {
    CubicPts cubic;
    ConicPts conic;
} cubicConicTests[] = {
#if 0
// FIXME: this triggers an assert in bool SkTSect::extractCoincident() at
// SkOPASSERT(oppStartT < oppEndT);
// Throwing an error here breaks one test, but only in release.
// More work to be done to figure this out.
    {{{{2.1883804947719909e-05, 3.6366123822517693e-05 },
        {2.9145950975362211e-05, 2.9117207304807380e-05 },
        {2.9113532946212217e-05, 2.9173743314458989e-05 },
        {0.00000000000000000, 5.8282588724978268e-05 }}},
    {{{{0.00000000000000000, 5.8282581449020654e-05 },
        {0.00000000000000000, 5.8282563259126619e-05 },
        {5.8282588724978268e-05, 0.00000000000000000 }}}, 53684.6563f}},
#endif

    {{{{188.60000610351562, 2041.5999755859375}, {188.60000610351562, 2065.39990234375},
        {208, 2084.800048828125}, {231.80000305175781, 2084.800048828125}}},
    {{{{231.80000305175781, 2084.800048828125}, {188.60000610351562, 2084.800048828125},
        {188.60000610351562, 2041.5999755859375}}}, 0.707107008f}},

    {{{{231.80000305175781, 2084.800048828125}, {255.60000610351562, 2084.800048828125},
        {275, 2065.39990234375}, {275, 2041.5999755859375}}},
    {{{{275, 2041.5999755859375}, {275, 2084.800048828125},
        {231.80000305175781, 2084.800048828125}}}, 0.707107008f}},
};

static const int cubicConicTests_count = (int) SK_ARRAY_COUNT(cubicConicTests);

static void cubicConicIntersection(skiatest::Reporter* reporter, int index) {
    const CubicPts& cu = cubicConicTests[index].cubic;
    SkDCubic cubic;
    cubic.debugSet(cu.fPts);
    SkASSERT(ValidCubic(cubic));
    const ConicPts& co = cubicConicTests[index].conic;
    SkDConic conic;
    conic.debugSet(co.fPts.fPts, co.fWeight);
    SkASSERT(ValidConic(conic));
    SkReduceOrder reduce1;
    SkReduceOrder reduce2;
    int order1 = reduce1.reduce(cubic, SkReduceOrder::kNo_Quadratics);
    int order2 = reduce2.reduce(conic.fPts);
    if (order1 != 4) {
        SkDebugf("[%d] cubic order=%d\n", index, order1);
        REPORTER_ASSERT(reporter, 0);
    }
    if (order2 != 3) {
        SkDebugf("[%d] conic order=%d\n", index, order2);
        REPORTER_ASSERT(reporter, 0);
    }
    SkIntersections i;
    int roots = i.intersect(cubic, conic);
    for (int pt = 0; pt < roots; ++pt) {
        double tt1 = i[0][pt];
        SkDPoint xy1 = cubic.ptAtT(tt1);
        double tt2 = i[1][pt];
        SkDPoint xy2 = conic.ptAtT(tt2);
        if (!xy1.approximatelyEqual(xy2)) {
            SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                __FUNCTION__, index, pt, tt1, xy1.fX, xy1.fY, tt2, xy2.fX, xy2.fY);
        }
        REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
    }
    reporter->bumpTestCount();
}

DEF_TEST(PathOpsCubicConicIntersection, reporter) {
    for (int index = 0; index < cubicConicTests_count; ++index) {
        cubicConicIntersection(reporter, index);
        reporter->bumpTestCount();
    }
}

DEF_TEST(PathOpsCubicConicIntersectionOneOff, reporter) {
    cubicConicIntersection(reporter, 0);
}
