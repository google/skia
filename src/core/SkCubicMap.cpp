/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCubicMap.h"
#include "include/private/SkNx.h"
#include "src/core/SkOpts.h"

//#define CUBICMAP_TRACK_MAX_ERROR

#ifdef CUBICMAP_TRACK_MAX_ERROR
#include "src/pathops/SkPathOpsCubic.h"
#endif

static inline bool nearly_zero(SkScalar x) {
    SkASSERT(x >= 0);
    return x <= 0.0000000001f;
}

#ifdef CUBICMAP_TRACK_MAX_ERROR
    static int max_iters;
#endif

#ifdef CUBICMAP_TRACK_MAX_ERROR
static float compute_slow(float A, float B, float C, float x) {
    double roots[3];
    SkDEBUGCODE(int count =) SkDCubic::RootsValidT(A, B, C, -x, roots);
    SkASSERT(count == 1);
    return (float)roots[0];
}

static float max_err;
#endif

static float compute_t_from_x(float A, float B, float C, float x) {
#ifdef CUBICMAP_TRACK_MAX_ERROR
    float answer = compute_slow(A, B, C, x);
#endif
    float answer2 = SkOpts::cubic_solver(A, B, C, -x);

#ifdef CUBICMAP_TRACK_MAX_ERROR
    float err = sk_float_abs(answer - answer2);
    if (err > max_err) {
        max_err = err;
        SkDebugf("max error %g\n", max_err);
    }
#endif
    return answer2;
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
        t = compute_t_from_x(fCoeff[0].fX, fCoeff[1].fX, fCoeff[2].fX, x);
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
}

SkPoint SkCubicMap::computeFromT(float t) const {
    Sk2s a = Sk2s::Load(&fCoeff[0]);
    Sk2s b = Sk2s::Load(&fCoeff[1]);
    Sk2s c = Sk2s::Load(&fCoeff[2]);

    SkPoint result;
    (((a * t + b) * t + c) * t).store(&result);
    return result;
}

SkCubicMapExperiment::SkCubicMapExperiment(SkPoint P1, SkPoint P2) {

    // 3 * p1 * t + (-6 * p1 + 3 * p2) * t^2 + (1 + 3 * p1 - 3 * p2) * t^3
    SkPoint P1x3 = P1 * 3,
            P2x3 = P2 * 3;

    // A, B, C for A * t^3 + B * t^2 + C * t
    fA = P1x3 - P2x3 + SkPoint{1, 1};
    fB = P1x3 * -2 + P2x3;
    fC = P1x3;

    // Derivative of x only.
    fDA = 3 * fA.fX;
    fDB = 2 * fB.fX;
    fDC = 1 * fC.fX;

    float a = fA.fX,
          b = fB.fX,
          c = fC.fX,
          d = 0;

    int quadCount = 2;
    float quadInterval = 1.0f / quadCount;

    for (int i = 0; i < quadCount; i++) {
        float s1 = i * quadInterval,
              s3 = (i + 1) * quadInterval,
              s2 = (s1 + s3) / 2;

        float d1213 = 1 / ((s1 - s2) * (s1 - s3)),
              d1223 = 1 / ((s1 - s2) * (s2 - s3)),
              d1323 = 1 / ((s1 - s3) * (s2 - s3));

        float v1 = EvalPoly(s1, a, b, c, d),
              v2 = EvalPoly(s2, a, b, c, d),
              v3 = EvalPoly(s3, a, b, c, d);

        fCuts[i] = v3;

        // qa = v1/((s1-s2)(s1-s3))-v2/((s1-s2)(s2-s3))+v3/((s1-s3)(s2-s3))
        // qb =
        // -((s3+s2)v1)/((s1-s2)(s1-s3))+((s1+s3)v2)/((s1-s2)(s2-s3))-((s2+s1)v3)/((s1-s3)(s2-s3))
        // qa = (s2 s3 v1)/((s1-s2)(s1-s3))-(s1 s3 v2)/((s1-s2)(s2-s3))+(s1 s2 v3)/((s1-s3)(s2-s3))
        float qa = v1 * d1213 - v2 * d1223 + v3 * d1323,
              qb = -((s2 + s3) * v1) * d1213 + ((s1 + s3) * v2) * d1223 - ((s1 + s2) * v3) * d1323,
              qc = (v1 * s2 * s3) * d1213 - (s1 * v2 * s3) * d1223 + (s1 * s2 * v3) * d1323;

        fT[i] = -qb / (2 * qa);
        fU[i] = 1 / (2 * qa);
        fV[i] = qb * qb;
        fW[i] = 4 * qa;
        fX[i] = qc;
    }

    A = {fA.fX, 0, 0, 0};
    B = {fB.fX, fDA, 0, 0};
    C = {fC.fX, fDB, 0, 0};
}

#ifdef SK_CUBICMAP_TRACK_MAX_ERROR
void SkCubicMapExperiment::StatsError::sample(SkScalar a, SkScalar b) {
    SkScalar e = std::abs(a - b);
    sum0 += 1;
    sum1 += e;
    maxError = std::max(maxError, e);
}

SkCubicMapExperiment::StatsError::~StatsError() {
    SkDebugf("maxError %g avg %g\n", maxError, sum1/sum0);
}
#endif