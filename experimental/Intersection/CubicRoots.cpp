#include "CubicIntersection.h"

//http://planetmath.org/encyclopedia/CubicEquation.html
/* the roots of x^3 + ax^2 + bx + c are
j = -2a^3 + 9ab - 27c
k = sqrt((2a^3 - 9ab + 27c)^2 + 4(-a^2 + 3b)^3)
t1 = -a/3 + cuberoot((j + k) / 54) + cuberoot((j - k) / 54)
t2 = -a/3 - ( 1 + i*cuberoot(3))/2 * cuberoot((j + k) / 54)
          + (-1 + i*cuberoot(3))/2 * cuberoot((j - k) / 54)
t3 = -a/3 + (-1 + i*cuberoot(3))/2 * cuberoot((j + k) / 54)
          - ( 1 + i*cuberoot(3))/2 * cuberoot((j - k) / 54)
*/


static bool is_unit_interval(double x) {
    return x > 0 && x < 1;
}

const double PI = 4 * atan(1);

// from SkGeometry.cpp
int cubic_roots(const double coeff[4], double tValues[3]) {
    if (approximately_zero(coeff[0]))   // we're just a quadratic
    {
        return quadratic_roots(&coeff[1], tValues);
    }
    double inva = 1 / coeff[0];
    double a = coeff[1] * inva;
    double b = coeff[2] * inva;
    double c = coeff[3] * inva;
    double a2 = a * a;
    double Q = (a2 - b * 3) / 9;
    double R = (2 * a2 * a - 9 * a * b + 27 * c) / 54;
    double Q3 = Q * Q * Q;
    double R2MinusQ3 = R * R - Q3;
    double adiv3 = a / 3;
    double* roots = tValues;
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
    return (int)(roots - tValues);
}
