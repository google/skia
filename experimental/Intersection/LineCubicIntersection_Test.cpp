/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "CurveUtilities.h"
#include "Intersection_Tests.h"
#include "Intersections.h"
#include "TestUtilities.h"

struct lineCubic {
    Cubic cubic;
    _Line line;
} lineCubicTests[] = {
    {{{0, 0}, {0, 1}, {0, 1}, {1, 1}}, {{0, 1}, {1, 0}}}
};

size_t lineCubicTests_count = sizeof(lineCubicTests) / sizeof(lineCubicTests[0]);

const int firstLineCubicIntersectionTest = 0;

void LineCubicIntersection_Test() {
    for (size_t index = firstLineCubicIntersectionTest; index < lineCubicTests_count; ++index) {
        const Cubic& cubic = lineCubicTests[index].cubic;
        const _Line& line = lineCubicTests[index].line;
        Cubic reduce1;
        _Line reduce2;
        int order1 = reduceOrder(cubic, reduce1, kReduceOrder_NoQuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        int order2 = reduceOrder(line, reduce2);
        if (order1 < 4) {
            printf("[%d] cubic order=%d\n", (int) index, order1);
        }
        if (order2 < 2) {
            printf("[%d] line order=%d\n", (int) index, order2);
        }
        if (order1 == 4 && order2 == 2) {
            Intersections i;
            double* range1 = i.fT[0];
            double* range2 = i.fT[1];
            int roots = intersect(reduce1, reduce2, i);
            for (int pt = 0; pt < roots; ++pt) {
                double tt1 = range1[pt];
                double tx1, ty1;
                xy_at_t(cubic, tt1, tx1, ty1);
                double tt2 = range2[pt];
                double tx2, ty2;
                xy_at_t(line, tt2, tx2, ty2);
                if (!AlmostEqualUlps(tx1, tx2)) {
                    printf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                        __FUNCTION__, (int)index, pt, tt1, tx1, ty1, tt2, tx2, ty2);
                }
                if (!AlmostEqualUlps(ty1, ty2)) {
                    printf("%s [%d,%d] y!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                        __FUNCTION__, (int)index, pt, tt1, tx1, ty1, tt2, tx2, ty2);
                }
            }
        }
    }
}
