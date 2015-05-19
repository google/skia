/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGeometry_DEFINED
#define SkGeometry_DEFINED

#include "SkMatrix.h"
#include "SkNx.h"

static inline Sk2s from_point(const SkPoint& point) {
    return Sk2s::Load(&point.fX);
}

static inline SkPoint to_point(const Sk2s& x) {
    SkPoint point;
    x.store(&point.fX);
    return point;
}

static inline Sk2s sk2s_cubic_eval(const Sk2s& A, const Sk2s& B, const Sk2s& C, const Sk2s& D,
                                   const Sk2s& t) {
    return ((A * t + B) * t + C) * t + D;
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
void SkEvalQuadAt(const SkPoint src[3], SkScalar t, SkPoint* pt, SkVector* tangent = NULL);

/**
 *  output is : eval(t) == coeff[0] * t^2 + coeff[1] * t + coeff[2]
 */
void SkQuadToCoeff(const SkPoint pts[3], SkPoint coeff[3]);

/**
 *  output is : eval(t) == coeff[0] * t^3 + coeff[1] * t^2 + coeff[2] * t + coeff[3]
 */
void SkCubicToCoeff(const SkPoint pts[4], SkPoint coeff[4]);

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
                              SkScalar tValues[3] = NULL);

bool SkChopMonoCubicAtX(SkPoint src[4], SkScalar y, SkPoint dst[7]);
bool SkChopMonoCubicAtY(SkPoint src[4], SkScalar x, SkPoint dst[7]);

enum SkCubicType {
    kSerpentine_SkCubicType,
    kCusp_SkCubicType,
    kLoop_SkCubicType,
    kQuadratic_SkCubicType,
    kLine_SkCubicType,
    kPoint_SkCubicType
};

/** Returns the cubic classification. Pass scratch storage for computing inflection data,
    which can be used with additional work to find the loop intersections and so on.
*/
SkCubicType SkClassifyCubic(const SkPoint p[4], SkScalar inflection[3]);

///////////////////////////////////////////////////////////////////////////////

enum SkRotationDirection {
    kCW_SkRotationDirection,
    kCCW_SkRotationDirection
};

/** Maximum number of points needed in the quadPoints[] parameter for
    SkBuildQuadArc()
*/
#define kSkBuildQuadArcStorage  17

/** Given 2 unit vectors and a rotation direction, fill out the specified
    array of points with quadratic segments. Return is the number of points
    written to, which will be { 0, 3, 5, 7, ... kSkBuildQuadArcStorage }

    matrix, if not null, is appled to the points before they are returned.
*/
int SkBuildQuadArc(const SkVector& unitStart, const SkVector& unitStop,
                   SkRotationDirection, const SkMatrix*, SkPoint quadPoints[]);

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
    void evalAt(SkScalar t, SkPoint* pos, SkVector* tangent = NULL) const;
    void chopAt(SkScalar t, SkConic dst[2]) const;
    void chop(SkConic dst[2]) const;

    SkPoint evalAt(SkScalar t) const;
    SkVector evalTangentAt(SkScalar t) const;

    void computeAsQuadError(SkVector* err) const;
    bool asQuadTol(SkScalar tol) const;

    /**
     *  return the power-of-2 number of quads needed to approximate this conic
     *  with a sequence of quads. Will be >= 0.
     */
    int computeQuadPOW2(SkScalar tol) const;

    /**
     *  Chop this conic into N quads, stored continguously in pts[], where
     *  N = 1 << pow2. The amount of storage needed is (1 + 2 * N)
     */
    int chopIntoQuadsPOW2(SkPoint pts[], int pow2) const;

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

#include "SkTemplates.h"

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
        conic.chopIntoQuadsPOW2(pts, pow2);
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
