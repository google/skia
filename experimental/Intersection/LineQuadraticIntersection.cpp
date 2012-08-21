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
 

class LineQuadraticIntersections : public Intersections {
public:

LineQuadraticIntersections(const Quadratic& q, const _Line& l, Intersections& i)
    : quad(q)
    , line(l)
    , intersections(i) {
}

bool intersect() {
    double slope;
    double axisIntercept;
    moreHorizontal = implicitLine(line, slope, axisIntercept);
    double A = quad[2].x; // c
    double B = quad[1].x; // b
    double C = quad[0].x; // a
    A += C - 2 * B; // A = a - 2*b + c
    B -= C; // B = -(a - b)
    double D = quad[2].y; // f
    double E = quad[1].y; // e
    double F = quad[0].y; // d
    D += F - 2 * E; // D = d - 2*e + f
    E -= F; // E = -(d - e)
    if (moreHorizontal) {
        A = A * slope - D;
        B = B * slope - E;
        C = C * slope - F + axisIntercept;
    } else {
        A = A - D * slope;
        B = B - E * slope;
        C = C - F * slope - axisIntercept;
    }
    double t[2];
    int roots = quadraticRoots(A, B, C, t);
    for (int x = 0; x < roots; ) {
        double lineT = findLineT(t[x]);
        if (lineT <= -FLT_EPSILON || lineT >= 1 + FLT_EPSILON) {
            if (x < --roots) {
                t[x] = t[roots];
            }
            continue;
        }
        if (lineT < FLT_EPSILON) {
            lineT = 0;
        } else if (lineT > 1 - FLT_EPSILON) {
            lineT = 1;
        }
        intersections.add(t[x], lineT);
        ++x;
    }
    return roots > 0;
}

int horizontalIntersect(double axisIntercept) {
    double D = quad[2].y; // f
    double E = quad[1].y; // e
    double F = quad[0].y; // d
    D += F - 2 * E; // D = d - 2*e + f
    E -= F; // E = -(d - e)
    F -= axisIntercept;
    return quadraticRoots(D, E, F, intersections.fT[0]);
}

int verticalIntersect(double axisIntercept) {
    double D = quad[2].x; // f
    double E = quad[1].x; // e
    double F = quad[0].x; // d
    D += F - 2 * E; // D = d - 2*e + f
    E -= F; // E = -(d - e)
    F -= axisIntercept;
    return quadraticRoots(D, E, F, intersections.fT[0]);
}

protected:
    
double findLineT(double t) {
    const double* qPtr;
    const double* lPtr;
    if (moreHorizontal) {
        qPtr = &quad[0].x;
        lPtr = &line[0].x;
    } else {
        qPtr = &quad[0].y;
        lPtr = &line[0].y;
    }
    double s = 1 - t;
    double quadVal = qPtr[0] * s * s + 2 * qPtr[2] * s * t + qPtr[4] * t * t;
    return (quadVal - lPtr[0]) / (lPtr[2] - lPtr[0]);
}

private:

const Quadratic& quad;
const _Line& line;
Intersections& intersections;
bool moreHorizontal;

};

// utility for pairs of coincident quads
static double horizontalIntersect(const Quadratic& quad, const _Point& pt) {
    Intersections intersections;
    LineQuadraticIntersections q(quad, *((_Line*) 0), intersections);
    int result = q.horizontalIntersect(pt.y);
    if (result == 0) {
        return -1;
    }
    assert(result == 1);
    double x, y;
    xy_at_t(quad, intersections.fT[0][0], x, y);
    if (approximately_equal(x, pt.x)) {
        return intersections.fT[0][0];
    }
    return -1;
}

static double verticalIntersect(const Quadratic& quad, const _Point& pt) {
    Intersections intersections;
    LineQuadraticIntersections q(quad, *((_Line*) 0), intersections);
    int result = q.horizontalIntersect(pt.x);
    if (result == 0) {
        return -1;
    }
    assert(result == 1);
    double x, y;
    xy_at_t(quad, intersections.fT[0][0], x, y);
    if (approximately_equal(y, pt.y)) {
        return intersections.fT[0][0];
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
    Intersections i;
    LineQuadraticIntersections q(quad, *((_Line*) 0), i);
    int result = q.horizontalIntersect(y);
    int tCount = 0;
    for (int index = 0; index < result; ++index) {
        double x, y;
        xy_at_t(quad, i.fT[0][index], x, y);
        if (x < left || x > right) {
            continue;
        }
        tRange[tCount++] = i.fT[0][index];
    }
    return tCount;
}

int horizontalIntersect(const Quadratic& quad, double left, double right, double y,
        bool flipped, Intersections& intersections) {
    LineQuadraticIntersections q(quad, *((_Line*) 0), intersections);
    int result = q.horizontalIntersect(y);
    for (int index = 0; index < result; ) {
        double x, y;
        xy_at_t(quad, intersections.fT[0][index], x, y);
        double lineT = (x - left) / (right - left);
        if (lineT <= -FLT_EPSILON || lineT >= 1 + FLT_EPSILON) {
            if (--result > index) {
                intersections.fT[0][index] = intersections.fT[0][result];
            }
            continue;
        }
        if (lineT < FLT_EPSILON) {
            lineT = 0;
        } else if (lineT > 1 - FLT_EPSILON) {
            lineT = 1;
        }
        intersections.fT[1][index] = lineT;
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

int verticalIntersect(const Quadratic& quad, double top, double bottom, double x,
        bool flipped, Intersections& intersections) {
    LineQuadraticIntersections q(quad, *((_Line*) 0), intersections);
    int result = q.verticalIntersect(x);
    for (int index = 0; index < result; ) {
        double x, y;
        xy_at_t(quad, intersections.fT[0][index], x, y);
        double lineT = (y - top) / (bottom - top);
        if (lineT <= -FLT_EPSILON || lineT >= 1 + FLT_EPSILON) {
            if (--result > index) {
                intersections.fT[0][index] = intersections.fT[0][result];
            }
            continue;
        }
        if (lineT < FLT_EPSILON) {
            lineT = 0;
        } else if (lineT > 1 - FLT_EPSILON) {
            lineT = 1;
        }
        intersections.fT[1][index] = lineT;
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

bool intersect(const Quadratic& quad, const _Line& line, Intersections& i) {
    LineQuadraticIntersections q(quad, line, i);
    return q.intersect();
}
