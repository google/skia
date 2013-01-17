/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "CurveUtilities.h"
#include "CubicIntersection_TestData.h"
#include "Intersection_Tests.h"
#include "Intersections.h"
#include "TestUtilities.h"

const int firstCubicIntersectionTest = 9;

void CubicIntersection_Test() {
    for (size_t index = firstCubicIntersectionTest; index < tests_count; ++index) {
        const Cubic& cubic1 = tests[index][0];
        const Cubic& cubic2 = tests[index][1];
        Cubic reduce1, reduce2;
        int order1 = reduceOrder(cubic1, reduce1, kReduceOrder_NoQuadraticsAllowed);
        int order2 = reduceOrder(cubic2, reduce2, kReduceOrder_NoQuadraticsAllowed);
        if (order1 < 4) {
            printf("%s [%d] cubic1 order=%d\n", __FUNCTION__, (int) index, order1);
            continue;
        }
        if (order2 < 4) {
            printf("%s [%d] cubic2 order=%d\n", __FUNCTION__, (int) index, order2);
            continue;
        }
        if (implicit_matches(reduce1, reduce2)) {
            printf("%s [%d] coincident\n", __FUNCTION__, (int) index);
            continue;
        }
        Intersections tIntersections;
        intersect(reduce1, reduce2, tIntersections);
        if (!tIntersections.intersected()) {
            printf("%s [%d] no intersection\n", __FUNCTION__, (int) index);
            continue;
        }
        for (int pt = 0; pt < tIntersections.used(); ++pt) {
            double tt1 = tIntersections.fT[0][pt];
            double tx1, ty1;
            xy_at_t(cubic1, tt1, tx1, ty1);
            double tt2 = tIntersections.fT[1][pt];
            double tx2, ty2;
            xy_at_t(cubic2, tt2, tx2, ty2);
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

static void oneOff(const Cubic& cubic1, const Cubic& cubic2) {
    SkTDArray<Quadratic> quads1;
    cubic_to_quadratics(cubic1, calcPrecision(cubic1), quads1);
    for (int index = 0; index < quads1.count(); ++index) {
        const Quadratic& q = quads1[index];
        SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},\n", q[0].x, q[0].y, 
                 q[1].x, q[1].y,  q[2].x, q[2].y);
    }
    SkDebugf("\n");
    SkTDArray<Quadratic> quads2;
    cubic_to_quadratics(cubic2, calcPrecision(cubic2), quads2);
    for (int index = 0; index < quads2.count(); ++index) {
        const Quadratic& q = quads2[index];
        SkDebugf("{{%1.9g,%1.9g}, {%1.9g,%1.9g}, {%1.9g,%1.9g}},\n", q[0].x, q[0].y, 
                 q[1].x, q[1].y,  q[2].x, q[2].y);
    }
    SkDebugf("\n");
    Intersections intersections2;
    intersect2(cubic1, cubic2, intersections2);
    for (int pt = 0; pt < intersections2.used(); ++pt) {
        double tt1 = intersections2.fT[0][pt];
        double tx1, ty1;
        xy_at_t(cubic1, tt1, tx1, ty1);
        int pt2 = intersections2.fFlip ? intersections2.used() - pt - 1 : pt;
        double tt2 = intersections2.fT[1][pt2];
        double tx2, ty2;
        xy_at_t(cubic2, tt2, tx2, ty2);
        SkDebugf("%s t1=%1.9g (%1.9g, %1.9g) (%1.9g, %1.9g) t2=%1.9g\n", __FUNCTION__,
            tt1, tx1, ty1, tx2, ty2, tt2);
    }
}

static const Cubic testSet[] = {
{{67.426548091427676, 37.993772624988935}, {23.483695892376684, 90.476863174921306}, {35.597065061143162, 79.872482633158796}, {75.38634169631932, 18.244890038969412}},
{{61.336508189019057, 82.693132843213675}, {44.639380902349664, 54.074825790745592}, {16.815615499771951, 20.049704667203923}, {41.866884958868326, 56.735503699973002}},

{{67.4265481, 37.9937726}, {23.4836959, 90.4768632}, {35.5970651, 79.8724826}, {75.3863417, 18.24489}},
{{61.3365082, 82.6931328}, {44.6393809, 54.0748258}, {16.8156155, 20.0497047}, {41.866885, 56.7355037}},

{{18.1312339, 31.6473732}, {95.5711034, 63.5350219}, {92.3283165, 62.0158945}, {18.5656052, 32.1268808}},
{{97.402018, 35.7169972}, {33.1127443, 25.8935163}, {1.13970027, 54.9424981}, {56.4860195, 60.529264}},
};

const size_t testSetCount = sizeof(testSet) / sizeof(testSet[0]);

void CubicIntersection_OneOffTest() {
    for (size_t outer = 0; outer < testSetCount - 1; ++outer) {
        SkDebugf("%s quads1[%d]\n", __FUNCTION__, outer);
        const Cubic& cubic1 = testSet[outer];
        for (size_t inner = outer + 1; inner < testSetCount; ++inner) {
        SkDebugf("%s quads2[%d]\n", __FUNCTION__, inner);
            const Cubic& cubic2 = testSet[inner];
            oneOff(cubic1, cubic2);
        }
    }
}

#define DEBUG_CRASH 1

class CubicChopper {
public:

// only finds one intersection
CubicChopper(const Cubic& c1, const Cubic& c2)
    : cubic1(c1)
    , cubic2(c2)
    , depth(0) {
}

bool intersect(double minT1, double maxT1, double minT2, double maxT2) {
    Cubic sub1, sub2;
    // FIXME: carry last subdivide and reduceOrder result with cubic
    sub_divide(cubic1, minT1, maxT1, sub1);
    sub_divide(cubic2, minT2, maxT2, sub2);
    Intersections i;
    intersect2(sub1, sub2, i);
    if (i.used() == 0) {
        return false;
    }
    double x1, y1, x2, y2;
    t1 = minT1 + i.fT[0][0] * (maxT1 - minT1);
    t2 = minT2 + i.fT[1][0] * (maxT2 - minT2);
    xy_at_t(cubic1, t1, x1, y1);
    xy_at_t(cubic2, t2, x2, y2);
    if (AlmostEqualUlps(x1, x2) && AlmostEqualUlps(y1, y2)) {
        return true;
    }
    double half1 = (minT1 + maxT1) / 2;
    double half2 = (minT2 + maxT2) / 2;
    ++depth;
    bool result;
    if (depth & 1) {
        result = intersect(minT1, half1, minT2, maxT2) || intersect(half1, maxT1, minT2, maxT2)
            || intersect(minT1, maxT1, minT2, half2) || intersect(minT1, maxT1, half2, maxT2);
    } else {
        result = intersect(minT1, maxT1, minT2, half2) || intersect(minT1, maxT1, half2, maxT2)
            || intersect(minT1, half1, minT2, maxT2) || intersect(half1, maxT1, minT2, maxT2);
    }
    --depth;
    return result;
}

const Cubic& cubic1;
const Cubic& cubic2;
double t1;
double t2;
int depth;
};

#define TRY_OLD 0 // old way fails on test == 1

void CubicIntersection_RandTest() {
    srand(0);
    const int tests = 1000000; // 10000000;
    double largestFactor = DBL_MAX;
    for (int test = 0; test < tests; ++test) {
        Cubic cubic1, cubic2;
        for (int i = 0; i < 4; ++i) {
            cubic1[i].x = (double) rand() / RAND_MAX * 100;
            cubic1[i].y = (double) rand() / RAND_MAX * 100;
            cubic2[i].x = (double) rand() / RAND_MAX * 100;
            cubic2[i].y = (double) rand() / RAND_MAX * 100;
        }
        if (test == 2513) { // the pair crosses three times, but the quadratic approximation
            continue; // only sees one -- should be OK to ignore the other two?
        }
        if (test == 12932) { // this exposes a weakness when one cubic touches the other but
            continue; // does not touch the quad approximation. Captured in qc.htm as cubic15
        }
    #if DEBUG_CRASH
        char str[1024];
        sprintf(str, "{{%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}},\n"
            "{{%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}, {%1.9g, %1.9g}},\n",
                cubic1[0].x, cubic1[0].y,  cubic1[1].x, cubic1[1].y, cubic1[2].x, cubic1[2].y,
                cubic1[3].x, cubic1[3].y,
                cubic2[0].x, cubic2[0].y,  cubic2[1].x, cubic2[1].y, cubic2[2].x, cubic2[2].y,
                cubic2[3].x, cubic2[3].y);
    #endif
        _Rect rect1, rect2;
        rect1.setBounds(cubic1);
        rect2.setBounds(cubic2);
        bool boundsIntersect = rect1.left <= rect2.right && rect2.left <= rect2.right
                && rect1.top <= rect2.bottom && rect2.top <= rect1.bottom;
        Intersections i1, i2;
    #if TRY_OLD
        bool oldIntersects = intersect(cubic1, cubic2, i1);
    #else
        bool oldIntersects = false;
    #endif
        if (test == -1) {
            SkDebugf("ready...\n");
        }
        bool newIntersects = intersect2(cubic1, cubic2, i2);
        if (!boundsIntersect && (oldIntersects || newIntersects)) {
            SkDebugf("%s %d unexpected intersection boundsIntersect=%d oldIntersects=%d"
                    " newIntersects=%d\n%s %s\n", __FUNCTION__, test, boundsIntersect,
                    oldIntersects, newIntersects, __FUNCTION__, str);
            assert(0);
        }
        if (oldIntersects && !newIntersects) {
            SkDebugf("%s %d missing intersection oldIntersects=%d newIntersects=%d\n%s %s\n",
                    __FUNCTION__, test, oldIntersects, newIntersects, __FUNCTION__, str);
            assert(0);
        }
        if (!oldIntersects && !newIntersects) {
            continue;
        }
        if (i2.used() > 1) {
            continue;
            // just look at single intercepts for simplicity
        }
        Intersections self1, self2; // self-intersect checks
        if (intersect(cubic1, self1)) {
            continue;
        }
        if (intersect(cubic2, self2)) {
            continue;
        }
        // binary search for range necessary to enclose real intersection
        CubicChopper c(cubic1, cubic2);
        bool result = c.intersect(0, 1, 0, 1);
        if (!result) {
            // FIXME: a failure here probably means that a core routine used by CubicChopper is failing
            continue;
        }
        double delta1 = fabs(c.t1 - i2.fT[0][0]);
        double delta2 = fabs(c.t2 - i2.fT[1][0]);
        double calc1 = calcPrecision(cubic1);
        double calc2 = calcPrecision(cubic2);
        double factor1 = calc1 / delta1;
        double factor2 = calc2 / delta2;
        SkDebugf("%s %d calc1=%1.9g delta1=%1.9g factor1=%1.9g calc2=%1.9g delta2=%1.9g"
                " factor2=%1.9g\n", __FUNCTION__, test,
                calc1, delta1, factor1, calc2, delta2, factor2);
        if (factor1 < largestFactor) {
            SkDebugf("WE HAVE A WINNER! %1.9g\n", factor1);
            SkDebugf("%s\n", str);
            oneOff(cubic1, cubic2);
            largestFactor = factor1;
        }
        if (factor2 < largestFactor) {
            SkDebugf("WE HAVE A WINNER! %1.9g\n", factor2);
            SkDebugf("%s\n", str);
            oneOff(cubic1, cubic2);
            largestFactor = factor2;
        }
    }
}
