#include "CubicIntersection_TestData.h"
#include "CubicUtilities.h"
#include "Intersection_Tests.h"
#include "QuadraticIntersection_TestData.h"
#include "TestUtilities.h"

void CubicToQuadratics_Test() {
    size_t index;
    enum {
        RunAll,
        RunPointDegenerates,
        RunNotPointDegenerates,
        RunLines,
        RunNotLines,
        RunModEpsilonLines,
        RunLessEpsilonLines,
        RunNegEpsilonLines,
        RunQuadraticLines,
        RunQuadraticModLines,
        RunComputedLines,
        RunNone
    } run = RunAll;
    int firstTestIndex = 0;
#if 0
    run = RunComputedLines;
    firstTestIndex = 18;
#endif
    int firstPointDegeneratesTest = run == RunAll ? 0 : run == RunPointDegenerates ? firstTestIndex : INT_MAX;
    int firstNotPointDegeneratesTest = run == RunAll ? 0 : run == RunNotPointDegenerates ? firstTestIndex : INT_MAX;
    int firstLinesTest = run == RunAll ? 0 : run == RunLines ? firstTestIndex : INT_MAX;
    int firstNotLinesTest = run == RunAll ? 0 : run == RunNotLines ? firstTestIndex : INT_MAX;
    int firstModEpsilonTest = run == RunAll ? 0 : run == RunModEpsilonLines ? firstTestIndex : INT_MAX;
    int firstLessEpsilonTest = run == RunAll ? 0 : run == RunLessEpsilonLines ? firstTestIndex : INT_MAX;
    int firstNegEpsilonTest = run == RunAll ? 0 : run == RunNegEpsilonLines ? firstTestIndex : INT_MAX;
    int firstQuadraticLineTest = run == RunAll ? 0 : run == RunQuadraticLines ? firstTestIndex : INT_MAX;
    int firstQuadraticModLineTest = run == RunAll ? 0 : run == RunQuadraticModLines ? firstTestIndex : INT_MAX;
    int firstComputedLinesTest = run == RunAll ? 0 : run == RunComputedLines ? firstTestIndex : INT_MAX;
    SkTDArray<Quadratic> quads;
    for (index = firstPointDegeneratesTest; index < pointDegenerates_count; ++index) {
        const Cubic& cubic = pointDegenerates[index];
        cubic_to_quadratics(cubic, quads);
        if (quads.count() != 1) {
            printf("[%d] pointDegenerates count=%d\n", (int) index, quads.count());
        }
    }
    for (index = firstNotPointDegeneratesTest; index < notPointDegenerates_count; ++index) {
        const Cubic& cubic = notPointDegenerates[index];
        cubic_to_quadratics(cubic, quads);
        if (quads.count() != 1) {
            printf("[%d] notPointDegenerates count=%d\n", (int) index, quads.count());
        }
    }
    for (index = firstLinesTest; index < lines_count; ++index) {
        const Cubic& cubic = lines[index];
        cubic_to_quadratics(cubic, quads);
        if (quads.count() != 1) {
            printf("[%d] lines count=%d\n", (int) index, quads.count());
        }
    }
    for (index = firstNotLinesTest; index < notLines_count; ++index) {
        const Cubic& cubic = notLines[index];
        cubic_to_quadratics(cubic, quads);
        if (quads.count() != 1) {
            printf("[%d] notLines order=%d\n", (int) index, quads.count());
        }
    }
    for (index = firstModEpsilonTest; index < modEpsilonLines_count; ++index) {
        const Cubic& cubic = modEpsilonLines[index];
        cubic_to_quadratics(cubic, quads);
        if (quads.count() != 1) {
            printf("[%d] line mod by epsilon order=%d\n", (int) index, quads.count());
        }
    }
    for (index = firstLessEpsilonTest; index < lessEpsilonLines_count; ++index) {
        const Cubic& cubic = lessEpsilonLines[index];
        cubic_to_quadratics(cubic, quads);
        if (quads.count() != 1) {
            printf("[%d] line less by epsilon/2 order=%d\n", (int) index, quads.count());
        }
    }
    for (index = firstNegEpsilonTest; index < negEpsilonLines_count; ++index) {
        const Cubic& cubic = negEpsilonLines[index];
        cubic_to_quadratics(cubic, quads);
        if (quads.count() != 1) {
            printf("[%d] line neg by epsilon/2 order=%d\n", (int) index, quads.count());
        }
    }
    for (index = firstQuadraticLineTest; index < quadraticLines_count; ++index) {
        const Quadratic& quad = quadraticLines[index];
        Cubic cubic;
        quad_to_cubic(quad, cubic);
        cubic_to_quadratics(cubic, quads);
        if (quads.count() != 1) {
            printf("[%d] line quad order=%d\n", (int) index, quads.count());
        }
    }
    for (index = firstQuadraticModLineTest; index < quadraticModEpsilonLines_count; ++index) {
        const Quadratic& quad = quadraticModEpsilonLines[index];
        Cubic cubic;
        quad_to_cubic(quad, cubic);
        cubic_to_quadratics(cubic, quads);
        if (quads.count() != 1) {
            printf("[%d] line mod quad order=%d\n", (int) index, quads.count());
        }
    }

    // test if computed line end points are valid
    for (index = firstComputedLinesTest; index < lines_count; ++index) {
        const Cubic& cubic = lines[index];
        cubic_to_quadratics(cubic, quads);
        if (cubic[0].x != quads[0][0].x && cubic[0].y != quads[0][0].y) {
            printf("[%d] unmatched start\n", (int) index);
        }
        int last = quads.count() - 1;
        if (cubic[3].x != quads[last][2].x && cubic[3].y != quads[last][2].y) {
            printf("[%d] unmatched end\n", (int) index);
        }
    }
}
