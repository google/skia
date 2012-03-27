// included by QuadraticParameterization.cpp
// accesses internal functions to validate parameterized coefficients

#include "Parameterization_Test.h"

bool point_on_parameterized_curve(const Quadratic& quad, const _Point& point) {
    double coeffs[coeff_count];
    implicit_coefficients(quad, coeffs);
    double  xx = coeffs[ xx_coeff] * point.x * point.x;
    double  xy = coeffs[ xy_coeff] * point.x * point.y;
    double  yy = coeffs[ yy_coeff] * point.y * point.y;
    double   x = coeffs[  x_coeff] * point.x;
    double   y = coeffs[  y_coeff] * point.y;
    double   c = coeffs[  c_coeff];
    double sum = xx + xy + yy + x + y + c;
    return approximately_zero(sum);
}
