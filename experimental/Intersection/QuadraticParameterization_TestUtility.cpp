// included by QuadraticParameterization.cpp
// accesses internal functions to validate parameterized coefficients

#include "Parameterization_Test.h"

bool point_on_parameterized_curve(const Quadratic& quad, const _Point& point) {
    QuadImplicitForm q(quad);
    double  xx = q.x2() * point.x * point.x;
    double  xy = q.xy() * point.x * point.y;
    double  yy = q.y2() * point.y * point.y;
    double   x = q.x() * point.x;
    double   y = q.y() * point.y;
    double   c = q.c();
    double sum = xx + xy + yy + x + y + c;
    return approximately_zero(sum);
}
