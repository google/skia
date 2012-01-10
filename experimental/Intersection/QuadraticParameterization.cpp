#include "CubicIntersection.h"
#include "QuadraticUtilities.h"

/* from http://tom.cs.byu.edu/~tom/papers/cvgip84.pdf 4.1
 *
 * This paper proves that Syvester's method can compute the implicit form of 
 * the quadratic from the parameterzied form.
 *
 * Given x = a*t*t + b*t + c  (the parameterized form)
 *       y = d*t*t + e*t + f
 *
 * we want to find an equation of the implicit form:
 *
 * A*x*x + B*x*y + C*y*y + D*x + E*y + F = 0
 *
 * The implicit form can be expressed as a 4x4 determinant, as shown.
 *
 * The resultant obtained by Syvester's method is
 *
 * |   a   b   (c - x)     0     |
 * |   0   a      b     (c - x)  |
 * |   d   e   (f - y)     0     |
 * |   0   d      e     (f - y)  |
 *
 * which expands to
 *
 * d*d*x*x + -2*a*d*x*y + a*a*y*y
 *         + (-2*c*d*d + b*e*d - a*e*e + 2*a*f*d)*x
 *         + (-2*f*a*a + e*b*a - d*b*b + 2*d*c*a)*y
 *         +
 * |   a   b   c   0   |
 * |   0   a   b   c   | == 0.
 * |   d   e   f   0   |
 * |   0   d   e   f   |
 *
 * Expanding the constant determinant results in
 *
 *   | a b c |     | b c 0 |
 * a*| e f 0 | + d*| a b c | ==
 *   | d e f |     | d e f |
 *
 * a*(a*f*f + c*e*e - c*f*d - b*e*f) + d*(b*b*f + c*c*d - c*a*f - c*e*b)
 *
 */

enum {
    xx_coeff,
    xy_coeff,
    yy_coeff,
    x_coeff,
    y_coeff,
    c_coeff,
    coeff_count
};

static bool straight_forward = true;

static void implicit_coefficients(const Quadratic& q, double p[coeff_count]) {
    double a, b, c;
    set_abc(&q[0].x, a, b, c);
    double d, e, f;
    set_abc(&q[0].y, d, e, f);
    // compute the implicit coefficients
    if (straight_forward) { // 42 muls, 13 adds
        p[xx_coeff] = d * d;
        p[xy_coeff] = -2 * a * d;
        p[yy_coeff] = a * a;
        p[x_coeff] = -2*c*d*d + b*e*d - a*e*e + 2*a*f*d;
        p[y_coeff] = -2*f*a*a + e*b*a - d*b*b + 2*d*c*a;
        p[c_coeff] = a*(a*f*f + c*e*e - c*f*d - b*e*f)
                   + d*(b*b*f + c*c*d - c*a*f - c*e*b);
    } else { // 26 muls, 11 adds
        double aa = a * a;
        double ad = a * d;
        double dd = d * d;
        p[xx_coeff] = dd;
        p[xy_coeff] = -2 * ad;
        p[yy_coeff] = aa;
        double be = b * e;
        double bde = be * d;
        double cdd = c * dd;
        double ee = e * e;
        p[x_coeff] =  -2*cdd + bde - a*ee + 2*ad*f;
        double aaf = aa * f;
        double abe = a * be;
        double ac = a * c;
        double bb_2ac = b*b - 2*ac;
        p[y_coeff] = -2*aaf + abe - d*bb_2ac;
        p[c_coeff] = aaf*f + ac*ee + d*f*bb_2ac - abe*f + c*cdd - c*bde;
    }
}

 /* Given a pair of quadratics, determine their parametric coefficients.
  * If the scaled coefficients are nearly equal, then the part of the quadratics
  * may be coincident.
  * FIXME: optimization -- since comparison short-circuits on no match,
  * lazily compute the coefficients, comparing the easiest to compute first.
  * xx and yy first; then xy; and so on.
  */
bool implicit_matches(const Quadratic& one, const Quadratic& two) {
    double p1[coeff_count]; // a'xx , b'xy , c'yy , d'x , e'y , f
    double p2[coeff_count];
    implicit_coefficients(one, p1);
    implicit_coefficients(two, p2);
    int first = 0;
    for (int index = 0; index < coeff_count; ++index) {
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

static double tangent(const double* quadratic, double t) {
    double a, b, c;
    set_abc(quadratic, a, b, c);
    return 2 * a * t + b;
}

void tangent(const Quadratic& quadratic, double t, _Point& result) {
    result.x = tangent(&quadratic[0].x, t);
    result.y = tangent(&quadratic[0].y, t);
}

