// included by CubicParameterization.cpp
// accesses internal functions to validate parameterized coefficients

#include "Parameterization_Test.h"

static void parameter_coeffs(const Cubic& cubic, double coeffs[coeff_count]) {
#if USE_SYVESTER
    double ax, bx, cx, dx;
    if (try_alt)
        alt_set_abcd(&cubic[0].x, ax, bx, cx, dx);
    else
        set_abcd(&cubic[0].x, ax, bx, cx, dx);
    double ay, by, cy, dy;
    if (try_alt)
        alt_set_abcd(&cubic[0].y, ay, by, cy, dy);
    else
        set_abcd(&cubic[0].y, ay, by, cy, dy);
    calc_ABCD(ax, ay, coeffs);
    if (!try_alt) calc_bc(dx, bx, cx);
    if (!try_alt) calc_bc(dy, by, cy);
#else
    double ax = cubic[0].x;
    double bx = cubic[1].x;
    double cx = cubic[2].x;
    double dx = cubic[3].x;
    double ay = cubic[0].y;
    double by = cubic[1].y;
    double cy = cubic[2].y;
    double dy = cubic[3].y;
    calc_ABCD(ax, bx, cx, dx, ay, by, cy, dy, coeffs);
#endif
    for (int index = xx_coeff; index < coeff_count; ++index) {
        int procIndex = index - xx_coeff;
        coeffs[index] = (*calc_proc[procIndex])(ax, bx, cx, dx, ay, by, cy, dy);
    }
}

bool point_on_parameterized_curve(const Cubic& cubic, const _Point& point) {
    double coeffs[coeff_count];
    parameter_coeffs(cubic, coeffs);
    double xxx = coeffs[xxx_coeff] * point.x * point.x * point.x;
    double xxy = coeffs[xxy_coeff] * point.x * point.x * point.y;
    double xyy = coeffs[xyy_coeff] * point.x * point.y * point.y;
    double yyy = coeffs[yyy_coeff] * point.y * point.y * point.y;
    double  xx = coeffs[ xx_coeff] * point.x * point.x;
    double  xy = coeffs[ xy_coeff] * point.x * point.y;
    double  yy = coeffs[ yy_coeff] * point.y * point.y;
    double   x = coeffs[  x_coeff] * point.x;
    double   y = coeffs[  y_coeff] * point.y;
    double   c = coeffs[  c_coeff];
    double sum = xxx + xxy + xyy + yyy + xx + xy + yy + x + y + c;
    return approximately_zero(sum);
}
