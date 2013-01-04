/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "DataTypes.h"
#include "Extrema.h"

static int validUnitDivide(double numer, double denom, double* ratio)
{
    if (numer < 0) {
        numer = -numer;
        denom = -denom;
    }
    if (denom == 0 || numer == 0 || numer >= denom)
        return 0;
    double r = numer / denom;
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
static int findUnitQuadRoots(double A, double B, double C, double roots[2])
{
    if (A == 0)
        return validUnitDivide(-C, B, roots);

    double* r = roots;

    double R = B*B - 4*A*C;
    if (R < 0) {  // complex roots
        return 0;
    }
    R = sqrt(R);

    double Q = (B < 0) ? -(B-R)/2 : -(B+R)/2;
    r += validUnitDivide(Q, A, r);
    r += validUnitDivide(C, Q, r);
    if (r - roots == 2 && AlmostEqualUlps(roots[0], roots[1])) { // nearly-equal?
        r -= 1; // skip the double root
    }
    return (int)(r - roots);
}

/** Cubic'(t) = At^2 + Bt + C, where
    A = 3(-a + 3(b - c) + d)
    B = 6(a - 2b + c)
    C = 3(b - a)
    Solve for t, keeping only those that fit between 0 < t < 1
*/
int findExtrema(double a, double b, double c, double d, double tValues[2])
{
    // we divide A,B,C by 3 to simplify
    double A = d - a + 3*(b - c);
    double B = 2*(a - b - b + c);
    double C = b - a;

    return findUnitQuadRoots(A, B, C, tValues);
}

/** Quad'(t) = At + B, where
    A = 2(a - 2b + c)
    B = 2(b - a)
    Solve for t, only if it fits between 0 < t < 1
*/
int findExtrema(double a, double b, double c, double tValue[1])
{
    /*  At + B == 0
        t = -B / A
    */
    return validUnitDivide(a - b, a - b - b + c, tValue);
}
