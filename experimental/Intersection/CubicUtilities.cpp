/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CubicUtilities.h"
#include "Extrema.h"
#include "LineUtilities.h"
#include "QuadraticUtilities.h"

const int gPrecisionUnit = 256; // FIXME: arbitrary -- should try different values in test framework

// FIXME: cache keep the bounds and/or precision with the caller?
double calcPrecision(const Cubic& cubic) {
    _Rect dRect;
    dRect.setBounds(cubic); // OPTIMIZATION: just use setRawBounds ?
    double width = dRect.right - dRect.left;
    double height = dRect.bottom - dRect.top;
    return (width > height ? width : height) / gPrecisionUnit;
}

#if SK_DEBUG
double calcPrecision(const Cubic& cubic, double t, double scale) {
    Cubic part;
    sub_divide(cubic, SkTMax(0., t - scale), SkTMin(1., t + scale), part);
    return calcPrecision(part);
}
#endif

bool clockwise(const Cubic& c) {
    double sum = (c[0].x - c[3].x) * (c[0].y + c[3].y);
    for (int idx = 0; idx < 3; ++idx){
        sum += (c[idx + 1].x - c[idx].x) * (c[idx + 1].y + c[idx].y);
    }
    return sum <= 0;
}

void coefficients(const double* cubic, double& A, double& B, double& C, double& D) {
    A = cubic[6]; // d
    B = cubic[4] * 3; // 3*c
    C = cubic[2] * 3; // 3*b
    D = cubic[0]; // a
    A -= D - C + B;     // A =   -a + 3*b - 3*c + d
    B += 3 * D - 2 * C; // B =  3*a - 6*b + 3*c
    C -= 3 * D;         // C = -3*a + 3*b
}

bool controls_contained_by_ends(const Cubic& c) {
    _Vector startTan = c[1] - c[0];
    if (startTan.x == 0 && startTan.y == 0) {
        startTan = c[2] - c[0];
    }
    _Vector endTan = c[2] - c[3];
    if (endTan.x == 0 && endTan.y == 0) {
        endTan = c[1] - c[3];
    }
    if (startTan.dot(endTan) >= 0) {
        return false;
    }
    _Line startEdge = {c[0], c[0]};
    startEdge[1].x -= startTan.y;
    startEdge[1].y += startTan.x;
    _Line endEdge = {c[3], c[3]};
    endEdge[1].x -= endTan.y;
    endEdge[1].y += endTan.x;
    double leftStart1 = is_left(startEdge, c[1]);
    if (leftStart1 * is_left(startEdge, c[2]) < 0) {
        return false;
    }
    double leftEnd1 = is_left(endEdge, c[1]);
    if (leftEnd1 * is_left(endEdge, c[2]) < 0) {
        return false;
    }
    return leftStart1 * leftEnd1 >= 0;
}

bool ends_are_extrema_in_x_or_y(const Cubic& c) {
    return (between(c[0].x, c[1].x, c[3].x) && between(c[0].x, c[2].x, c[3].x))
            || (between(c[0].y, c[1].y, c[3].y) && between(c[0].y, c[2].y, c[3].y));
}

bool monotonic_in_y(const Cubic& c) {
    return between(c[0].y, c[1].y, c[3].y) && between(c[0].y, c[2].y, c[3].y);
}

bool serpentine(const Cubic& c) {
    if (!controls_contained_by_ends(c)) {
        return false;
    }
    double wiggle = (c[0].x - c[2].x) * (c[0].y + c[2].y);
    for (int idx = 0; idx < 2; ++idx){
        wiggle += (c[idx + 1].x - c[idx].x) * (c[idx + 1].y + c[idx].y);
    }
    double waggle = (c[1].x - c[3].x) * (c[1].y + c[3].y);
    for (int idx = 1; idx < 3; ++idx){
        waggle += (c[idx + 1].x - c[idx].x) * (c[idx + 1].y + c[idx].y);
    }
    return wiggle * waggle < 0;
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
    mathematica_ize(str, sizeof(str));
#if ONE_OFF_DEBUG && ONE_OFF_DEBUG_MATHEMATICA
    SkDebugf("%s\n", str);
#endif
#endif
    if (approximately_zero(A)
            && approximately_zero_when_compared_to(A, B)
            && approximately_zero_when_compared_to(A, C)
            && approximately_zero_when_compared_to(A, D)) {  // we're just a quadratic
        return quadraticRootsReal(B, C, D, s);
    }
    if (approximately_zero_when_compared_to(D, A)
            && approximately_zero_when_compared_to(D, B)
            && approximately_zero_when_compared_to(D, C)) { // 0 is one root
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
_Vector dxdy_at_t(const Cubic& cubic, double t) {
    _Vector result = { derivativeAtT(&cubic[0].x, t), derivativeAtT(&cubic[0].y, t) };
    return result;
}

// OPTIMIZE? share code with formulate_F1DotF2
int find_cubic_inflections(const Cubic& src, double tValues[])
{
    double Ax = src[1].x - src[0].x;
    double Ay = src[1].y - src[0].y;
    double Bx = src[2].x - 2 * src[1].x + src[0].x;
    double By = src[2].y - 2 * src[1].y + src[0].y;
    double Cx = src[3].x + 3 * (src[1].x - src[2].x) - src[0].x;
    double Cy = src[3].y + 3 * (src[1].y - src[2].y) - src[0].y;
    return quadraticRootsValidT(Bx * Cy - By * Cx, Ax * Cy - Ay * Cx, Ax * By - Ay * Bx, tValues);
}

static void formulate_F1DotF2(const double src[], double coeff[4])
{
    double a = src[2] - src[0];
    double b = src[4] - 2 * src[2] + src[0];
    double c = src[6] + 3 * (src[2] - src[4]) - src[0];
    coeff[0] = c * c;
    coeff[1] = 3 * b * c;
    coeff[2] = 2 * b * b + c * a;
    coeff[3] = a * b;
}

/*  from SkGeometry.cpp
    Looking for F' dot F'' == 0

    A = b - a
    B = c - 2b + a
    C = d - 3c + 3b - a

    F' = 3Ct^2 + 6Bt + 3A
    F'' = 6Ct + 6B

    F' dot F'' -> CCt^3 + 3BCt^2 + (2BB + CA)t + AB
*/
int find_cubic_max_curvature(const Cubic& src, double tValues[])
{
    double coeffX[4], coeffY[4];
    int i;
    formulate_F1DotF2(&src[0].x, coeffX);
    formulate_F1DotF2(&src[0].y, coeffY);
    for (i = 0; i < 4; i++) {
        coeffX[i] = coeffX[i] + coeffY[i];
    }
    return cubicRootsValidT(coeffX[0], coeffX[1], coeffX[2], coeffX[3], tValues);
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

_Point top(const Cubic& cubic, double startT, double endT) {
    Cubic sub;
    sub_divide(cubic, startT, endT, sub);
    _Point topPt = sub[0];
    if (topPt.y > sub[3].y || (topPt.y == sub[3].y && topPt.x > sub[3].x)) {
        topPt = sub[3];
    }
    double extremeTs[2];
    if (!monotonic_in_y(sub)) {
        int roots = findExtrema(sub[0].y, sub[1].y, sub[2].y, sub[3].y, extremeTs);
        for (int index = 0; index < roots; ++index) {
            _Point mid;
            double t = startT + (endT - startT) * extremeTs[index];
            xy_at_t(cubic, t, mid.x, mid.y);
            if (topPt.y > mid.y || (topPt.y == mid.y && topPt.x > mid.x)) {
                topPt = mid;
            }
        }
    }
    return topPt;
}

// OPTIMIZE: avoid computing the unused half
void xy_at_t(const Cubic& cubic, double t, double& x, double& y) {
    _Point xy = xy_at_t(cubic, t);
    if (&x) {
        x = xy.x;
    }
    if (&y) {
        y = xy.y;
    }
}

_Point xy_at_t(const Cubic& cubic, double t) {
    double one_t = 1 - t;
    double one_t2 = one_t * one_t;
    double a = one_t2 * one_t;
    double b = 3 * one_t2 * t;
    double t2 = t * t;
    double c = 3 * one_t * t2;
    double d = t2 * t;
    _Point result = {a * cubic[0].x + b * cubic[1].x + c * cubic[2].x + d * cubic[3].x,
            a * cubic[0].y + b * cubic[1].y + c * cubic[2].y + d * cubic[3].y};
    return result;
}
