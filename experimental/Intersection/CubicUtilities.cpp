/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CubicUtilities.h"
#include "QuadraticUtilities.h"

const int precisionUnit = 256; // FIXME: arbitrary -- should try different values in test framework

// FIXME: cache keep the bounds and/or precision with the caller?
double calcPrecision(const Cubic& cubic) {
    _Rect dRect;
    dRect.setBounds(cubic); // OPTIMIZATION: just use setRawBounds ?
    double width = dRect.right - dRect.left;
    double height = dRect.bottom - dRect.top;
    return (width > height ? width : height) / precisionUnit;
}

#if SK_DEBUG
double calcPrecision(const Cubic& cubic, double t, double scale) {
    Cubic part;
    sub_divide(cubic, SkTMax(0., t - scale), SkTMin(1., t + scale), part);
    return calcPrecision(part);
}
#endif


void coefficients(const double* cubic, double& A, double& B, double& C, double& D) {
    A = cubic[6]; // d
    B = cubic[4] * 3; // 3*c
    C = cubic[2] * 3; // 3*b
    D = cubic[0]; // a
    A -= D - C + B;     // A =   -a + 3*b - 3*c + d
    B += 3 * D - 2 * C; // B =  3*a - 6*b + 3*c
    C -= 3 * D;         // C = -3*a + 3*b
}

// cubic roots

const double PI = 4 * atan(1);

// from SkGeometry.cpp (and Numeric Solutions, 5.6)
int cubicRootsValidT(double A, double B, double C, double D, double t[3]) {
#if 0
    if (approximately_zero(A)) {  // we're just a quadratic
        return quadraticRootsValidT(B, C, D, t);
    }
    double a, b, c;
    {
        double invA = 1 / A;
        a = B * invA;
        b = C * invA;
        c = D * invA;
    }
    double a2 = a * a;
    double Q = (a2 - b * 3) / 9;
    double R = (2 * a2 * a - 9 * a * b + 27 * c) / 54;
    double Q3 = Q * Q * Q;
    double R2MinusQ3 = R * R - Q3;
    double adiv3 = a / 3;
    double* roots = t;
    double r;

    if (R2MinusQ3 < 0)   // we have 3 real roots
    {
        double theta = acos(R / sqrt(Q3));
        double neg2RootQ = -2 * sqrt(Q);

        r = neg2RootQ * cos(theta / 3) - adiv3;
        if (is_unit_interval(r))
            *roots++ = r;

        r = neg2RootQ * cos((theta + 2 * PI) / 3) - adiv3;
        if (is_unit_interval(r))
            *roots++ = r;

        r = neg2RootQ * cos((theta - 2 * PI) / 3) - adiv3;
        if (is_unit_interval(r))
            *roots++ = r;
    }
    else                // we have 1 real root
    {
        double A = fabs(R) + sqrt(R2MinusQ3);
        A = cube_root(A);
        if (R > 0) {
            A = -A;
        }
        if (A != 0) {
            A += Q / A;
        }
        r = A - adiv3;
        if (is_unit_interval(r))
            *roots++ = r;
    }
    return (int)(roots - t);
#else
    double s[3];
    int realRoots = cubicRootsReal(A, B, C, D, s);
    int foundRoots = add_valid_ts(s, realRoots, t);
    return foundRoots;
#endif
}

int cubicRootsReal(double A, double B, double C, double D, double s[3]) {
#if SK_DEBUG
    // create a string mathematica understands
    // GDB set print repe 15 # if repeated digits is a bother
    //     set print elements 400 # if line doesn't fit
    char str[1024];
    bzero(str, sizeof(str));
    sprintf(str, "Solve[%1.19g x^3 + %1.19g x^2 + %1.19g x + %1.19g == 0, x]", A, B, C, D);
#endif
    if (approximately_zero(A)) {  // we're just a quadratic
        return quadraticRootsReal(B, C, D, s);
    }
    if (approximately_zero(D)) { // 0 is one root
        int num = quadraticRootsReal(A, B, C, s);
        for (int i = 0; i < num; ++i) {
            if (approximately_zero(s[i])) {
                return num;
            }
        }
        s[num++] = 0;
        return num;
    }
    if (approximately_zero(A + B + C + D)) { // 1 is one root
        int num = quadraticRootsReal(A, A + B, -D, s);
        for (int i = 0; i < num; ++i) {
            if (AlmostEqualUlps(s[i], 1)) {
                return num;
            }
        }
        s[num++] = 1;
        return num;
    }
    double a, b, c;
    {
        double invA = 1 / A;
        a = B * invA;
        b = C * invA;
        c = D * invA;
    }
    double a2 = a * a;
    double Q = (a2 - b * 3) / 9;
    double R = (2 * a2 * a - 9 * a * b + 27 * c) / 54;
    double R2 = R * R;
    double Q3 = Q * Q * Q;
    double R2MinusQ3 = R2 - Q3;
    double adiv3 = a / 3;
    double r;
    double* roots = s;
#if 0
    if (approximately_zero_squared(R2MinusQ3) && AlmostEqualUlps(R2, Q3)) {
        if (approximately_zero_squared(R)) {/* one triple solution */
            *roots++ = -adiv3;
        } else { /* one single and one double solution */

            double u = cube_root(-R);
            *roots++ = 2 * u - adiv3;
            *roots++ = -u - adiv3;
        }
    }
    else
#endif
    if (R2MinusQ3 < 0)   // we have 3 real roots
    {
        double theta = acos(R / sqrt(Q3));
        double neg2RootQ = -2 * sqrt(Q);

        r = neg2RootQ * cos(theta / 3) - adiv3;
        *roots++ = r;

        r = neg2RootQ * cos((theta + 2 * PI) / 3) - adiv3;
        if (!AlmostEqualUlps(s[0], r)) {
            *roots++ = r;
        }
        r = neg2RootQ * cos((theta - 2 * PI) / 3) - adiv3;
        if (!AlmostEqualUlps(s[0], r) && (roots - s == 1 || !AlmostEqualUlps(s[1], r))) {
            *roots++ = r;
        }
    }
    else                // we have 1 real root
    {
        double sqrtR2MinusQ3 = sqrt(R2MinusQ3);
        double A = fabs(R) + sqrtR2MinusQ3;
        A = cube_root(A);
        if (R > 0) {
            A = -A;
        }
        if (A != 0) {
            A += Q / A;
        }
        r = A - adiv3;
        *roots++ = r;
        if (AlmostEqualUlps(R2, Q3)) {
            r = -A / 2 - adiv3;
            if (!AlmostEqualUlps(s[0], r)) {
                *roots++ = r;
            }
        }
    }
    return (int)(roots - s);
}

// from http://www.cs.sunysb.edu/~qin/courses/geometry/4.pdf
// c(t)  = a(1-t)^3 + 3bt(1-t)^2 + 3c(1-t)t^2 + dt^3
// c'(t) = -3a(1-t)^2 + 3b((1-t)^2 - 2t(1-t)) + 3c(2t(1-t) - t^2) + 3dt^2
//       = 3(b-a)(1-t)^2 + 6(c-b)t(1-t) + 3(d-c)t^2
static double derivativeAtT(const double* cubic, double t) {
    double one_t = 1 - t;
    double a = cubic[0];
    double b = cubic[2];
    double c = cubic[4];
    double d = cubic[6];
    return 3 * ((b - a) * one_t * one_t + 2 * (c - b) * t * one_t + (d - c) * t * t);
}

double dx_at_t(const Cubic& cubic, double t) {
    return derivativeAtT(&cubic[0].x, t);
}

double dy_at_t(const Cubic& cubic, double t) {
    return derivativeAtT(&cubic[0].y, t);
}

// OPTIMIZE? compute t^2, t(1-t), and (1-t)^2 and pass them to another version of derivative at t?
void dxdy_at_t(const Cubic& cubic, double t, _Point& dxdy) {
    dxdy.x = derivativeAtT(&cubic[0].x, t);
    dxdy.y = derivativeAtT(&cubic[0].y, t);
}


int find_cubic_inflections(const Cubic& src, double tValues[])
{
    double Ax = src[1].x - src[0].x;
    double Ay = src[1].y - src[0].y;
    double Bx = src[2].x - 2 * src[1].x + src[0].x;
    double By = src[2].y - 2 * src[1].y + src[0].y;
    double Cx = src[3].x + 3 * (src[1].x - src[2].x) - src[0].x;
    double Cy = src[3].y + 3 * (src[1].y - src[2].y) - src[0].y;
    return quadraticRootsValidT(Bx * Cy - By * Cx, (Ax * Cy - Ay * Cx), Ax * By - Ay * Bx, tValues);
}

bool rotate(const Cubic& cubic, int zero, int index, Cubic& rotPath) {
    double dy = cubic[index].y - cubic[zero].y;
    double dx = cubic[index].x - cubic[zero].x;
    if (approximately_zero(dy)) {
        if (approximately_zero(dx)) {
            return false;
        }
        memcpy(rotPath, cubic, sizeof(Cubic));
        return true;
    }
    for (int index = 0; index < 4; ++index) {
        rotPath[index].x = cubic[index].x * dx + cubic[index].y * dy;
        rotPath[index].y = cubic[index].y * dx - cubic[index].x * dy;
    }
    return true;
}

#if 0 // unused for now
double secondDerivativeAtT(const double* cubic, double t) {
    double a = cubic[0];
    double b = cubic[2];
    double c = cubic[4];
    double d = cubic[6];
    return (c - 2 * b + a) * (1 - t) + (d - 2 * c + b) * t;
}
#endif

void xy_at_t(const Cubic& cubic, double t, double& x, double& y) {
    double one_t = 1 - t;
    double one_t2 = one_t * one_t;
    double a = one_t2 * one_t;
    double b = 3 * one_t2 * t;
    double t2 = t * t;
    double c = 3 * one_t * t2;
    double d = t2 * t;
    if (&x) {
        x = a * cubic[0].x + b * cubic[1].x + c * cubic[2].x + d * cubic[3].x;
    }
    if (&y) {
        y = a * cubic[0].y + b * cubic[1].y + c * cubic[2].y + d * cubic[3].y;
    }
}
