/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCubicSolver_DEFINED
#define SkCubicSolver_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"

namespace SK_OPTS_NS {

    static float eval_poly(float t, float b) {
        return b;
    }

    template <typename... Rest>
    static float eval_poly(float t, float m, float b, Rest... rest) {
        return eval_poly(t, sk_fmaf(m,t,b), rest...);
    }

    inline float cubic_solver(float A, float B, float C, float D) {

    #ifdef SK_DEBUG
        auto valid = [](float t) {
            return t >= 0 && t <= 1;
        };
    #endif

        auto guess_nice_cubic_root = [](float a, float b, float c, float d) {
            return -d;
        };
        float t = guess_nice_cubic_root(A, B, C, D);

        int iters = 0;
        const int MAX_ITERS = 8;
        for (; iters < MAX_ITERS; ++iters) {
            SkASSERT(valid(t));
            float f = eval_poly(t, A,B,C,D);        // f   = At^3 + Bt^2 + Ct + D
            if (sk_float_abs(f) <= 0.00005f) {
                break;
            }
            float fp  = eval_poly(t, 3*A, 2*B, C);  // f'  = 3At^2 + 2Bt + C
            float fpp = eval_poly(t, 3*A+3*A, 2*B); // f'' = 6At + 2B

            float numer = 2 * fp * f;
            float denom = sk_fmaf(2*fp, fp, -(f*fpp));

            t -= numer / denom;
        }

        SkASSERT(valid(t));
        return t;
    }

}  // namespace SK_OPTS_NS
#endif
