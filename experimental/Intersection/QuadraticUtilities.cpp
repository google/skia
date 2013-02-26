/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CubicUtilities.h"
#include "Extrema.h"
#include "QuadraticUtilities.h"
#include "TriangleUtilities.h"

// from http://blog.gludion.com/2009/08/distance-to-quadratic-bezier-curve.html
double nearestT(const Quadratic& quad, const _Point& pt) {
    _Vector pos = quad[0] - pt;
    // search points P of bezier curve with PM.(dP / dt) = 0
    // a calculus leads to a 3d degree equation :
    _Vector A = quad[1] - quad[0];
    _Vector B = quad[2] - quad[1];
    B -= A;
    double a = B.dot(B);
    double b = 3 * A.dot(B);
    double c = 2 * A.dot(A) + pos.dot(B);
    double d = pos.dot(A);
    double ts[3];
    int roots = cubicRootsValidT(a, b, c, d, ts);
    double d0 = pt.distanceSquared(quad[0]);
    double d2 = pt.distanceSquared(quad[2]);
    double distMin = SkTMin(d0, d2);
    int bestIndex = -1;
    for (int index = 0; index < roots; ++index) {
        _Point onQuad;
        xy_at_t(quad, ts[index], onQuad.x, onQuad.y);
        double dist = pt.distanceSquared(onQuad);
        if (distMin > dist) {
            distMin = dist;
            bestIndex = index;
        }
    }
    if (bestIndex >= 0) {
        return ts[bestIndex];
    }
    return d0 < d2 ? 0 : 1;
}

bool point_in_hull(const Quadratic& quad, const _Point& pt) {
    return pointInTriangle((const Triangle&) quad, pt);
}

_Point top(const Quadratic& quad, double startT, double endT) {
    Quadratic sub;
    sub_divide(quad, startT, endT, sub);
    _Point topPt = sub[0];
    if (topPt.y > sub[2].y || (topPt.y == sub[2].y && topPt.x > sub[2].x)) {
        topPt = sub[2];
    }
    if (!between(sub[0].y, sub[1].y, sub[2].y)) {
        double extremeT;
        if (findExtrema(sub[0].y, sub[1].y, sub[2].y, &extremeT)) {
            extremeT = startT + (endT - startT) * extremeT;
            _Point test;
            xy_at_t(quad, extremeT, test.x, test.y);
            if (topPt.y > test.y || (topPt.y == test.y && topPt.x > test.x)) {
                topPt = test;
            }
        }
    }
    return topPt;
}

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

void toCubic(const Quadratic& quad, Cubic& cubic) {
    cubic[0] = quad[0];
    cubic[2] = quad[1];
    cubic[3] = quad[2];
    cubic[1].x = (cubic[0].x + cubic[2].x * 2) / 3;
    cubic[1].y = (cubic[0].y + cubic[2].y * 2) / 3;
    cubic[2].x = (cubic[3].x + cubic[2].x * 2) / 3;
    cubic[2].y = (cubic[3].y + cubic[2].y * 2) / 3;
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

_Vector dxdy_at_t(const Quadratic& quad, double t) {
    double a = t - 1;
    double b = 1 - 2 * t;
    double c = t;
    _Vector result = { a * quad[0].x + b * quad[1].x + c * quad[2].x,
            a * quad[0].y + b * quad[1].y + c * quad[2].y };
    return result;
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

_Point xy_at_t(const Quadratic& quad, double t) {
    double one_t = 1 - t;
    double a = one_t * one_t;
    double b = 2 * one_t * t;
    double c = t * t;
    _Point result = { a * quad[0].x + b * quad[1].x + c * quad[2].x,
            a * quad[0].y + b * quad[1].y + c * quad[2].y };
    return result;
}
