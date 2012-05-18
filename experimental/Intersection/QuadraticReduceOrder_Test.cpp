#include "CurveIntersection.h"
#include "Intersection_Tests.h"
#include "QuadraticIntersection_TestData.h"
#include "TestUtilities.h"
#include "SkTypes.h"

static const Quadratic testSet[] = {
    {{1, 1}, {2, 2}, {1, 1.000003}},
    {{1, 0}, {2, 6}, {3, 0}}
};

static const size_t testSetCount = sizeof(testSet) / sizeof(testSet[0]);


static void oneOffTest() {
    SkDebugf("%s FLT_EPSILON=%1.9g\n", __FUNCTION__, FLT_EPSILON);
    for (int index = 0; index < testSetCount; ++index) {
        const Quadratic& quad = testSet[index];
        Quadratic reduce;
        int order = reduceOrder(quad, reduce);
        SkASSERT(order == 3);
    }
}

static void standardTestCases() {
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

void QuadraticReduceOrder_Test() {
    oneOffTest();
    standardTestCases();
}
