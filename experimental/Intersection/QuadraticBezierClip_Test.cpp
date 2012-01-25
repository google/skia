#include "CubicIntersection.h"
#include "Intersection_Tests.h"
#include "QuadraticIntersection_TestData.h"

void QuadraticBezierClip_Test() {
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
