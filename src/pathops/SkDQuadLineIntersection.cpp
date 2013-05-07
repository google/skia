/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkIntersections.h"
#include "SkPathOpsLine.h"
#include "SkPathOpsQuad.h"

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
    LineQuadraticIntersections(const SkDQuad& q, const SkDLine& l, SkIntersections* i)
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
        A = line[1].fX - line[0].fX (adjacent side of the right triangle)
        O = line[1].fY - line[0].fY (opposite side of the right triangle)
        for each of the three points (e.g. n = 0 to 2)
        quad[n].fY' = (quad[n].fY - line[0].fY) * A - (quad[n].fX - line[0].fX) * O
    */
        double adj = line[1].fX - line[0].fX;
        double opp = line[1].fY - line[0].fY;
        double r[3];
        for (int n = 0; n < 3; ++n) {
            r[n] = (quad[n].fY - line[0].fY) * adj - (quad[n].fX - line[0].fX) * opp;
        }
        double A = r[2];
        double B = r[1];
        double C = r[0];
        A += C - 2 * B;  // A = a - 2*b + c
        B -= C;  // B = -(b - c)
        return SkDQuad::RootsValidT(A, 2 * B, C, roots);
    }

    int intersect() {
        addEndPoints();
        double rootVals[2];
        int roots = intersectRay(rootVals);
        for (int index = 0; index < roots; ++index) {
            double quadT = rootVals[index];
            double lineT = findLineT(quadT);
            if (PinTs(&quadT, &lineT)) {
                SkDPoint pt = line.xyAtT(lineT);
                intersections->insert(quadT, lineT, pt);
            }
        }
        return intersections->used();
    }

    int horizontalIntersect(double axisIntercept, double roots[2]) {
        double D = quad[2].fY;  // f
        double E = quad[1].fY;  // e
        double F = quad[0].fY;  // d
        D += F - 2 * E;         // D = d - 2*e + f
        E -= F;                 // E = -(d - e)
        F -= axisIntercept;
        return SkDQuad::RootsValidT(D, 2 * E, F, roots);
    }

    int horizontalIntersect(double axisIntercept, double left, double right, bool flipped) {
        addHorizontalEndPoints(left, right, axisIntercept);
        double rootVals[2];
        int roots = horizontalIntersect(axisIntercept, rootVals);
        for (int index = 0; index < roots; ++index) {
            double quadT = rootVals[index];
            SkDPoint pt = quad.xyAtT(quadT);
            double lineT = (pt.fX - left) / (right - left);
            if (PinTs(&quadT, &lineT)) {
                intersections->insert(quadT, lineT, pt);
            }
        }
        if (flipped) {
            intersections->flip();
        }
        return intersections->used();
    }

    int verticalIntersect(double axisIntercept, double roots[2]) {
        double D = quad[2].fX;  // f
        double E = quad[1].fX;  // e
        double F = quad[0].fX;  // d
        D += F - 2 * E;         // D = d - 2*e + f
        E -= F;                 // E = -(d - e)
        F -= axisIntercept;
        return SkDQuad::RootsValidT(D, 2 * E, F, roots);
    }

    int verticalIntersect(double axisIntercept, double top, double bottom, bool flipped) {
        addVerticalEndPoints(top, bottom, axisIntercept);
        double rootVals[2];
        int roots = verticalIntersect(axisIntercept, rootVals);
        for (int index = 0; index < roots; ++index) {
            double quadT = rootVals[index];
            SkDPoint pt = quad.xyAtT(quadT);
            double lineT = (pt.fY - top) / (bottom - top);
            if (PinTs(&quadT, &lineT)) {
                intersections->insert(quadT, lineT, pt);
            }
        }
        if (flipped) {
            intersections->flip();
        }
        return intersections->used();
    }

protected:
    // add endpoints first to get zero and one t values exactly
    void addEndPoints() {
        for (int qIndex = 0; qIndex < 3; qIndex += 2) {
            for (int lIndex = 0; lIndex < 2; lIndex++) {
                if (quad[qIndex] == line[lIndex]) {
                    intersections->insert(qIndex >> 1, lIndex, line[lIndex]);
                }
            }
        }
    }

    void addHorizontalEndPoints(double left, double right, double y) {
        for (int qIndex = 0; qIndex < 3; qIndex += 2) {
            if (quad[qIndex].fY != y) {
                continue;
            }
            if (quad[qIndex].fX == left) {
                intersections->insert(qIndex >> 1, 0, quad[qIndex]);
            }
            if (quad[qIndex].fX == right) {
                intersections->insert(qIndex >> 1, 1, quad[qIndex]);
            }
        }
    }

    void addVerticalEndPoints(double top, double bottom, double x) {
        for (int qIndex = 0; qIndex < 3; qIndex += 2) {
            if (quad[qIndex].fX != x) {
                continue;
            }
            if (quad[qIndex].fY == top) {
                intersections->insert(qIndex >> 1, 0, quad[qIndex]);
            }
            if (quad[qIndex].fY == bottom) {
                intersections->insert(qIndex >> 1, 1, quad[qIndex]);
            }
        }
    }

    double findLineT(double t) {
        SkDPoint xy = quad.xyAtT(t);
        double dx = line[1].fX - line[0].fX;
        double dy = line[1].fY - line[0].fY;
        if (fabs(dx) > fabs(dy)) {
            return (xy.fX - line[0].fX) / dx;
        }
        return (xy.fY - line[0].fY) / dy;
    }

    static bool PinTs(double* quadT, double* lineT) {
        if (!approximately_one_or_less(*lineT)) {
            return false;
        }
        if (!approximately_zero_or_more(*lineT)) {
            return false;
        }
        if (precisely_less_than_zero(*quadT)) {
            *quadT = 0;
        } else if (precisely_greater_than_one(*quadT)) {
            *quadT = 1;
        }
        if (precisely_less_than_zero(*lineT)) {
            *lineT = 0;
        } else if (precisely_greater_than_one(*lineT)) {
            *lineT = 1;
        }
        return true;
    }

private:
    const SkDQuad& quad;
    const SkDLine& line;
    SkIntersections* intersections;
};

// utility for pairs of coincident quads
static double horizontalIntersect(const SkDQuad& quad, const SkDPoint& pt) {
    LineQuadraticIntersections q(quad, *(static_cast<SkDLine*>(0)),
            static_cast<SkIntersections*>(0));
    double rootVals[2];
    int roots = q.horizontalIntersect(pt.fY, rootVals);
    for (int index = 0; index < roots; ++index) {
        double t = rootVals[index];
        SkDPoint qPt = quad.xyAtT(t);
        if (AlmostEqualUlps(qPt.fX, pt.fX)) {
            return t;
        }
    }
    return -1;
}

static double verticalIntersect(const SkDQuad& quad, const SkDPoint& pt) {
    LineQuadraticIntersections q(quad, *(static_cast<SkDLine*>(0)),
            static_cast<SkIntersections*>(0));
    double rootVals[2];
    int roots = q.verticalIntersect(pt.fX, rootVals);
    for (int index = 0; index < roots; ++index) {
        double t = rootVals[index];
        SkDPoint qPt = quad.xyAtT(t);
        if (AlmostEqualUlps(qPt.fY, pt.fY)) {
            return t;
        }
    }
    return -1;
}

double SkIntersections::Axial(const SkDQuad& q1, const SkDPoint& p, bool vertical) {
    if (vertical) {
        return verticalIntersect(q1, p);
    }
    return horizontalIntersect(q1, p);
}

int SkIntersections::horizontal(const SkDQuad& quad, double left, double right, double y,
                                bool flipped) {
    LineQuadraticIntersections q(quad, *(static_cast<SkDLine*>(0)), this);
    return q.horizontalIntersect(y, left, right, flipped);
}

int SkIntersections::vertical(const SkDQuad& quad, double top, double bottom, double x,
                              bool flipped) {
    LineQuadraticIntersections q(quad, *(static_cast<SkDLine*>(0)), this);
    return q.verticalIntersect(x, top, bottom, flipped);
}

int SkIntersections::intersect(const SkDQuad& quad, const SkDLine& line) {
    LineQuadraticIntersections q(quad, line, this);
    return q.intersect();
}

int SkIntersections::intersectRay(const SkDQuad& quad, const SkDLine& line) {
    LineQuadraticIntersections q(quad, line, this);
    fUsed = q.intersectRay(fT[0]);
    for (int index = 0; index < fUsed; ++index) {
        fPt[index] = quad.xyAtT(fT[0][index]);
    }
    return fUsed;
}
