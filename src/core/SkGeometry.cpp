
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkGeometry.h"
#include "Sk64.h"
#include "SkMatrix.h"

bool SkXRayCrossesLine(const SkXRay& pt, const SkPoint pts[2], bool* ambiguous) {
    if (ambiguous) {
        *ambiguous = false;
    }
    // Determine quick discards.
    // Consider query line going exactly through point 0 to not
    // intersect, for symmetry with SkXRayCrossesMonotonicCubic.
    if (pt.fY == pts[0].fY) {
        if (ambiguous) {
            *ambiguous = true;
        }
        return false;
    }
    if (pt.fY < pts[0].fY && pt.fY < pts[1].fY)
        return false;
    if (pt.fY > pts[0].fY && pt.fY > pts[1].fY)
        return false;
    if (pt.fX > pts[0].fX && pt.fX > pts[1].fX)
        return false;
    // Determine degenerate cases
    if (SkScalarNearlyZero(pts[0].fY - pts[1].fY))
        return false;
    if (SkScalarNearlyZero(pts[0].fX - pts[1].fX)) {
        // We've already determined the query point lies within the
        // vertical range of the line segment.
        if (pt.fX <= pts[0].fX) {
            if (ambiguous) {
                *ambiguous = (pt.fY == pts[1].fY);
            }
            return true;
        }
        return false;
    }
    // Ambiguity check
    if (pt.fY == pts[1].fY) {
        if (pt.fX <= pts[1].fX) {
            if (ambiguous) {
                *ambiguous = true;
            }
            return true;
        }
        return false;
    }
    // Full line segment evaluation
    SkScalar delta_y = pts[1].fY - pts[0].fY;
    SkScalar delta_x = pts[1].fX - pts[0].fX;
    SkScalar slope = SkScalarDiv(delta_y, delta_x);
    SkScalar b = pts[0].fY - SkScalarMul(slope, pts[0].fX);
    // Solve for x coordinate at y = pt.fY
    SkScalar x = SkScalarDiv(pt.fY - b, slope);
    return pt.fX <= x;
}

/** If defined, this makes eval_quad and eval_cubic do more setup (sometimes
    involving integer multiplies by 2 or 3, but fewer calls to SkScalarMul.
    May also introduce overflow of fixed when we compute our setup.
*/
#ifdef SK_SCALAR_IS_FIXED
    #define DIRECT_EVAL_OF_POLYNOMIALS
#endif

////////////////////////////////////////////////////////////////////////

#ifdef SK_SCALAR_IS_FIXED
    static int is_not_monotonic(int a, int b, int c, int d)
    {
        return (((a - b) | (b - c) | (c - d)) & ((b - a) | (c - b) | (d - c))) >> 31;
    }

    static int is_not_monotonic(int a, int b, int c)
    {
        return (((a - b) | (b - c)) & ((b - a) | (c - b))) >> 31;
    }
#else
    static int is_not_monotonic(float a, float b, float c)
    {
        float ab = a - b;
        float bc = b - c;
        if (ab < 0)
            bc = -bc;
        return ab == 0 || bc < 0;
    }
#endif

////////////////////////////////////////////////////////////////////////

static bool is_unit_interval(SkScalar x)
{
    return x > 0 && x < SK_Scalar1;
}

static int valid_unit_divide(SkScalar numer, SkScalar denom, SkScalar* ratio)
{
    SkASSERT(ratio);

    if (numer < 0)
    {
        numer = -numer;
        denom = -denom;
    }

    if (denom == 0 || numer == 0 || numer >= denom)
        return 0;

    SkScalar r = SkScalarDiv(numer, denom);
    if (SkScalarIsNaN(r)) {
        return 0;
    }
    SkASSERT(r >= 0 && r < SK_Scalar1);
    if (r == 0) // catch underflow if numer <<<< denom
        return 0;
    *ratio = r;
    return 1;
}

/** From Numerical Recipes in C.

    Q = -1/2 (B + sign(B) sqrt[B*B - 4*A*C])
    x1 = Q / A
    x2 = C / Q
*/
int SkFindUnitQuadRoots(SkScalar A, SkScalar B, SkScalar C, SkScalar roots[2])
{
    SkASSERT(roots);

    if (A == 0)
        return valid_unit_divide(-C, B, roots);

    SkScalar* r = roots;

#ifdef SK_SCALAR_IS_FLOAT
    float R = B*B - 4*A*C;
    if (R < 0 || SkScalarIsNaN(R)) {  // complex roots
        return 0;
    }
    R = sk_float_sqrt(R);
#else
    Sk64    RR, tmp;

    RR.setMul(B,B);
    tmp.setMul(A,C);
    tmp.shiftLeft(2);
    RR.sub(tmp);
    if (RR.isNeg())
        return 0;
    SkFixed R = RR.getSqrt();
#endif

    SkScalar Q = (B < 0) ? -(B-R)/2 : -(B+R)/2;
    r += valid_unit_divide(Q, A, r);
    r += valid_unit_divide(C, Q, r);
    if (r - roots == 2)
    {
        if (roots[0] > roots[1])
            SkTSwap<SkScalar>(roots[0], roots[1]);
        else if (roots[0] == roots[1])  // nearly-equal?
            r -= 1; // skip the double root
    }
    return (int)(r - roots);
}

#ifdef SK_SCALAR_IS_FIXED
/** Trim A/B/C down so that they are all <= 32bits
    and then call SkFindUnitQuadRoots()
*/
static int Sk64FindFixedQuadRoots(const Sk64& A, const Sk64& B, const Sk64& C, SkFixed roots[2])
{
    int na = A.shiftToMake32();
    int nb = B.shiftToMake32();
    int nc = C.shiftToMake32();

    int shift = SkMax32(na, SkMax32(nb, nc));
    SkASSERT(shift >= 0);

    return SkFindUnitQuadRoots(A.getShiftRight(shift), B.getShiftRight(shift), C.getShiftRight(shift), roots);
}
#endif

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

static SkScalar eval_quad(const SkScalar src[], SkScalar t)
{
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= SK_Scalar1);

#ifdef DIRECT_EVAL_OF_POLYNOMIALS
    SkScalar    C = src[0];
    SkScalar    A = src[4] - 2 * src[2] + C;
    SkScalar    B = 2 * (src[2] - C);
    return SkScalarMulAdd(SkScalarMulAdd(A, t, B), t, C);
#else
    SkScalar    ab = SkScalarInterp(src[0], src[2], t);
    SkScalar    bc = SkScalarInterp(src[2], src[4], t);
    return SkScalarInterp(ab, bc, t);
#endif
}

static SkScalar eval_quad_derivative(const SkScalar src[], SkScalar t)
{
    SkScalar A = src[4] - 2 * src[2] + src[0];
    SkScalar B = src[2] - src[0];

    return 2 * SkScalarMulAdd(A, t, B);
}

static SkScalar eval_quad_derivative_at_half(const SkScalar src[])
{
    SkScalar A = src[4] - 2 * src[2] + src[0];
    SkScalar B = src[2] - src[0];
    return A + 2 * B;
}

void SkEvalQuadAt(const SkPoint src[3], SkScalar t, SkPoint* pt, SkVector* tangent)
{
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= SK_Scalar1);

    if (pt)
        pt->set(eval_quad(&src[0].fX, t), eval_quad(&src[0].fY, t));
    if (tangent)
        tangent->set(eval_quad_derivative(&src[0].fX, t),
                     eval_quad_derivative(&src[0].fY, t));
}

void SkEvalQuadAtHalf(const SkPoint src[3], SkPoint* pt, SkVector* tangent)
{
    SkASSERT(src);

    if (pt)
    {
        SkScalar x01 = SkScalarAve(src[0].fX, src[1].fX);
        SkScalar y01 = SkScalarAve(src[0].fY, src[1].fY);
        SkScalar x12 = SkScalarAve(src[1].fX, src[2].fX);
        SkScalar y12 = SkScalarAve(src[1].fY, src[2].fY);
        pt->set(SkScalarAve(x01, x12), SkScalarAve(y01, y12));
    }
    if (tangent)
        tangent->set(eval_quad_derivative_at_half(&src[0].fX),
                     eval_quad_derivative_at_half(&src[0].fY));
}

static void interp_quad_coords(const SkScalar* src, SkScalar* dst, SkScalar t)
{
    SkScalar    ab = SkScalarInterp(src[0], src[2], t);
    SkScalar    bc = SkScalarInterp(src[2], src[4], t);

    dst[0] = src[0];
    dst[2] = ab;
    dst[4] = SkScalarInterp(ab, bc, t);
    dst[6] = bc;
    dst[8] = src[4];
}

void SkChopQuadAt(const SkPoint src[3], SkPoint dst[5], SkScalar t)
{
    SkASSERT(t > 0 && t < SK_Scalar1);

    interp_quad_coords(&src[0].fX, &dst[0].fX, t);
    interp_quad_coords(&src[0].fY, &dst[0].fY, t);
}

void SkChopQuadAtHalf(const SkPoint src[3], SkPoint dst[5])
{
    SkScalar x01 = SkScalarAve(src[0].fX, src[1].fX);
    SkScalar y01 = SkScalarAve(src[0].fY, src[1].fY);
    SkScalar x12 = SkScalarAve(src[1].fX, src[2].fX);
    SkScalar y12 = SkScalarAve(src[1].fY, src[2].fY);

    dst[0] = src[0];
    dst[1].set(x01, y01);
    dst[2].set(SkScalarAve(x01, x12), SkScalarAve(y01, y12));
    dst[3].set(x12, y12);
    dst[4] = src[2];
}

/** Quad'(t) = At + B, where
    A = 2(a - 2b + c)
    B = 2(b - a)
    Solve for t, only if it fits between 0 < t < 1
*/
int SkFindQuadExtrema(SkScalar a, SkScalar b, SkScalar c, SkScalar tValue[1])
{
    /*  At + B == 0
        t = -B / A
    */
#ifdef SK_SCALAR_IS_FIXED
    return is_not_monotonic(a, b, c) && valid_unit_divide(a - b, a - b - b + c, tValue);
#else
    return valid_unit_divide(a - b, a - b - b + c, tValue);
#endif
}

static inline void flatten_double_quad_extrema(SkScalar coords[14])
{
    coords[2] = coords[6] = coords[4];
}

/*  Returns 0 for 1 quad, and 1 for two quads, either way the answer is
 stored in dst[]. Guarantees that the 1/2 quads will be monotonic.
 */
int SkChopQuadAtYExtrema(const SkPoint src[3], SkPoint dst[5])
{
    SkASSERT(src);
    SkASSERT(dst);

#if 0
    static bool once = true;
    if (once)
    {
        once = false;
        SkPoint s[3] = { 0, 26398, 0, 26331, 0, 20621428 };
        SkPoint d[6];

        int n = SkChopQuadAtYExtrema(s, d);
        SkDebugf("chop=%d, Y=[%x %x %x %x %x %x]\n", n, d[0].fY, d[1].fY, d[2].fY, d[3].fY, d[4].fY, d[5].fY);
    }
#endif

    SkScalar a = src[0].fY;
    SkScalar b = src[1].fY;
    SkScalar c = src[2].fY;

    if (is_not_monotonic(a, b, c))
    {
        SkScalar    tValue;
        if (valid_unit_divide(a - b, a - b - b + c, &tValue))
        {
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
int SkChopQuadAtXExtrema(const SkPoint src[3], SkPoint dst[5])
{
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
float SkFindQuadMaxCurvature(const SkPoint src[3]) {
    SkScalar    Ax = src[1].fX - src[0].fX;
    SkScalar    Ay = src[1].fY - src[0].fY;
    SkScalar    Bx = src[0].fX - src[1].fX - src[1].fX + src[2].fX;
    SkScalar    By = src[0].fY - src[1].fY - src[1].fY + src[2].fY;
    SkScalar    t = 0;  // 0 means don't chop

#ifdef SK_SCALAR_IS_FLOAT
    (void)valid_unit_divide(-(Ax * Bx + Ay * By), Bx * Bx + By * By, &t);
#else
    // !!! should I use SkFloat here? seems like it
    Sk64    numer, denom, tmp;

    numer.setMul(Ax, -Bx);
    tmp.setMul(Ay, -By);
    numer.add(tmp);

    if (numer.isPos())  // do nothing if numer <= 0
    {
        denom.setMul(Bx, Bx);
        tmp.setMul(By, By);
        denom.add(tmp);
        SkASSERT(!denom.isNeg());
        if (numer < denom)
        {
            t = numer.getFixedDiv(denom);
            SkASSERT(t >= 0 && t <= SK_Fixed1);     // assert that we're numerically stable (ha!)
            if ((unsigned)t >= SK_Fixed1)           // runtime check for numerical stability
                t = 0;  // ignore the chop
        }
    }
#endif
    return t;
}

int SkChopQuadAtMaxCurvature(const SkPoint src[3], SkPoint dst[5])
{
    SkScalar t = SkFindQuadMaxCurvature(src);
    if (t == 0) {
        memcpy(dst, src, 3 * sizeof(SkPoint));
        return 1;
    } else {
        SkChopQuadAt(src, dst, t);
        return 2;
    }
}

#ifdef SK_SCALAR_IS_FLOAT
    #define SK_ScalarTwoThirds  (0.666666666f)
#else
    #define SK_ScalarTwoThirds  ((SkFixed)(43691))
#endif

void SkConvertQuadToCubic(const SkPoint src[3], SkPoint dst[4]) {
    const SkScalar scale = SK_ScalarTwoThirds;
    dst[0] = src[0];
    dst[1].set(src[0].fX + SkScalarMul(src[1].fX - src[0].fX, scale),
               src[0].fY + SkScalarMul(src[1].fY - src[0].fY, scale));
    dst[2].set(src[2].fX + SkScalarMul(src[1].fX - src[2].fX, scale),
               src[2].fY + SkScalarMul(src[1].fY - src[2].fY, scale));
    dst[3] = src[2];
}

////////////////////////////////////////////////////////////////////////////////////////
///// CUBICS // CUBICS // CUBICS // CUBICS // CUBICS // CUBICS // CUBICS // CUBICS /////
////////////////////////////////////////////////////////////////////////////////////////

static void get_cubic_coeff(const SkScalar pt[], SkScalar coeff[4])
{
    coeff[0] = pt[6] + 3*(pt[2] - pt[4]) - pt[0];
    coeff[1] = 3*(pt[4] - pt[2] - pt[2] + pt[0]);
    coeff[2] = 3*(pt[2] - pt[0]);
    coeff[3] = pt[0];
}

void SkGetCubicCoeff(const SkPoint pts[4], SkScalar cx[4], SkScalar cy[4])
{
    SkASSERT(pts);

    if (cx)
        get_cubic_coeff(&pts[0].fX, cx);
    if (cy)
        get_cubic_coeff(&pts[0].fY, cy);
}

static SkScalar eval_cubic(const SkScalar src[], SkScalar t)
{
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= SK_Scalar1);

    if (t == 0)
        return src[0];

#ifdef DIRECT_EVAL_OF_POLYNOMIALS
    SkScalar D = src[0];
    SkScalar A = src[6] + 3*(src[2] - src[4]) - D;
    SkScalar B = 3*(src[4] - src[2] - src[2] + D);
    SkScalar C = 3*(src[2] - D);

    return SkScalarMulAdd(SkScalarMulAdd(SkScalarMulAdd(A, t, B), t, C), t, D);
#else
    SkScalar    ab = SkScalarInterp(src[0], src[2], t);
    SkScalar    bc = SkScalarInterp(src[2], src[4], t);
    SkScalar    cd = SkScalarInterp(src[4], src[6], t);
    SkScalar    abc = SkScalarInterp(ab, bc, t);
    SkScalar    bcd = SkScalarInterp(bc, cd, t);
    return SkScalarInterp(abc, bcd, t);
#endif
}

/** return At^2 + Bt + C
*/
static SkScalar eval_quadratic(SkScalar A, SkScalar B, SkScalar C, SkScalar t)
{
    SkASSERT(t >= 0 && t <= SK_Scalar1);

    return SkScalarMulAdd(SkScalarMulAdd(A, t, B), t, C);
}

static SkScalar eval_cubic_derivative(const SkScalar src[], SkScalar t)
{
    SkScalar A = src[6] + 3*(src[2] - src[4]) - src[0];
    SkScalar B = 2*(src[4] - 2 * src[2] + src[0]);
    SkScalar C = src[2] - src[0];

    return eval_quadratic(A, B, C, t);
}

static SkScalar eval_cubic_2ndDerivative(const SkScalar src[], SkScalar t)
{
    SkScalar A = src[6] + 3*(src[2] - src[4]) - src[0];
    SkScalar B = src[4] - 2 * src[2] + src[0];

    return SkScalarMulAdd(A, t, B);
}

void SkEvalCubicAt(const SkPoint src[4], SkScalar t, SkPoint* loc, SkVector* tangent, SkVector* curvature)
{
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= SK_Scalar1);

    if (loc)
        loc->set(eval_cubic(&src[0].fX, t), eval_cubic(&src[0].fY, t));
    if (tangent)
        tangent->set(eval_cubic_derivative(&src[0].fX, t),
                     eval_cubic_derivative(&src[0].fY, t));
    if (curvature)
        curvature->set(eval_cubic_2ndDerivative(&src[0].fX, t),
                       eval_cubic_2ndDerivative(&src[0].fY, t));
}

/** Cubic'(t) = At^2 + Bt + C, where
    A = 3(-a + 3(b - c) + d)
    B = 6(a - 2b + c)
    C = 3(b - a)
    Solve for t, keeping only those that fit betwee 0 < t < 1
*/
int SkFindCubicExtrema(SkScalar a, SkScalar b, SkScalar c, SkScalar d, SkScalar tValues[2])
{
#ifdef SK_SCALAR_IS_FIXED
    if (!is_not_monotonic(a, b, c, d))
        return 0;
#endif

    // we divide A,B,C by 3 to simplify
    SkScalar A = d - a + 3*(b - c);
    SkScalar B = 2*(a - b - b + c);
    SkScalar C = b - a;

    return SkFindUnitQuadRoots(A, B, C, tValues);
}

static void interp_cubic_coords(const SkScalar* src, SkScalar* dst, SkScalar t)
{
    SkScalar    ab = SkScalarInterp(src[0], src[2], t);
    SkScalar    bc = SkScalarInterp(src[2], src[4], t);
    SkScalar    cd = SkScalarInterp(src[4], src[6], t);
    SkScalar    abc = SkScalarInterp(ab, bc, t);
    SkScalar    bcd = SkScalarInterp(bc, cd, t);
    SkScalar    abcd = SkScalarInterp(abc, bcd, t);

    dst[0] = src[0];
    dst[2] = ab;
    dst[4] = abc;
    dst[6] = abcd;
    dst[8] = bcd;
    dst[10] = cd;
    dst[12] = src[6];
}

void SkChopCubicAt(const SkPoint src[4], SkPoint dst[7], SkScalar t)
{
    SkASSERT(t > 0 && t < SK_Scalar1);

    interp_cubic_coords(&src[0].fX, &dst[0].fX, t);
    interp_cubic_coords(&src[0].fY, &dst[0].fY, t);
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

void SkChopCubicAt(const SkPoint src[4], SkPoint dst[], const SkScalar tValues[], int roots)
{
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

    if (dst)
    {
        if (roots == 0) // nothing to chop
            memcpy(dst, src, 4*sizeof(SkPoint));
        else
        {
            SkScalar    t = tValues[0];
            SkPoint     tmp[4];

            for (int i = 0; i < roots; i++)
            {
                SkChopCubicAt(src, dst, t);
                if (i == roots - 1)
                    break;

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

void SkChopCubicAtHalf(const SkPoint src[4], SkPoint dst[7])
{
    SkScalar x01 = SkScalarAve(src[0].fX, src[1].fX);
    SkScalar y01 = SkScalarAve(src[0].fY, src[1].fY);
    SkScalar x12 = SkScalarAve(src[1].fX, src[2].fX);
    SkScalar y12 = SkScalarAve(src[1].fY, src[2].fY);
    SkScalar x23 = SkScalarAve(src[2].fX, src[3].fX);
    SkScalar y23 = SkScalarAve(src[2].fY, src[3].fY);

    SkScalar x012 = SkScalarAve(x01, x12);
    SkScalar y012 = SkScalarAve(y01, y12);
    SkScalar x123 = SkScalarAve(x12, x23);
    SkScalar y123 = SkScalarAve(y12, y23);

    dst[0] = src[0];
    dst[1].set(x01, y01);
    dst[2].set(x012, y012);
    dst[3].set(SkScalarAve(x012, x123), SkScalarAve(y012, y123));
    dst[4].set(x123, y123);
    dst[5].set(x23, y23);
    dst[6] = src[3];
}

static void flatten_double_cubic_extrema(SkScalar coords[14])
{
    coords[4] = coords[8] = coords[6];
}

/** Given 4 points on a cubic bezier, chop it into 1, 2, 3 beziers such that
    the resulting beziers are monotonic in Y. This is called by the scan converter.
    Depending on what is returned, dst[] is treated as follows
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
int SkFindCubicInflections(const SkPoint src[4], SkScalar tValues[])
{
    SkScalar    Ax = src[1].fX - src[0].fX;
    SkScalar    Ay = src[1].fY - src[0].fY;
    SkScalar    Bx = src[2].fX - 2 * src[1].fX + src[0].fX;
    SkScalar    By = src[2].fY - 2 * src[1].fY + src[0].fY;
    SkScalar    Cx = src[3].fX + 3 * (src[1].fX - src[2].fX) - src[0].fX;
    SkScalar    Cy = src[3].fY + 3 * (src[1].fY - src[2].fY) - src[0].fY;
    int         count;

#ifdef SK_SCALAR_IS_FLOAT
    count = SkFindUnitQuadRoots(Bx*Cy - By*Cx, Ax*Cy - Ay*Cx, Ax*By - Ay*Bx, tValues);
#else
    Sk64    A, B, C, tmp;

    A.setMul(Bx, Cy);
    tmp.setMul(By, Cx);
    A.sub(tmp);

    B.setMul(Ax, Cy);
    tmp.setMul(Ay, Cx);
    B.sub(tmp);

    C.setMul(Ax, By);
    tmp.setMul(Ay, Bx);
    C.sub(tmp);

    count = Sk64FindFixedQuadRoots(A, B, C, tValues);
#endif

    return count;
}

int SkChopCubicAtInflections(const SkPoint src[], SkPoint dst[10])
{
    SkScalar    tValues[2];
    int         count = SkFindCubicInflections(src, tValues);

    if (dst)
    {
        if (count == 0)
            memcpy(dst, src, 4 * sizeof(SkPoint));
        else
            SkChopCubicAt(src, dst, tValues, count);
    }
    return count + 1;
}

template <typename T> void bubble_sort(T array[], int count)
{
    for (int i = count - 1; i > 0; --i)
        for (int j = i; j > 0; --j)
            if (array[j] < array[j-1])
            {
                T   tmp(array[j]);
                array[j] = array[j-1];
                array[j-1] = tmp;
            }
}

#include "SkFP.h"

// newton refinement
#if 0
static SkScalar refine_cubic_root(const SkFP coeff[4], SkScalar root)
{
    //  x1 = x0 - f(t) / f'(t)

    SkFP    T = SkScalarToFloat(root);
    SkFP    N, D;

    // f' = 3*coeff[0]*T^2 + 2*coeff[1]*T + coeff[2]
    D = SkFPMul(SkFPMul(coeff[0], SkFPMul(T,T)), 3);
    D = SkFPAdd(D, SkFPMulInt(SkFPMul(coeff[1], T), 2));
    D = SkFPAdd(D, coeff[2]);

    if (D == 0)
        return root;

    // f = coeff[0]*T^3 + coeff[1]*T^2 + coeff[2]*T + coeff[3]
    N = SkFPMul(SkFPMul(SkFPMul(T, T), T), coeff[0]);
    N = SkFPAdd(N, SkFPMul(SkFPMul(T, T), coeff[1]));
    N = SkFPAdd(N, SkFPMul(T, coeff[2]));
    N = SkFPAdd(N, coeff[3]);

    if (N)
    {
        SkScalar delta = SkFPToScalar(SkFPDiv(N, D));

        if (delta)
            root -= delta;
    }
    return root;
}
#endif

/**
 *  Given an array and count, remove all pair-wise duplicates from the array,
 *  keeping the existing sorting, and return the new count
 */
static int collaps_duplicates(float array[], int count) {
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
    const float src0[] = { 0 };
    const float src1[] = { 0, 0 };
    const float src2[] = { 0, 1 };
    const float src3[] = { 0, 0, 0 };
    const float src4[] = { 0, 0, 1 };
    const float src5[] = { 0, 1, 1 };
    const float src6[] = { 0, 1, 2 };
    const struct {
        const float* fData;
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
        float dst[3];
        memcpy(dst, data[i].fData, data[i].fCount * sizeof(dst[0]));
        int count = collaps_duplicates(dst, data[i].fCount);
        SkASSERT(data[i].fCollapsedCount == count);
        for (int j = 1; j < count; ++j) {
            SkASSERT(dst[j-1] < dst[j]);
        }
    }
}
#endif

#if defined _WIN32 && _MSC_VER >= 1300  && defined SK_SCALAR_IS_FIXED // disable warning : unreachable code if building fixed point for windows desktop
#pragma warning ( disable : 4702 )
#endif

/*  Solve coeff(t) == 0, returning the number of roots that
    lie withing 0 < t < 1.
    coeff[0]t^3 + coeff[1]t^2 + coeff[2]t + coeff[3]

    Eliminates repeated roots (so that all tValues are distinct, and are always
    in increasing order.
*/
static int solve_cubic_polynomial(const SkFP coeff[4], SkScalar tValues[3])
{
#ifndef SK_SCALAR_IS_FLOAT
    return 0;   // this is not yet implemented for software float
#endif

    if (SkScalarNearlyZero(coeff[0]))   // we're just a quadratic
    {
        return SkFindUnitQuadRoots(coeff[1], coeff[2], coeff[3], tValues);
    }

    SkFP    a, b, c, Q, R;

    {
        SkASSERT(coeff[0] != 0);

        SkFP inva = SkFPInvert(coeff[0]);
        a = SkFPMul(coeff[1], inva);
        b = SkFPMul(coeff[2], inva);
        c = SkFPMul(coeff[3], inva);
    }
    Q = SkFPDivInt(SkFPSub(SkFPMul(a,a), SkFPMulInt(b, 3)), 9);
//  R = (2*a*a*a - 9*a*b + 27*c) / 54;
    R = SkFPMulInt(SkFPMul(SkFPMul(a, a), a), 2);
    R = SkFPSub(R, SkFPMulInt(SkFPMul(a, b), 9));
    R = SkFPAdd(R, SkFPMulInt(c, 27));
    R = SkFPDivInt(R, 54);

    SkFP Q3 = SkFPMul(SkFPMul(Q, Q), Q);
    SkFP R2MinusQ3 = SkFPSub(SkFPMul(R,R), Q3);
    SkFP adiv3 = SkFPDivInt(a, 3);

    SkScalar*   roots = tValues;
    SkScalar    r;

    if (SkFPLT(R2MinusQ3, 0))   // we have 3 real roots
    {
#ifdef SK_SCALAR_IS_FLOAT
        float theta = sk_float_acos(R / sk_float_sqrt(Q3));
        float neg2RootQ = -2 * sk_float_sqrt(Q);

        r = neg2RootQ * sk_float_cos(theta/3) - adiv3;
        if (is_unit_interval(r))
            *roots++ = r;

        r = neg2RootQ * sk_float_cos((theta + 2*SK_ScalarPI)/3) - adiv3;
        if (is_unit_interval(r))
            *roots++ = r;

        r = neg2RootQ * sk_float_cos((theta - 2*SK_ScalarPI)/3) - adiv3;
        if (is_unit_interval(r))
            *roots++ = r;

        SkDEBUGCODE(test_collaps_duplicates();)

        // now sort the roots
        int count = (int)(roots - tValues);
        SkASSERT((unsigned)count <= 3);
        bubble_sort(tValues, count);
        count = collaps_duplicates(tValues, count);
        roots = tValues + count;    // so we compute the proper count below
#endif
    }
    else                // we have 1 real root
    {
        SkFP A = SkFPAdd(SkFPAbs(R), SkFPSqrt(R2MinusQ3));
        A = SkFPCubeRoot(A);
        if (SkFPGT(R, 0))
            A = SkFPNeg(A);

        if (A != 0)
            A = SkFPAdd(A, SkFPDiv(Q, A));
        r = SkFPToScalar(SkFPSub(A, adiv3));
        if (is_unit_interval(r))
            *roots++ = r;
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
static void formulate_F1DotF2(const SkScalar src[], SkFP coeff[4])
{
    SkScalar    a = src[2] - src[0];
    SkScalar    b = src[4] - 2 * src[2] + src[0];
    SkScalar    c = src[6] + 3 * (src[2] - src[4]) - src[0];

    SkFP    A = SkScalarToFP(a);
    SkFP    B = SkScalarToFP(b);
    SkFP    C = SkScalarToFP(c);

    coeff[0] = SkFPMul(C, C);
    coeff[1] = SkFPMulInt(SkFPMul(B, C), 3);
    coeff[2] = SkFPMulInt(SkFPMul(B, B), 2);
    coeff[2] = SkFPAdd(coeff[2], SkFPMul(C, A));
    coeff[3] = SkFPMul(A, B);
}

// EXPERIMENTAL: can set this to zero to accept all t-values 0 < t < 1
//#define kMinTValueForChopping (SK_Scalar1 / 256)
#define kMinTValueForChopping   0

/*  Looking for F' dot F'' == 0

    A = b - a
    B = c - 2b + a
    C = d - 3c + 3b - a

    F' = 3Ct^2 + 6Bt + 3A
    F'' = 6Ct + 6B

    F' dot F'' -> CCt^3 + 3BCt^2 + (2BB + CA)t + AB
*/
int SkFindCubicMaxCurvature(const SkPoint src[4], SkScalar tValues[3])
{
    SkFP    coeffX[4], coeffY[4];
    int     i;

    formulate_F1DotF2(&src[0].fX, coeffX);
    formulate_F1DotF2(&src[0].fY, coeffY);

    for (i = 0; i < 4; i++)
        coeffX[i] = SkFPAdd(coeffX[i],coeffY[i]);

    SkScalar    t[3];
    int         count = solve_cubic_polynomial(coeffX, t);
    int         maxCount = 0;

    // now remove extrema where the curvature is zero (mins)
    // !!!! need a test for this !!!!
    for (i = 0; i < count; i++)
    {
        // if (not_min_curvature())
        if (t[i] > kMinTValueForChopping && t[i] < SK_Scalar1 - kMinTValueForChopping)
            tValues[maxCount++] = t[i];
    }
    return maxCount;
}

int SkChopCubicAtMaxCurvature(const SkPoint src[4], SkPoint dst[13], SkScalar tValues[3])
{
    SkScalar    t_storage[3];

    if (tValues == NULL)
        tValues = t_storage;

    int count = SkFindCubicMaxCurvature(src, tValues);

    if (dst) {
        if (count == 0)
            memcpy(dst, src, 4 * sizeof(SkPoint));
        else
            SkChopCubicAt(src, dst, tValues, count);
    }
    return count + 1;
}

bool SkXRayCrossesMonotonicCubic(const SkXRay& pt, const SkPoint cubic[4], bool* ambiguous) {
    if (ambiguous) {
        *ambiguous = false;
    }

    // Find the minimum and maximum y of the extrema, which are the
    // first and last points since this cubic is monotonic
    SkScalar min_y = SkMinScalar(cubic[0].fY, cubic[3].fY);
    SkScalar max_y = SkMaxScalar(cubic[0].fY, cubic[3].fY);

    if (pt.fY == cubic[0].fY
        || pt.fY < min_y
        || pt.fY > max_y) {
        // The query line definitely does not cross the curve
        if (ambiguous) {
            *ambiguous = (pt.fY == cubic[0].fY);
        }
        return false;
    }

    bool pt_at_extremum = (pt.fY == cubic[3].fY);

    SkScalar min_x =
        SkMinScalar(
            SkMinScalar(
                SkMinScalar(cubic[0].fX, cubic[1].fX),
                cubic[2].fX),
            cubic[3].fX);
    if (pt.fX < min_x) {
        // The query line definitely crosses the curve
        if (ambiguous) {
            *ambiguous = pt_at_extremum;
        }
        return true;
    }

    SkScalar max_x =
        SkMaxScalar(
            SkMaxScalar(
                SkMaxScalar(cubic[0].fX, cubic[1].fX),
                cubic[2].fX),
            cubic[3].fX);
    if (pt.fX > max_x) {
        // The query line definitely does not cross the curve
        return false;
    }

    // Do a binary search to find the parameter value which makes y as
    // close as possible to the query point. See whether the query
    // line's origin is to the left of the associated x coordinate.

    // kMaxIter is chosen as the number of mantissa bits for a float,
    // since there's no way we are going to get more precision by
    // iterating more times than that.
    const int kMaxIter = 23;
    SkPoint eval;
    int iter = 0;
    SkScalar upper_t;
    SkScalar lower_t;
    // Need to invert direction of t parameter if cubic goes up
    // instead of down
    if (cubic[3].fY > cubic[0].fY) {
        upper_t = SK_Scalar1;
        lower_t = SkFloatToScalar(0);
    } else {
        upper_t = SkFloatToScalar(0);
        lower_t = SK_Scalar1;
    }
    do {
        SkScalar t = SkScalarAve(upper_t, lower_t);
        SkEvalCubicAt(cubic, t, &eval, NULL, NULL);
        if (pt.fY > eval.fY) {
            lower_t = t;
        } else {
            upper_t = t;
        }
    } while (++iter < kMaxIter
             && !SkScalarNearlyZero(eval.fY - pt.fY));
    if (pt.fX <= eval.fX) {
        if (ambiguous) {
            *ambiguous = pt_at_extremum;
        }
        return true;
    }
    return false;
}

int SkNumXRayCrossingsForCubic(const SkXRay& pt, const SkPoint cubic[4], bool* ambiguous) {
    int num_crossings = 0;
    SkPoint monotonic_cubics[10];
    int num_monotonic_cubics = SkChopCubicAtYExtrema(cubic, monotonic_cubics);
    if (ambiguous) {
        *ambiguous = false;
    }
    bool locally_ambiguous;
    if (SkXRayCrossesMonotonicCubic(pt, &monotonic_cubics[0], &locally_ambiguous))
        ++num_crossings;
    if (ambiguous) {
        *ambiguous |= locally_ambiguous;
    }
    if (num_monotonic_cubics > 0)
        if (SkXRayCrossesMonotonicCubic(pt, &monotonic_cubics[3], &locally_ambiguous))
            ++num_crossings;
    if (ambiguous) {
        *ambiguous |= locally_ambiguous;
    }
    if (num_monotonic_cubics > 1)
        if (SkXRayCrossesMonotonicCubic(pt, &monotonic_cubics[6], &locally_ambiguous))
            ++num_crossings;
    if (ambiguous) {
        *ambiguous |= locally_ambiguous;
    }
    return num_crossings;
}
////////////////////////////////////////////////////////////////////////////////

/*  Find t value for quadratic [a, b, c] = d.
    Return 0 if there is no solution within [0, 1)
*/
static SkScalar quad_solve(SkScalar a, SkScalar b, SkScalar c, SkScalar d)
{
    // At^2 + Bt + C = d
    SkScalar A = a - 2 * b + c;
    SkScalar B = 2 * (b - a);
    SkScalar C = a - d;

    SkScalar    roots[2];
    int         count = SkFindUnitQuadRoots(A, B, C, roots);

    SkASSERT(count <= 1);
    return count == 1 ? roots[0] : 0;
}

/*  given a quad-curve and a point (x,y), chop the quad at that point and place
    the new off-curve point and endpoint into 'dest'.
    Should only return false if the computed pos is the start of the curve
    (i.e. root == 0)
*/
static bool truncate_last_curve(const SkPoint quad[3], SkScalar x, SkScalar y, SkPoint* dest)
{
    const SkScalar* base;
    SkScalar        value;

    if (SkScalarAbs(x) < SkScalarAbs(y)) {
        base = &quad[0].fX;
        value = x;
    } else {
        base = &quad[0].fY;
        value = y;
    }

    // note: this returns 0 if it thinks value is out of range, meaning the
    // root might return something outside of [0, 1)
    SkScalar t = quad_solve(base[0], base[2], base[4], value);

    if (t > 0)
    {
        SkPoint tmp[5];
        SkChopQuadAt(quad, tmp, t);
        dest[0] = tmp[1];
        dest[1].set(x, y);
        return true;
    } else {
        /*  t == 0 means either the value triggered a root outside of [0, 1)
            For our purposes, we can ignore the <= 0 roots, but we want to
            catch the >= 1 roots (which given our caller, will basically mean
            a root of 1, give-or-take numerical instability). If we are in the
            >= 1 case, return the existing offCurve point.

            The test below checks to see if we are close to the "end" of the
            curve (near base[4]). Rather than specifying a tolerance, I just
            check to see if value is on to the right/left of the middle point
            (depending on the direction/sign of the end points).
        */
        if ((base[0] < base[4] && value > base[2]) ||
            (base[0] > base[4] && value < base[2]))   // should root have been 1
        {
            dest[0] = quad[1];
            dest[1].set(x, y);
            return true;
        }
    }
    return false;
}

#ifdef SK_SCALAR_IS_FLOAT

// Due to floating point issues (i.e., 1.0f - SK_ScalarRoot2Over2 !=
// SK_ScalarRoot2Over2 - SK_ScalarTanPIOver8), the "correct" off curve
// control points cause the quadratic circle approximation to be concave.
// SK_OffEps is used to pull in the off-curve control points a bit
// to make the quadratic approximation convex.
// Pulling the off-curve controls points in is preferable to pushing some
// of the on-curve points off.
#define SK_OffEps 0.0001f
#else
#define SK_OffEps 0
#endif


static const SkPoint gQuadCirclePts[kSkBuildQuadArcStorage] = {
    { SK_Scalar1,                      0                                  },
    { SK_Scalar1 - SK_OffEps,          SK_ScalarTanPIOver8 - SK_OffEps    },
    { SK_ScalarRoot2Over2,             SK_ScalarRoot2Over2                },
    { SK_ScalarTanPIOver8 - SK_OffEps, SK_Scalar1 - SK_OffEps             },

    { 0,                               SK_Scalar1                         },
    { -SK_ScalarTanPIOver8 + SK_OffEps,SK_Scalar1 - SK_OffEps             },
    { -SK_ScalarRoot2Over2,            SK_ScalarRoot2Over2                },
    { -SK_Scalar1 + SK_OffEps,         SK_ScalarTanPIOver8 - SK_OffEps    },

    { -SK_Scalar1,                     0                                  },
    { -SK_Scalar1 + SK_OffEps,         -SK_ScalarTanPIOver8 + SK_OffEps   },
    { -SK_ScalarRoot2Over2,            -SK_ScalarRoot2Over2               },
    { -SK_ScalarTanPIOver8 + SK_OffEps,-SK_Scalar1 + SK_OffEps            },

    { 0,                               -SK_Scalar1                        },
    { SK_ScalarTanPIOver8 - SK_OffEps, -SK_Scalar1 + SK_OffEps            },
    { SK_ScalarRoot2Over2,             -SK_ScalarRoot2Over2               },
    { SK_Scalar1 - SK_OffEps,          -SK_ScalarTanPIOver8 + SK_OffEps   },

    { SK_Scalar1,                      0                                  }
};

int SkBuildQuadArc(const SkVector& uStart, const SkVector& uStop,
                   SkRotationDirection dir, const SkMatrix* userMatrix,
                   SkPoint quadPoints[])
{
    // rotate by x,y so that uStart is (1.0)
    SkScalar x = SkPoint::DotProduct(uStart, uStop);
    SkScalar y = SkPoint::CrossProduct(uStart, uStop);

    SkScalar absX = SkScalarAbs(x);
    SkScalar absY = SkScalarAbs(y);

    int pointCount;

    // check for (effectively) coincident vectors
    // this can happen if our angle is nearly 0 or nearly 180 (y == 0)
    // ... we use the dot-prod to distinguish between 0 and 180 (x > 0)
    if (absY <= SK_ScalarNearlyZero && x > 0 &&
        ((y >= 0 && kCW_SkRotationDirection == dir) ||
         (y <= 0 && kCCW_SkRotationDirection == dir))) {

        // just return the start-point
        quadPoints[0].set(SK_Scalar1, 0);
        pointCount = 1;
    } else {
        if (dir == kCCW_SkRotationDirection)
            y = -y;

        // what octant (quadratic curve) is [xy] in?
        int oct = 0;
        bool sameSign = true;

        if (0 == y)
        {
            oct = 4;        // 180
            SkASSERT(SkScalarAbs(x + SK_Scalar1) <= SK_ScalarNearlyZero);
        }
        else if (0 == x)
        {
            SkASSERT(absY - SK_Scalar1 <= SK_ScalarNearlyZero);
            if (y > 0)
                oct = 2;    // 90
            else
                oct = 6;    // 270
        }
        else
        {
            if (y < 0)
                oct += 4;
            if ((x < 0) != (y < 0))
            {
                oct += 2;
                sameSign = false;
            }
            if ((absX < absY) == sameSign)
                oct += 1;
        }

        int wholeCount = oct << 1;
        memcpy(quadPoints, gQuadCirclePts, (wholeCount + 1) * sizeof(SkPoint));

        const SkPoint* arc = &gQuadCirclePts[wholeCount];
        if (truncate_last_curve(arc, x, y, &quadPoints[wholeCount + 1]))
        {
            wholeCount += 2;
        }
        pointCount = wholeCount + 1;
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
    matrix.mapPoints(quadPoints, pointCount);
    return pointCount;
}

///////////////////////////////////////////////////////////////////////////////

// F = (A (1 - t)^2 + C t^2 + 2 B (1 - t) t w)
//     ------------------------------------------
//         ((1 - t)^2 + t^2 + 2 (1 - t) t w)
//
//   = {t^2 (P0 + P2 - 2 P1 w), t (-2 P0 + 2 P1 w), P0}
//     ------------------------------------------------
//             {t^2 (2 - 2 w), t (-2 + 2 w), 1}
//

// Take the parametric specification for the conic (either X or Y) and return
// in coeff[] the coefficients for the simple quadratic polynomial
//    coeff[0] for t^2
//    coeff[1] for t
//    coeff[2] for constant term
//
static SkScalar conic_eval_pos(const SkScalar src[], SkScalar w, SkScalar t) {
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= SK_Scalar1);

    SkScalar    src2w = SkScalarMul(src[2], w);
    SkScalar    C = src[0];
    SkScalar    A = src[4] - 2 * src2w + C;
    SkScalar    B = 2 * (src2w - C);
    SkScalar numer = SkScalarMulAdd(SkScalarMulAdd(A, t, B), t, C);

    B = 2 * (w - SK_Scalar1);
    C = SK_Scalar1;
    A = -B;
    SkScalar denom = SkScalarMulAdd(SkScalarMulAdd(A, t, B), t, C);

    return SkScalarDiv(numer, denom);
}

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
static void conic_deriv_coeff(const SkScalar src[], SkScalar w, SkScalar coeff[3]) {
    const SkScalar P20 = src[4] - src[0];
    const SkScalar P10 = src[2] - src[0];
    const SkScalar wP10 = w * P10;
    coeff[0] = w * P20 - P20;
    coeff[1] = P20 - 2 * wP10;
    coeff[2] = wP10;
}

static SkScalar conic_eval_tan(const SkScalar coord[], SkScalar w, SkScalar t) {
    SkScalar coeff[3];
    conic_deriv_coeff(coord, w, coeff);
    return t * (t * coeff[0] + coeff[1]) + coeff[2];
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

// we just return the middle 3 points, since the first and last are dups of src
//
static void p3d_interp(const SkScalar src[3], SkScalar dst[3], SkScalar t) {
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

void SkConic::evalAt(SkScalar t, SkPoint* pt, SkVector* tangent) const {
    SkASSERT(t >= 0 && t <= SK_Scalar1);

    if (pt) {
        pt->set(conic_eval_pos(&fPts[0].fX, fW, t),
                conic_eval_pos(&fPts[0].fY, fW, t));
    }
    if (tangent) {
        tangent->set(conic_eval_tan(&fPts[0].fX, fW, t),
                     conic_eval_tan(&fPts[0].fY, fW, t));
    }
}

void SkConic::chopAt(SkScalar t, SkConic dst[2]) const {
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
    // However, in our case, we know that for dst[0], w0 == 1, and for dst[1], w2 == 1
    //
    SkScalar root = SkScalarSqrt(tmp2[1].fZ);
    dst[0].fW = tmp2[0].fZ / root;
    dst[1].fW = tmp2[2].fZ / root;
}

static SkScalar subdivide_w_value(SkScalar w) {
    return SkScalarSqrt(SK_ScalarHalf + w * SK_ScalarHalf);
}

void SkConic::chop(SkConic dst[2]) const {
    SkScalar scale = SkScalarInvert(SK_Scalar1 + fW);
    SkScalar p1x = fW * fPts[1].fX;
    SkScalar p1y = fW * fPts[1].fY;
    SkScalar mx = (fPts[0].fX + 2 * p1x + fPts[2].fX) * scale * SK_ScalarHalf;
    SkScalar my = (fPts[0].fY + 2 * p1y + fPts[2].fY) * scale * SK_ScalarHalf;

    dst[0].fPts[0] = fPts[0];
    dst[0].fPts[1].set((fPts[0].fX + p1x) * scale,
                       (fPts[0].fY + p1y) * scale);
    dst[0].fPts[2].set(mx, my);

    dst[1].fPts[0].set(mx, my);
    dst[1].fPts[1].set((p1x + fPts[2].fX) * scale,
                       (p1y + fPts[2].fY) * scale);
    dst[1].fPts[2] = fPts[2];

    dst[0].fW = dst[1].fW = subdivide_w_value(fW);
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

int SkConic::computeQuadPOW2(SkScalar tol) const {
    AS_QUAD_ERROR_SETUP
    SkScalar error = SkScalarSqrt(x * x + y * y) - tol;

    if (error <= 0) {
        return 0;
    }
    uint32_t ierr = (uint32_t)error;
    return (34 - SkCLZ(ierr)) >> 1;
}

static SkPoint* subdivide(const SkConic& src, SkPoint pts[], int level) {
    SkASSERT(level >= 0);

    if (0 == level) {
        memcpy(pts, &src.fPts[1], 2 * sizeof(SkPoint));
        return pts + 2;
    } else {
        SkConic dst[2];
        src.chop(dst);
        --level;
        pts = subdivide(dst[0], pts, level);
        return subdivide(dst[1], pts, level);
    }
}

int SkConic::chopIntoQuadsPOW2(SkPoint pts[], int pow2) const {
    SkASSERT(pow2 >= 0);
    *pts = fPts[0];
    SkDEBUGCODE(SkPoint* endPts =) subdivide(*this, pts + 1, pow2);
    SkASSERT(endPts - pts == (2 * (1 << pow2) + 1));
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
        this->chopAt(t, dst);
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
        this->chopAt(t, dst);
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
