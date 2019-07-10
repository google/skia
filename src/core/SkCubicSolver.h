/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCubicSolver_DEFINED
#define SkCubicSolver_DEFINED

#include <stdint.h>
#include "include/core/SkTypes.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkNx.h"

//#define CUBICMAP_TRACK_MAX_ERROR

#ifdef CUBICMAP_TRACK_MAX_ERROR
#include "src/pathops/SkPathOpsCubic.h"
#endif

namespace SK_OPTS_NS {

static float eval_poly3(float a, float b, float c, float d, float t) {
    return sk_fmaf(sk_fmaf(sk_fmaf(a, t, b), t, c), t, d);
}

static float eval_poly2(float a, float b, float c, float t) {
    return sk_fmaf(sk_fmaf(a, t, b), t, c);
}

static float eval_poly1(float a, float b, float t) {
    return sk_fmaf(a, t, b);
}

static float guess_nice_cubic_root(float A, float B, float C, float D) {
    return -D;
}

#ifdef SK_DEBUG
static bool valid(float r) {
    return r >= 0 && r <= 1;
}
#endif

static inline bool delta_nearly_zero(float delta) {
    return sk_float_abs(delta) <= 0.00005f;
}

#ifdef CUBICMAP_TRACK_MAX_ERROR
    static int max_iters;
#endif

/*
 *  TODO: will this be faster if we algebraically compute the polynomials for the numer and denom
 *        rather than compute them in parts?
 */
inline float cubic_solver(float A, float B, float C, float D) {
//This pragma gives us ~5+% on iMacPro, but is not portable
//#pragma clang fp contract(fast)
    const int MAX_ITERS = 8;
    const float A3 = 3 * A;
    const float B2 = B + B;

    float t = guess_nice_cubic_root(A, B, C, D);
    int iters = 0;
    for (; iters < MAX_ITERS; ++iters) {
        float f = eval_poly3(A, B, C, D, t);    // f   = At^3 + Bt^2 + Ct + D
        if (delta_nearly_zero(f)) {
            break;
        }
        float fp = eval_poly2(A3, B2, C, t);    // f'  = 3At^2 + 2Bt + C
        float fpp = eval_poly1(A3 + A3, B2, t); // f'' = 6At + 2B

        float numer = 2 * fp * f;
        float denom = 2 * fp * fp - f * fpp;
        float delta = numer / denom;
        float new_t = t - delta;
        SkASSERT(valid(new_t));
        t = new_t;
    }
    SkASSERT(valid(t));
#ifdef CUBICMAP_TRACK_MAX_ERROR
    if (iters > max_iters) {
        max_iters = iters;
        SkDebugf("max_iters %d\n", max_iters);
    }
#endif
    return t;
}

}
#endif
