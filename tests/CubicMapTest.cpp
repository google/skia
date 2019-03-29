/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCubicMap.h"
#include "SkGeometry.h"
#include "SkRandom.h"
#include "Test.h"
#include "../../src/pathops/SkPathOpsCubic.h"

static float accurate_t(float A, float B, float C, float D) {
    double roots[3];
    SkDEBUGCODE(int count =) SkDCubic::RootsValidT(A, B, C, D, roots);
    SkASSERT(count == 1);
    return (float)roots[0];
}

static float accurate_solve(SkPoint p1, SkPoint p2, SkScalar x) {
    SkPoint array[] = { {0, 0}, p1, p2, {1,1} };
    SkCubicCoeff coeff(array);

    float t = accurate_t(coeff.fA[0], coeff.fB[0], coeff.fC[0], coeff.fD[0] - x);
    SkASSERT(t >= 0 && t <= 1);
    float y = coeff.eval(t)[1];
    SkASSERT(y >= 0 && y <= 1.0001f);
    return y;
}

static bool nearly_le(SkScalar a, SkScalar b) {
    return a <= b || SkScalarNearlyZero(a - b);
}

static void exercise_cubicmap(SkPoint p1, SkPoint p2, skiatest::Reporter* reporter) {
    const SkScalar MAX_SOLVER_ERR = 0.008f; // found by running w/ current impl

    SkCubicMap cmap(p1, p2);

    SkScalar prev_y = 0;
    SkScalar dx = 1.0f / 512;
    for (SkScalar x = dx; x < 1; x += dx) {
        SkScalar y = cmap.computeYFromX(x);
        // are we valid and (mostly) monotonic?
        if (!nearly_le(prev_y, y)) {
            cmap.computeYFromX(x);
            REPORTER_ASSERT(reporter, false);
        }
        prev_y = y;

        // are we close to the "correct" answer?
        SkScalar yy = accurate_solve(p1, p2, x);
        SkScalar diff = SkScalarAbs(yy - y);
        REPORTER_ASSERT(reporter, diff < MAX_SOLVER_ERR);
    }
}

DEF_TEST(CubicMap, r) {
    const SkScalar values[] = {
        0, 1, 0.5f, 0.0000001f, 0.999999f,
    };

    for (SkScalar x0 : values) {
        for (SkScalar y0 : values) {
            for (SkScalar x1 : values) {
                for (SkScalar y1 : values) {
                    exercise_cubicmap({ x0, y0 }, { x1, y1 }, r);
                }
            }
        }
    }
}
