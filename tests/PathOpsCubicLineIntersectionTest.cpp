/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkIntersections.h"
#include "SkPathOpsCubic.h"
#include "SkPathOpsLine.h"
#include "SkReduceOrder.h"
#include "Test.h"

static struct lineCubic {
    SkDCubic cubic;
    SkDLine line;
} lineCubicTests[] = {
    {{{{1, 2}, {2, 6}, {2, 0}, {1, 0}}}, {{{1, 0}, {1, 2}}}},
    {{{{0, 0}, {0, 1}, {0, 1}, {1, 1}}}, {{{0, 1}, {1, 0}}}},
};

static const size_t lineCubicTests_count = sizeof(lineCubicTests) / sizeof(lineCubicTests[0]);

static void CubicLineIntersectionTest(skiatest::Reporter* reporter) {
    for (size_t index = 0; index < lineCubicTests_count; ++index) {
        int iIndex = static_cast<int>(index);
        const SkDCubic& cubic = lineCubicTests[index].cubic;
        const SkDLine& line = lineCubicTests[index].line;
        SkReduceOrder reduce1;
        SkReduceOrder reduce2;
        int order1 = reduce1.reduce(cubic, SkReduceOrder::kNo_Quadratics,
                SkReduceOrder::kFill_Style);
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
                SkDPoint xy1 = cubic.xyAtT(tt1);
                double tt2 = i[1][pt];
                SkDPoint xy2 = line.xyAtT(tt2);
                if (!xy1.approximatelyEqual(xy2)) {
                    SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                        __FUNCTION__, iIndex, pt, tt1, xy1.fX, xy1.fY, tt2, xy2.fX, xy2.fY);
                }
                REPORTER_ASSERT(reporter, xy1.approximatelyEqual(xy2));
            }
        }
    }
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("PathOpsCubicLineIntersection", CubicLineIntersectionTestClass, \
        CubicLineIntersectionTest)
