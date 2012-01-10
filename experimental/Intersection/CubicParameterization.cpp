#include "CubicIntersection.h"

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
    xxx_coeff,
    xxy_coeff,
    xyy_coeff,
    yyy_coeff,
    xx_coeff,
    xy_coeff,
    yy_coeff,
    x_coeff,
    y_coeff,
    c_coeff,
    coeff_count
};

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
static double calc_E(double a, double b, double c, double d,
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

static double calc_F(double a, double b, double c, double d,
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

static double calc_G(double a, double b, double c, double d,
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

static double calc_H(double a, double b, double c, double d,
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

static double calc_I(double a, double b, double c, double d,
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

static double calc_J(double a, double b, double c, double d,
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

static double (*calc_proc[])(double a, double b, double c, double d,
                             double e, double f, double g, double h) = {
    calc_E, calc_F, calc_G, calc_H, calc_I, calc_J
};

/* Control points to parametric coefficients
    s = 1 - t
    Attt + 3Btt2 + 3Ctss + Dsss ==
    Attt + 3B(1 - t)tt + 3C(1 - t)(t - tt) + D(1 - t)(1 - 2t + tt) ==
    Attt + 3B(tt - ttt) + 3C(t - tt - tt + ttt) + D(1-2t+tt-t+2tt-ttt) ==
    Attt + 3Btt - 3Bttt + 3Ct - 6Ctt + 3Cttt + D - 3Dt + 3Dtt - Dttt ==
    D + (3C - 3D)t + (3B - 6C + 3D)tt + (A - 3B + 3C - D)ttt
    a = A - 3*B + 3*C -   D
    b =     3*B - 6*C + 3*D
    c =           3*C - 3*D
    d =                   D
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

bool implicit_matches(const Cubic& one, const Cubic& two) {
    double p1[coeff_count]; // a'xxx , b'xxy , c'xyy , d'xx , e'xy , f'yy, etc.
    double p2[coeff_count];
    double a1, b1, c1, d1;
    set_abcd(&one[0].x, a1, b1, c1, d1);
    double e1, f1, g1, h1;
    set_abcd(&one[0].y, e1, f1, g1, h1);
    calc_ABCD(a1, e1, p1);
    double a2, b2, c2, d2;
    set_abcd(&two[0].x, a2, b2, c2, d2);
    double e2, f2, g2, h2;
    set_abcd(&two[0].y, e2, f2, g2, h2);
    calc_ABCD(a2, e2, p2);
    int first = 0;
    for (int index = 0; index < coeff_count; ++index) {
        if (index == xx_coeff) {
            calc_bc(d1, b1, c1);
            calc_bc(h1, f1, g1);
            calc_bc(d2, b2, c2);
            calc_bc(h2, f2, g2);
        }
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
    set_abcd(cubic, a, b, c, d);
    calc_bc(d, b, c);
    return 3 * a * t * t + 2 * b * t + c;
}

void tangent(const Cubic& cubic, double t, _Point& result) {
    result.x = tangent(&cubic[0].x, t);
    result.y = tangent(&cubic[0].y, t);
}

