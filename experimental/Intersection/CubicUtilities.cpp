#include "CubicUtilities.h"
#include "DataTypes.h"
#include "QuadraticUtilities.h"

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

static bool is_unit_interval(double x) {
    return x > 0 && x < 1;
}

// from SkGeometry.cpp (and Numeric Solutions, 5.6)
int cubicRoots(double A, double B, double C, double D, double t[3]) {
    if (approximately_zero(A)) {  // we're just a quadratic
        return quadraticRoots(B, C, D, t);
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
}

// from http://www.cs.sunysb.edu/~qin/courses/geometry/4.pdf
// c(t)  = a(1-t)^3 + 3bt(1-t)^2 + 3c(1-t)t^2 + dt^3
// c'(t) = -3a(1-t)^2 + 3b((1-t)^2 - 2t(1-t)) + 3c(2t(1-t) - t^2) + 3dt^2
//       = 3(b-a)(1-t)^2 + 6(c-b)t(1-t) + 3(d-c)t^2
double derivativeAtT(const double* cubic, double t) {
    double one_t = 1 - t;
    double a = cubic[0];
    double b = cubic[2];
    double c = cubic[4];
    double d = cubic[6];
    return (b - a) * one_t * one_t + 2 * (c - b) * t * one_t + (d - c) * t * t;
}

// same as derivativeAtT
// which is more accurate? which is faster?
double derivativeAtT_2(const double* cubic, double t) {
    double a = cubic[2] - cubic[0];
    double b = cubic[4] - 2 * cubic[2] + cubic[0];
    double c = cubic[6] + 3 * (cubic[2] - cubic[4]) - cubic[0];
    return c * c * t * t + 2 * b * t + a;
}

void dxdy_at_t(const Cubic& cubic, double t, double& dx, double& dy) {
    if (&dx) {
        dx = derivativeAtT(&cubic[0].x, t);
    }
    if (&dy) {
        dy = derivativeAtT(&cubic[0].y, t);
    }
}

bool rotate(const Cubic& cubic, int zero, int index, Cubic& rotPath) {
    double dy = cubic[index].y - cubic[zero].y;
    double dx = cubic[index].x - cubic[zero].x;
    if (approximately_equal(dy, 0)) {
        if (approximately_equal(dx, 0)) {
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

double secondDerivativeAtT(const double* cubic, double t) {
    double a = cubic[0];
    double b = cubic[2];
    double c = cubic[4];
    double d = cubic[6];
    return (c - 2 * b + a) * (1 - t) + (d - 2 * c + b) * t;
}

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
