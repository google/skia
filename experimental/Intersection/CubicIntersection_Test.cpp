#include "CubicIntersection.h"
#include "CubicIntersection_TestData.h"
#include "CubicIntersection_Tests.h"
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
            printf("[%d] cubic1 order=%d\n", (int) index, order1);
        }
        if (order2 < 4) {
            printf("[%d] cubic2 order=%d\n", (int) index, order2);
        }
        if (order1 == 4 && order2 == 4) {
            Intersections tIntersections;
            intersectStartT(reduce1, reduce2, tIntersections);
#ifdef COMPUTE_BY_CUBIC_SUBDIVISION
            Intersections intersections;
            intersectStart(reduce1, reduce2, intersections);
#endif
            if (tIntersections.intersected()) {
                for (int pt = 0; pt < tIntersections.used(); ++pt) {
#ifdef COMPUTE_BY_CUBIC_SUBDIVISION
                    double t1 = intersections.fT[0][pt];
                    double x1, y1;
                    xy_at_t(cubic1, t1, x1, y1);
                    double t2 = intersections.fT[1][pt];
                    double x2, y2;
                    xy_at_t(cubic2, t2, x2, y2);
                    if (!approximately_equal(x1, x2)) {
                        printf("%s [%d] (1) t1=%g (%g,%g) t2=%g (%g,%g)\n",
                            __FUNCTION__, pt, t1, x1, y1, t2, x2, y2);
                    }
                    if (!approximately_equal(y1, y2)) {
                        printf("%s [%d] (2) t1=%g (%g,%g) t2=%g (%g,%g)\n",
                            __FUNCTION__, pt, t1, x1, y1, t2, x2, y2);
                    }
#endif
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
    }
}
