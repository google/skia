/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGeometry.h"
#include "SkMatrix.h"
#include "SkNx.h"

static SkVector to_vector(const Sk2s& x) {
    SkVector vector;
    x.store(&vector);
    return vector;
}

////////////////////////////////////////////////////////////////////////

static int is_not_monotonic(SkScalar a, SkScalar b, SkScalar c) {
    SkScalar ab = a - b;
    SkScalar bc = b - c;
    if (ab < 0) {
        bc = -bc;
    }
    return ab == 0 || bc < 0;
}

////////////////////////////////////////////////////////////////////////

static bool is_unit_interval(SkScalar x) {
    return x > 0 && x < SK_Scalar1;
}

static int valid_unit_divide(SkScalar numer, SkScalar denom, SkScalar* ratio) {
    SkASSERT(ratio);

    if (numer < 0) {
        numer = -numer;
        denom = -denom;
    }

    if (denom == 0 || numer == 0 || numer >= denom) {
        return 0;
    }

    SkScalar r = numer / denom;
    if (SkScalarIsNaN(r)) {
        return 0;
    }
    SkASSERTF(r >= 0 && r < SK_Scalar1, "numer %f, denom %f, r %f", numer, denom, r);
    if (r == 0) { // catch underflow if numer <<<< denom
        return 0;
    }
    *ratio = r;
    return 1;
}

/** From Numerical Recipes in C.

    Q = -1/2 (B + sign(B) sqrt[B*B - 4*A*C])
    x1 = Q / A
    x2 = C / Q
*/
int SkFindUnitQuadRoots(SkScalar A, SkScalar B, SkScalar C, SkScalar roots[2]) {
    SkASSERT(roots);

    if (A == 0) {
        return valid_unit_divide(-C, B, roots);
    }

    SkScalar* r = roots;

    SkScalar R = B*B - 4*A*C;
    if (R < 0 || !SkScalarIsFinite(R)) {  // complex roots
        // if R is infinite, it's possible that it may still produce
        // useful results if the operation was repeated in doubles
        // the flipside is determining if the more precise answer
        // isn't useful because surrounding machinery (e.g., subtracting
        // the axis offset from C) already discards the extra precision
        // more investigation and unit tests required...
        return 0;
    }
    R = SkScalarSqrt(R);

    SkScalar Q = (B < 0) ? -(B-R)/2 : -(B+R)/2;
    r += valid_unit_divide(Q, A, r);
    r += valid_unit_divide(C, Q, r);
    if (r - roots == 2) {
        if (roots[0] > roots[1])
            SkTSwap<SkScalar>(roots[0], roots[1]);
        else if (roots[0] == roots[1])  // nearly-equal?
            r -= 1; // skip the double root
    }
    return (int)(r - roots);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void SkEvalQuadAt(const SkPoint src[3], SkScalar t, SkPoint* pt, SkVector* tangent) {
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= SK_Scalar1);

    if (pt) {
        *pt = SkEvalQuadAt(src, t);
    }
    if (tangent) {
        *tangent = SkEvalQuadTangentAt(src, t);
    }
}

SkPoint SkEvalQuadAt(const SkPoint src[3], SkScalar t) {
    return to_point(SkQuadCoeff(src).eval(t));
}

SkVector SkEvalQuadTangentAt(const SkPoint src[3], SkScalar t) {
    // The derivative equation is 2(b - a +(a - 2b +c)t). This returns a
    // zero tangent vector when t is 0 or 1, and the control point is equal
    // to the end point. In this case, use the quad end points to compute the tangent.
    if ((t == 0 && src[0] == src[1]) || (t == 1 && src[1] == src[2])) {
        return src[2] - src[0];
    }
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= SK_Scalar1);

    Sk2s P0 = from_point(src[0]);
    Sk2s P1 = from_point(src[1]);
    Sk2s P2 = from_point(src[2]);

    Sk2s B = P1 - P0;
    Sk2s A = P2 - P1 - B;
    Sk2s T = A * Sk2s(t) + B;

    return to_vector(T + T);
}

static inline Sk2s interp(const Sk2s& v0, const Sk2s& v1, const Sk2s& t) {
    return v0 + (v1 - v0) * t;
}

void SkChopQuadAt(const SkPoint src[3], SkPoint dst[5], SkScalar t) {
    SkASSERT(t > 0 && t < SK_Scalar1);

    Sk2s p0 = from_point(src[0]);
    Sk2s p1 = from_point(src[1]);
    Sk2s p2 = from_point(src[2]);
    Sk2s tt(t);

    Sk2s p01 = interp(p0, p1, tt);
    Sk2s p12 = interp(p1, p2, tt);

    dst[0] = to_point(p0);
    dst[1] = to_point(p01);
    dst[2] = to_point(interp(p01, p12, tt));
    dst[3] = to_point(p12);
    dst[4] = to_point(p2);
}

void SkChopQuadAtHalf(const SkPoint src[3], SkPoint dst[5]) {
    SkChopQuadAt(src, dst, 0.5f);
}

/** Quad'(t) = At + B, where
    A = 2(a - 2b + c)
    B = 2(b - a)
    Solve for t, only if it fits between 0 < t < 1
*/
int SkFindQuadExtrema(SkScalar a, SkScalar b, SkScalar c, SkScalar tValue[1]) {
    /*  At + B == 0
        t = -B / A
    */
    return valid_unit_divide(a - b, a - b - b + c, tValue);
}

static inline void flatten_double_quad_extrema(SkScalar coords[14]) {
    coords[2] = coords[6] = coords[4];
}

/*  Returns 0 for 1 quad, and 1 for two quads, either way the answer is
 stored in dst[]. Guarantees that the 1/2 quads will be monotonic.
 */
int SkChopQuadAtYExtrema(const SkPoint src[3], SkPoint dst[5]) {
    SkASSERT(src);
    SkASSERT(dst);

    SkScalar a = src[0].fY;
    SkScalar b = src[1].fY;
    SkScalar c = src[2].fY;

    if (is_not_monotonic(a, b, c)) {
        SkScalar    tValue;
        if (valid_unit_divide(a - b, a - b - b + c, &tValue)) {
            SkChopQuadAt(src, dst, tValue);
            flatten_double_quad_extrema(&dst[0].fY);
            return 1;
        }
        // if we get here, we need to force dst to be monotonic, even though
        // we couldn't compute a unit_divide value (probably underflow).
        b = SkScalarAbs(a - b) < SkScalarAbs(b - c) ? a : c;
    }
    dst[0].set(src[0].fX, a);
    dst[1].set(src[1].fX, b);
    dst[2].set(src[2].fX, c);
    return 0;
}

/*  Returns 0 for 1 quad, and 1 for two quads, either way the answer is
    stored in dst[]. Guarantees that the 1/2 quads will be monotonic.
 */
int SkChopQuadAtXExtrema(const SkPoint src[3], SkPoint dst[5]) {
    SkASSERT(src);
    SkASSERT(dst);

    SkScalar a = src[0].fX;
    SkScalar b = src[1].fX;
    SkScalar c = src[2].fX;

    if (is_not_monotonic(a, b, c)) {
        SkScalar tValue;
        if (valid_unit_divide(a - b, a - b - b + c, &tValue)) {
            SkChopQuadAt(src, dst, tValue);
            flatten_double_quad_extrema(&dst[0].fX);
            return 1;
        }
        // if we get here, we need to force dst to be monotonic, even though
        // we couldn't compute a unit_divide value (probably underflow).
        b = SkScalarAbs(a - b) < SkScalarAbs(b - c) ? a : c;
    }
    dst[0].set(a, src[0].fY);
    dst[1].set(b, src[1].fY);
    dst[2].set(c, src[2].fY);
    return 0;
}

//  F(t)    = a (1 - t) ^ 2 + 2 b t (1 - t) + c t ^ 2
//  F'(t)   = 2 (b - a) + 2 (a - 2b + c) t
//  F''(t)  = 2 (a - 2b + c)
//
//  A = 2 (b - a)
//  B = 2 (a - 2b + c)
//
//  Maximum curvature for a quadratic means solving
//  Fx' Fx'' + Fy' Fy'' = 0
//
//  t = - (Ax Bx + Ay By) / (Bx ^ 2 + By ^ 2)
//
SkScalar SkFindQuadMaxCurvature(const SkPoint src[3]) {
    SkScalar    Ax = src[1].fX - src[0].fX;
    SkScalar    Ay = src[1].fY - src[0].fY;
    SkScalar    Bx = src[0].fX - src[1].fX - src[1].fX + src[2].fX;
    SkScalar    By = src[0].fY - src[1].fY - src[1].fY + src[2].fY;
    SkScalar    t = 0;  // 0 means don't chop

    (void)valid_unit_divide(-(Ax * Bx + Ay * By), Bx * Bx + By * By, &t);
    return t;
}

int SkChopQuadAtMaxCurvature(const SkPoint src[3], SkPoint dst[5]) {
    SkScalar t = SkFindQuadMaxCurvature(src);
    if (t == 0) {
        memcpy(dst, src, 3 * sizeof(SkPoint));
        return 1;
    } else {
        SkChopQuadAt(src, dst, t);
        return 2;
    }
}

void SkConvertQuadToCubic(const SkPoint src[3], SkPoint dst[4]) {
    Sk2s scale(SkDoubleToScalar(2.0 / 3.0));
    Sk2s s0 = from_point(src[0]);
    Sk2s s1 = from_point(src[1]);
    Sk2s s2 = from_point(src[2]);

    dst[0] = src[0];
    dst[1] = to_point(s0 + (s1 - s0) * scale);
    dst[2] = to_point(s2 + (s1 - s2) * scale);
    dst[3] = src[2];
}

//////////////////////////////////////////////////////////////////////////////
///// CUBICS // CUBICS // CUBICS // CUBICS // CUBICS // CUBICS // CUBICS /////
//////////////////////////////////////////////////////////////////////////////

static SkVector eval_cubic_derivative(const SkPoint src[4], SkScalar t) {
    SkQuadCoeff coeff;
    Sk2s P0 = from_point(src[0]);
    Sk2s P1 = from_point(src[1]);
    Sk2s P2 = from_point(src[2]);
    Sk2s P3 = from_point(src[3]);

    coeff.fA = P3 + Sk2s(3) * (P1 - P2) - P0;
    coeff.fB = times_2(P2 - times_2(P1) + P0);
    coeff.fC = P1 - P0;
    return to_vector(coeff.eval(t));
}

static SkVector eval_cubic_2ndDerivative(const SkPoint src[4], SkScalar t) {
    Sk2s P0 = from_point(src[0]);
    Sk2s P1 = from_point(src[1]);
    Sk2s P2 = from_point(src[2]);
    Sk2s P3 = from_point(src[3]);
    Sk2s A = P3 + Sk2s(3) * (P1 - P2) - P0;
    Sk2s B = P2 - times_2(P1) + P0;

    return to_vector(A * Sk2s(t) + B);
}

void SkEvalCubicAt(const SkPoint src[4], SkScalar t, SkPoint* loc,
                   SkVector* tangent, SkVector* curvature) {
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= SK_Scalar1);

    if (loc) {
        *loc = to_point(SkCubicCoeff(src).eval(t));
    }
    if (tangent) {
        // The derivative equation returns a zero tangent vector when t is 0 or 1, and the
        // adjacent control point is equal to the end point. In this case, use the
        // next control point or the end points to compute the tangent.
        if ((t == 0 && src[0] == src[1]) || (t == 1 && src[2] == src[3])) {
            if (t == 0) {
                *tangent = src[2] - src[0];
            } else {
                *tangent = src[3] - src[1];
            }
            if (!tangent->fX && !tangent->fY) {
                *tangent = src[3] - src[0];
            }
        } else {
            *tangent = eval_cubic_derivative(src, t);
        }
    }
    if (curvature) {
        *curvature = eval_cubic_2ndDerivative(src, t);
    }
}

/** Cubic'(t) = At^2 + Bt + C, where
    A = 3(-a + 3(b - c) + d)
    B = 6(a - 2b + c)
    C = 3(b - a)
    Solve for t, keeping only those that fit betwee 0 < t < 1
*/
int SkFindCubicExtrema(SkScalar a, SkScalar b, SkScalar c, SkScalar d,
                       SkScalar tValues[2]) {
    // we divide A,B,C by 3 to simplify
    SkScalar A = d - a + 3*(b - c);
    SkScalar B = 2*(a - b - b + c);
    SkScalar C = b - a;

    return SkFindUnitQuadRoots(A, B, C, tValues);
}

void SkChopCubicAt(const SkPoint src[4], SkPoint dst[7], SkScalar t) {
    SkASSERT(t > 0 && t < SK_Scalar1);

    Sk2s    p0 = from_point(src[0]);
    Sk2s    p1 = from_point(src[1]);
    Sk2s    p2 = from_point(src[2]);
    Sk2s    p3 = from_point(src[3]);
    Sk2s    tt(t);

    Sk2s    ab = interp(p0, p1, tt);
    Sk2s    bc = interp(p1, p2, tt);
    Sk2s    cd = interp(p2, p3, tt);
    Sk2s    abc = interp(ab, bc, tt);
    Sk2s    bcd = interp(bc, cd, tt);
    Sk2s    abcd = interp(abc, bcd, tt);

    dst[0] = src[0];
    dst[1] = to_point(ab);
    dst[2] = to_point(abc);
    dst[3] = to_point(abcd);
    dst[4] = to_point(bcd);
    dst[5] = to_point(cd);
    dst[6] = src[3];
}

/*  http://code.google.com/p/skia/issues/detail?id=32

    This test code would fail when we didn't check the return result of
    valid_unit_divide in SkChopCubicAt(... tValues[], int roots). The reason is
    that after the first chop, the parameters to valid_unit_divide are equal
    (thanks to finite float precision and rounding in the subtracts). Thus
    even though the 2nd tValue looks < 1.0, after we renormalize it, we end
    up with 1.0, hence the need to check and just return the last cubic as
    a degenerate clump of 4 points in the sampe place.

    static void test_cubic() {
        SkPoint src[4] = {
            { 556.25000, 523.03003 },
            { 556.23999, 522.96002 },
            { 556.21997, 522.89001 },
            { 556.21997, 522.82001 }
        };
        SkPoint dst[10];
        SkScalar tval[] = { 0.33333334f, 0.99999994f };
        SkChopCubicAt(src, dst, tval, 2);
    }
 */

void SkChopCubicAt(const SkPoint src[4], SkPoint dst[],
                   const SkScalar tValues[], int roots) {
#ifdef SK_DEBUG
    {
        for (int i = 0; i < roots - 1; i++)
        {
            SkASSERT(is_unit_interval(tValues[i]));
            SkASSERT(is_unit_interval(tValues[i+1]));
            SkASSERT(tValues[i] < tValues[i+1]);
        }
    }
#endif

    if (dst) {
        if (roots == 0) { // nothing to chop
            memcpy(dst, src, 4*sizeof(SkPoint));
        } else {
            SkScalar    t = tValues[0];
            SkPoint     tmp[4];

            for (int i = 0; i < roots; i++) {
                SkChopCubicAt(src, dst, t);
                if (i == roots - 1) {
                    break;
                }

                dst += 3;
                // have src point to the remaining cubic (after the chop)
                memcpy(tmp, dst, 4 * sizeof(SkPoint));
                src = tmp;

                // watch out in case the renormalized t isn't in range
                if (!valid_unit_divide(tValues[i+1] - tValues[i],
                                       SK_Scalar1 - tValues[i], &t)) {
                    // if we can't, just create a degenerate cubic
                    dst[4] = dst[5] = dst[6] = src[3];
                    break;
                }
            }
        }
    }
}

void SkChopCubicAtHalf(const SkPoint src[4], SkPoint dst[7]) {
    SkChopCubicAt(src, dst, 0.5f);
}

static void flatten_double_cubic_extrema(SkScalar coords[14]) {
    coords[4] = coords[8] = coords[6];
}

/** Given 4 points on a cubic bezier, chop it into 1, 2, 3 beziers such that
    the resulting beziers are monotonic in Y. This is called by the scan
    converter.  Depending on what is returned, dst[] is treated as follows:
    0   dst[0..3] is the original cubic
    1   dst[0..3] and dst[3..6] are the two new cubics
    2   dst[0..3], dst[3..6], dst[6..9] are the three new cubics
    If dst == null, it is ignored and only the count is returned.
*/
int SkChopCubicAtYExtrema(const SkPoint src[4], SkPoint dst[10]) {
    SkScalar    tValues[2];
    int         roots = SkFindCubicExtrema(src[0].fY, src[1].fY, src[2].fY,
                                           src[3].fY, tValues);

    SkChopCubicAt(src, dst, tValues, roots);
    if (dst && roots > 0) {
        // we do some cleanup to ensure our Y extrema are flat
        flatten_double_cubic_extrema(&dst[0].fY);
        if (roots == 2) {
            flatten_double_cubic_extrema(&dst[3].fY);
        }
    }
    return roots;
}

int SkChopCubicAtXExtrema(const SkPoint src[4], SkPoint dst[10]) {
    SkScalar    tValues[2];
    int         roots = SkFindCubicExtrema(src[0].fX, src[1].fX, src[2].fX,
                                           src[3].fX, tValues);

    SkChopCubicAt(src, dst, tValues, roots);
    if (dst && roots > 0) {
        // we do some cleanup to ensure our Y extrema are flat
        flatten_double_cubic_extrema(&dst[0].fX);
        if (roots == 2) {
            flatten_double_cubic_extrema(&dst[3].fX);
        }
    }
    return roots;
}

/** http://www.faculty.idc.ac.il/arik/quality/appendixA.html

    Inflection means that curvature is zero.
    Curvature is [F' x F''] / [F'^3]
    So we solve F'x X F''y - F'y X F''y == 0
    After some canceling of the cubic term, we get
    A = b - a
    B = c - 2b + a
    C = d - 3c + 3b - a
    (BxCy - ByCx)t^2 + (AxCy - AyCx)t + AxBy - AyBx == 0
*/
int SkFindCubicInflections(const SkPoint src[4], SkScalar tValues[]) {
    SkScalar    Ax = src[1].fX - src[0].fX;
    SkScalar    Ay = src[1].fY - src[0].fY;
    SkScalar    Bx = src[2].fX - 2 * src[1].fX + src[0].fX;
    SkScalar    By = src[2].fY - 2 * src[1].fY + src[0].fY;
    SkScalar    Cx = src[3].fX + 3 * (src[1].fX - src[2].fX) - src[0].fX;
    SkScalar    Cy = src[3].fY + 3 * (src[1].fY - src[2].fY) - src[0].fY;

    return SkFindUnitQuadRoots(Bx*Cy - By*Cx,
                               Ax*Cy - Ay*Cx,
                               Ax*By - Ay*Bx,
                               tValues);
}

int SkChopCubicAtInflections(const SkPoint src[], SkPoint dst[10]) {
    SkScalar    tValues[2];
    int         count = SkFindCubicInflections(src, tValues);

    if (dst) {
        if (count == 0) {
            memcpy(dst, src, 4 * sizeof(SkPoint));
        } else {
            SkChopCubicAt(src, dst, tValues, count);
        }
    }
    return count + 1;
}

// See "Resolution Independent Curve Rendering using Programmable Graphics Hardware"
// https://www.microsoft.com/en-us/research/wp-content/uploads/2005/01/p1000-loop.pdf
// discr(I) = 3*d2^2 - 4*d1*d3
// Classification:
// d1 != 0, discr(I) > 0        Serpentine
// d1 != 0, discr(I) < 0        Loop
// d1 != 0, discr(I) = 0        Cusp (with inflection at infinity)
// d1 = 0, d2 != 0              Cusp (with cusp at infinity)
// d1 = d2 = 0, d3 != 0         Quadratic
// d1 = d2 = d3 = 0             Line or Point
static SkCubicType classify_cubic(SkScalar d[4]) {
    if (!SkScalarNearlyZero(d[1])) {
        d[0] = 3 * d[2] * d[2] - 4 * d[1] * d[3];
        if (d[0] > 0) {
            return SkCubicType::kSerpentine;
        } else if (d[0] < 0) {
            return SkCubicType::kLoop;
        } else {
            SkASSERT(0 == d[0]); // Detect NaN.
            return SkCubicType::kLocalCusp;
        }
    } else {
        if (!SkScalarNearlyZero(d[2])) {
            return SkCubicType::kInfiniteCusp;
        } else if (!SkScalarNearlyZero(d[3])) {
            return SkCubicType::kQuadratic;
        } else {
            return SkCubicType::kLineOrPoint;
        }
    }
}

// Assumes the third component of points is 1.
// Calcs p0 . (p1 x p2)
static SkScalar calc_dot_cross_cubic(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2) {
    const SkScalar xComp = p0.fX * (p1.fY - p2.fY);
    const SkScalar yComp = p0.fY * (p2.fX - p1.fX);
    const SkScalar wComp = p1.fX * p2.fY - p1.fY * p2.fX;
    return (xComp + yComp + wComp);
}

// Calc coefficients of I(s,t) where roots of I are inflection points of curve
// I(s,t) = t*(3*d0*s^2 - 3*d1*s*t + d2*t^2)
// d0 = a1 - 2*a2+3*a3
// d1 = -a2 + 3*a3
// d2 = 3*a3
// a1 = p0 . (p3 x p2)
// a2 = p1 . (p0 x p3)
// a3 = p2 . (p1 x p0)
// Places the values of d1, d2, d3 in array d passed in
static void calc_cubic_inflection_func(const SkPoint p[4], SkScalar d[4]) {
    SkScalar a1 = calc_dot_cross_cubic(p[0], p[3], p[2]);
    SkScalar a2 = calc_dot_cross_cubic(p[1], p[0], p[3]);
    SkScalar a3 = calc_dot_cross_cubic(p[2], p[1], p[0]);

    // need to scale a's or values in later calculations will grow to high
    SkScalar max = SkScalarAbs(a1);
    max = SkMaxScalar(max, SkScalarAbs(a2));
    max = SkMaxScalar(max, SkScalarAbs(a3));
    if (0 != max) {
        max = 1.f/max;
        a1 = a1 * max;
        a2 = a2 * max;
        a3 = a3 * max;
    }

    d[3] = 3.f * a3;
    d[2] = d[3] - a2;
    d[1] = d[2] - a2 + a1;
    d[0] = 0;
}

SkCubicType SkClassifyCubic(const SkPoint src[4], SkScalar d[4]) {
    calc_cubic_inflection_func(src, d);
    return classify_cubic(d);
}

template <typename T> void bubble_sort(T array[], int count) {
    for (int i = count - 1; i > 0; --i)
        for (int j = i; j > 0; --j)
            if (array[j] < array[j-1])
            {
                T   tmp(array[j]);
                array[j] = array[j-1];
                array[j-1] = tmp;
            }
}

/**
 *  Given an array and count, remove all pair-wise duplicates from the array,
 *  keeping the existing sorting, and return the new count
 */
static int collaps_duplicates(SkScalar array[], int count) {
    for (int n = count; n > 1; --n) {
        if (array[0] == array[1]) {
            for (int i = 1; i < n; ++i) {
                array[i - 1] = array[i];
            }
            count -= 1;
        } else {
            array += 1;
        }
    }
    return count;
}

#ifdef SK_DEBUG

#define TEST_COLLAPS_ENTRY(array)   array, SK_ARRAY_COUNT(array)

static void test_collaps_duplicates() {
    static bool gOnce;
    if (gOnce) { return; }
    gOnce = true;
    const SkScalar src0[] = { 0 };
    const SkScalar src1[] = { 0, 0 };
    const SkScalar src2[] = { 0, 1 };
    const SkScalar src3[] = { 0, 0, 0 };
    const SkScalar src4[] = { 0, 0, 1 };
    const SkScalar src5[] = { 0, 1, 1 };
    const SkScalar src6[] = { 0, 1, 2 };
    const struct {
        const SkScalar* fData;
        int fCount;
        int fCollapsedCount;
    } data[] = {
        { TEST_COLLAPS_ENTRY(src0), 1 },
        { TEST_COLLAPS_ENTRY(src1), 1 },
        { TEST_COLLAPS_ENTRY(src2), 2 },
        { TEST_COLLAPS_ENTRY(src3), 1 },
        { TEST_COLLAPS_ENTRY(src4), 2 },
        { TEST_COLLAPS_ENTRY(src5), 2 },
        { TEST_COLLAPS_ENTRY(src6), 3 },
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(data); ++i) {
        SkScalar dst[3];
        memcpy(dst, data[i].fData, data[i].fCount * sizeof(dst[0]));
        int count = collaps_duplicates(dst, data[i].fCount);
        SkASSERT(data[i].fCollapsedCount == count);
        for (int j = 1; j < count; ++j) {
            SkASSERT(dst[j-1] < dst[j]);
        }
    }
}
#endif

static SkScalar SkScalarCubeRoot(SkScalar x) {
    return SkScalarPow(x, 0.3333333f);
}

/*  Solve coeff(t) == 0, returning the number of roots that
    lie withing 0 < t < 1.
    coeff[0]t^3 + coeff[1]t^2 + coeff[2]t + coeff[3]

    Eliminates repeated roots (so that all tValues are distinct, and are always
    in increasing order.
*/
static int solve_cubic_poly(const SkScalar coeff[4], SkScalar tValues[3]) {
    if (SkScalarNearlyZero(coeff[0])) {  // we're just a quadratic
        return SkFindUnitQuadRoots(coeff[1], coeff[2], coeff[3], tValues);
    }

    SkScalar a, b, c, Q, R;

    {
        SkASSERT(coeff[0] != 0);

        SkScalar inva = SkScalarInvert(coeff[0]);
        a = coeff[1] * inva;
        b = coeff[2] * inva;
        c = coeff[3] * inva;
    }
    Q = (a*a - b*3) / 9;
    R = (2*a*a*a - 9*a*b + 27*c) / 54;

    SkScalar Q3 = Q * Q * Q;
    SkScalar R2MinusQ3 = R * R - Q3;
    SkScalar adiv3 = a / 3;

    SkScalar*   roots = tValues;
    SkScalar    r;

    if (R2MinusQ3 < 0) { // we have 3 real roots
        // the divide/root can, due to finite precisions, be slightly outside of -1...1
        SkScalar theta = SkScalarACos(SkScalarPin(R / SkScalarSqrt(Q3), -1, 1));
        SkScalar neg2RootQ = -2 * SkScalarSqrt(Q);

        r = neg2RootQ * SkScalarCos(theta/3) - adiv3;
        if (is_unit_interval(r)) {
            *roots++ = r;
        }
        r = neg2RootQ * SkScalarCos((theta + 2*SK_ScalarPI)/3) - adiv3;
        if (is_unit_interval(r)) {
            *roots++ = r;
        }
        r = neg2RootQ * SkScalarCos((theta - 2*SK_ScalarPI)/3) - adiv3;
        if (is_unit_interval(r)) {
            *roots++ = r;
        }
        SkDEBUGCODE(test_collaps_duplicates();)

        // now sort the roots
        int count = (int)(roots - tValues);
        SkASSERT((unsigned)count <= 3);
        bubble_sort(tValues, count);
        count = collaps_duplicates(tValues, count);
        roots = tValues + count;    // so we compute the proper count below
    } else {              // we have 1 real root
        SkScalar A = SkScalarAbs(R) + SkScalarSqrt(R2MinusQ3);
        A = SkScalarCubeRoot(A);
        if (R > 0) {
            A = -A;
        }
        if (A != 0) {
            A += Q / A;
        }
        r = A - adiv3;
        if (is_unit_interval(r)) {
            *roots++ = r;
        }
    }

    return (int)(roots - tValues);
}

/*  Looking for F' dot F'' == 0

    A = b - a
    B = c - 2b + a
    C = d - 3c + 3b - a

    F' = 3Ct^2 + 6Bt + 3A
    F'' = 6Ct + 6B

    F' dot F'' -> CCt^3 + 3BCt^2 + (2BB + CA)t + AB
*/
static void formulate_F1DotF2(const SkScalar src[], SkScalar coeff[4]) {
    SkScalar    a = src[2] - src[0];
    SkScalar    b = src[4] - 2 * src[2] + src[0];
    SkScalar    c = src[6] + 3 * (src[2] - src[4]) - src[0];

    coeff[0] = c * c;
    coeff[1] = 3 * b * c;
    coeff[2] = 2 * b * b + c * a;
    coeff[3] = a * b;
}

/*  Looking for F' dot F'' == 0

    A = b - a
    B = c - 2b + a
    C = d - 3c + 3b - a

    F' = 3Ct^2 + 6Bt + 3A
    F'' = 6Ct + 6B

    F' dot F'' -> CCt^3 + 3BCt^2 + (2BB + CA)t + AB
*/
int SkFindCubicMaxCurvature(const SkPoint src[4], SkScalar tValues[3]) {
    SkScalar coeffX[4], coeffY[4];
    int      i;

    formulate_F1DotF2(&src[0].fX, coeffX);
    formulate_F1DotF2(&src[0].fY, coeffY);

    for (i = 0; i < 4; i++) {
        coeffX[i] += coeffY[i];
    }

    SkScalar    t[3];
    int         count = solve_cubic_poly(coeffX, t);
    int         maxCount = 0;

    // now remove extrema where the curvature is zero (mins)
    // !!!! need a test for this !!!!
    for (i = 0; i < count; i++) {
        // if (not_min_curvature())
        if (t[i] > 0 && t[i] < SK_Scalar1) {
            tValues[maxCount++] = t[i];
        }
    }
    return maxCount;
}

int SkChopCubicAtMaxCurvature(const SkPoint src[4], SkPoint dst[13],
                              SkScalar tValues[3]) {
    SkScalar    t_storage[3];

    if (tValues == nullptr) {
        tValues = t_storage;
    }

    int count = SkFindCubicMaxCurvature(src, tValues);

    if (dst) {
        if (count == 0) {
            memcpy(dst, src, 4 * sizeof(SkPoint));
        } else {
            SkChopCubicAt(src, dst, tValues, count);
        }
    }
    return count + 1;
}

#include "../pathops/SkPathOpsCubic.h"

typedef int (SkDCubic::*InterceptProc)(double intercept, double roots[3]) const;

static bool cubic_dchop_at_intercept(const SkPoint src[4], SkScalar intercept, SkPoint dst[7],
                                     InterceptProc method) {
    SkDCubic cubic;
    double roots[3];
    int count = (cubic.set(src).*method)(intercept, roots);
    if (count > 0) {
        SkDCubicPair pair = cubic.chopAt(roots[0]);
        for (int i = 0; i < 7; ++i) {
            dst[i] = pair.pts[i].asSkPoint();
        }
        return true;
    }
    return false;
}

bool SkChopMonoCubicAtY(SkPoint src[4], SkScalar y, SkPoint dst[7]) {
    return cubic_dchop_at_intercept(src, y, dst, &SkDCubic::horizontalIntersect);
}

bool SkChopMonoCubicAtX(SkPoint src[4], SkScalar x, SkPoint dst[7]) {
    return cubic_dchop_at_intercept(src, x, dst, &SkDCubic::verticalIntersect);
}

///////////////////////////////////////////////////////////////////////////////
//
// NURB representation for conics.  Helpful explanations at:
//
// http://citeseerx.ist.psu.edu/viewdoc/
//   download?doi=10.1.1.44.5740&rep=rep1&type=ps
// and
// http://www.cs.mtu.edu/~shene/COURSES/cs3621/NOTES/spline/NURBS/RB-conics.html
//
// F = (A (1 - t)^2 + C t^2 + 2 B (1 - t) t w)
//     ------------------------------------------
//         ((1 - t)^2 + t^2 + 2 (1 - t) t w)
//
//   = {t^2 (P0 + P2 - 2 P1 w), t (-2 P0 + 2 P1 w), P0}
//     ------------------------------------------------
//             {t^2 (2 - 2 w), t (-2 + 2 w), 1}
//

// F' = 2 (C t (1 + t (-1 + w)) - A (-1 + t) (t (-1 + w) - w) + B (1 - 2 t) w)
//
//  t^2 : (2 P0 - 2 P2 - 2 P0 w + 2 P2 w)
//  t^1 : (-2 P0 + 2 P2 + 4 P0 w - 4 P1 w)
//  t^0 : -2 P0 w + 2 P1 w
//
//  We disregard magnitude, so we can freely ignore the denominator of F', and
//  divide the numerator by 2
//
//    coeff[0] for t^2
//    coeff[1] for t^1
//    coeff[2] for t^0
//
static void conic_deriv_coeff(const SkScalar src[],
                              SkScalar w,
                              SkScalar coeff[3]) {
    const SkScalar P20 = src[4] - src[0];
    const SkScalar P10 = src[2] - src[0];
    const SkScalar wP10 = w * P10;
    coeff[0] = w * P20 - P20;
    coeff[1] = P20 - 2 * wP10;
    coeff[2] = wP10;
}

static bool conic_find_extrema(const SkScalar src[], SkScalar w, SkScalar* t) {
    SkScalar coeff[3];
    conic_deriv_coeff(src, w, coeff);

    SkScalar tValues[2];
    int roots = SkFindUnitQuadRoots(coeff[0], coeff[1], coeff[2], tValues);
    SkASSERT(0 == roots || 1 == roots);

    if (1 == roots) {
        *t = tValues[0];
        return true;
    }
    return false;
}

struct SkP3D {
    SkScalar fX, fY, fZ;

    void set(SkScalar x, SkScalar y, SkScalar z) {
        fX = x; fY = y; fZ = z;
    }

    void projectDown(SkPoint* dst) const {
        dst->set(fX / fZ, fY / fZ);
    }
};

// We only interpolate one dimension at a time (the first, at +0, +3, +6).
static void p3d_interp(const SkScalar src[7], SkScalar dst[7], SkScalar t) {
    SkScalar ab = SkScalarInterp(src[0], src[3], t);
    SkScalar bc = SkScalarInterp(src[3], src[6], t);
    dst[0] = ab;
    dst[3] = SkScalarInterp(ab, bc, t);
    dst[6] = bc;
}

static void ratquad_mapTo3D(const SkPoint src[3], SkScalar w, SkP3D dst[]) {
    dst[0].set(src[0].fX * 1, src[0].fY * 1, 1);
    dst[1].set(src[1].fX * w, src[1].fY * w, w);
    dst[2].set(src[2].fX * 1, src[2].fY * 1, 1);
}

// return false if infinity or NaN is generated; caller must check
bool SkConic::chopAt(SkScalar t, SkConic dst[2]) const {
    SkP3D tmp[3], tmp2[3];

    ratquad_mapTo3D(fPts, fW, tmp);

    p3d_interp(&tmp[0].fX, &tmp2[0].fX, t);
    p3d_interp(&tmp[0].fY, &tmp2[0].fY, t);
    p3d_interp(&tmp[0].fZ, &tmp2[0].fZ, t);

    dst[0].fPts[0] = fPts[0];
    tmp2[0].projectDown(&dst[0].fPts[1]);
    tmp2[1].projectDown(&dst[0].fPts[2]); dst[1].fPts[0] = dst[0].fPts[2];
    tmp2[2].projectDown(&dst[1].fPts[1]);
    dst[1].fPts[2] = fPts[2];

    // to put in "standard form", where w0 and w2 are both 1, we compute the
    // new w1 as sqrt(w1*w1/w0*w2)
    // or
    // w1 /= sqrt(w0*w2)
    //
    // However, in our case, we know that for dst[0]:
    //     w0 == 1, and for dst[1], w2 == 1
    //
    SkScalar root = SkScalarSqrt(tmp2[1].fZ);
    dst[0].fW = tmp2[0].fZ / root;
    dst[1].fW = tmp2[2].fZ / root;
    SkASSERT(sizeof(dst[0]) == sizeof(SkScalar) * 7);
    SkASSERT(0 == offsetof(SkConic, fPts[0].fX));
    return SkScalarsAreFinite(&dst[0].fPts[0].fX, 7 * 2);
}

void SkConic::chopAt(SkScalar t1, SkScalar t2, SkConic* dst) const {
    if (0 == t1 || 1 == t2) {
        if (0 == t1 && 1 == t2) {
            *dst = *this;
            return;
        } else {
            SkConic pair[2];
            if (this->chopAt(t1 ? t1 : t2, pair)) {
                *dst = pair[SkToBool(t1)];
                return;
            }
        }
    }
    SkConicCoeff coeff(*this);
    Sk2s tt1(t1);
    Sk2s aXY = coeff.fNumer.eval(tt1);
    Sk2s aZZ = coeff.fDenom.eval(tt1);
    Sk2s midTT((t1 + t2) / 2);
    Sk2s dXY = coeff.fNumer.eval(midTT);
    Sk2s dZZ = coeff.fDenom.eval(midTT);
    Sk2s tt2(t2);
    Sk2s cXY = coeff.fNumer.eval(tt2);
    Sk2s cZZ = coeff.fDenom.eval(tt2);
    Sk2s bXY = times_2(dXY) - (aXY + cXY) * Sk2s(0.5f);
    Sk2s bZZ = times_2(dZZ) - (aZZ + cZZ) * Sk2s(0.5f);
    dst->fPts[0] = to_point(aXY / aZZ);
    dst->fPts[1] = to_point(bXY / bZZ);
    dst->fPts[2] = to_point(cXY / cZZ);
    Sk2s ww = bZZ / (aZZ * cZZ).sqrt();
    dst->fW = ww[0];
}

SkPoint SkConic::evalAt(SkScalar t) const {
    return to_point(SkConicCoeff(*this).eval(t));
}

SkVector SkConic::evalTangentAt(SkScalar t) const {
    // The derivative equation returns a zero tangent vector when t is 0 or 1,
    // and the control point is equal to the end point.
    // In this case, use the conic endpoints to compute the tangent.
    if ((t == 0 && fPts[0] == fPts[1]) || (t == 1 && fPts[1] == fPts[2])) {
        return fPts[2] - fPts[0];
    }
    Sk2s p0 = from_point(fPts[0]);
    Sk2s p1 = from_point(fPts[1]);
    Sk2s p2 = from_point(fPts[2]);
    Sk2s ww(fW);

    Sk2s p20 = p2 - p0;
    Sk2s p10 = p1 - p0;

    Sk2s C = ww * p10;
    Sk2s A = ww * p20 - p20;
    Sk2s B = p20 - C - C;

    return to_vector(SkQuadCoeff(A, B, C).eval(t));
}

void SkConic::evalAt(SkScalar t, SkPoint* pt, SkVector* tangent) const {
    SkASSERT(t >= 0 && t <= SK_Scalar1);

    if (pt) {
        *pt = this->evalAt(t);
    }
    if (tangent) {
        *tangent = this->evalTangentAt(t);
    }
}

static SkScalar subdivide_w_value(SkScalar w) {
    return SkScalarSqrt(SK_ScalarHalf + w * SK_ScalarHalf);
}

void SkConic::chop(SkConic * SK_RESTRICT dst) const {
    Sk2s scale = Sk2s(SkScalarInvert(SK_Scalar1 + fW));
    SkScalar newW = subdivide_w_value(fW);

    Sk2s p0 = from_point(fPts[0]);
    Sk2s p1 = from_point(fPts[1]);
    Sk2s p2 = from_point(fPts[2]);
    Sk2s ww(fW);

    Sk2s wp1 = ww * p1;
    Sk2s m = (p0 + times_2(wp1) + p2) * scale * Sk2s(0.5f);

    dst[0].fPts[0] = fPts[0];
    dst[0].fPts[1] = to_point((p0 + wp1) * scale);
    dst[0].fPts[2] = dst[1].fPts[0] = to_point(m);
    dst[1].fPts[1] = to_point((wp1 + p2) * scale);
    dst[1].fPts[2] = fPts[2];

    dst[0].fW = dst[1].fW = newW;
}

/*
 *  "High order approximation of conic sections by quadratic splines"
 *      by Michael Floater, 1993
 */
#define AS_QUAD_ERROR_SETUP                                         \
    SkScalar a = fW - 1;                                            \
    SkScalar k = a / (4 * (2 + a));                                 \
    SkScalar x = k * (fPts[0].fX - 2 * fPts[1].fX + fPts[2].fX);    \
    SkScalar y = k * (fPts[0].fY - 2 * fPts[1].fY + fPts[2].fY);

void SkConic::computeAsQuadError(SkVector* err) const {
    AS_QUAD_ERROR_SETUP
    err->set(x, y);
}

bool SkConic::asQuadTol(SkScalar tol) const {
    AS_QUAD_ERROR_SETUP
    return (x * x + y * y) <= tol * tol;
}

// Limit the number of suggested quads to approximate a conic
#define kMaxConicToQuadPOW2     5

int SkConic::computeQuadPOW2(SkScalar tol) const {
    if (tol < 0 || !SkScalarIsFinite(tol)) {
        return 0;
    }

    AS_QUAD_ERROR_SETUP

    SkScalar error = SkScalarSqrt(x * x + y * y);
    int pow2;
    for (pow2 = 0; pow2 < kMaxConicToQuadPOW2; ++pow2) {
        if (error <= tol) {
            break;
        }
        error *= 0.25f;
    }
    // float version -- using ceil gives the same results as the above.
    if (false) {
        SkScalar err = SkScalarSqrt(x * x + y * y);
        if (err <= tol) {
            return 0;
        }
        SkScalar tol2 = tol * tol;
        if (tol2 == 0) {
            return kMaxConicToQuadPOW2;
        }
        SkScalar fpow2 = SkScalarLog2((x * x + y * y) / tol2) * 0.25f;
        int altPow2 = SkScalarCeilToInt(fpow2);
        if (altPow2 != pow2) {
            SkDebugf("pow2 %d altPow2 %d fbits %g err %g tol %g\n", pow2, altPow2, fpow2, err, tol);
        }
        pow2 = altPow2;
    }
    return pow2;
}

// This was originally developed and tested for pathops: see SkOpTypes.h 
// returns true if (a <= b <= c) || (a >= b >= c)
static bool between(SkScalar a, SkScalar b, SkScalar c) {
    return (a - b) * (c - b) <= 0;
}

static SkPoint* subdivide(const SkConic& src, SkPoint pts[], int level) {
    SkASSERT(level >= 0);

    if (0 == level) {
        memcpy(pts, &src.fPts[1], 2 * sizeof(SkPoint));
        return pts + 2;
    } else {
        SkConic dst[2];
        src.chop(dst);
        const SkScalar startY = src.fPts[0].fY;
        const SkScalar endY = src.fPts[2].fY;
        if (between(startY, src.fPts[1].fY, endY)) {
            // If the input is monotonic and the output is not, the scan converter hangs.
            // Ensure that the chopped conics maintain their y-order.
            SkScalar midY = dst[0].fPts[2].fY;
            if (!between(startY, midY, endY)) {
                // If the computed midpoint is outside the ends, move it to the closer one.
                SkScalar closerY = SkTAbs(midY - startY) < SkTAbs(midY - endY) ? startY : endY;
                dst[0].fPts[2].fY = dst[1].fPts[0].fY = closerY;
            }
            if (!between(startY, dst[0].fPts[1].fY, dst[0].fPts[2].fY)) {
                // If the 1st control is not between the start and end, put it at the start.
                // This also reduces the quad to a line.
                dst[0].fPts[1].fY = startY;
            }
            if (!between(dst[1].fPts[0].fY, dst[1].fPts[1].fY, endY)) {
                // If the 2nd control is not between the start and end, put it at the end.
                // This also reduces the quad to a line.
                dst[1].fPts[1].fY = endY;
            }
            // Verify that all five points are in order.
            SkASSERT(between(startY, dst[0].fPts[1].fY, dst[0].fPts[2].fY));
            SkASSERT(between(dst[0].fPts[1].fY, dst[0].fPts[2].fY, dst[1].fPts[1].fY));
            SkASSERT(between(dst[0].fPts[2].fY, dst[1].fPts[1].fY, endY));
        }
        --level;
        pts = subdivide(dst[0], pts, level);
        return subdivide(dst[1], pts, level);
    }
}

int SkConic::chopIntoQuadsPOW2(SkPoint pts[], int pow2) const {
    SkASSERT(pow2 >= 0);
    *pts = fPts[0];
    SkDEBUGCODE(SkPoint* endPts);
    if (pow2 == kMaxConicToQuadPOW2) {  // If an extreme weight generates many quads ...
        SkConic dst[2];
        this->chop(dst);
        // check to see if the first chop generates a pair of lines
        if (dst[0].fPts[1].equalsWithinTolerance(dst[0].fPts[2])
                && dst[1].fPts[0].equalsWithinTolerance(dst[1].fPts[1])) {
            pts[1] = pts[2] = pts[3] = dst[0].fPts[1];  // set ctrl == end to make lines
            pts[4] = dst[1].fPts[2];
            pow2 = 1;
            SkDEBUGCODE(endPts = &pts[5]);
            goto commonFinitePtCheck;
        }
    }
    SkDEBUGCODE(endPts = ) subdivide(*this, pts + 1, pow2);
commonFinitePtCheck:
    const int quadCount = 1 << pow2;
    const int ptCount = 2 * quadCount + 1;
    SkASSERT(endPts - pts == ptCount);
    if (!SkPointsAreFinite(pts, ptCount)) {
        // if we generated a non-finite, pin ourselves to the middle of the hull,
        // as our first and last are already on the first/last pts of the hull.
        for (int i = 1; i < ptCount - 1; ++i) {
            pts[i] = fPts[1];
        }
    }
    return 1 << pow2;
}

bool SkConic::findXExtrema(SkScalar* t) const {
    return conic_find_extrema(&fPts[0].fX, fW, t);
}

bool SkConic::findYExtrema(SkScalar* t) const {
    return conic_find_extrema(&fPts[0].fY, fW, t);
}

bool SkConic::chopAtXExtrema(SkConic dst[2]) const {
    SkScalar t;
    if (this->findXExtrema(&t)) {
        if (!this->chopAt(t, dst)) {
            // if chop can't return finite values, don't chop
            return false;
        }
        // now clean-up the middle, since we know t was meant to be at
        // an X-extrema
        SkScalar value = dst[0].fPts[2].fX;
        dst[0].fPts[1].fX = value;
        dst[1].fPts[0].fX = value;
        dst[1].fPts[1].fX = value;
        return true;
    }
    return false;
}

bool SkConic::chopAtYExtrema(SkConic dst[2]) const {
    SkScalar t;
    if (this->findYExtrema(&t)) {
        if (!this->chopAt(t, dst)) {
            // if chop can't return finite values, don't chop
            return false;
        }
        // now clean-up the middle, since we know t was meant to be at
        // an Y-extrema
        SkScalar value = dst[0].fPts[2].fY;
        dst[0].fPts[1].fY = value;
        dst[1].fPts[0].fY = value;
        dst[1].fPts[1].fY = value;
        return true;
    }
    return false;
}

void SkConic::computeTightBounds(SkRect* bounds) const {
    SkPoint pts[4];
    pts[0] = fPts[0];
    pts[1] = fPts[2];
    int count = 2;

    SkScalar t;
    if (this->findXExtrema(&t)) {
        this->evalAt(t, &pts[count++]);
    }
    if (this->findYExtrema(&t)) {
        this->evalAt(t, &pts[count++]);
    }
    bounds->set(pts, count);
}

void SkConic::computeFastBounds(SkRect* bounds) const {
    bounds->set(fPts, 3);
}

#if 0  // unimplemented
bool SkConic::findMaxCurvature(SkScalar* t) const {
    // TODO: Implement me
    return false;
}
#endif

SkScalar SkConic::TransformW(const SkPoint pts[], SkScalar w,
                             const SkMatrix& matrix) {
    if (!matrix.hasPerspective()) {
        return w;
    }

    SkP3D src[3], dst[3];

    ratquad_mapTo3D(pts, w, src);

    matrix.mapHomogeneousPoints(&dst[0].fX, &src[0].fX, 3);

    // w' = sqrt(w1*w1/w0*w2)
    SkScalar w0 = dst[0].fZ;
    SkScalar w1 = dst[1].fZ;
    SkScalar w2 = dst[2].fZ;
    w = SkScalarSqrt((w1 * w1) / (w0 * w2));
    return w;
}

int SkConic::BuildUnitArc(const SkVector& uStart, const SkVector& uStop, SkRotationDirection dir,
                          const SkMatrix* userMatrix, SkConic dst[kMaxConicsForArc]) {
    // rotate by x,y so that uStart is (1.0)
    SkScalar x = SkPoint::DotProduct(uStart, uStop);
    SkScalar y = SkPoint::CrossProduct(uStart, uStop);

    SkScalar absY = SkScalarAbs(y);

    // check for (effectively) coincident vectors
    // this can happen if our angle is nearly 0 or nearly 180 (y == 0)
    // ... we use the dot-prod to distinguish between 0 and 180 (x > 0)
    if (absY <= SK_ScalarNearlyZero && x > 0 && ((y >= 0 && kCW_SkRotationDirection == dir) ||
                                                 (y <= 0 && kCCW_SkRotationDirection == dir))) {
        return 0;
    }

    if (dir == kCCW_SkRotationDirection) {
        y = -y;
    }

    // We decide to use 1-conic per quadrant of a circle. What quadrant does [xy] lie in?
    //      0 == [0  .. 90)
    //      1 == [90 ..180)
    //      2 == [180..270)
    //      3 == [270..360)
    //
    int quadrant = 0;
    if (0 == y) {
        quadrant = 2;        // 180
        SkASSERT(SkScalarAbs(x + SK_Scalar1) <= SK_ScalarNearlyZero);
    } else if (0 == x) {
        SkASSERT(absY - SK_Scalar1 <= SK_ScalarNearlyZero);
        quadrant = y > 0 ? 1 : 3; // 90 : 270
    } else {
        if (y < 0) {
            quadrant += 2;
        }
        if ((x < 0) != (y < 0)) {
            quadrant += 1;
        }
    }

    const SkPoint quadrantPts[] = {
        { 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 }
    };
    const SkScalar quadrantWeight = SK_ScalarRoot2Over2;

    int conicCount = quadrant;
    for (int i = 0; i < conicCount; ++i) {
        dst[i].set(&quadrantPts[i * 2], quadrantWeight);
    }

    // Now compute any remaing (sub-90-degree) arc for the last conic
    const SkPoint finalP = { x, y };
    const SkPoint& lastQ = quadrantPts[quadrant * 2];  // will already be a unit-vector
    const SkScalar dot = SkVector::DotProduct(lastQ, finalP);
    SkASSERT(0 <= dot && dot <= SK_Scalar1 + SK_ScalarNearlyZero);

    if (dot < 1) {
        SkVector offCurve = { lastQ.x() + x, lastQ.y() + y };
        // compute the bisector vector, and then rescale to be the off-curve point.
        // we compute its length from cos(theta/2) = length / 1, using half-angle identity we get
        // length = sqrt(2 / (1 + cos(theta)). We already have cos() when to computed the dot.
        // This is nice, since our computed weight is cos(theta/2) as well!
        //
        const SkScalar cosThetaOver2 = SkScalarSqrt((1 + dot) / 2);
        offCurve.setLength(SkScalarInvert(cosThetaOver2));
        if (!lastQ.equalsWithinTolerance(offCurve)) {
            dst[conicCount].set(lastQ, offCurve, finalP, cosThetaOver2);
            conicCount += 1;
        } 
    }

    // now handle counter-clockwise and the initial unitStart rotation
    SkMatrix    matrix;
    matrix.setSinCos(uStart.fY, uStart.fX);
    if (dir == kCCW_SkRotationDirection) {
        matrix.preScale(SK_Scalar1, -SK_Scalar1);
    }
    if (userMatrix) {
        matrix.postConcat(*userMatrix);
    }
    for (int i = 0; i < conicCount; ++i) {
        matrix.mapPoints(dst[i].fPts, 3);
    }
    return conicCount;
}
