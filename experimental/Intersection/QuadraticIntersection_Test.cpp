#include "CurveIntersection.h"
#include "Intersection_Tests.h"
#include "Intersections.h"
#include "QuadraticIntersection_TestData.h"
#include "TestUtilities.h"

const int firstQuadIntersectionTest = 9;

void QuadraticIntersection_Test() {
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

