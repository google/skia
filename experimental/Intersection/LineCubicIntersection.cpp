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

class LineCubicIntersections {
public:

LineCubicIntersections(const Cubic& c, const _Line& l, Intersections& i)
    : cubic(c)
    , line(l)
    , intersections(i) {
}

// see parallel routine in line quadratic intersections
int intersectRay(double roots[3]) {
    double adj = line[1].x - line[0].x;
    double opp = line[1].y - line[0].y;
    Cubic r;
    for (int n = 0; n < 4; ++n) {
        r[n].x = (cubic[n].y - line[0].y) * adj - (cubic[n].x - line[0].x) * opp;
    }
    double A, B, C, D;
    coefficients(&r[0].x, A, B, C, D);
    return cubicRootsValidT(A, B, C, D, roots);
}

int intersect() {
    addEndPoints();
    double rootVals[3];
    int roots = intersectRay(rootVals);
    for (int index = 0; index < roots; ++index) {
        double cubicT = rootVals[index];
        double lineT = findLineT(cubicT);
        if (pinTs(cubicT, lineT)) {
            _Point pt;
            xy_at_t(line, lineT, pt.x, pt.y);
            intersections.insert(cubicT, lineT, pt);
        }
    }
    return intersections.fUsed;
}

int horizontalIntersect(double axisIntercept, double roots[3]) {
    double A, B, C, D;
    coefficients(&cubic[0].y, A, B, C, D);
    D -= axisIntercept;
    return cubicRootsValidT(A, B, C, D, roots);
}

int horizontalIntersect(double axisIntercept, double left, double right, bool flipped) {
    addHorizontalEndPoints(left, right, axisIntercept);
    double rootVals[3];
    int roots = horizontalIntersect(axisIntercept, rootVals);
    for (int index = 0; index < roots; ++index) {
        _Point pt;
        double cubicT = rootVals[index];
        xy_at_t(cubic, cubicT, pt.x, pt.y);
        double lineT = (pt.x - left) / (right - left);
        if (pinTs(cubicT, lineT)) {
            intersections.insert(cubicT, lineT, pt);
        }
    }
    if (flipped) {
        flip();
    }
    return intersections.fUsed;
}

int verticalIntersect(double axisIntercept, double roots[3]) {
    double A, B, C, D;
    coefficients(&cubic[0].x, A, B, C, D);
    D -= axisIntercept;
    return cubicRootsValidT(A, B, C, D, roots);
}

int verticalIntersect(double axisIntercept, double top, double bottom, bool flipped) {
    addVerticalEndPoints(top, bottom, axisIntercept);
    double rootVals[3];
    int roots = verticalIntersect(axisIntercept, rootVals);
    for (int index = 0; index < roots; ++index) {
        _Point pt;
        double cubicT = rootVals[index];
        xy_at_t(cubic, cubicT, pt.x, pt.y);
        double lineT = (pt.y - top) / (bottom - top);
        if (pinTs(cubicT, lineT)) {
            intersections.insert(cubicT, lineT, pt);
        }
    }
    if (flipped) {
        flip();
    }
    return intersections.fUsed;
}

protected:

void addEndPoints()
{
    for (int cIndex = 0; cIndex < 4; cIndex += 3) {
        for (int lIndex = 0; lIndex < 2; lIndex++) {
            if (cubic[cIndex] == line[lIndex]) {
                intersections.insert(cIndex >> 1, lIndex, line[lIndex]);
            }
        }
    }
}

void addHorizontalEndPoints(double left, double right, double y)
{
    for (int cIndex = 0; cIndex < 4; cIndex += 3) {
        if (cubic[cIndex].y != y) {
            continue;
        }
        if (cubic[cIndex].x == left) {
            intersections.insert(cIndex >> 1, 0, cubic[cIndex]);
        }
        if (cubic[cIndex].x == right) {
            intersections.insert(cIndex >> 1, 1, cubic[cIndex]);
        }
    }
}

void addVerticalEndPoints(double top, double bottom, double x)
{
    for (int cIndex = 0; cIndex < 4; cIndex += 3) {
        if (cubic[cIndex].x != x) {
            continue;
        }
        if (cubic[cIndex].y == top) {
            intersections.insert(cIndex >> 1, 0, cubic[cIndex]);
        }
        if (cubic[cIndex].y == bottom) {
            intersections.insert(cIndex >> 1, 1, cubic[cIndex]);
        }
    }
}

double findLineT(double t) {
    double x, y;
    xy_at_t(cubic, t, x, y);
    double dx = line[1].x - line[0].x;
    double dy = line[1].y - line[0].y;
    if (fabs(dx) > fabs(dy)) {
        return (x - line[0].x) / dx;
    }
    return (y - line[0].y) / dy;
}

void flip() {
    // OPTIMIZATION: instead of swapping, pass original line, use [1].y - [0].y
    int roots = intersections.fUsed;
    for (int index = 0; index < roots; ++index) {
        intersections.fT[1][index] = 1 - intersections.fT[1][index];
    }
}

static bool pinTs(double& cubicT, double& lineT) {
    if (!approximately_one_or_less(lineT)) {
        return false;
    }
    if (!approximately_zero_or_more(lineT)) {
        return false;
    }
    if (precisely_less_than_zero(cubicT)) {
        cubicT = 0;
    } else if (precisely_greater_than_one(cubicT)) {
        cubicT = 1;
    }
    if (precisely_less_than_zero(lineT)) {
        lineT = 0;
    } else if (precisely_greater_than_one(lineT)) {
        lineT = 1;
    }
    return true;
}

private:

const Cubic& cubic;
const _Line& line;
Intersections& intersections;
};

int horizontalIntersect(const Cubic& cubic, double left, double right, double y,
        double tRange[3]) {
    LineCubicIntersections c(cubic, *((_Line*) 0), *((Intersections*) 0));
    double rootVals[3];
    int result = c.horizontalIntersect(y, rootVals);
    int tCount = 0;
    for (int index = 0; index < result; ++index) {
        double x, y;
        xy_at_t(cubic, rootVals[index], x, y);
        if (x < left || x > right) {
            continue;
        }
        tRange[tCount++] = rootVals[index];
    }
    return result;
}

int horizontalIntersect(const Cubic& cubic, double left, double right, double y,
        bool flipped, Intersections& intersections) {
    LineCubicIntersections c(cubic, *((_Line*) 0), intersections);
    return c.horizontalIntersect(y, left, right, flipped);
}

int verticalIntersect(const Cubic& cubic, double top, double bottom, double x,
        bool flipped, Intersections& intersections) {
    LineCubicIntersections c(cubic, *((_Line*) 0), intersections);
    return c.verticalIntersect(x, top, bottom, flipped);
}

int intersect(const Cubic& cubic, const _Line& line, Intersections& i) {
    LineCubicIntersections c(cubic, line, i);
    return c.intersect();
}

int intersectRay(const Cubic& cubic, const _Line& line, Intersections& i) {
    LineCubicIntersections c(cubic, line, i);
    return c.intersectRay(i.fT[0]);
}
