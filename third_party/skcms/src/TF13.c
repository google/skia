/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../skcms.h"
#include "GaussNewton.h"
#include "PortableMath.h"
#include <limits.h>
#include <stdlib.h>

// Evaluating skcms_TF13{A,B} at x:
//   f(x) = Ax^3 + Bx^2 + (1-A-B)x
//
//   ∂f/∂A = x^3 - x
//   ∂f/∂B = x^2 - x

static float eval_13(float x, const void* ctx, const float P[4]) {
    (void)ctx;
    return P[0]*x*x*x
         + P[1]*x*x
         + (1 - P[0] - P[1])*x;
}
static void grad_13(float x, const void* ctx, const float P[4], float dfdP[4]) {
    (void)ctx;
    (void)P;
    dfdP[0] = x*x*x - x;
    dfdP[1] = x*x   - x;
}

bool skcms_ApproximateCurve13(const skcms_Curve* curve, skcms_TF13* approx, float* max_error) {
    // Start a guess at skcms_TF13{0,1}, i.e. f(x) = x^2, i.e. gamma = 2.
    // TODO: guess better somehow, like we do in skcms_ApproximateCurve()?
    float P[4] = { 0,1, 0,0 };

    if (curve->table_entries > (uint32_t)INT_MAX) {
        // That's just crazy.
        return false;
    }
    const int N = curve->table_entries == 0 ? 257 /*TODO: tune?*/
                                            : (int)curve->table_entries;

    for (int i = 0; i < 3/*TODO: Tune???*/; i++) {
        if (!skcms_gauss_newton_step(skcms_eval_curve, curve,
                                     eval_13, NULL,
                                     grad_13, NULL,
                                     P,
                                     0,1,N)) {
            return false;
        }
    }

    *max_error = 0;
    for (int i = 0; i < N; i++) {
        float x = i * (1.0f / (N-1));

        const float got  = eval_13(x,NULL,P),
                    want = skcms_eval_curve(x, curve);

        const float err = fabsf_(got - want);
        if (err > *max_error) {
            *max_error = err;
        }

        // Compare what bytes we'd choose for these floats, rounded, and scaled by 255,
        // but intentionally not clamped... if this goes negative, we want it to hurt.

        const int gbyte = (int)(255.0f * got  + 0.5f),
                  wbyte = (int)(255.0f * want + 0.5f);

        // Allow no more than 1/256 error, and no error at all at the beginning or end.
        const int tol = (i == 0 || i == N-1) ? 0
                                             : 1;
        if (abs(gbyte - wbyte) > tol) {
            return false;
        }
    }

    approx->A = P[0];
    approx->B = P[1];
    return true;
}
