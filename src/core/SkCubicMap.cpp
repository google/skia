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
    SkScalar DA = 3 * fA.fX,
             DB = 2 * fB.fX,
             DC = 1 * fC.fX;

    // The cubic will be sampled at three equally space places (remember the end are 0 and 1), so
    // there will be 5 points to generate 3 quadratics. The sample arguments .25, .50, .75
    // result in the values v25, v50, v75.


    SkScalar v25 = EvalPoly(0.25f, fA.fX, fB.fX, fC.fX, 0),
             v50 = EvalPoly(0.50f, fA.fX, fB.fX, fC.fX, 0),
             v75 = EvalPoly(0.75f, fA.fX, fB.fX, fC.fX, 0);

    // The switch from one quadratic to the next happens at the point v50.
    fSwitchPoint = v50;

    // (-b + sqrt(b^2 - 4a(c - x))) / 2a
    // -b/(2a) + 1/(2a) * sqrt(b^2 - 4a(c-x))
    // T + U * sqrt(V - W(X-x))
    {
        // a = 8*(h-2*q), b = -2*(h-4*q), c = 0
        float a = 8 * (v50 - 2 * v25);
        float b = -2 * (v50 - 4 * v25);
        float c = 0;
        fT[0] = -b / (2 * a);
        fU[0] = 1 / (2 * a);
        fV[0] = b * b;
        fW[0] = 4 * a;
        fX[0] = c;
    }
    {
        // a = 8*(1 + h - 2*qqq), b = -2*(5 + 7*h - 12*qqq), c = 3 + 6*h - 8*qqq
        float a = 8 * (1 + v50 - 2 * v75);
        float b = -2 * (5 + 7 * v50 - 12 * v75);
        float c = 3 + 6 * v50 - 8 * v75;
        fT[1] = -b / (2 * a);
        fU[1] = 1 / (2 * a);
        fV[1] = b * b;
        fW[1] = 4 * a;
        fX[1] = c;
    }

    fPA = SkNx<4, float>{fA.fX,  0, 0, 0};//.store(fPA);
    fPB = SkNx<4, float>{fB.fX, DA, 0, 0};//.store(fPB);
    fPC = SkNx<4, float>{fC.fX, DB, 0, 0};//.store(fPC);
    fPD = SkNx<4, float>{    0, DC, 0, 0};//.store(fPD);
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