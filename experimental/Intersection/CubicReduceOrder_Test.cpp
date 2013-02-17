/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "CubicIntersection_TestData.h"
#include "Intersection_Tests.h"
#include "QuadraticIntersection_TestData.h"
#include "TestUtilities.h"

void CubicReduceOrder_Test() {
    size_t index;
    Cubic reduce;
    int order;
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
    int firstPointDegeneratesTest = run == RunAll ? 0 : run == RunPointDegenerates ? firstTestIndex : SK_MaxS32;
    int firstNotPointDegeneratesTest = run == RunAll ? 0 : run == RunNotPointDegenerates ? firstTestIndex : SK_MaxS32;
    int firstLinesTest = run == RunAll ? 0 : run == RunLines ? firstTestIndex : SK_MaxS32;
    int firstNotLinesTest = run == RunAll ? 0 : run == RunNotLines ? firstTestIndex : SK_MaxS32;
    int firstModEpsilonTest = run == RunAll ? 0 : run == RunModEpsilonLines ? firstTestIndex : SK_MaxS32;
    int firstLessEpsilonTest = run == RunAll ? 0 : run == RunLessEpsilonLines ? firstTestIndex : SK_MaxS32;
    int firstNegEpsilonTest = run == RunAll ? 0 : run == RunNegEpsilonLines ? firstTestIndex : SK_MaxS32;
    int firstQuadraticLineTest = run == RunAll ? 0 : run == RunQuadraticLines ? firstTestIndex : SK_MaxS32;
    int firstQuadraticModLineTest = run == RunAll ? 0 : run == RunQuadraticModLines ? firstTestIndex : SK_MaxS32;
    int firstComputedLinesTest = run == RunAll ? 0 : run == RunComputedLines ? firstTestIndex : SK_MaxS32;

    for (index = firstPointDegeneratesTest; index < pointDegenerates_count; ++index) {
        const Cubic& cubic = pointDegenerates[index];
        order = reduceOrder(cubic, reduce, kReduceOrder_QuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (order != 1) {
            SkDebugf("[%d] pointDegenerates order=%d\n", (int) index, order);
        }
    }
    for (index = firstNotPointDegeneratesTest; index < notPointDegenerates_count; ++index) {
        const Cubic& cubic = notPointDegenerates[index];
        order = reduceOrder(cubic, reduce, kReduceOrder_QuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (order == 1) {
            SkDebugf("[%d] notPointDegenerates order=%d\n", (int) index, order);
        }
    }
    for (index = firstLinesTest; index < lines_count; ++index) {
        const Cubic& cubic = lines[index];
        order = reduceOrder(cubic, reduce, kReduceOrder_QuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (order != 2) {
            SkDebugf("[%d] lines order=%d\n", (int) index, order);
        }
    }
    for (index = firstNotLinesTest; index < notLines_count; ++index) {
        const Cubic& cubic = notLines[index];
        order = reduceOrder(cubic, reduce, kReduceOrder_QuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (order == 2) {
            SkDebugf("[%d] notLines order=%d\n", (int) index, order);
        }
    }
    for (index = firstModEpsilonTest; index < modEpsilonLines_count; ++index) {
        const Cubic& cubic = modEpsilonLines[index];
        order = reduceOrder(cubic, reduce, kReduceOrder_QuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (order == 2) {
            SkDebugf("[%d] line mod by epsilon order=%d\n", (int) index, order);
        }
    }
    for (index = firstLessEpsilonTest; index < lessEpsilonLines_count; ++index) {
        const Cubic& cubic = lessEpsilonLines[index];
        order = reduceOrder(cubic, reduce, kReduceOrder_QuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (order != 2) {
            SkDebugf("[%d] line less by epsilon/2 order=%d\n", (int) index, order);
        }
    }
    for (index = firstNegEpsilonTest; index < negEpsilonLines_count; ++index) {
        const Cubic& cubic = negEpsilonLines[index];
        order = reduceOrder(cubic, reduce, kReduceOrder_QuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (order != 2) {
            SkDebugf("[%d] line neg by epsilon/2 order=%d\n", (int) index, order);
        }
    }
    for (index = firstQuadraticLineTest; index < quadraticLines_count; ++index) {
        const Quadratic& quad = quadraticLines[index];
        Cubic cubic;
        quad_to_cubic(quad, cubic);
        order = reduceOrder(cubic, reduce, kReduceOrder_QuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (order != 2) {
            SkDebugf("[%d] line quad order=%d\n", (int) index, order);
        }
    }
    for (index = firstQuadraticModLineTest; index < quadraticModEpsilonLines_count; ++index) {
        const Quadratic& quad = quadraticModEpsilonLines[index];
        Cubic cubic;
        quad_to_cubic(quad, cubic);
        order = reduceOrder(cubic, reduce, kReduceOrder_QuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (order != 3) {
            SkDebugf("[%d] line mod quad order=%d\n", (int) index, order);
        }
    }

    // test if computed line end points are valid
    for (index = firstComputedLinesTest; index < lines_count; ++index) {
        const Cubic& cubic = lines[index];
        bool controlsInside = controls_inside(cubic);
        order = reduceOrder(cubic, reduce, kReduceOrder_QuadraticsAllowed,
                kReduceOrder_TreatAsFill);
        if (reduce[0].x == reduce[1].x && reduce[0].y == reduce[1].y) {
            SkDebugf("[%d] line computed ends match order=%d\n", (int) index, order);
        }
        if (controlsInside) {
            if (       (reduce[0].x != cubic[0].x && reduce[0].x != cubic[3].x)
                    || (reduce[0].y != cubic[0].y && reduce[0].y != cubic[3].y)
                    || (reduce[1].x != cubic[0].x && reduce[1].x != cubic[3].x)
                    || (reduce[1].y != cubic[0].y && reduce[1].y != cubic[3].y)) {
                SkDebugf("[%d] line computed ends order=%d\n", (int) index, order);
            }
        } else {
            // binary search for extrema, compare against actual results
                // while a control point is outside of bounding box formed by end points, split
            _Rect bounds = {DBL_MAX, DBL_MAX, -DBL_MAX, -DBL_MAX};
            find_tight_bounds(cubic, bounds);
            if (       (!AlmostEqualUlps(reduce[0].x, bounds.left) && !AlmostEqualUlps(reduce[0].x, bounds.right))
                    || (!AlmostEqualUlps(reduce[0].y, bounds.top) && !AlmostEqualUlps(reduce[0].y, bounds.bottom))
                    || (!AlmostEqualUlps(reduce[1].x, bounds.left) && !AlmostEqualUlps(reduce[1].x, bounds.right))
                    || (!AlmostEqualUlps(reduce[1].y, bounds.top) && !AlmostEqualUlps(reduce[1].y, bounds.bottom))) {
                SkDebugf("[%d] line computed tight bounds order=%d\n", (int) index, order);
            }

        }
    }
}
