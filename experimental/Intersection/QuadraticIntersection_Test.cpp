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
#include "QuadraticIntersection_TestData.h"
#include "TestUtilities.h"
#include "SkTypes.h"

const int firstQuadIntersectionTest = 9;

static void standardTestCases() {
    for (size_t index = firstQuadIntersectionTest; index < quadraticTests_count; ++index) {
        const Quadratic& quad1 = quadraticTests[index][0];
        const Quadratic& quad2 = quadraticTests[index][1];
        Quadratic reduce1, reduce2;
        int order1 = reduceOrder(quad1, reduce1);
        int order2 = reduceOrder(quad2, reduce2);
        if (order1 < 3) {
            printf("[%d] quad1 order=%d\n", (int) index, order1);
        }
        if (order2 < 3) {
            printf("[%d] quad2 order=%d\n", (int) index, order2);
        }
        if (order1 == 3 && order2 == 3) {
            Intersections intersections, intersections2;
            intersect(reduce1, reduce2, intersections);
            intersect2(reduce1, reduce2, intersections2);
            SkASSERT(intersections.used() == intersections2.used());
            if (intersections.intersected()) {
                for (int pt = 0; pt < intersections.used(); ++pt) {
                    double tt1 = intersections.fT[0][pt];
                    double tx1, ty1;
                    xy_at_t(quad1, tt1, tx1, ty1);
                    double tt2 = intersections.fT[1][pt];
                    double tx2, ty2;
                    xy_at_t(quad2, tt2, tx2, ty2);
                    if (!approximately_equal(tx1, tx2)) {
                        printf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                            __FUNCTION__, (int)index, pt, tt1, tx1, ty1, tt2, tx2, ty2);
                    }
                    if (!approximately_equal(ty1, ty2)) {
                        printf("%s [%d,%d] y!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                            __FUNCTION__, (int)index, pt, tt1, tx1, ty1, tt2, tx2, ty2);
                    }
                    tt1 = intersections2.fT[0][pt];
                    SkASSERT(approximately_equal(intersections.fT[0][pt], tt1));
                    tt2 = intersections2.fT[1][pt];
                    SkASSERT(approximately_equal(intersections.fT[1][pt], tt2));
                }
            }
        }
    }
}

static const Quadratic testSet[] = {
{{0, 0}, {1, 0}, {0, 3}},
{{1, 0}, {0, 1}, {1, 1}},
{{369.848602,145.680267}, {382.360413,121.298294}, {406.207703,121.298294}},
{{369.961151,137.980698}, {383.970093,121.298294}, {406.213287,121.298294}},
{{353.2948,194.351074}, {353.2948,173.767563}, {364.167572,160.819855}},
{{360.416077,166.795715}, {370.126831,147.872162}, {388.635406,147.872162}},
{{406.236359,121.254936}, {409.445679,121.254936}, {412.975952,121.789818}},
{{406.235992,121.254936}, {425.705902,121.254936}, {439.71994,137.087616}},

{{369.8543701171875, 145.66734313964844}, {382.36788940429688, 121.28203582763672}, {406.21844482421875, 121.28203582763672}},
{{369.96469116210938, 137.96672058105469}, {383.97555541992188, 121.28203582763672}, {406.2218017578125, 121.28203582763672}},

    {{369.850525, 145.675964}, {382.362915, 121.29287}, {406.211273, 121.29287}},
    {{369.962311, 137.976044}, {383.971893, 121.29287}, {406.216125, 121.29287}},

    {{400.121704, 149.468719}, {391.949493, 161.037186}, {391.949493, 181.202423}},
    {{391.946747, 181.839218}, {391.946747, 155.62442}, {406.115479, 138.855438}},
    {{360.048828125, 229.2578125}, {360.048828125, 224.4140625}, {362.607421875, 221.3671875}},
    {{362.607421875, 221.3671875}, {365.166015625, 218.3203125}, {369.228515625, 218.3203125}},
    {{8, 8}, {10, 10}, {8, -10}},
    {{8, 8}, {12, 12}, {14, 4}},
    {{8, 8}, {9, 9}, {10, 8}}
};

const size_t testSetCount = sizeof(testSet) / sizeof(testSet[0]);

static void oneOffTest() {
    for (size_t outer = 0; outer < testSetCount - 1; ++outer) {
        for (size_t inner = outer + 1; inner < testSetCount; ++inner) {
            const Quadratic& quad1 = testSet[outer];
            const Quadratic& quad2 = testSet[inner];
            double tt1, tt2;
            Intersections intersections2;
            intersect2(quad1, quad2, intersections2);
            for (int pt = 0; pt < intersections2.used(); ++pt) {
                tt1 = intersections2.fT[0][pt];
                double tx1, ty1;
                xy_at_t(quad1, tt1, tx1, ty1);
                int pt2 = intersections2.fFlip ? intersections2.used() - pt - 1 : pt;
                tt2 = intersections2.fT[1][pt2];
                double tx2, ty2;
                xy_at_t(quad2, tt2, tx2, ty2);
                if (!approximately_equal(tx1, tx2)) {
                    SkDebugf("%s [%d,%d] x!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                        __FUNCTION__, (int)index, pt, tt1, tx1, ty1, tt2, tx2, ty2);
                    SkASSERT(0);
                }
                if (!approximately_equal(ty1, ty2)) {
                    SkDebugf("%s [%d,%d] y!= t1=%g (%g,%g) t2=%g (%g,%g)\n",
                        __FUNCTION__, (int)index, pt, tt1, tx1, ty1, tt2, tx2, ty2);
                    SkASSERT(0);
                }
                SkDebugf("%s [%d][%d] t1=%1.9g (%1.9g, %1.9g) t2=%1.9g\n", __FUNCTION__,
                    outer, inner, tt1, tx1, tx2, tt2);
            }
        }
    }
}

static const Quadratic coincidentTestSet[] = {
    {{369.850525, 145.675964}, {382.362915, 121.29287}, {406.211273, 121.29287}},
    {{369.850525, 145.675964}, {382.362915, 121.29287}, {406.211273, 121.29287}},
    {{8, 8}, {10, 10}, {8, -10}},
    {{8, -10}, {10, 10}, {8, 8}},
};

const size_t coincidentTestSetCount = sizeof(coincidentTestSet) / sizeof(coincidentTestSet[0]);

static void coincidentTest() {
    for (size_t testIndex = 0; testIndex < coincidentTestSetCount - 1; testIndex += 2) {
        const Quadratic& quad1 = coincidentTestSet[testIndex];
        const Quadratic& quad2 = coincidentTestSet[testIndex + 1];
        Intersections intersections2;
        intersect2(quad1, quad2, intersections2);
        SkASSERT(intersections2.coincidentUsed() == 2);
        for (int pt = 0; pt < intersections2.coincidentUsed(); ++pt) {
            double tt1 = intersections2.fT[0][pt];
            double tt2 = intersections2.fT[1][pt];
            SkASSERT(approximately_equal(1, tt1) || approximately_zero(tt1));
            SkASSERT(approximately_equal(1, tt2) || approximately_zero(tt2));
        }
    }
}

void QuadraticIntersection_Test() {
    oneOffTest();
    coincidentTest();
    standardTestCases();
}
