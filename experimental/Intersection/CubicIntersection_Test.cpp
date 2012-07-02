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
