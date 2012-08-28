/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "QuadraticUtilities.h"
#include <math.h>

/*

Numeric Solutions (5.6) suggests to solve the quadratic by computing

       Q = -1/2(B + sgn(B)Sqrt(B^2 - 4 A C))

and using the roots

      t1 = Q / A
      t2 = C / Q

*/

// note: caller expects multiple results to be sorted smaller first
// note: http://en.wikipedia.org/wiki/Loss_of_significance has an interesting
//  analysis of the quadratic equation, suggesting why the following looks at
//  the sign of B -- and further suggesting that the greatest loss of precision
//  is in b squared less two a c
int quadraticRoots(double A, double B, double C, double t[2]) {
    B *= 2;
    double square = B * B - 4 * A * C;
    if (square < 0) {
        return 0;
    }
    double squareRt = sqrt(square);
    double Q = (B + (B < 0 ? -squareRt : squareRt)) / -2;
    int foundRoots = 0;
    double ratio = Q / A;
    if (approximately_zero_or_more(ratio) && approximately_one_or_less(ratio)) {
        if (approximately_less_than_zero(ratio)) {
            ratio = 0;
        } else if (approximately_greater_than_one(ratio)) {
            ratio = 1;
        }
        t[0] = ratio;
        ++foundRoots;
    }
    ratio = C / Q;
    if (approximately_zero_or_more(ratio) && approximately_one_or_less(ratio)) {
        if (approximately_less_than_zero(ratio)) {
            ratio = 0;
        } else if (approximately_greater_than_one(ratio)) {
            ratio = 1;
        }
        if (foundRoots == 0 || !approximately_negative(ratio - t[0])) {
            t[foundRoots++] = ratio;
        } else if (!approximately_negative(t[0] - ratio)) {
            t[foundRoots++] = t[0];
            t[0] = ratio;
        }
    }
    return foundRoots;
}

void dxdy_at_t(const Quadratic& quad, double t, double& x, double& y) {
    double a = t - 1;
    double b = 1 - 2 * t;
    double c = t;
    if (&x) {
        x = a * quad[0].x + b * quad[1].x + c * quad[2].x;
    }
    if (&y) {
        y = a * quad[0].y + b * quad[1].y + c * quad[2].y;
    }
}

void xy_at_t(const Quadratic& quad, double t, double& x, double& y) {
    double one_t = 1 - t;
    double a = one_t * one_t;
    double b = 2 * one_t * t;
    double c = t * t;
    if (&x) {
        x = a * quad[0].x + b * quad[1].x + c * quad[2].x;
    }
    if (&y) {
        y = a * quad[0].y + b * quad[1].y + c * quad[2].y;
    }
}
