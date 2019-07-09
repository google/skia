/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCubicMap_DEFINED
#define SkCubicMap_DEFINED

#include "include/core/SkPoint.h"
#include "include/private/SkNx.h"

/**
 *  Fast evaluation of a cubic ease-in / ease-out curve. This is defined as a parametric cubic
 *  curve inside the unit square.
 *
 *  pt[0] is implicitly { 0, 0 }
 *  pt[3] is implicitly { 1, 1 }
 *  pts[1,2].X are inside the unit [0..1]
 */
class SK_API SkCubicMap {
public:
    SkCubicMap(SkPoint p1, SkPoint p2);

    static bool IsLinear(SkPoint p1, SkPoint p2) {
        return SkScalarNearlyEqual(p1.fX, p1.fY) && SkScalarNearlyEqual(p2.fX, p2.fY);
    }

    float computeYFromX(float x) const;

    SkPoint computeFromT(float t) const;

private:
    enum Type {
        kLine_Type,     // x == y
        kCubeRoot_Type, // At^3 == x
        kSolver_Type,   // general monotonic cubic solver
    };

    SkPoint fCoeff[3];
    Type    fType;
};

//#define SK_CUBICMAP_TRACK_MAX_ERROR

class SK_API SkCubicMapExperiment {
public:
    SkCubicMapExperiment(SkPoint p1, SkPoint p2);
    float computeYFromX(float x) const;

private:
#ifdef SK_CUBICMAP_TRACK_MAX_ERROR
    struct StatsError {
        void sample(SkScalar a, SkScalar b);
        ~StatsError();

        SkScalar sum0 = 0;
        SkScalar sum1 = 0;
        SkScalar maxError = 0;
    };
#else
    struct StatsError {
        void sample(SkScalar a, SkScalar b) {}
        ~StatsError() = default;
    };
#endif

    static float EvalPoly(float t, float b) {
        return b;
    }

    template <typename... Rest>
    static float EvalPoly(float t, float m, float b, Rest... rest) {
        return EvalPoly(t, sk_fmaf(m, t, b), rest...);
    }

    static SkNx<4,float> EvalPoly(SkNx<4, float> t, SkNx<4, float> b) {
        return b;
    }

    template <typename... Rest>
    static SkNx<4,float> EvalPoly(SkNx<4,float> t, SkNx<4,float> m, SkNx<4,float> b, Rest... rest) {
        return EvalPoly(t, SkNx_fma(m, t, b), rest...);
    }

    SkPoint fA, fB, fC;
    //SkScalar fDA, fDB, fDC;

    SkScalar fT[2], fU[2], fV[2], fW[2], fX[2];
    SkScalar fSwitchPoint;
    SkNx<4, float> fPA, fPB, fPC;
    mutable SkNx<4, float> fPD;
    //SkNx<4, float> A, B, C;
    mutable StatsError stats;
};

inline SkScalar SkCubicMapExperiment::computeYFromX(SkScalar x) const {
    static const SkScalar epsilon01 = 0.000001f;
    if (x <= epsilon01) {return x;}
    if (x >= (1 - epsilon01)) {return x;}

    int i = (x < fSwitchPoint) ? 0 : 1;

    // T + U * sqrt(V - W(X-x))
    SkScalar t = fT[i] + fU[i] * std::sqrtf(fV[i] - fW[i] * (fX[i] - x));
    SkASSERT(0 <= t && t <= 1);

    SkNx<4, float> D = {-x, fPD[1], 0, 0};

    SkNx<4, float> XdX = EvalPoly(t, fPA, fPB, fPC, D);
    SkNx<4, float> inv = XdX.invert();

    t = t - XdX[0] * inv[1];

    stats.sample(x, EvalPoly(t, fA.fX, fB.fX, fC.fX, 0));

    return EvalPoly(t, fA.fY, fB.fY, fC.fY, 0);
}

#endif

