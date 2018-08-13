/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCubicMap.h"
#include "SkNx.h"
#include "../../src/pathops/SkPathOpsCubic.h"

static float eval_poly3(float a, float b, float c, float d, float t) {
    return ((a * t + b) * t + c) * t + d;
}

static float eval_poly2(float a, float b, float c, float t) {
    return (a * t + b) * t + c;
}

static float ave(float a, float b) { return (a + b) * 0.5f; }
static float guess_nice_cubic_root(float A, float B, float C, float D) {
    return -D;
    float t = 0.5f;
    float min = 0;
    float max = 1;
    // bisect to refine guess
    for (int i = 0; i < 16; ++i) {
        float newt;
        if (eval_poly3(A, B, C, D, t) > 0) {
            newt = ave(t, min);
            min = t;
        } else {
            newt = ave(t, max);
            max = t;
        }
        t = newt;
    }
    return t;
}

static float solve_nice_cubic_halley(float A, float B, float C, float D) {
  //  const float MIN_DELTA = 0.0001;
    const int MAX_ITERS = 2;
    const float A3 = 3 * A;
    const float B2 = B + B;

    float t = guess_nice_cubic_root(A, B, C, D);
    for (int iters = 0; iters < MAX_ITERS; ++iters) {
        float f = eval_poly3(A, B, C, D, t);
        float fp = eval_poly2(A3, B2, C, t);    // 3At^2 + 2Bt + C
        float fpp = (A3 + A3) * t + B2;         // 6At + 2B

        float numer = 2 * fp * f;
        float denom = 2 * fp * fp - f * fpp;
        float delta = numer / denom;
  //      if (sk_float_abs(delta) <= MIN_DELTA) {
   //         break;
  //      }
        float new_t = t - delta;
        SkASSERT(new_t >= 0 && new_t <= 1);
        t = new_t;
    }
    SkASSERT(t >= 0 && t <= 1);
    return t;
}

static bool valid(float r) {
    return r >= 0 && r <= 1;
}

static float solve_nice_cubic(float A, float a, float b, float c) {
    float inv_a = 1.0f / A;
    a *= inv_a;
    b *= inv_a;
    c *= inv_a;

    const float one_over_3 = 1.0f / 3;
    const float one_over_9 = 1.0f / 9;
    const float one_over_54 = 1.0f / 54;

    float a2 = a * a;
    float Q = (a2 - b * 3) * one_over_9;
    float R = (2 * a2 * a - 9 * a * b + 27 * c) * one_over_54;
    float R2 = R * R;
    float Q3 = Q * Q * Q;
    float R2MinusQ3 = R2 - Q3;
    float adiv3 = a * one_over_3;
    float r;

    if (R2MinusQ3 < 0) {   // we have 3 real roots
        // the divide/root can, due to finite precisions, be slightly outside of -1...1
        float theta = sk_float_acos(SkTPin(R / sk_float_sqrt(Q3), -1.f, 1.f));
        float neg2RootQ = -2 * sk_float_sqrt(Q);

        r = neg2RootQ * sk_float_cos(theta * one_over_3) - adiv3;
        if (valid(r)) {
            return r;
        }

        r = neg2RootQ * sk_float_cos((theta + 2 * SK_ScalarPI) * one_over_3) - adiv3;
        if (valid(r)) {
            return r;
        }
        r = neg2RootQ * sk_float_cos((theta - 2 * SK_ScalarPI) * one_over_3) - adiv3;
        SkASSERT(valid(r));
    } else {
        float sqrtR2MinusQ3 = sk_float_sqrt(R2MinusQ3);
        float A = sk_float_abs(R) + sqrtR2MinusQ3;
        A = powf(A, one_over_3);
        if (R > 0) {
            A = -A;
        }
        if (A != 0) {
            A += Q / A;
        }
        r = A - adiv3;
    }
    return r;
}

void SkCubicMap::setPts(SkPoint p1, SkPoint p2) {
    Sk2s s1 = Sk2s::Load(&p1) * 3;
    Sk2s s2 = Sk2s::Load(&p2) * 3;

    s1 = Sk2s::Min(Sk2s::Max(s1, 0), 3);
    s2 = Sk2s::Min(Sk2s::Max(s2, 0), 3);

    (Sk2s(1) + s1 - s2).store(&fCoeff[0]);
    (s2 - s1 - s1).store(&fCoeff[1]);
    s1.store(&fCoeff[2]);

    this->buildXTable();
}

SkPoint SkCubicMap::computeFromT(float t) const {
    Sk2s a = Sk2s::Load(&fCoeff[0]);
    Sk2s b = Sk2s::Load(&fCoeff[1]);
    Sk2s c = Sk2s::Load(&fCoeff[2]);

    SkPoint result;
    (((a * t + b) * t + c) * t).store(&result);
    return result;
}

float SkCubicMap::computeYFromX(float x) const {
    x = SkTPin<float>(x, 0, 0.99999f) * kTableCount;
    float ix = sk_float_floor(x);
    int index = (int)ix;
    SkASSERT((unsigned)index < SK_ARRAY_COUNT(fXTable));
    return this->computeFromT(fXTable[index].fT0 + fXTable[index].fDT * (x - ix)).fY;
}

float SkCubicMap::hackYFromX(float x) const {
    x = SkTPin<float>(x, 0, 0.99999f) * kTableCount;
    float ix = sk_float_floor(x);
    int index = (int)ix;
    SkASSERT((unsigned)index < SK_ARRAY_COUNT(fXTable));
    return fXTable[index].fY0 + fXTable[index].fDY * (x - ix);
}

static float compute_slow(float A, float B, float C, float x) {
    double roots[3];
    SkDEBUGCODE(int count =) SkDCubic::RootsValidT(A, B, C, -x, roots);
    SkASSERT(count == 1);
//    float r = solve_nice_cubic(A, B, C, -x);
//    SkDebugf("cary %g, cheap %g\n", (float)roots[0], r);
    return (float)roots[0];
}

static float compute_t_from_x(float A, float B, float C, float x) {
    if (0) {
        return compute_slow(A, B, C, x);
    }
#ifdef SK_DEBUG
    float answer = compute_slow(A, B, C, x);
#endif
    float answer2;
    if (0) {
        return solve_nice_cubic(A, B, C, -x);
    } else {
        answer2 = solve_nice_cubic_halley(A, B, C, -x);
    }
#ifdef SK_DEBUG
    SkASSERT(sk_float_abs(answer - answer2) <= 0.0001f);
#endif
    return answer2;
}

void SkCubicMap::buildXTable() {
    float prevT = 0;

    const float dx = 1.0f / kTableCount;
    float x = dx;

    fXTable[0].fT0 = 0;
    fXTable[0].fY0 = 0;
    for (int i = 1; i < kTableCount; ++i) {
        float t = compute_t_from_x(fCoeff[0].fX, fCoeff[1].fX, fCoeff[2].fX, x);
        SkASSERT(t > prevT);

        fXTable[i - 1].fDT = t - prevT;
        fXTable[i].fT0 = t;

        SkPoint p = this->computeFromT(t);
        fXTable[i - 1].fDY = p.fY - fXTable[i - 1].fY0;
        fXTable[i].fY0 = p.fY;

        prevT = t;
        x += dx;
    }
    fXTable[kTableCount - 1].fDT = 1 - prevT;
    fXTable[kTableCount - 1].fDY = 1 - fXTable[kTableCount - 1].fY0;
}
