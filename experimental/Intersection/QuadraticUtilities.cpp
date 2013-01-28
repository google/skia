/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "QuadraticUtilities.h"
#include "SkTypes.h"
#include <math.h>

/*

Numeric Solutions (5.6) suggests to solve the quadratic by computing

       Q = -1/2(B + sgn(B)Sqrt(B^2 - 4 A C))

and using the roots

      t1 = Q / A
      t2 = C / Q

*/


int add_valid_ts(double s[], int realRoots, double* t) {
    int foundRoots = 0;
    for (int index = 0; index < realRoots; ++index) {
        double tValue = s[index];
        if (approximately_zero_or_more(tValue) && approximately_one_or_less(tValue)) {
            if (approximately_less_than_zero(tValue)) {
                tValue = 0;
            } else if (approximately_greater_than_one(tValue)) {
                tValue = 1;
            }
            for (int idx2 = 0; idx2 < foundRoots; ++idx2) {
                if (approximately_equal(t[idx2], tValue)) {
                    goto nextRoot;
                }
            }
            t[foundRoots++] = tValue;
        }
nextRoot:
        ;
    }
    return foundRoots;
}

// note: caller expects multiple results to be sorted smaller first
// note: http://en.wikipedia.org/wiki/Loss_of_significance has an interesting
//  analysis of the quadratic equation, suggesting why the following looks at
//  the sign of B -- and further suggesting that the greatest loss of precision
//  is in b squared less two a c
int quadraticRootsValidT(double A, double B, double C, double t[2]) {
#if 0
    B *= 2;
    double square = B * B - 4 * A * C;
    if (approximately_negative(square)) {
        if (!approximately_positive(square)) {
            return 0;
        }
        square = 0;
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
#else
    double s[2];
    int realRoots = quadraticRootsReal(A, B, C, s);
    int foundRoots = add_valid_ts(s, realRoots, t);
#endif
    return foundRoots;
}

// unlike quadratic roots, this does not discard real roots <= 0 or >= 1
int quadraticRootsReal(const double A, const double B, const double C, double s[2]) {
    const double p = B / (2 * A);
    const double q = C / A;
    if (approximately_zero(A) && (approximately_zero_inverse(p) || approximately_zero_inverse(q))) {
        if (approximately_zero(B)) {
            s[0] = 0;
            return C == 0;
        }
        s[0] = -C / B;
        return 1;
    }
    /* normal form: x^2 + px + q = 0 */
    const double p2 = p * p;
#if 0
    double D = AlmostEqualUlps(p2, q) ? 0 : p2 - q;
    if (D <= 0) {
        if (D < 0) {
            return 0;
        }
        s[0] = -p;
        SkDebugf("[%d] %1.9g\n", 1, s[0]);
        return 1;
    }
    double sqrt_D = sqrt(D);
    s[0] = sqrt_D - p;
    s[1] = -sqrt_D - p;
    SkDebugf("[%d] %1.9g %1.9g\n", 2, s[0], s[1]);
    return 2;
#else
    if (!AlmostEqualUlps(p2, q) && p2 < q) {
        return 0;
    }
    double sqrt_D = 0;
    if (p2 > q) {
        sqrt_D = sqrt(p2 - q);
    }
    s[0] = sqrt_D - p;
    s[1] = -sqrt_D - p;
#if 0
    if (AlmostEqualUlps(s[0], s[1])) {
        SkDebugf("[%d] %1.9g\n", 1, s[0]);
    } else {
        SkDebugf("[%d] %1.9g %1.9g\n", 2, s[0], s[1]);
    }
#endif
    return 1 + !AlmostEqualUlps(s[0], s[1]);
#endif
}

static double derivativeAtT(const double* quad, double t) {
    double a = t - 1;
    double b = 1 - 2 * t;
    double c = t;
    return a * quad[0] + b * quad[2] + c * quad[4];
}

double dx_at_t(const Quadratic& quad, double t) {
    return derivativeAtT(&quad[0].x, t);
}

double dy_at_t(const Quadratic& quad, double t) {
    return derivativeAtT(&quad[0].y, t);
}

void dxdy_at_t(const Quadratic& quad, double t, _Point& dxy) {
    double a = t - 1;
    double b = 1 - 2 * t;
    double c = t;
    dxy.x = a * quad[0].x + b * quad[1].x + c * quad[2].x;
    dxy.y = a * quad[0].y + b * quad[1].y + c * quad[2].y;
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
