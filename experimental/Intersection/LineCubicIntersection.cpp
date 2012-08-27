/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "CubicUtilities.h"
#include "Intersections.h"
#include "LineUtilities.h"

/*
Find the interection of a line and cubic by solving for valid t values.

Analogous to line-quadratic intersection, solve line-cubic intersection by
representing the cubic as:
  x = a(1-t)^3 + 2b(1-t)^2t + c(1-t)t^2 + dt^3
  y = e(1-t)^3 + 2f(1-t)^2t + g(1-t)t^2 + ht^3
and the line as:
  y = i*x + j  (if the line is more horizontal)
or:
  x = i*y + j  (if the line is more vertical)

Then using Mathematica, solve for the values of t where the cubic intersects the
line:

  (in) Resultant[
        a*(1 - t)^3 + 3*b*(1 - t)^2*t + 3*c*(1 - t)*t^2 + d*t^3 - x,
        e*(1 - t)^3 + 3*f*(1 - t)^2*t + 3*g*(1 - t)*t^2 + h*t^3 - i*x - j, x]
  (out) -e     +   j     +
       3 e t   - 3 f t   -
       3 e t^2 + 6 f t^2 - 3 g t^2 +
         e t^3 - 3 f t^3 + 3 g t^3 - h t^3 +
     i ( a     -
       3 a t + 3 b t +
       3 a t^2 - 6 b t^2 + 3 c t^2 -
         a t^3 + 3 b t^3 - 3 c t^3 + d t^3 )

if i goes to infinity, we can rewrite the line in terms of x. Mathematica:

  (in) Resultant[
        a*(1 - t)^3 + 3*b*(1 - t)^2*t + 3*c*(1 - t)*t^2 + d*t^3 - i*y - j,
        e*(1 - t)^3 + 3*f*(1 - t)^2*t + 3*g*(1 - t)*t^2 + h*t^3 - y,       y]
  (out)  a     -   j     -
       3 a t   + 3 b t   +
       3 a t^2 - 6 b t^2 + 3 c t^2 -
         a t^3 + 3 b t^3 - 3 c t^3 + d t^3 -
     i ( e     -
       3 e t   + 3 f t   +
       3 e t^2 - 6 f t^2 + 3 g t^2 -
         e t^3 + 3 f t^3 - 3 g t^3 + h t^3 )

Solving this with Mathematica produces an expression with hundreds of terms;
instead, use Numeric Solutions recipe to solve the cubic.

The near-horizontal case, in terms of:  Ax^3 + Bx^2 + Cx + D == 0
    A =   (-(-e + 3*f - 3*g + h) + i*(-a + 3*b - 3*c + d)     )
    B = 3*(-( e - 2*f +   g    ) + i*( a - 2*b +   c    )     )
    C = 3*(-(-e +   f          ) + i*(-a +   b          )     )
    D =   (-( e                ) + i*( a                ) + j )

The near-vertical case, in terms of:  Ax^3 + Bx^2 + Cx + D == 0
    A =   ( (-a + 3*b - 3*c + d) - i*(-e + 3*f - 3*g + h)     )
    B = 3*( ( a - 2*b +   c    ) - i*( e - 2*f +   g    )     )
    C = 3*( (-a +   b          ) - i*(-e +   f          )     )
    D =   ( ( a                ) - i*( e                ) - j )

For horizontal lines:
(in) Resultant[
      a*(1 - t)^3 + 3*b*(1 - t)^2*t + 3*c*(1 - t)*t^2 + d*t^3 - j,
      e*(1 - t)^3 + 3*f*(1 - t)^2*t + 3*g*(1 - t)*t^2 + h*t^3 - y, y]
(out)  e     -   j     -
     3 e t   + 3 f t   +
     3 e t^2 - 6 f t^2 + 3 g t^2 -
       e t^3 + 3 f t^3 - 3 g t^3 + h t^3
So the cubic coefficients are:

 */

class LineCubicIntersections : public Intersections {
public:

LineCubicIntersections(const Cubic& c, const _Line& l, double r[3])
    : cubic(c)
    , line(l)
    , range(r) {
}

int intersect() {
    double slope;
    double axisIntercept;
    moreHorizontal = implicitLine(line, slope, axisIntercept);
    double A, B, C, D;
    coefficients(&cubic[0].x, A, B, C, D);
    double E, F, G, H;
    coefficients(&cubic[0].y, E, F, G, H);
    if (moreHorizontal) {
        A = A * slope - E;
        B = B * slope - F;
        C = C * slope - G;
        D = D * slope - H + axisIntercept;
    } else {
        A = A - E * slope;
        B = B - F * slope;
        C = C - G * slope;
        D = D - H * slope - axisIntercept;
    }
    return cubicRoots(A, B, C, D, range);
}

int horizontalIntersect(double axisIntercept) {
    double A, B, C, D;
    coefficients(&cubic[0].y, A, B, C, D);
    D -= axisIntercept;
    return cubicRoots(A, B, C, D, range);
}

int verticalIntersect(double axisIntercept) {
    double A, B, C, D;
    coefficients(&cubic[0].x, A, B, C, D);
    D -= axisIntercept;
    return cubicRoots(A, B, C, D, range);
}

double findLineT(double t) {
    const double* cPtr;
    const double* lPtr;
    if (moreHorizontal) {
        cPtr = &cubic[0].x;
        lPtr = &line[0].x;
    } else {
        cPtr = &cubic[0].y;
        lPtr = &line[0].y;
    }
    // FIXME: should fold the following in with TestUtilities.cpp xy_at_t()
    double s = 1 - t;
    double cubicVal = cPtr[0] * s * s * s + 3 * cPtr[2] * s * s * t
                + 3 * cPtr[4] * s * t * t + cPtr[6] * t * t * t;
    return (cubicVal - lPtr[0]) / (lPtr[2] - lPtr[0]);
}

private:

const Cubic& cubic;
const _Line& line;
double* range;
bool moreHorizontal;

};

int horizontalIntersect(const Cubic& cubic, double y, double tRange[3]) {
    LineCubicIntersections c(cubic, *((_Line*) 0), tRange);
    return c.horizontalIntersect(y);
}

int horizontalIntersect(const Cubic& cubic, double left, double right, double y,
        double tRange[3]) {
    LineCubicIntersections c(cubic, *((_Line*) 0), tRange);
    int result = c.horizontalIntersect(y);
    for (int index = 0; index < result; ) {
        double x, y;
        xy_at_t(cubic, tRange[index], x, y);
        if (x < left || x > right) {
            if (--result > index) {
                tRange[index] = tRange[result];
            }
            continue;
        }
        ++index;
    }
    return result;
}

int horizontalIntersect(const Cubic& cubic, double left, double right, double y,
        bool flipped, Intersections& intersections) {
    LineCubicIntersections c(cubic, *((_Line*) 0), intersections.fT[0]);
    int result = c.horizontalIntersect(y);
    for (int index = 0; index < result; ) {
        double x, y;
        xy_at_t(cubic, intersections.fT[0][index], x, y);
        if (x < left || x > right) {
            if (--result > index) {
                intersections.fT[0][index] = intersections.fT[0][result];
            }
            continue;
        }
        intersections.fT[1][index] = (x - left) / (right - left);
        ++index;
    }
    if (flipped) {
        // OPTIMIZATION: instead of swapping, pass original line, use [1].x - [0].x
        for (int index = 0; index < result; ++index) {
            intersections.fT[1][index] = 1 - intersections.fT[1][index];
        }
    }
    return result;
}

int verticalIntersect(const Cubic& cubic, double top, double bottom, double x,
        bool flipped, Intersections& intersections) {
    LineCubicIntersections c(cubic, *((_Line*) 0), intersections.fT[0]);
    int result = c.verticalIntersect(x);
    for (int index = 0; index < result; ) {
        double x, y;
        xy_at_t(cubic, intersections.fT[0][index], x, y);
        if (y < top || y > bottom) {
            if (--result > index) {
                intersections.fT[1][index] = intersections.fT[0][result];
            }
            continue;
        }
        intersections.fT[0][index] = (y - top) / (bottom - top);
        ++index;
    }
    if (flipped) {
        // OPTIMIZATION: instead of swapping, pass original line, use [1].x - [0].x
        for (int index = 0; index < result; ++index) {
            intersections.fT[1][index] = 1 - intersections.fT[1][index];
        }
    }
    return result;
}

int intersect(const Cubic& cubic, const _Line& line, double cRange[3], double lRange[3]) {
    LineCubicIntersections c(cubic, line, cRange);
    int roots;
    if (approximately_equal(line[0].y, line[1].y)) {
        roots = c.horizontalIntersect(line[0].y);
    } else {
        roots = c.intersect();
    }
    for (int index = 0; index < roots; ++index) {
        lRange[index] = c.findLineT(cRange[index]);
    }
    return roots;
}
