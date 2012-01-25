#include "CubicIntersection.h"
#include "Intersection_Tests.h"
#include "QuadraticIntersection_TestData.h"
#include "TestUtilities.h"

void QuadraticReduceOrder_Test() {
    size_t index;
    Quadratic reduce;
    int order;
    enum {
        RunAll,
        RunQuadraticLines,
        RunQuadraticModLines,
        RunNone
    } run = RunAll;
    int firstTestIndex = 0;
#if 0
    run = RunQuadraticLines;
    firstTestIndex = 1;
#endif
    int firstQuadraticLineTest = run == RunAll ? 0 : run == RunQuadraticLines ? firstTestIndex : INT_MAX;
    int firstQuadraticModLineTest = run == RunAll ? 0 : run == RunQuadraticModLines ? firstTestIndex : INT_MAX;

    for (index = firstQuadraticLineTest; index < quadraticLines_count; ++index) {
        const Quadratic& quad = quadraticLines[index];
        order = reduceOrder(quad, reduce);
        if (order != 2) {
            printf("[%d] line quad order=%d\n", (int) index, order);
        }
    }
    for (index = firstQuadraticModLineTest; index < quadraticModEpsilonLines_count; ++index) {
        const Quadratic& quad = quadraticModEpsilonLines[index];
        order = reduceOrder(quad, reduce);
        if (order != 3) {
            printf("[%d] line mod quad order=%d\n", (int) index, order);
        }
    }
}
