/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "CubicIntersection_TestData.h"
#include "Intersection_Tests.h"

void CubicBezierClip_Test() {
    for (size_t index = 0; index < tests_count; ++index) {
        const Cubic& cubic1 = tests[index][0];
        const Cubic& cubic2 = tests[index][1];
        Cubic reduce1, reduce2;
        int order1 = reduceOrder(cubic1, reduce1, kReduceOrder_NoQuadraticsAllowed);
        int order2 = reduceOrder(cubic2, reduce2, kReduceOrder_NoQuadraticsAllowed);
        if (order1 < 4) {
            printf("%s [%d] cubic1 order=%d\n", __FUNCTION__, (int) index, order1);
        }
        if (order2 < 4) {
            printf("%s [%d] cubic2 order=%d\n", __FUNCTION__, (int) index, order2);
        }
        if (order1 == 4 && order2 == 4) {
            double minT = 0;
            double maxT = 1;
            bezier_clip(reduce1, reduce2, minT, maxT);
        }
    }
}
