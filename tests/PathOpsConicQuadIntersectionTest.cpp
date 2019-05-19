/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/pathops/SkIntersections.h"
#include "src/pathops/SkPathOpsConic.h"
#include "src/pathops/SkPathOpsQuad.h"
#include "src/pathops/SkReduceOrder.h"
#include "tests/PathOpsTestCommon.h"
#include "tests/Test.h"

static struct conicQuad {
    ConicPts conic;
    QuadPts quad;
} conicQuadTests[] = {
   {{{{{0.00000000000000000, -1.8135968446731567 },
    {0.00000000000000000, -1.0033817291259766 },
    {-0.0073835160583257675, 0.00000000000000000 }}}, 2.26585215e+11f},
    {{{0.00000000000000000, -1.0000113248825073 },
    {-2.4824290449032560e-05, -1.0000115633010864 },
    {-0.0073835160583257675, 0.00000000000000000 }}}},

   {{{{{494.348663,224.583771}, {494.365143,224.633194}, {494.376404,224.684067}}}, 0.998645842f},
    {{{494.30481,224.474213}, {494.334961,224.538284}, {494.355774,224.605927}}}},

   {{{{{494.348663,224.583771}, {494.365143,224.633194}, {494.376404,224.684067}}}, 0.998645842f},
    {{{494.355774f, 224.605927f}, {494.363708f, 224.631714f}, {494.370148f, 224.657471f}}}},
};

static const int conicQuadTests_count = (int) SK_ARRAY_COUNT(conicQuadTests);

static void conicQuadIntersection(skiatest::Reporter* reporter, int index) {
    const ConicPts& c = conicQuadTests[index].conic;
    SkDConic conic;
    conic.debugSet(c.fPts.fPts, c.fWeight);
    SkASSERT(ValidConic(conic));
    const QuadPts& q = conicQuadTests[index].quad;
    SkDQuad quad;
    quad.debugSet(q.fPts);
    SkASSERT(ValidQuad(quad));
    SkReduceOrder reduce1;
    SkReduceOrder reduce2;
    int order1 = reduce2.reduce(conic.fPts);
    int order2 = reduce1.reduce(quad);
    if (order2 != 3) {
        SkDebugf("[%d] conic order=%d\n", index, order1);
        REPORTER_ASSERT(reporter, 0);
    }
    if (order1 != 3) {
        SkDebugf("[%d] quad order=%d\n", index, order2);
        REPORTER_ASSERT(reporter, 0);
    }
    SkIntersections i;
    int roots = i.intersect(conic, quad);
    for (int pt = 0; pt < roots; ++pt) {
        double tt1 = i[0][pt];
        SkDPoint xy1 = conic.ptAtT(tt1);
        double tt2 = i[1][pt];
        SkDPoint xy2 = quad.ptAtT(tt2);
        if (!xy1.approximatelyEqual(xy2)) {
            SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                __FUNCTION__, index, pt, tt1, xy1.fX, xy1.fY, tt2, xy2.fX, xy2.fY);
        }
        REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
    }
    reporter->bumpTestCount();
}

DEF_TEST(PathOpsConicQuadIntersection, reporter) {
    for (int index = 0; index < conicQuadTests_count; ++index) {
        conicQuadIntersection(reporter, index);
        reporter->bumpTestCount();
    }
}

DEF_TEST(PathOpsConicQuadIntersectionOneOff, reporter) {
    conicQuadIntersection(reporter, 0);
}
