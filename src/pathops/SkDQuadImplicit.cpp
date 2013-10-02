/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkDQuadImplicit.h"

/* from http://tom.cs.byu.edu/~tom/papers/cvgip84.pdf 4.1
 *
 * This paper proves that Syvester's method can compute the implicit form of
 * the quadratic from the parameterized form.
 *
 * Given x = a*t*t + b*t + c  (the parameterized form)
 *       y = d*t*t + e*t + f
 *
 * we want to find an equation of the implicit form:
 *
 * A*x*x + B*x*y + C*y*y + D*x + E*y + F = 0
 *
 * The implicit form can be expressed as a 4x4 determinant, as shown.
 *
 * The resultant obtained by Syvester's method is
 *
 * |   a   b   (c - x)     0     |
 * |   0   a      b     (c - x)  |
 * |   d   e   (f - y)     0     |
 * |   0   d      e     (f - y)  |
 *
 * which expands to
 *
 * d*d*x*x + -2*a*d*x*y + a*a*y*y
 *         + (-2*c*d*d + b*e*d - a*e*e + 2*a*f*d)*x
 *         + (-2*f*a*a + e*b*a - d*b*b + 2*d*c*a)*y
 *         +
 * |   a   b   c   0   |
 * |   0   a   b   c   | == 0.
 * |   d   e   f   0   |
 * |   0   d   e   f   |
 *
 * Expanding the constant determinant results in
 *
 *   | a b c |     | b c 0 |
 * a*| e f 0 | + d*| a b c | ==
 *   | d e f |     | d e f |
 *
 * a*(a*f*f + c*e*e - c*f*d - b*e*f) + d*(b*b*f + c*c*d - c*a*f - c*e*b)
 *
 */

// use the tricky arithmetic path, but leave the original to compare just in case
static bool straight_forward = false;

SkDQuadImplicit::SkDQuadImplicit(const SkDQuad& q) {
    double a, b, c;
    SkDQuad::SetABC(&q[0].fX, &a, &b, &c);
    double d, e, f;
    SkDQuad::SetABC(&q[0].fY, &d, &e, &f);
    // compute the implicit coefficients
    if (straight_forward) {  // 42 muls, 13 adds
        fP[kXx_Coeff] = d * d;
        fP[kXy_Coeff] = -2 * a * d;
        fP[kYy_Coeff] = a * a;
        fP[kX_Coeff] = -2*c*d*d + b*e*d - a*e*e + 2*a*f*d;
        fP[kY_Coeff] = -2*f*a*a + e*b*a - d*b*b + 2*d*c*a;
        fP[kC_Coeff] = a*(a*f*f + c*e*e - c*f*d - b*e*f)
                   + d*(b*b*f + c*c*d - c*a*f - c*e*b);
    } else {  // 26 muls, 11 adds
        double aa = a * a;
        double ad = a * d;
        double dd = d * d;
        fP[kXx_Coeff] = dd;
        fP[kXy_Coeff] = -2 * ad;
        fP[kYy_Coeff] = aa;
        double be = b * e;
        double bde = be * d;
        double cdd = c * dd;
        double ee = e * e;
        fP[kX_Coeff] =  -2*cdd + bde - a*ee + 2*ad*f;
        double aaf = aa * f;
        double abe = a * be;
        double ac = a * c;
        double bb_2ac = b*b - 2*ac;
        fP[kY_Coeff] = -2*aaf + abe - d*bb_2ac;
        fP[kC_Coeff] = aaf*f + ac*ee + d*f*bb_2ac - abe*f + c*cdd - c*bde;
    }
}

 /* Given a pair of quadratics, determine their parametric coefficients.
  * If the scaled coefficients are nearly equal, then the part of the quadratics
  * may be coincident.
  * OPTIMIZATION -- since comparison short-circuits on no match,
  * lazily compute the coefficients, comparing the easiest to compute first.
  * xx and yy first; then xy; and so on.
  */
bool SkDQuadImplicit::match(const SkDQuadImplicit& p2) const {
    int first = 0;
    for (int index = 0; index <= kC_Coeff; ++index) {
        if (approximately_zero(fP[index]) && approximately_zero(p2.fP[index])) {
            first += first == index;
            continue;
        }
        if (first == index) {
            continue;
        }
        if (!AlmostDequalUlps(fP[index] * p2.fP[first], fP[first] * p2.fP[index])) {
            return false;
        }
    }
    return true;
}

bool SkDQuadImplicit::Match(const SkDQuad& quad1, const SkDQuad& quad2) {
    SkDQuadImplicit i1(quad1);  // a'xx , b'xy , c'yy , d'x , e'y , f
    SkDQuadImplicit i2(quad2);
    return i1.match(i2);
}
