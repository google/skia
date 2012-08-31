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
            Intersections intersections;
            intersect(reduce1, reduce2, intersections);
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
                }
            }
        }
    }
}

static const Quadratic testSet[] = {
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
            Intersections intersections;
            intersect(quad1, quad2, intersections);
            if (!intersections.intersected()) {
                SkDebugf("%s no intersection!\n", __FUNCTION__);
            }
            for (int pt = 0; pt < intersections.used(); ++pt) {
                double tt1 = intersections.fT[0][pt];
                double tx1, ty1;
                xy_at_t(quad1, tt1, tx1, ty1);
                double tt2 = intersections.fT[1][pt];
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
    {{8, 8}, {10, 10}, {8, -10}},
    {{8, -10}, {10, 10}, {8, 8}},
};

const size_t coincidentTestSetCount = sizeof(coincidentTestSet) / sizeof(coincidentTestSet[0]);

static void coincidentTest() {
    for (size_t testIndex = 0; testIndex < coincidentTestSetCount - 1; testIndex += 2) {
        const Quadratic& quad1 = coincidentTestSet[testIndex];
        const Quadratic& quad2 = coincidentTestSet[testIndex + 1];
        Intersections intersections;
        intersect(quad1, quad2, intersections);
        SkASSERT(intersections.coincidentUsed() == 2);
        for (int pt = 0; pt < intersections.coincidentUsed(); ++pt) {
            double tt1 = intersections.fT[0][pt];
            double tx1, ty1;
            xy_at_t(quad1, tt1, tx1, ty1);
            double tt2 = intersections.fT[1][pt];
            double tx2, ty2;
            xy_at_t(quad2, tt2, tx2, ty2);
            SkDebugf("%s [%d,%d] t1=%g (%g,%g) t2=%g (%g,%g)\n",
                __FUNCTION__, (int)testIndex, pt, tt1, tx1, ty1, tt2, tx2, ty2);
        }
    }
}

void QuadraticIntersection_Test() {
    coincidentTest();
    oneOffTest();
    standardTestCases();
}
