/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGeometry_DEFINED
#define SkGeometry_DEFINED

#include "include/core/SkMatrix.h"
#include "include/private/SkNx.h"

static inline Sk2s from_point(const SkPoint& point) {
    return Sk2s::Load(&point);
}

static inline SkPoint to_point(const Sk2s& x) {
    SkPoint point;
    x.store(&point);
    return point;
}

static Sk2s times_2(const Sk2s& value) {
    return value + value;
}

/** Given a quadratic equation Ax^2 + Bx + C = 0, return 0, 1, 2 roots for the
    equation.
*/
int SkFindUnitQuadRoots(SkScalar A, SkScalar B, SkScalar C, SkScalar roots[2]);

///////////////////////////////////////////////////////////////////////////////

SkPoint SkEvalQuadAt(const SkPoint src[3], SkScalar t);
SkPoint SkEvalQuadTangentAt(const SkPoint src[3], SkScalar t);

/** Set pt to the point on the src quadratic specified by t. t must be
    0 <= t <= 1.0
*/
void SkEvalQuadAt(const SkPoint src[3], SkScalar t, SkPoint* pt, SkVector* tangent = nullptr);

/** Given a src quadratic bezier, chop it at the specified t value,
    where 0 < t < 1, and return the two new quadratics in dst:
    dst[0..2] and dst[2..4]
*/
void SkChopQuadAt(const SkPoint src[3], SkPoint dst[5], SkScalar t);

/** Given a src quadratic bezier, chop it at the specified t == 1/2,
    The new quads are returned in dst[0..2] and dst[2..4]
*/
void SkChopQuadAtHalf(const SkPoint src[3], SkPoint dst[5]);

/** Given the 3 coefficients for a quadratic bezier (either X or Y values), look
    for extrema, and return the number of t-values that are found that represent
    these extrema. If the quadratic has no extrema betwee (0..1) exclusive, the
    function returns 0.
    Returned count      tValues[]
    0                   ignored
    1                   0 < tValues[0] < 1
*/
int SkFindQuadExtrema(SkScalar a, SkScalar b, SkScalar c, SkScalar tValues[1]);

/** Given 3 points on a quadratic bezier, chop it into 1, 2 beziers such that
    the resulting beziers are monotonic in Y. This is called by the scan converter.
    Depending on what is returned, dst[] is treated as follows
    0   dst[0..2] is the original quad
    1   dst[0..2] and dst[2..4] are the two new quads
*/
int SkChopQuadAtYExtrema(const SkPoint src[3], SkPoint dst[5]);
int SkChopQuadAtXExtrema(const SkPoint src[3], SkPoint dst[5]);

/** Given 3 points on a quadratic bezier, if the point of maximum
    curvature exists on the segment, returns the t value for this
    point along the curve. Otherwise it will return a value of 0.
*/
SkScalar SkFindQuadMaxCurvature(const SkPoint src[3]);

/** Given 3 points on a quadratic bezier, divide it into 2 quadratics
    if the point of maximum curvature exists on the quad segment.
    Depending on what is returned, dst[] is treated as follows
    1   dst[0..2] is the original quad
    2   dst[0..2] and dst[2..4] are the two new quads
    If dst == null, it is ignored and only the count is returned.
*/
int SkChopQuadAtMaxCurvature(const SkPoint src[3], SkPoint dst[5]);

/** Given 3 points on a quadratic bezier, use degree elevation to
    convert it into the cubic fitting the same curve. The new cubic
    curve is returned in dst[0..3].
*/
SK_API void SkConvertQuadToCubic(const SkPoint src[3], SkPoint dst[4]);

///////////////////////////////////////////////////////////////////////////////

/** Set pt to the point on the src cubic specified by t. t must be
    0 <= t <= 1.0
*/
void SkEvalCubicAt(const SkPoint src[4], SkScalar t, SkPoint* locOrNull,
                   SkVector* tangentOrNull, SkVector* curvatureOrNull);

/** Given a src cubic bezier, chop it at the specified t value,
    where 0 < t < 1, and return the two new cubics in dst:
    dst[0..3] and dst[3..6]
*/
void SkChopCubicAt(const SkPoint src[4], SkPoint dst[7], SkScalar t);

/** Given a src cubic bezier, chop it at the specified t values,
    where 0 < t < 1, and return the new cubics in dst:
    dst[0..3],dst[3..6],...,dst[3*t_count..3*(t_count+1)]
*/
void SkChopCubicAt(const SkPoint src[4], SkPoint dst[], const SkScalar t[],
                   int t_count);

/** Given a src cubic bezier, chop it at the specified t == 1/2,
    The new cubics are returned in dst[0..3] and dst[3..6]
*/
void SkChopCubicAtHalf(const SkPoint src[4], SkPoint dst[7]);

/** Given the 4 coefficients for a cubic bezier (either X or Y values), look
    for extrema, and return the number of t-values that are found that represent
    these extrema. If the cubic has no extrema betwee (0..1) exclusive, the
    function returns 0.
    Returned count      tValues[]
    0                   ignored
    1                   0 < tValues[0] < 1
    2                   0 < tValues[0] < tValues[1] < 1
*/
int SkFindCubicExtrema(SkScalar a, SkScalar b, SkScalar c, SkScalar d,
                       SkScalar tValues[2]);

/** Given 4 points on a cubic bezier, chop it into 1, 2, 3 beziers such that
    the resulting beziers are monotonic in Y. This is called by the scan converter.
    Depending on what is returned, dst[] is treated as follows
    0   dst[0..3] is the original cubic
    1   dst[0..3] and dst[3..6] are the two new cubics
    2   dst[0..3], dst[3..6], dst[6..9] are the three new cubics
    If dst == null, it is ignored and only the count is returned.
*/
int SkChopCubicAtYExtrema(const SkPoint src[4], SkPoint dst[10]);
int SkChopCubicAtXExtrema(const SkPoint src[4], SkPoint dst[10]);

/** Given a cubic bezier, return 0, 1, or 2 t-values that represent the
    inflection points.
*/
int SkFindCubicInflections(const SkPoint src[4], SkScalar tValues[2]);

/** Return 1 for no chop, 2 for having chopped the cubic at a single
    inflection point, 3 for having chopped at 2 inflection points.
    dst will hold the resulting 1, 2, or 3 cubics.
*/
int SkChopCubicAtInflections(const SkPoint src[4], SkPoint dst[10]);

int SkFindCubicMaxCurvature(const SkPoint src[4], SkScalar tValues[3]);
int SkChopCubicAtMaxCurvature(const SkPoint src[4], SkPoint dst[13],
                              SkScalar tValues[3] = nullptr);
/** Returns t value of cusp if cubic has one; returns -1 otherwise.
 */
SkScalar SkFindCubicCusp(const SkPoint src[4]);

bool SkChopMonoCubicAtX(SkPoint src[4], SkScalar y, SkPoint dst[7]);
bool SkChopMonoCubicAtY(SkPoint src[4], SkScalar x, SkPoint dst[7]);

enum class SkCubicType {
    kSerpentine,
    kLoop,
    kLocalCusp,       // Cusp at a non-infinite parameter value with an inflection at t=infinity.
    kCuspAtInfinity,  // Cusp with a cusp at t=infinity and a local inflection.
    kQuadratic,
    kLineOrPoint
};

static inline bool SkCubicIsDegenerate(SkCubicType type) {
    switch (type) {
        case SkCubicType::kSerpentine:
        case SkCubicType::kLoop:
        case SkCubicType::kLocalCusp:
        case SkCubicType::kCuspAtInfinity:
            return false;
        case SkCubicType::kQuadratic:
        case SkCubicType::kLineOrPoint:
            return true;
    }
    SK_ABORT("Invalid SkCubicType");
}

static inline const char* SkCubicTypeName(SkCubicType type) {
    switch (type) {
        case SkCubicType::kSerpentine: return "kSerpentine";
        case SkCubicType::kLoop: return "kLoop";
        case SkCubicType::kLocalCusp: return "kLocalCusp";
        case SkCubicType::kCuspAtInfinity: return "kCuspAtInfinity";
        case SkCubicType::kQuadratic: return "kQuadratic";
        case SkCubicType::kLineOrPoint: return "kLineOrPoint";
    }
    SK_ABORT("Invalid SkCubicType");
}

/** Returns the cubic classification.

    t[],s[] are set to the two homogeneous parameter values at which points the lines L & M
    intersect with K, sorted from smallest to largest and oriented so positive values of the
    implicit are on the "left" side. For a serpentine curve they are the inflection points. For a
    loop they are the double point. For a local cusp, they are both equal and denote the cusp point.
    For a cusp at an infinite parameter value, one will be the local inflection point and the other
    +inf (t,s = 1,0). If the curve is degenerate (i.e. quadratic or linear) they are both set to a
    parameter value of +inf (t,s = 1,0).

    d[] is filled with the cubic inflection function coefficients. See "Resolution Independent
    Curve Rendering using Programmable Graphics Hardware", 4.2 Curve Categorization:

    If the input points contain infinities or NaN, the return values are undefined.

    https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
*/
SkCubicType SkClassifyCubic(const SkPoint p[4], double t[2] = nullptr, double s[2] = nullptr,
                            double d[4] = nullptr);

///////////////////////////////////////////////////////////////////////////////

enum SkRotationDirection {
    kCW_SkRotationDirection,
    kCCW_SkRotationDirection
};

struct SkConic {
    SkConic() {}
    SkConic(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2, SkScalar w) {
        fPts[0] = p0;
        fPts[1] = p1;
        fPts[2] = p2;
        fW = w;
    }
    SkConic(const SkPoint pts[3], SkScalar w) {
        memcpy(fPts, pts, sizeof(fPts));
        fW = w;
    }

    SkPoint  fPts[3];
    SkScalar fW;

    void set(const SkPoint pts[3], SkScalar w) {
        memcpy(fPts, pts, 3 * sizeof(SkPoint));
        fW = w;
    }

    void set(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2, SkScalar w) {
        fPts[0] = p0;
        fPts[1] = p1;
        fPts[2] = p2;
        fW = w;
    }

    /**
     *  Given a t-value [0...1] return its position and/or tangent.
     *  If pos is not null, return its position at the t-value.
     *  If tangent is not null, return its tangent at the t-value. NOTE the
     *  tangent value's length is arbitrary, and only its direction should
     *  be used.
     */
    void evalAt(SkScalar t, SkPoint* pos, SkVector* tangent = nullptr) const;
    bool SK_WARN_UNUSED_RESULT chopAt(SkScalar t, SkConic dst[2]) const;
    void chopAt(SkScalar t1, SkScalar t2, SkConic* dst) const;
    void chop(SkConic dst[2]) const;

    SkPoint evalAt(SkScalar t) const;
    SkVector evalTangentAt(SkScalar t) const;

    void computeAsQuadError(SkVector* err) const;
    bool asQuadTol(SkScalar tol) const;

    /**
     *  return the power-of-2 number of quads needed to approximate this conic
     *  with a sequence of quads. Will be >= 0.
     */
    int SK_API computeQuadPOW2(SkScalar tol) const;

    /**
     *  Chop this conic into N quads, stored continguously in pts[], where
     *  N = 1 << pow2. The amount of storage needed is (1 + 2 * N)
     */
    int SK_API SK_WARN_UNUSED_RESULT chopIntoQuadsPOW2(SkPoint pts[], int pow2) const;

    bool findXExtrema(SkScalar* t) const;
    bool findYExtrema(SkScalar* t) const;
    bool chopAtXExtrema(SkConic dst[2]) const;
    bool chopAtYExtrema(SkConic dst[2]) const;

    void computeTightBounds(SkRect* bounds) const;
    void computeFastBounds(SkRect* bounds) const;

    /** Find the parameter value where the conic takes on its maximum curvature.
     *
     *  @param t   output scalar for max curvature.  Will be unchanged if
     *             max curvature outside 0..1 range.
     *
     *  @return  true if max curvature found inside 0..1 range, false otherwise
     */
//    bool findMaxCurvature(SkScalar* t) const;  // unimplemented

    static SkScalar TransformW(const SkPoint[3], SkScalar w, const SkMatrix&);

    enum {
        kMaxConicsForArc = 5
    };
    static int BuildUnitArc(const SkVector& start, const SkVector& stop, SkRotationDirection,
                            const SkMatrix*, SkConic conics[kMaxConicsForArc]);
};

// inline helpers are contained in a namespace to avoid external leakage to fragile SkNx members
namespace {  // NOLINT(google-build-namespaces)

/**
 *  use for : eval(t) == A * t^2 + B * t + C
 */
struct SkQuadCoeff {
    SkQuadCoeff() {}

    SkQuadCoeff(const Sk2s& A, const Sk2s& B, const Sk2s& C)
        : fA(A)
        , fB(B)
        , fC(C)
    {
    }

    SkQuadCoeff(const SkPoint src[3]) {
        fC = from_point(src[0]);
        Sk2s P1 = from_point(src[1]);
        Sk2s P2 = from_point(src[2]);
        fB = times_2(P1 - fC);
        fA = P2 - times_2(P1) + fC;
    }

    Sk2s eval(SkScalar t) {
        Sk2s tt(t);
        return eval(tt);
    }

    Sk2s eval(const Sk2s& tt) {
        return (fA * tt + fB) * tt + fC;
    }

    Sk2s fA;
    Sk2s fB;
    Sk2s fC;
};

struct SkConicCoeff {
    SkConicCoeff(const SkConic& conic) {
        Sk2s p0 = from_point(conic.fPts[0]);
        Sk2s p1 = from_point(conic.fPts[1]);
        Sk2s p2 = from_point(conic.fPts[2]);
        Sk2s ww(conic.fW);

        Sk2s p1w = p1 * ww;
        fNumer.fC = p0;
        fNumer.fA = p2 - times_2(p1w) + p0;
        fNumer.fB = times_2(p1w - p0);

        fDenom.fC = Sk2s(1);
        fDenom.fB = times_2(ww - fDenom.fC);
        fDenom.fA = Sk2s(0) - fDenom.fB;
    }

    Sk2s eval(SkScalar t) {
        Sk2s tt(t);
        Sk2s numer = fNumer.eval(tt);
        Sk2s denom = fDenom.eval(tt);
        return numer / denom;
    }

    SkQuadCoeff fNumer;
    SkQuadCoeff fDenom;
};

struct SkCubicCoeff {
    SkCubicCoeff(const SkPoint src[4]) {
        Sk2s P0 = from_point(src[0]);
        Sk2s P1 = from_point(src[1]);
        Sk2s P2 = from_point(src[2]);
        Sk2s P3 = from_point(src[3]);
        Sk2s three(3);
        fA = P3 + three * (P1 - P2) - P0;
        fB = three * (P2 - times_2(P1) + P0);
        fC = three * (P1 - P0);
        fD = P0;
    }

    Sk2s eval(SkScalar t) {
        Sk2s tt(t);
        return eval(tt);
    }

    Sk2s eval(const Sk2s& t) {
        return ((fA * t + fB) * t + fC) * t + fD;
    }

    Sk2s fA;
    Sk2s fB;
    Sk2s fC;
    Sk2s fD;
};

}

#include "include/private/SkTemplates.h"

/**
 *  Help class to allocate storage for approximating a conic with N quads.
 */
class SkAutoConicToQuads {
public:
    SkAutoConicToQuads() : fQuadCount(0) {}

    /**
     *  Given a conic and a tolerance, return the array of points for the
     *  approximating quad(s). Call countQuads() to know the number of quads
     *  represented in these points.
     *
     *  The quads are allocated to share end-points. e.g. if there are 4 quads,
     *  there will be 9 points allocated as follows
     *      quad[0] == pts[0..2]
     *      quad[1] == pts[2..4]
     *      quad[2] == pts[4..6]
     *      quad[3] == pts[6..8]
     */
    const SkPoint* computeQuads(const SkConic& conic, SkScalar tol) {
        int pow2 = conic.computeQuadPOW2(tol);
        fQuadCount = 1 << pow2;
        SkPoint* pts = fStorage.reset(1 + 2 * fQuadCount);
        fQuadCount = conic.chopIntoQuadsPOW2(pts, pow2);
        return pts;
    }

    const SkPoint* computeQuads(const SkPoint pts[3], SkScalar weight,
                                SkScalar tol) {
        SkConic conic;
        conic.set(pts, weight);
        return computeQuads(conic, tol);
    }

    int countQuads() const { return fQuadCount; }

private:
    enum {
        kQuadCount = 8, // should handle most conics
        kPointCount = 1 + 2 * kQuadCount,
    };
    SkAutoSTMalloc<kPointCount, SkPoint> fStorage;
    int fQuadCount; // #quads for current usage
};

#endif
