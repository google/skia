/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "CubicUtilities.h"

/* from http://tom.cs.byu.edu/~tom/papers/cvgip84.pdf 4.1
 *
 * This paper proves that Syvester's method can compute the implicit form of
 * the quadratic from the parameterzied form.
 *
 * Given x = a*t*t*t + b*t*t + c*t + d  (the parameterized form)
 *       y = e*t*t*t + f*t*t + g*t + h
 *
 * we want to find an equation of the implicit form:
 *
 * A*x^3 + B*x*x*y + C*x*y*y + D*y^3 + E*x*x + F*x*y + G*y*y + H*x + I*y + J = 0
 *
 * The implicit form can be expressed as a 6x6 determinant, as shown.
 *
 * The resultant obtained by Syvester's method is
 *
 * |   a   b   c  (d - x)     0        0     |
 * |   0   a   b     c     (d - x)     0     |
 * |   0   0   a     b        c     (d - x)  |
 * |   e   f   g  (h - y)     0        0     |
 * |   0   e   f     g     (h - y)     0     |
 * |   0   0   e     f        g     (h - y)  |
 *
 * which, according to Mathematica, expands as shown below.
 *
 * Resultant[a*t^3 + b*t^2 + c*t + d - x, e*t^3 + f*t^2 + g*t + h - y, t]
 *
 *  -d^3 e^3 + c d^2 e^2 f - b d^2 e f^2 + a d^2 f^3 - c^2 d e^2 g +
 *  2 b d^2 e^2 g + b c d e f g - 3 a d^2 e f g - a c d f^2 g -
 *  b^2 d e g^2 + 2 a c d e g^2 + a b d f g^2 - a^2 d g^3 + c^3 e^2 h -
 *  3 b c d e^2 h + 3 a d^2 e^2 h - b c^2 e f h + 2 b^2 d e f h +
 *  a c d e f h + a c^2 f^2 h - 2 a b d f^2 h + b^2 c e g h -
 *  2 a c^2 e g h - a b d e g h - a b c f g h + 3 a^2 d f g h +
 *  a^2 c g^2 h - b^3 e h^2 + 3 a b c e h^2 - 3 a^2 d e h^2 +
 *  a b^2 f h^2 - 2 a^2 c f h^2 - a^2 b g h^2 + a^3 h^3 + 3 d^2 e^3 x -
 *  2 c d e^2 f x + 2 b d e f^2 x - 2 a d f^3 x + c^2 e^2 g x -
 *  4 b d e^2 g x - b c e f g x + 6 a d e f g x + a c f^2 g x +
 *  b^2 e g^2 x - 2 a c e g^2 x - a b f g^2 x + a^2 g^3 x +
 *  3 b c e^2 h x - 6 a d e^2 h x - 2 b^2 e f h x - a c e f h x +
 *  2 a b f^2 h x + a b e g h x - 3 a^2 f g h x + 3 a^2 e h^2 x -
 *  3 d e^3 x^2 + c e^2 f x^2 - b e f^2 x^2 + a f^3 x^2 +
 *  2 b e^2 g x^2 - 3 a e f g x^2 + 3 a e^2 h x^2 + e^3 x^3 -
 *  c^3 e^2 y + 3 b c d e^2 y - 3 a d^2 e^2 y + b c^2 e f y -
 *  2 b^2 d e f y - a c d e f y - a c^2 f^2 y + 2 a b d f^2 y -
 *  b^2 c e g y + 2 a c^2 e g y + a b d e g y + a b c f g y -
 *  3 a^2 d f g y - a^2 c g^2 y + 2 b^3 e h y - 6 a b c e h y +
 *  6 a^2 d e h y - 2 a b^2 f h y + 4 a^2 c f h y + 2 a^2 b g h y -
 *  3 a^3 h^2 y - 3 b c e^2 x y + 6 a d e^2 x y + 2 b^2 e f x y +
 *  a c e f x y - 2 a b f^2 x y - a b e g x y + 3 a^2 f g x y -
 *  6 a^2 e h x y - 3 a e^2 x^2 y - b^3 e y^2 + 3 a b c e y^2 -
 *  3 a^2 d e y^2 + a b^2 f y^2 - 2 a^2 c f y^2 - a^2 b g y^2 +
 *  3 a^3 h y^2 + 3 a^2 e x y^2 - a^3 y^3
 */

enum {
    xxx_coeff, // A
    xxy_coeff, // B
    xyy_coeff, // C
    yyy_coeff, // D
    xx_coeff,
    xy_coeff,
    yy_coeff,
    x_coeff,
    y_coeff,
    c_coeff,
    coeff_count
};

#define USE_SYVESTER 0 // if 0, use control-point base parametric form
#if USE_SYVESTER

// FIXME: factoring version unwritten
// static bool straight_forward = true;

/* from CubicParameterizationCode.cpp output:
 *  double A =      e * e * e;
 *  double B = -3 * a * e * e;
 *  double C =  3 * a * a * e;
 *  double D =     -a * a * a;
 */
static void calc_ABCD(double a, double e, double p[coeff_count]) {
    double ee = e * e;
    p[xxx_coeff] = e * ee;
    p[xxy_coeff] = -3 * a * ee;
    double aa = a * a;
    p[xyy_coeff] = 3 * aa * e;
    p[yyy_coeff] = -aa * a;
}

/* CubicParameterizationCode.cpp turns Mathematica output into C.
 * Rather than edit the lines below, please edit the code there instead.
 */
// start of generated code
static double calc_xx(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
         -3 * d * e * e * e
        +     c * e * e * f
        -     b * e * f * f
        +     a * f * f * f
        + 2 * b * e * e * g
        - 3 * a * e * f * g
        + 3 * a * e * e * h;
}

static double calc_xy(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
         -3 * b * c * e * e
        + 6 * a * d * e * e
        + 2 * b * b * e * f
        +     a * c * e * f
        - 2 * a * b * f * f
        -     a * b * e * g
        + 3 * a * a * f * g
        - 6 * a * a * e * h;
}

static double calc_yy(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
             -b * b * b * e
        + 3 * a * b * c * e
        - 3 * a * a * d * e
        +     a * b * b * f
        - 2 * a * a * c * f
        -     a * a * b * g
        + 3 * a * a * a * h;
}

static double calc_x(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
          3 * d * d * e * e * e
        - 2 * c * d * e * e * f
        + 2 * b * d * e * f * f
        - 2 * a * d * f * f * f
        +     c * c * e * e * g
        - 4 * b * d * e * e * g
        -     b * c * e * f * g
        + 6 * a * d * e * f * g
        +     a * c * f * f * g
        +     b * b * e * g * g
        - 2 * a * c * e * g * g
        -     a * b * f * g * g
        +     a * a * g * g * g
        + 3 * b * c * e * e * h
        - 6 * a * d * e * e * h
        - 2 * b * b * e * f * h
        -     a * c * e * f * h
        + 2 * a * b * f * f * h
        +     a * b * e * g * h
        - 3 * a * a * f * g * h
        + 3 * a * a * e * h * h;
}

static double calc_y(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
             -c * c * c * e * e
        + 3 * b * c * d * e * e
        - 3 * a * d * d * e * e
        +     b * c * c * e * f
        - 2 * b * b * d * e * f
        -     a * c * d * e * f
        -     a * c * c * f * f
        + 2 * a * b * d * f * f
        -     b * b * c * e * g
        + 2 * a * c * c * e * g
        +     a * b * d * e * g
        +     a * b * c * f * g
        - 3 * a * a * d * f * g
        -     a * a * c * g * g
        + 2 * b * b * b * e * h
        - 6 * a * b * c * e * h
        + 6 * a * a * d * e * h
        - 2 * a * b * b * f * h
        + 4 * a * a * c * f * h
        + 2 * a * a * b * g * h
        - 3 * a * a * a * h * h;
}

static double calc_c(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
             -d * d * d * e * e * e
        +     c * d * d * e * e * f
        -     b * d * d * e * f * f
        +     a * d * d * f * f * f
        -     c * c * d * e * e * g
        + 2 * b * d * d * e * e * g
        +     b * c * d * e * f * g
        - 3 * a * d * d * e * f * g
        -     a * c * d * f * f * g
        -     b * b * d * e * g * g
        + 2 * a * c * d * e * g * g
        +     a * b * d * f * g * g
        -     a * a * d * g * g * g
        +     c * c * c * e * e * h
        - 3 * b * c * d * e * e * h
        + 3 * a * d * d * e * e * h
        -     b * c * c * e * f * h
        + 2 * b * b * d * e * f * h
        +     a * c * d * e * f * h
        +     a * c * c * f * f * h
        - 2 * a * b * d * f * f * h
        +     b * b * c * e * g * h
        - 2 * a * c * c * e * g * h
        -     a * b * d * e * g * h
        -     a * b * c * f * g * h
        + 3 * a * a * d * f * g * h
        +     a * a * c * g * g * h
        -     b * b * b * e * h * h
        + 3 * a * b * c * e * h * h
        - 3 * a * a * d * e * h * h
        +     a * b * b * f * h * h
        - 2 * a * a * c * f * h * h
        -     a * a * b * g * h * h
        +     a * a * a * h * h * h;
}
// end of generated code

#else

/* more Mathematica generated code. This takes a different tack, starting with
   the control-point based parametric formulas.  The C code is unoptimized --
   in this form, this is a proof of concept (since the other code didn't work)
*/
static double calc_c(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
d*d*d*e*e*e - 3*d*d*(3*c*e*e*f + 3*b*e*(-3*f*f + 2*e*g) + a*(9*f*f*f - 9*e*f*g + e*e*h)) -
   h*(27*c*c*c*e*e - 27*c*c*(3*b*e*f - 3*a*f*f + 2*a*e*g) +
      h*(-27*b*b*b*e + 27*a*b*b*f - 9*a*a*b*g + a*a*a*h) +
      9*c*(9*b*b*e*g + a*b*(-9*f*g + 3*e*h) + a*a*(3*g*g - 2*f*h))) +
   3*d*(9*c*c*e*e*g + 9*b*b*e*(3*g*g - 2*f*h) + 3*a*b*(-9*f*g*g + 6*f*f*h + e*g*h) +
      a*a*(9*g*g*g - 9*f*g*h + e*h*h) + 3*c*(3*b*e*(-3*f*g + e*h) + a*(9*f*f*g - 6*e*g*g - e*f*h)))
    ;
}

// - Power(e - 3*f + 3*g - h,3)*Power(x,3)
static double calc_xxx(double e3f3gh) {
    return -e3f3gh * e3f3gh * e3f3gh;
}

static double calc_y(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
+ 3*(6*b*d*d*e*e - d*d*d*e*e + 18*b*b*d*e*f - 18*b*d*d*e*f -
      9*b*d*d*f*f - 54*b*b*d*e*g + 12*b*d*d*e*g - 27*b*b*d*g*g - 18*b*b*b*e*h + 18*b*b*d*e*h +
      18*b*b*d*f*h + a*a*a*h*h - 9*b*b*b*h*h + 9*c*c*c*e*(e + 2*h) +
      a*a*(-3*b*h*(2*g + h) + d*(-27*g*g + 9*g*h - h*(2*e + h) + 9*f*(g + h))) +
      a*(9*b*b*h*(2*f + h) - 3*b*d*(6*f*f - 6*f*(3*g - 2*h) + g*(-9*g + h) + e*(g + h)) +
         d*d*(e*e + 9*f*(3*f - g) + e*(-9*f - 9*g + 2*h))) -
      9*c*c*(d*e*(e + 2*g) + 3*b*(f*h + e*(f + h)) + a*(-3*f*f - 6*f*h + 2*(g*h + e*(g + h)))) +
      3*c*(d*d*e*(e + 2*f) + a*a*(3*g*g + 6*g*h - 2*h*(2*f + h)) + 9*b*b*(g*h + e*(g + h)) +
         a*d*(-9*f*f - 18*f*g + 6*g*g + f*h + e*(f + 12*g + h)) +
         b*(d*(-3*e*e + 9*f*g + e*(9*f + 9*g - 6*h)) + 3*a*(h*(2*e - 3*g + h) - 3*f*(g + h))))) // *y
    ;
}

static double calc_yy(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
- 3*(18*c*c*c*e - 18*c*c*d*e + 6*c*d*d*e - d*d*d*e + 3*c*d*d*f - 9*c*c*d*g + a*a*a*h + 9*c*c*c*h -
      9*b*b*b*(e + 2*h) - a*a*(d*(e - 9*f + 18*g - 7*h) + 3*c*(2*f - 6*g + h)) +
      a*(-9*c*c*(2*e - 6*f + 2*g - h) + d*d*(-7*e + 18*f - 9*g + h) + 3*c*d*(7*e - 17*f + 3*g + h)) +
      9*b*b*(3*c*(e + g + h) + a*(f + 2*h) - d*(e - 2*(f - 3*g + h))) -
      3*b*(-(d*d*(e - 6*f + 2*g)) - 3*c*d*(e + 3*f + 3*g - h) + 9*c*c*(e + f + h) + a*a*(g + 2*h) +
         a*(c*(-3*e + 9*f + 9*g + 3*h) + d*(e + 3*f - 17*g + 7*h)))) // *Power(y,2)
    ;
}

// + Power(a - 3*b + 3*c - d,3)*Power(y,3)
static double calc_yyy(double a3b3cd) {
    return a3b3cd * a3b3cd * a3b3cd;
}

static double calc_xx(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
// + Power(x,2)*
(-3*(-9*b*e*f*f + 9*a*f*f*f + 6*b*e*e*g - 9*a*e*f*g + 27*b*e*f*g - 27*a*f*f*g + 18*a*e*g*g - 54*b*e*g*g +
         27*a*f*g*g + 27*b*f*g*g - 18*a*g*g*g + a*e*e*h - 9*b*e*e*h + 3*a*e*f*h + 9*b*e*f*h + 9*a*f*f*h -
         18*b*f*f*h - 21*a*e*g*h + 51*b*e*g*h - 9*a*f*g*h - 27*b*f*g*h + 18*a*g*g*h + 7*a*e*h*h - 18*b*e*h*h - 3*a*f*h*h +
         18*b*f*h*h - 6*a*g*h*h - 3*b*g*h*h + a*h*h*h +
         3*c*(-9*f*f*(g - 2*h) + 3*g*g*h - f*h*(9*g + 2*h) + e*e*(f - 6*g + 6*h) +
            e*(9*f*g + 6*g*g - 17*f*h - 3*g*h + 3*h*h)) -
         d*(e*e*e + e*e*(-6*f - 3*g + 7*h) - 9*(2*f - g)*(f*f + g*g - f*(g + h)) +
            e*(18*f*f + 9*g*g + 3*g*h + h*h - 3*f*(3*g + 7*h)))) )
    ;
}

// + Power(x,2)*(3*(a - 3*b + 3*c - d)*Power(e - 3*f + 3*g - h,2)*y)
static double calc_xxy(double a3b3cd, double e3f3gh) {
    return 3 * a3b3cd * e3f3gh * e3f3gh;
}

static double calc_x(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
// + x*
(-3*(27*b*b*e*g*g - 27*a*b*f*g*g + 9*a*a*g*g*g - 18*b*b*e*f*h + 18*a*b*f*f*h + 3*a*b*e*g*h -
         27*b*b*e*g*h - 9*a*a*f*g*h + 27*a*b*f*g*h - 9*a*a*g*g*h + a*a*e*h*h - 9*a*b*e*h*h +
         27*b*b*e*h*h + 6*a*a*f*h*h - 18*a*b*f*h*h - 9*b*b*f*h*h + 3*a*a*g*h*h +
         6*a*b*g*h*h - a*a*h*h*h + 9*c*c*(e*e*(g - 3*h) - 3*f*f*h + e*(3*f + 2*g)*h) +
         d*d*(e*e*e - 9*f*f*f + 9*e*f*(f + g) - e*e*(3*f + 6*g + h)) +
         d*(-3*c*(-9*f*f*g + e*e*(2*f - 6*g - 3*h) + e*(9*f*g + 6*g*g + f*h)) +
            a*(-18*f*f*f - 18*e*g*g + 18*g*g*g - 2*e*e*h + 3*e*g*h + 2*e*h*h + 9*f*f*(3*g + 2*h) +
               3*f*(6*e*g - 9*g*g - e*h - 6*g*h)) - 3*b*(9*f*g*g + e*e*(4*g - 3*h) - 6*f*f*h -
               e*(6*f*f + g*(18*g + h) - 3*f*(3*g + 4*h)))) +
         3*c*(3*b*(e*e*h + 3*f*g*h - e*(3*f*g - 6*f*h + 6*g*h + h*h)) +
            a*(9*f*f*(g - 2*h) + f*h*(-e + 9*g + 4*h) - 3*(2*g*g*h + e*(2*g*g - 4*g*h + h*h))))) )
    ;
}

static double calc_xy(double a, double b, double c, double d,
                     double e, double f, double g, double h) {
    return
// + x*3*
(-2*a*d*e*e - 7*d*d*e*e + 15*a*d*e*f + 21*d*d*e*f - 9*a*d*f*f - 18*d*d*f*f - 15*a*d*e*g -
         3*d*d*e*g - 9*a*a*f*g + 9*d*d*f*g + 18*a*a*g*g + 9*a*d*g*g + 2*a*a*e*h - 2*d*d*e*h +
         3*a*a*f*h + 15*a*d*f*h - 21*a*a*g*h - 15*a*d*g*h + 7*a*a*h*h + 2*a*d*h*h -
         9*c*c*(2*e*e + 3*f*f + 3*f*h - 2*g*h + e*(-3*f - 4*g + h)) +
         9*b*b*(3*g*g - 3*g*h + 2*h*(-2*f + h) + e*(-2*f + 3*g + h)) +
         3*b*(3*c*(e*e + 3*e*(f - 3*g) + (9*f - 3*g - h)*h) + a*(6*f*f + e*g - 9*f*g - 9*g*g - 5*e*h + 9*f*h + 14*g*h - 7*h*h) +
            d*(-e*e + 12*f*f - 27*f*g + e*(-9*f + 20*g - 5*h) + g*(9*g + h))) +
         3*c*(a*(-(e*f) - 9*f*f + 27*f*g - 12*g*g + 5*e*h - 20*f*h + 9*g*h + h*h) +
            d*(7*e*e + 9*f*f + 9*f*g - 6*g*g - f*h + e*(-14*f - 9*g + 5*h)))) // *y
    ;
}

// - x*3*Power(a - 3*b + 3*c - d,2)*(e - 3*f + 3*g - h)*Power(y,2)
static double calc_xyy(double a3b3cd, double e3f3gh) {
    return -3 * a3b3cd * a3b3cd * e3f3gh;
}

#endif

static double (*calc_proc[])(double a, double b, double c, double d,
                             double e, double f, double g, double h) = {
    calc_xx, calc_xy, calc_yy, calc_x, calc_y, calc_c
};

#if USE_SYVESTER
/* Control points to parametric coefficients
    s = 1 - t
    Attt + 3Btts + 3Ctss + Dsss ==
    Attt + 3B(1 - t)tt + 3C(1 - t)(t - tt) + D(1 - t)(1 - 2t + tt) ==
    Attt + 3B(tt - ttt) + 3C(t - tt - tt + ttt) + D(1-2t+tt-t+2tt-ttt) ==
    Attt + 3Btt - 3Bttt + 3Ct - 6Ctt + 3Cttt + D - 3Dt + 3Dtt - Dttt ==
    D + (3C - 3D)t + (3B - 6C + 3D)tt + (A - 3B + 3C - D)ttt
    a = A - 3*B + 3*C -   D
    b =     3*B - 6*C + 3*D
    c =           3*C - 3*D
    d =                   D
 */

 /* http://www.algorithmist.net/bezier3.html
    p = 3 * A
    q = 3 * B
    r = 3 * C
    a = A
    b = q - p
    c = p - 2 * q + r
    d = D - A + q - r

 B(t) = a + t * (b + t * (c + t * d))

 so

 B(t) = a + t*b + t*t*(c + t*d)
      = a + t*b + t*t*c + t*t*t*d
  */
static void set_abcd(const double* cubic, double& a, double& b, double& c,
                     double& d) {
    a = cubic[0];     // a = A
    b = 3 * cubic[2]; // b = 3*B (compute rest of b lazily)
    c = 3 * cubic[4]; // c = 3*C (compute rest of c lazily)
    d = cubic[6];     // d = D
    a += -b + c - d;  // a = A - 3*B + 3*C - D
}

static void calc_bc(const double d, double& b, double& c) {
    b -= 3 * c; // b = 3*B - 3*C
    c -= 3 * d; // c = 3*C - 3*D
    b -= c;     // b = 3*B - 6*C + 3*D
}

static void alt_set_abcd(const double* cubic, double& a, double& b, double& c,
                     double& d) {
    a = cubic[0];
    double p = 3 * a;
    double q = 3 * cubic[2];
    double r = 3 * cubic[4];
    b = q - p;
    c = p - 2 * q + r;
    d = cubic[6] - a + q - r;
}

const bool try_alt = true;

#else

static void calc_ABCD(double a, double b, double c, double d,
                      double e, double f, double g, double h,
                      double p[coeff_count]) {
    double a3b3cd = a - 3 * (b - c) - d;
    double e3f3gh = e - 3 * (f - g) - h;
    p[xxx_coeff] = calc_xxx(e3f3gh);
    p[xxy_coeff] = calc_xxy(a3b3cd, e3f3gh);
    p[xyy_coeff] = calc_xyy(a3b3cd, e3f3gh);
    p[yyy_coeff] = calc_yyy(a3b3cd);
}
#endif

bool implicit_matches(const Cubic& one, const Cubic& two) {
    double p1[coeff_count]; // a'xxx , b'xxy , c'xyy , d'xx , e'xy , f'yy, etc.
    double p2[coeff_count];
#if USE_SYVESTER
    double a1, b1, c1, d1;
    if (try_alt)
        alt_set_abcd(&one[0].x, a1, b1, c1, d1);
    else
        set_abcd(&one[0].x, a1, b1, c1, d1);
    double e1, f1, g1, h1;
    if (try_alt)
        alt_set_abcd(&one[0].y, e1, f1, g1, h1);
    else
        set_abcd(&one[0].y, e1, f1, g1, h1);
    calc_ABCD(a1, e1, p1);
    double a2, b2, c2, d2;
    if (try_alt)
        alt_set_abcd(&two[0].x, a2, b2, c2, d2);
    else
        set_abcd(&two[0].x, a2, b2, c2, d2);
    double e2, f2, g2, h2;
    if (try_alt)
        alt_set_abcd(&two[0].y, e2, f2, g2, h2);
    else
        set_abcd(&two[0].y, e2, f2, g2, h2);
    calc_ABCD(a2, e2, p2);
#else
    double a1 = one[0].x;
    double b1 = one[1].x;
    double c1 = one[2].x;
    double d1 = one[3].x;
    double e1 = one[0].y;
    double f1 = one[1].y;
    double g1 = one[2].y;
    double h1 = one[3].y;
    calc_ABCD(a1, b1, c1, d1, e1, f1, g1, h1, p1);
    double a2 = two[0].x;
    double b2 = two[1].x;
    double c2 = two[2].x;
    double d2 = two[3].x;
    double e2 = two[0].y;
    double f2 = two[1].y;
    double g2 = two[2].y;
    double h2 = two[3].y;
    calc_ABCD(a2, b2, c2, d2, e2, f2, g2, h2, p2);
#endif
    int first = 0;
    for (int index = 0; index < coeff_count; ++index) {
#if USE_SYVESTER
        if (!try_alt && index == xx_coeff) {
            calc_bc(d1, b1, c1);
            calc_bc(h1, f1, g1);
            calc_bc(d2, b2, c2);
            calc_bc(h2, f2, g2);
        }
#endif
        if (index >= xx_coeff) {
            int procIndex = index - xx_coeff;
            p1[index] = (*calc_proc[procIndex])(a1, b1, c1, d1, e1, f1, g1, h1);
            p2[index] = (*calc_proc[procIndex])(a2, b2, c2, d2, e2, f2, g2, h2);
        }
        if (approximately_zero(p1[index]) || approximately_zero(p2[index])) {
            first += first == index;
            continue;
        }
        if (first == index) {
            continue;
        }
        if (!approximately_equal(p1[index] * p2[first],
                p1[first] * p2[index])) {
            return false;
        }
    }
    return true;
}

static double tangent(const double* cubic, double t) {
    double a, b, c, d;
#if USE_SYVESTER
    set_abcd(cubic, a, b, c, d);
    calc_bc(d, b, c);
#else
    coefficients(cubic, a, b, c, d);
#endif
    return 3 * a * t * t + 2 * b * t + c;
}

void tangent(const Cubic& cubic, double t, _Point& result) {
    result.x = tangent(&cubic[0].x, t);
    result.y = tangent(&cubic[0].y, t);
}

// unit test to return and validate parametric coefficients
#include "CubicParameterization_TestUtility.cpp"


