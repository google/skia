/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "PathOpsTestCommon.h"
#include "SkIntersections.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsQuad.h"
#include "SkReduceOrder.h"
#include "Test.h"

static struct lineCubic {
    SkDCubic cubic;
    SkDQuad quad;
    int answerCount;
    SkDPoint answers[2];
} quadCubicTests[] = {
    {{{{10,234}, {10,229.58172607421875}, {13.581720352172852,226}, {18,226}}},
        {{{18,226}, {14.686291694641113,226}, {12.342399597167969,228.3424072265625}}}, 1,
        {{18,226}, {0,0}}},
    {{{{10,234}, {10,229.58172607421875}, {13.581720352172852,226}, {18,226}}},
        {{{12.342399597167969,228.3424072265625}, {10,230.68629455566406}, {10,234}}}, 1,
        {{10,234}, {0,0}}},
};

static const size_t quadCubicTests_count = SK_ARRAY_COUNT(quadCubicTests);

static void PathOpsCubicQuadIntersectionTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < quadCubicTests_count; ++index) {
        int iIndex = static_cast<int>(index);
        const SkDCubic& cubic = quadCubicTests[index].cubic;
        SkASSERT(ValidCubic(cubic));
        const SkDQuad& quad = quadCubicTests[index].quad;
        SkASSERT(ValidQuad(quad));
        SkReduceOrder reduce1;
        SkReduceOrder reduce2;
        int order1 = reduce1.reduce(cubic, SkReduceOrder::kNo_Quadratics,
                SkReduceOrder::kFill_Style);
        int order2 = reduce2.reduce(quad, SkReduceOrder::kFill_Style);
        if (order1 != 4) {
            SkDebugf("[%d] cubic order=%d\n", iIndex, order1);
            REPORTER_ASSERT(reporter, 0);
        }
        if (order2 != 3) {
            SkDebugf("[%d] quad order=%d\n", iIndex, order2);
            REPORTER_ASSERT(reporter, 0);
        }
        SkIntersections i;
        int roots = i.intersect(cubic, quad);
        SkASSERT(roots == quadCubicTests[index].answerCount);
        for (int pt = 0; pt < roots; ++pt) {
            double tt1 = i[0][pt];
            SkDPoint xy1 = cubic.ptAtT(tt1);
            double tt2 = i[1][pt];
            SkDPoint xy2 = quad.ptAtT(tt2);
            if (!xy1.approximatelyEqual(xy2)) {
                SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                    __FUNCTION__, iIndex, pt, tt1, xy1.fX, xy1.fY, tt2, xy2.fX, xy2.fY);
            }
            REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
            bool found = false;
            for (int idx2 = 0; idx2 < quadCubicTests[index].answerCount; ++idx2) {
                found |= quadCubicTests[index].answers[idx2].approximatelyEqual(xy1);
            }
            REPORTER_ASSERT(reporter, found);
        }
        reporter->bumpTestCount();
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS_SHORT(PathOpsCubicQuadIntersectionTest)
