#include "CubicIntersection.h"
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
        e*(1 - t)^3 + 3*f*(1 - t)^2*t  + 3*g*(1 - t)*t^2 + h*t^3 - i*x - j, x]
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
        e*(1 - t)^3 + 3*f*(1 - t)^2*t + 3*g*(1 - t)*t^2 + h*t^3 - y, y]
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
 */

class LineCubicIntersections : public Intersections {
public:

LineCubicIntersections(const Cubic& c, const _Line& l, Intersections& i)
    : cubic(c)
    , line(l)
    , intersections(i) {
}

bool intersect() {
    double slope;
    double axisIntercept;
    moreHorizontal = implicitLine(line, slope, axisIntercept);
    double A = cubic[3].x; // d
    double B = cubic[2].x * 3; // 3*c
    double C = cubic[1].x * 3; // 3*b
    double D = cubic[0].x; // a
    A -= D - C + B;     // A =   -a + 3*b - 3*c + d
    B += 3 * D - 2 * C; // B =  3*a - 6*b + 3*c
    C -= 3 * D;         // C = -3*a + 3*b
    double E = cubic[3].y; // h
    double F = cubic[2].y * 3; // 3*g
    double G = cubic[1].y * 3; // 3*f
    double H = cubic[0].y; // e
    E -= H - G + F;     // E =   -e + 3*f - 3*g + h
    F += 3 * H - 2 * G; // F =  3*e - 6*f + 3*g
    G -= 3 * H;         // G = -3*e + 3*f
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
    double t[3];
    int roots = cubicRoots(A, B, C, D, t);
    for (int x = 0; x < roots; ++x) {
        intersections.add(t[x], findLineT(t[x]));
    }
    return roots > 0;
}

protected:
    
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
    double s = 1 - t;
    double cubicVal = cPtr[0] * s * s * s + 3 * cPtr[2] * s * s * t
                + 3 * cPtr[4] * s * t * t + cPtr[6] * t * t * t;
    return (cubicVal - lPtr[0]) / (lPtr[2] - lPtr[0]);
}

private:

const Cubic& cubic;
const _Line& line;
Intersections& intersections;
bool moreHorizontal;

};
 
bool intersectStart(const Cubic& cubic, const _Line& line, Intersections& i) {
    LineCubicIntersections c(cubic, line, i);
    return c.intersect();
}
