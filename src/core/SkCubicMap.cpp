/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCubicMap.h"
#include "SkNx.h"
#include "../../src/pathops/SkPathOpsCubic.h"

static bool valid(float r) {
    return r >= 0 && r <= 1;
}

static float solve_nice_cubic(float A, float a, float b, float c) {
    float inv_a = 1.0f / A;
    a *= inv_a;
    b *= inv_a;
    c *= inv_a;

    const float one_third = 1.0f / 3;

    float a2 = a * a;
    float Q = (a2 - b * 3) / 9;
    float R = (2 * a2 * a - 9 * a * b + 27 * c) / 54;
    float R2 = R * R;
    float Q3 = Q * Q * Q;
    float R2MinusQ3 = R2 - Q3;
    float adiv3 = a * one_third;
    float r;

    if (R2MinusQ3 < 0) {   // we have 3 real roots
        // the divide/root can, due to finite precisions, be slightly outside of -1...1
        float theta = sk_float_acos(SkTPin(R / sk_float_sqrt(Q3), -1.f, 1.f));
        float neg2RootQ = -2 * sk_float_sqrt(Q);

        r = neg2RootQ * sk_float_cos(theta / 3) - adiv3;
        if (valid(r)) {
            return r;
        }

        r = neg2RootQ * sk_float_cos((theta + 2 * SK_ScalarPI) / 3) - adiv3;
        if (valid(r)) {
            return r;
        }
        r = neg2RootQ * sk_float_cos((theta - 2 * SK_ScalarPI) / 3) - adiv3;
        SkASSERT(valid(r));
    } else {
        float sqrtR2MinusQ3 = sk_float_sqrt(R2MinusQ3);
        float A = sk_float_abs(R) + sqrtR2MinusQ3;
        A = powf(A, one_third);
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

static float compute_t_from_x(float A, float B, float C, float x) {
#if 0
    double roots[3];
    SkDEBUGCODE(int count =) SkDCubic::RootsValidT(A, B, C, -x, roots);
    SkASSERT(count == 1);
    float r = solve_nice_cubic(A, B, C, -x);
    SkDebugf("cary %g, cheap %g\n", (float)roots[0], r);
    return (float)roots[0];
#else
    return solve_nice_cubic(A, B, C, -x);
#endif
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
