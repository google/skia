/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCubicMap.h"
#include "include/private/SkNx.h"

#ifdef SK_DEBUG
#define CUBICMAP_TRACK_MAX_ERROR
#endif

#ifdef CUBICMAP_TRACK_MAX_ERROR
#include "src/pathops/SkPathOpsCubic.h"
#endif

static float eval_poly3(float a, float b, float c, float d, float t) {
    return ((a * t + b) * t + c) * t + d;
}

static float eval_poly2(float a, float b, float c, float t) {
    return (a * t + b) * t + c;
}

static float eval_poly1(float a, float b, float t) {
    return a * t + b;
}

static float guess_nice_cubic_root(float A, float B, float C, float D) {
    return -D;
}

#ifdef SK_DEBUG
static bool valid(float r) {
    return r >= 0 && r <= 1;
}
#endif

static inline bool nearly_zero(SkScalar x) {
    SkASSERT(x >= 0);
    return x <= 0.0000000001f;
}

static inline bool delta_nearly_zero(float delta) {
    return sk_float_abs(delta) <= 0.0001f;
}

#ifdef CUBICMAP_TRACK_MAX_ERROR
    static int max_iters;
#endif

static float solve_halley(float A, float B, float C, float D, float guess) {
    const int MAX_ITERS = 8;
    const float A3 = 3 * A;
    const float B2 = B + B;

    float t = true ? guess : guess_nice_cubic_root(A, B, C, D);
    int iters = 0;
    for (; iters < MAX_ITERS; ++iters) {
        float f = eval_poly3(A, B, C, D, t);    // f   = At^3 + Bt^2 + Ct + D
        float fp = eval_poly2(A3, B2, C, t);    // f'  = 3At^2 + 2Bt + C
        float fpp = eval_poly1(A3 + A3, B2, t); // f'' = 6At + 2B

        float numer = 2 * fp * f;
        if (numer == 0) {
            break;
        }
        float denom = 2 * fp * fp - f * fpp;
        float delta = numer / denom;
//      SkDebugf("[%d] delta %g t %g\n", iters, delta, t);
        if (delta_nearly_zero(delta)) {
            break;
        }
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

static float solve_newton(float A, float B, float C, float D, float guess) {
    const int MAX_ITERS = 8;
    const float A3 = 3 * A;
    const float B2 = B + B;

    SkASSERT(valid(guess));
    float t = guess;
    int iters = 0;
    for (; iters < MAX_ITERS; ++iters) {
        float f = eval_poly3(A, B, C, D, t);    // f   = At^3 + Bt^2 + Ct + D
        if (delta_nearly_zero(f)) {
            SkASSERT(valid(guess));
            return t;
        }
        float fp = eval_poly2(A3, B2, C, t);    // f'  = 3At^2 + 2Bt + C
        if (delta_nearly_zero(fp)) {
            break;  // failed, try halley
        }
        float new_t = t - f / fp;
        t = new_t;
    }
    return solve_halley(A, B, C, D, guess);
}


#ifdef CUBICMAP_TRACK_MAX_ERROR
static float compute_slow(float A, float B, float C, float x) {
    double roots[3];
    SkDEBUGCODE(int count =) SkDCubic::RootsValidT(A, B, C, -x, roots);
    SkASSERT(count == 1);
    return (float)roots[0];
}

static float max_err;
#endif

static float compute_t_from_x(float A, float B, float C, float x, float guess) {
#ifdef CUBICMAP_TRACK_MAX_ERROR
    float answer = compute_slow(A, B, C, x);
#endif
    float answer2 = false ?
                    solve_halley(A, B, C, -x, guess) :
                    solve_newton(A, B, C, -x, guess);

#ifdef CUBICMAP_TRACK_MAX_ERROR
    float err = sk_float_abs(answer - answer2);
    if (err > max_err) {
        max_err = err;
        SkDebugf("max error %g\n", max_err);
    }
#endif
    return answer2;
}

float SkCubicMap::guessTFromX(float x) const {
    const float delta = 1.0f / N;
    float offset = 0;
    int i = 1;
    for (; i < N; ++i) {
        if (x <= fTableX[i]) {
            break;
        }
        offset += delta;
    }
    float guess = offset + delta * (x - fTableX[i-1]) / (fTableX[i] - fTableX[i-1]);
    SkASSERT(valid(guess));
    return guess;
}

float SkCubicMap::computeYFromX(float x) const {
    x = SkScalarPin(x, 0, 1);

    if (nearly_zero(x) || nearly_zero(1 - x)) {
        return x;
    }
    if (fType == kLine_Type) {
        return x;
    }
    float t;
    if (fType == kCubeRoot_Type) {
        t = sk_float_pow(x / fCoeff[0].fX, 1.0f / 3);
    } else {
        float guess = this->guessTFromX(x);
        t = compute_t_from_x(fCoeff[0].fX, fCoeff[1].fX, fCoeff[2].fX, x, guess);
    }
    float a = fCoeff[0].fY;
    float b = fCoeff[1].fY;
    float c = fCoeff[2].fY;
    float y = ((a * t + b) * t + c) * t;

    return y;
}

static inline bool coeff_nearly_zero(float delta) {
    return sk_float_abs(delta) <= 0.0000001f;
}

SkCubicMap::SkCubicMap(SkPoint p1, SkPoint p2) {
    // Clamp X values only (we allow Ys outside [0..1]).
    p1.fX = SkTMin(SkTMax(p1.fX, 0.0f), 1.0f);
    p2.fX = SkTMin(SkTMax(p2.fX, 0.0f), 1.0f);

    Sk2s s1 = Sk2s::Load(&p1) * 3;
    Sk2s s2 = Sk2s::Load(&p2) * 3;

    (Sk2s(1) + s1 - s2).store(&fCoeff[0]);
    (s2 - s1 - s1).store(&fCoeff[1]);
    s1.store(&fCoeff[2]);

    fType = kSolver_Type;
    if (SkScalarNearlyEqual(p1.fX, p1.fY) && SkScalarNearlyEqual(p2.fX, p2.fY)) {
        fType = kLine_Type;
    } else if (coeff_nearly_zero(fCoeff[1].fX) && coeff_nearly_zero(fCoeff[2].fX)) {
        fType = kCubeRoot_Type;
    }

    const float delta = 1.0f / N;
    fTableX[0] = 0;
    for (int i = 1; i < N; ++i) {
        fTableX[i] = eval_poly3(fCoeff[0].fX, fCoeff[1].fX, fCoeff[2].fX, 0, i * delta);
    }
    fTableX[N] = 1;
}

SkPoint SkCubicMap::computeFromT(float t) const {
    Sk2s a = Sk2s::Load(&fCoeff[0]);
    Sk2s b = Sk2s::Load(&fCoeff[1]);
    Sk2s c = Sk2s::Load(&fCoeff[2]);

    SkPoint result;
    (((a * t + b) * t + c) * t).store(&result);
    return result;
}
