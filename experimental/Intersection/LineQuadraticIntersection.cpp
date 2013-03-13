/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "CurveIntersection.h"
#include "Intersections.h"
#include "LineUtilities.h"
#include "QuadraticUtilities.h"

/*
Find the interection of a line and quadratic by solving for valid t values.

From http://stackoverflow.com/questions/1853637/how-to-find-the-mathematical-function-defining-a-bezier-curve

"A Bezier curve is a parametric function. A quadratic Bezier curve (i.e. three
control points) can be expressed as: F(t) = A(1 - t)^2 + B(1 - t)t + Ct^2 where
A, B and C are points and t goes from zero to one.

This will give you two equations:

  x = a(1 - t)^2 + b(1 - t)t + ct^2
  y = d(1 - t)^2 + e(1 - t)t + ft^2

If you add for instance the line equation (y = kx + m) to that, you'll end up
with three equations and three unknowns (x, y and t)."

Similar to above, the quadratic is represented as
  x = a(1-t)^2 + 2b(1-t)t + ct^2
  y = d(1-t)^2 + 2e(1-t)t + ft^2
and the line as
  y = g*x + h

Using Mathematica, solve for the values of t where the quadratic intersects the
line:

  (in)  t1 = Resultant[a*(1 - t)^2 + 2*b*(1 - t)*t + c*t^2 - x,
                       d*(1 - t)^2 + 2*e*(1 - t)*t  + f*t^2 - g*x - h, x]
  (out) -d + h + 2 d t - 2 e t - d t^2 + 2 e t^2 - f t^2 +
         g  (a - 2 a t + 2 b t + a t^2 - 2 b t^2 + c t^2)
  (in)  Solve[t1 == 0, t]
  (out) {
    {t -> (-2 d + 2 e +   2 a g - 2 b g    -
      Sqrt[(2 d - 2 e -   2 a g + 2 b g)^2 -
          4 (-d + 2 e - f + a g - 2 b g    + c g) (-d + a g + h)]) /
         (2 (-d + 2 e - f + a g - 2 b g    + c g))
         },
    {t -> (-2 d + 2 e +   2 a g - 2 b g    +
      Sqrt[(2 d - 2 e -   2 a g + 2 b g)^2 -
          4 (-d + 2 e - f + a g - 2 b g    + c g) (-d + a g + h)]) /
         (2 (-d + 2 e - f + a g - 2 b g    + c g))
         }
        }

Using the results above (when the line tends towards horizontal)
       A =   (-(d - 2*e + f) + g*(a - 2*b + c)     )
       B = 2*( (d -   e    ) - g*(a -   b    )     )
       C =   (-(d          ) + g*(a          ) + h )

If g goes to infinity, we can rewrite the line in terms of x.
  x = g'*y + h'

And solve accordingly in Mathematica:

  (in)  t2 = Resultant[a*(1 - t)^2 + 2*b*(1 - t)*t + c*t^2 - g'*y - h',
                       d*(1 - t)^2 + 2*e*(1 - t)*t  + f*t^2 - y, y]
  (out)  a - h' - 2 a t + 2 b t + a t^2 - 2 b t^2 + c t^2 -
         g'  (d - 2 d t + 2 e t + d t^2 - 2 e t^2 + f t^2)
  (in)  Solve[t2 == 0, t]
  (out) {
    {t -> (2 a - 2 b -   2 d g' + 2 e g'    -
    Sqrt[(-2 a + 2 b +   2 d g' - 2 e g')^2 -
          4 (a - 2 b + c - d g' + 2 e g' - f g') (a - d g' - h')]) /
         (2 (a - 2 b + c - d g' + 2 e g' - f g'))
         },
    {t -> (2 a - 2 b -   2 d g' + 2 e g'    +
    Sqrt[(-2 a + 2 b +   2 d g' - 2 e g')^2 -
          4 (a - 2 b + c - d g' + 2 e g' - f g') (a - d g' - h')])/
         (2 (a - 2 b + c - d g' + 2 e g' - f g'))
         }
        }

Thus, if the slope of the line tends towards vertical, we use:
       A =   ( (a - 2*b + c) - g'*(d  - 2*e + f)      )
       B = 2*(-(a -   b    ) + g'*(d  -   e    )      )
       C =   ( (a          ) - g'*(d           ) - h' )
 */


class LineQuadraticIntersections {
public:

LineQuadraticIntersections(const Quadratic& q, const _Line& l, Intersections& i)
    : quad(q)
    , line(l)
    , intersections(i) {
}

int intersectRay(double roots[2]) {
/*
    solve by rotating line+quad so line is horizontal, then finding the roots
    set up matrix to rotate quad to x-axis
    |cos(a) -sin(a)|
    |sin(a)  cos(a)|
    note that cos(a) = A(djacent) / Hypoteneuse
              sin(a) = O(pposite) / Hypoteneuse
    since we are computing Ts, we can ignore hypoteneuse, the scale factor:
    |  A     -O    |
    |  O      A    |
    A = line[1].x - line[0].x (adjacent side of the right triangle)
    O = line[1].y - line[0].y (opposite side of the right triangle)
    for each of the three points (e.g. n = 0 to 2)
    quad[n].y' = (quad[n].y - line[0].y) * A - (quad[n].x - line[0].x) * O
*/
    double adj = line[1].x - line[0].x;
    double opp = line[1].y - line[0].y;
    double r[3];
    for (int n = 0; n < 3; ++n) {
        r[n] = (quad[n].y - line[0].y) * adj - (quad[n].x - line[0].x) * opp;
    }
    double A = r[2];
    double B = r[1];
    double C = r[0];
    A += C - 2 * B; // A = a - 2*b + c
    B -= C; // B = -(b - c)
    return quadraticRootsValidT(A, 2 * B, C, roots);
}

int intersect() {
    addEndPoints();
    double rootVals[2];
    int roots = intersectRay(rootVals);
    for (int index = 0; index < roots; ++index) {
        double quadT = rootVals[index];
        double lineT = findLineT(quadT);
        if (pinTs(quadT, lineT)) {
            _Point pt;
            xy_at_t(line, lineT, pt.x, pt.y);
            intersections.insert(quadT, lineT, pt);
        }
    }
    return intersections.fUsed;
}

int horizontalIntersect(double axisIntercept, double roots[2]) {
    double D = quad[2].y; // f
    double E = quad[1].y; // e
    double F = quad[0].y; // d
    D += F - 2 * E; // D = d - 2*e + f
    E -= F; // E = -(d - e)
    F -= axisIntercept;
    return quadraticRootsValidT(D, 2 * E, F, roots);
}

int horizontalIntersect(double axisIntercept, double left, double right, bool flipped) {
    addHorizontalEndPoints(left, right, axisIntercept);
    double rootVals[2];
    int roots = horizontalIntersect(axisIntercept, rootVals);
    for (int index = 0; index < roots; ++index) {
        _Point pt;
        double quadT = rootVals[index];
        xy_at_t(quad, quadT, pt.x, pt.y);
        double lineT = (pt.x - left) / (right - left);
        if (pinTs(quadT, lineT)) {
            intersections.insert(quadT, lineT, pt);
        }
    }
    if (flipped) {
        flip();
    }
    return intersections.fUsed;
}

int verticalIntersect(double axisIntercept, double roots[2]) {
    double D = quad[2].x; // f
    double E = quad[1].x; // e
    double F = quad[0].x; // d
    D += F - 2 * E; // D = d - 2*e + f
    E -= F; // E = -(d - e)
    F -= axisIntercept;
    return quadraticRootsValidT(D, 2 * E, F, roots);
}

int verticalIntersect(double axisIntercept, double top, double bottom, bool flipped) {
    addVerticalEndPoints(top, bottom, axisIntercept);
    double rootVals[2];
    int roots = verticalIntersect(axisIntercept, rootVals);
    for (int index = 0; index < roots; ++index) {
        _Point pt;
        double quadT = rootVals[index];
        xy_at_t(quad, quadT, pt.x, pt.y);
        double lineT = (pt.y - top) / (bottom - top);
        if (pinTs(quadT, lineT)) {
            intersections.insert(quadT, lineT, pt);
        }
    }
    if (flipped) {
        flip();
    }
    return intersections.fUsed;
}

protected:

// add endpoints first to get zero and one t values exactly
void addEndPoints()
{
    for (int qIndex = 0; qIndex < 3; qIndex += 2) {
        for (int lIndex = 0; lIndex < 2; lIndex++) {
            if (quad[qIndex] == line[lIndex]) {
                intersections.insert(qIndex >> 1, lIndex, line[lIndex]);
            }
        }
    }
}

void addHorizontalEndPoints(double left, double right, double y)
{
    for (int qIndex = 0; qIndex < 3; qIndex += 2) {
        if (quad[qIndex].y != y) {
            continue;
        }
        if (quad[qIndex].x == left) {
            intersections.insert(qIndex >> 1, 0, quad[qIndex]);
        }
        if (quad[qIndex].x == right) {
            intersections.insert(qIndex >> 1, 1, quad[qIndex]);
        }
    }
}

void addVerticalEndPoints(double top, double bottom, double x)
{
    for (int qIndex = 0; qIndex < 3; qIndex += 2) {
        if (quad[qIndex].x != x) {
            continue;
        }
        if (quad[qIndex].y == top) {
            intersections.insert(qIndex >> 1, 0, quad[qIndex]);
        }
        if (quad[qIndex].y == bottom) {
            intersections.insert(qIndex >> 1, 1, quad[qIndex]);
        }
    }
}

double findLineT(double t) {
    double x, y;
    xy_at_t(quad, t, x, y);
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

static bool pinTs(double& quadT, double& lineT) {
    if (!approximately_one_or_less(lineT)) {
        return false;
    }
    if (!approximately_zero_or_more(lineT)) {
        return false;
    }
    if (precisely_less_than_zero(quadT)) {
        quadT = 0;
    } else if (precisely_greater_than_one(quadT)) {
        quadT = 1;
    }
    if (precisely_less_than_zero(lineT)) {
        lineT = 0;
    } else if (precisely_greater_than_one(lineT)) {
        lineT = 1;
    }
    return true;
}

private:

const Quadratic& quad;
const _Line& line;
Intersections& intersections;
};

// utility for pairs of coincident quads
static double horizontalIntersect(const Quadratic& quad, const _Point& pt) {
    LineQuadraticIntersections q(quad, *((_Line*) 0), *((Intersections*) 0));
    double rootVals[2];
    int roots = q.horizontalIntersect(pt.y, rootVals);
    for (int index = 0; index < roots; ++index) {
        double x;
        double t = rootVals[index];
        xy_at_t(quad, t, x, *(double*) 0);
        if (AlmostEqualUlps(x, pt.x)) {
            return t;
        }
    }
    return -1;
}

static double verticalIntersect(const Quadratic& quad, const _Point& pt) {
    LineQuadraticIntersections q(quad, *((_Line*) 0), *((Intersections*) 0));
    double rootVals[2];
    int roots = q.verticalIntersect(pt.x, rootVals);
    for (int index = 0; index < roots; ++index) {
        double y;
        double t = rootVals[index];
        xy_at_t(quad, t, *(double*) 0, y);
        if (AlmostEqualUlps(y, pt.y)) {
            return t;
        }
    }
    return -1;
}

double axialIntersect(const Quadratic& q1, const _Point& p, bool vertical) {
    if (vertical) {
        return verticalIntersect(q1, p);
    }
    return horizontalIntersect(q1, p);
}

int horizontalIntersect(const Quadratic& quad, double left, double right,
        double y, double tRange[2]) {
    LineQuadraticIntersections q(quad, *((_Line*) 0), *((Intersections*) 0));
    double rootVals[2];
    int result = q.horizontalIntersect(y, rootVals);
    int tCount = 0;
    for (int index = 0; index < result; ++index) {
        double x, y;
        xy_at_t(quad, rootVals[index], x, y);
        if (x < left || x > right) {
            continue;
        }
        tRange[tCount++] = rootVals[index];
    }
    return tCount;
}

int horizontalIntersect(const Quadratic& quad, double left, double right, double y,
        bool flipped, Intersections& intersections) {
    LineQuadraticIntersections q(quad, *((_Line*) 0), intersections);
    return q.horizontalIntersect(y, left, right, flipped);
}

int verticalIntersect(const Quadratic& quad, double top, double bottom, double x,
        bool flipped, Intersections& intersections) {
    LineQuadraticIntersections q(quad, *((_Line*) 0), intersections);
    return q.verticalIntersect(x, top, bottom, flipped);
}

int intersect(const Quadratic& quad, const _Line& line, Intersections& i) {
    LineQuadraticIntersections q(quad, line, i);
    return q.intersect();
}

int intersectRay(const Quadratic& quad, const _Line& line, Intersections& i) {
    LineQuadraticIntersections q(quad, line, i);
    return q.intersectRay(i.fT[0]);
}
