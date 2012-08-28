/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "Intersection_Tests.h"
#include "QuadraticIntersection_TestData.h"

static const Quadratic testSet[] = {
    // data for oneOffTest
    {{8.0000000000000071, 8.0000000000000071},
     {8.7289570079366854, 8.7289570079366889},
     {9.3914917259458743, 9.0593802763083691}},
    {{8.0000000000000142, 8.0000000000000142},
     {8.1250000000000107, 8.1250000000000071},
     {8.2500000000000071, 8.2187500000000053}},
     // data for oneAtEndTest
    {{0.91292418204644155, 0.41931201426549197},
     {0.70491388044579517, 0.64754305977710236},
     {0,                   1                  }},
    {{0.21875,             0.765625           },
     {0.125,               0.875              },
     {0,                   1                  }}
};

static void oneAtEndTest() {
    const Quadratic& quad1 = testSet[2];
    const Quadratic& quad2 = testSet[3];
    double minT = 0;
    double maxT = 1;
    bezier_clip(quad1, quad2, minT, maxT);
}


static void oneOffTest() {
    const Quadratic& quad1 = testSet[0];
    const Quadratic& quad2 = testSet[1];
    double minT = 0;
    double maxT = 1;
    bezier_clip(quad1, quad2, minT, maxT);
}

static void standardTestCases() {
    for (size_t index = 0; index < quadraticTests_count; ++index) {
        const Quadratic& quad1 = quadraticTests[index][0];
        const Quadratic& quad2 = quadraticTests[index][1];
        Quadratic reduce1, reduce2;
        int order1 = reduceOrder(quad1, reduce1);
        int order2 = reduceOrder(quad2, reduce2);
        if (order1 < 3) {
            printf("%s [%d] quad1 order=%d\n", __FUNCTION__, (int)index, order1);
        }
        if (order2 < 3) {
            printf("%s [%d] quad2 order=%d\n", __FUNCTION__, (int)index, order2);
        }
        if (order1 == 3 && order2 == 3) {
            double minT = 0;
            double maxT = 1;
            bezier_clip(reduce1, reduce2, minT, maxT);
        }
    }
}

void QuadraticBezierClip_Test() {
    oneAtEndTest();
    oneOffTest();
    standardTestCases();
}
