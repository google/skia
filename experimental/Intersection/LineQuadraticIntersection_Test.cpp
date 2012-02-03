#include "CurveIntersection.h"
#include "Intersection_Tests.h"
#include "Intersections.h"
#include "LineUtilities.h"
#include "TestUtilities.h"

struct lineQuad {
    Quadratic quad;
    _Line line;
} lineQuadTests[] = {
    {{{0, 0}, {0, 1}, {1, 1}}, {{0, 1}, {1, 0}}}
};

size_t lineQuadTests_count = sizeof(lineQuadTests) / sizeof(lineQuadTests[0]);

const int firstLineQuadIntersectionTest = 0;

void LineQuadraticIntersection_Test() {
    for (size_t index = firstLineQuadIntersectionTest; index < lineQuadTests_count; ++index) {
        const Quadratic& quad = lineQuadTests[index].quad;
        const _Line& line = lineQuadTests[index].line;
        Quadratic reduce1;
        _Line reduce2;
        int order1 = reduceOrder(quad, reduce1);
        int order2 = reduceOrder(line, reduce2);
        if (order1 < 3) {
            printf("[%d] quad order=%d\n", (int) index, order1);
        }
        if (order2 < 2) {
            printf("[%d] line order=%d\n", (int) index, order2);
        }
        if (order1 == 3 && order2 == 2) {
            Intersections intersections;
            intersect(reduce1, reduce2, intersections);
            if (intersections.intersected()) {
                for (int pt = 0; pt < intersections.used(); ++pt) {
                    double tt1 = intersections.fT[0][pt];
                    double tx1, ty1;
                    xy_at_t(quad, tt1, tx1, ty1);
                    double tt2 = intersections.fT[1][pt];
                    double tx2, ty2;
                    xy_at_t(line, tt2, tx2, ty2);
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
