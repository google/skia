/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../skcms.h"
#include "GaussNewton.h"
#include "Macros.h"
#include "PortableMath.h"
#include "TransferFunction.h"
#include <limits.h>
#include <stdlib.h>

// f(x) = skcms_PolyTF{A,B,C,D}(x) =
//     Cx                     x < D
//     Ax^3 + Bx^2 + (1-A-B)  x ≥ D
//
// We'll fit C and D directly, and then hold them constant
// and fit the other part using Gauss-Newton, subject to
// the constraint that both parts meet at x=D:
//
//     CD = AD^3 + BD^2 + (1-A-B)
//
// This lets us solve for B, reducing the optimization problem
// for that part down to just a single parameter A:
//
//     CD = A(D^3-1) + B(D^2-1) + 1
//
//         CD - A(D^3-1) - 1
//     B = -----------------
//               D^2-1
//
//                    x^2-1
//  f(x) = A(x^3-1) + ----- [CD - A(D^3-1) - 1] + 1
//                    D^2-1
//
//                  (x^2-1) (D^3-1)
//  ∂f/∂A = x^3-1 - ---------------
//                       D^2-1

static float eval_poly_tf(float x, const void* ctx, const float P[4]) {
    const skcms_PolyTF* tf = (const skcms_PolyTF*)ctx;

    float A = P[0],
          C = tf->C,
          D = tf->D;
    float B = (C*D - A*(D*D*D - 1) - 1) / (D*D - 1);

    return x < D ? C*x
                 : A*x*x*x + B*x*x + (1-A-B);
}

static void grad_poly_tf(float x, const void* ctx, const float P[4], float dfdP[4]) {
    const skcms_PolyTF* tf = (const skcms_PolyTF*)ctx;
    (void)P;
    float D = tf->D;

    dfdP[0] = (x*x*x - 1) - (x*x-1)*(D*D*D-1)/(D*D-1);
}

static bool fit_poly_tf(const skcms_Curve* curve, skcms_PolyTF* tf) {
    if (curve->table_entries > (uint32_t)INT_MAX) {
        return false;
    }

    const int N = curve->table_entries == 0 ? 256
                                            :(int)curve->table_entries;

    // We'll test the quality of our fit by roundtripping through a skcms_TransferFunction,
    // either the inverse of the curve itself if it is parametric, or of its approximation if not.
    skcms_TransferFunction baseline;
    float err;
    if (curve->table_entries == 0) {
        baseline = curve->parametric;
    } else if (!skcms_ApproximateCurve(curve, &baseline, &err)) {
        return false;
    }
    skcms_TransferFunction inv;
    if (!skcms_TransferFunction_invert(&baseline, &inv)) {
        return false;
    }

    const float kTolerances[] = { 1.5f / 65535.0f, 1.0f / 512.0f };
    for (int t = 0; t < ARRAY_COUNT(kTolerances); t++) {
        float f;
        const int L = skcms_fit_linear(curve, N, kTolerances[t], &tf->C, &tf->D, &f);
        if (f != 0) {
            return false;
        }

        if (tf->D == 1) {
            tf->A = 0;
            tf->B = 0;
            return true;
        }

        // Start with guess A = 0, i.e. f(x) = x^2, gamma = 2.
        float P[4] = {0, 0,0,0};

        for (int i = 0; i < 3; i++) {
            if (!skcms_gauss_newton_step(skcms_eval_curve, curve,
                                         eval_poly_tf, tf,
                                         grad_poly_tf, tf,
                                         P,
                                         tf->D, 1, N-L)) {
                goto NEXT;
            }
        }

        float A = tf->A = P[0],
              C = tf->C,
              D = tf->D;
        tf->B = (C*D - A*(D*D*D - 1) - 1) / (D*D - 1);

        for (int i = 0; i < N; i++) {
            float x = i * (1.0f/(N-1));

            float rt = skcms_TransferFunction_eval(&inv, eval_poly_tf(x, tf, P));
            if (!isfinitef_(rt)) {
                goto NEXT;
            }

            const int tol = (i == 0 || i == N-1) ? 0
                                                 : N/256;
            int ix = (int)((N-1) * rt + 0.5f);
            if (abs(i - ix) > tol) {
                goto NEXT;
            }
        }
        return true;

    NEXT: ;
    }

    return false;
}

void skcms_OptimizeForSpeed(skcms_ICCProfile* profile) {
    for (int i = 0; profile->has_trc && i < 3; i++) {
        profile->has_poly_tf[i] = fit_poly_tf(&profile->trc[i], &profile->poly_tf[i]);
    }
}
