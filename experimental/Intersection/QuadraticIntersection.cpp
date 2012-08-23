#include "CurveIntersection.h"
#include "Intersections.h"
#include "IntersectionUtilities.h"
#include "LineIntersection.h"

class QuadraticIntersections : public Intersections {
public:

QuadraticIntersections(const Quadratic& q1, const Quadratic& q2, Intersections& i)
    : quad1(q1)
    , quad2(q2)
    , intersections(i)
    , depth(0)
    , splits(0) {
}

bool intersect() {
    double minT1, minT2, maxT1, maxT2;
    if (!bezier_clip(quad2, quad1, minT1, maxT1)) {
        return false;
    }
    if (!bezier_clip(quad1, quad2, minT2, maxT2)) {
        return false;
    }
    int split;
    if (maxT1 - minT1 < maxT2 - minT2) {
        intersections.swap();
        minT2 = 0;
        maxT2 = 1;
        split = maxT1 - minT1 > tClipLimit;
    } else {
        minT1 = 0;
        maxT1 = 1;
        split = (maxT2 - minT2 > tClipLimit) << 1;
    }
    return chop(minT1, maxT1, minT2, maxT2, split);
}

protected:

bool intersect(double minT1, double maxT1, double minT2, double maxT2) {
    Quadratic smaller, larger;
    // FIXME: carry last subdivide and reduceOrder result with quad
    sub_divide(quad1, minT1, maxT1, intersections.swapped() ? larger : smaller);
    sub_divide(quad2, minT2, maxT2, intersections.swapped() ? smaller : larger);
    Quadratic smallResult;
    if (reduceOrder(smaller, smallResult) <= 2) {
        Quadratic largeResult;
        if (reduceOrder(larger, largeResult) <= 2) {
            double smallT[2], largeT[2];
            const _Line& smallLine = (const _Line&) smallResult;
            const _Line& largeLine = (const _Line&) largeResult;
            // FIXME: this doesn't detect or deal with coincident lines
            if (!::intersect(smallLine, largeLine, smallT, largeT)) {
                return false;
            }
            if (intersections.swapped()) {
                smallT[0] = interp(minT2, maxT2, smallT[0]);
                largeT[0] = interp(minT1, maxT1, largeT[0]);
            } else {
                smallT[0] = interp(minT1, maxT1, smallT[0]);
                largeT[0] = interp(minT2, maxT2, largeT[0]);
            }
            intersections.add(smallT[0], largeT[0]);
            return true;
        }
    }
    double minT, maxT;
    if (!bezier_clip(smaller, larger, minT, maxT)) {
        if (minT == maxT) {
            if (intersections.swapped()) {
                minT1 = (minT1 + maxT1) / 2;
                minT2 = interp(minT2, maxT2, minT);
            } else {
                minT1 = interp(minT1, maxT1, minT);
                minT2 = (minT2 + maxT2) / 2;
            }
            intersections.add(minT1, minT2);
            return true;
        }
        return false;
    }

    int split;
    if (intersections.swapped()) {
        double newMinT1 = interp(minT1, maxT1, minT);
        double newMaxT1 = interp(minT1, maxT1, maxT);
        split = (newMaxT1 - newMinT1 > (maxT1 - minT1) * tClipLimit) << 1;
#define VERBOSE 0
#if VERBOSE
        printf("%s d=%d s=%d new1=(%g,%g) old1=(%g,%g) split=%d\n", __FUNCTION__, depth,
            splits, newMinT1, newMaxT1, minT1, maxT1, split);
#endif
        minT1 = newMinT1;
        maxT1 = newMaxT1;
    } else {
        double newMinT2 = interp(minT2, maxT2, minT);
        double newMaxT2 = interp(minT2, maxT2, maxT);
        split = newMaxT2 - newMinT2 > (maxT2 - minT2) * tClipLimit;
#if VERBOSE
        printf("%s d=%d s=%d new2=(%g,%g) old2=(%g,%g) split=%d\n", __FUNCTION__, depth,
            splits, newMinT2, newMaxT2, minT2, maxT2, split);
#endif
        minT2 = newMinT2;
        maxT2 = newMaxT2;
    }
    return chop(minT1, maxT1, minT2, maxT2, split);
}

bool chop(double minT1, double maxT1, double minT2, double maxT2, int split) {
    ++depth;
    intersections.swap();
    if (split) {
        ++splits;
        if (split & 2) {
            double middle1 = (maxT1 + minT1) / 2;
            intersect(minT1, middle1, minT2, maxT2);
            intersect(middle1, maxT1, minT2, maxT2);
        } else {
            double middle2 = (maxT2 + minT2) / 2;
            intersect(minT1, maxT1, minT2, middle2);
            intersect(minT1, maxT1, middle2, maxT2);
        }
        --splits;
        intersections.swap();
        --depth;
        return intersections.intersected();
    }
    bool result = intersect(minT1, maxT1, minT2, maxT2);
    intersections.swap();
    --depth;
    return result;
}

private:

static const double tClipLimit = 0.8; // http://cagd.cs.byu.edu/~tom/papers/bezclip.pdf see Multiple intersections
const Quadratic& quad1;
const Quadratic& quad2;
Intersections& intersections;
int depth;
int splits;
};

bool intersect(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
    if (implicit_matches(q1, q2)) {
        // FIXME: compute T values
        // compute the intersections of the ends to find the coincident span
        bool useVertical = fabs(q1[0].x - q1[2].x) < fabs(q1[0].y - q1[2].y);
        double t;
        if ((t = axialIntersect(q1, q2[0], useVertical)) >= 0) {
            i.fT[0][0] = t;
            i.fT[1][0] = 0;
            i.fUsed++;
        }
        if ((t = axialIntersect(q1, q2[2], useVertical)) >= 0) {
            i.fT[0][i.fUsed] = t;
            i.fT[1][i.fUsed] = 1;
            i.fUsed++;
        }
        useVertical = fabs(q2[0].x - q2[2].x) < fabs(q2[0].y - q2[2].y);
        if ((t = axialIntersect(q2, q1[0], useVertical)) >= 0) {
            i.fT[0][i.fUsed] = 0;
            i.fT[1][i.fUsed] = t;
            i.fUsed++;
        }
        if ((t = axialIntersect(q2, q1[2], useVertical)) >= 0) {
            i.fT[0][i.fUsed] = 1;
            i.fT[1][i.fUsed] = t;
            i.fUsed++;
        }
        assert(i.fUsed <= 2);
        return i.fUsed > 0;
    }
    QuadraticIntersections q(q1, q2, i);
    return q.intersect();
}


// Another approach is to start with the implicit form of one curve and solve
// by substituting in the parametric form of the other.
// The downside of this approach is that early rejects are difficult to come by.
// http://planetmath.org/encyclopedia/GaloisTheoreticDerivationOfTheQuarticFormula.html#step
/*
given x^4 + ax^3 + bx^2 + cx + d
the resolvent cubic is x^3 - 2bx^2 + (b^2 + ac - 4d)x + (c^2 + a^2d - abc)
use the cubic formula (CubicRoots.cpp) to find the radical expressions t1, t2, and t3.

(x - r1 r2) (x - r3 r4) = x^2 - (t2 + t3 - t1) / 2 x + d
s = r1*r2 = ((t2 + t3 - t1) + sqrt((t2 + t3 - t1)^2 - 16*d)) / 4
t = r3*r4 = ((t2 + t3 - t1) - sqrt((t2 + t3 - t1)^2 - 16*d)) / 4

u = r1+r2 = (-a + sqrt(a^2 - 4*t1)) / 2
v = r3+r4 = (-a - sqrt(a^2 - 4*t1)) / 2

r1 = (u + sqrt(u^2 - 4*s)) / 2
r2 = (u - sqrt(u^2 - 4*s)) / 2
r3 = (v + sqrt(v^2 - 4*t)) / 2
r4 = (v - sqrt(v^2 - 4*t)) / 2
*/


/* square root of complex number
http://en.wikipedia.org/wiki/Square_root#Square_roots_of_negative_and_complex_numbers
Algebraic formula
When the number is expressed using Cartesian coordinates the following formula
 can be used for the principal square root:[5][6]

sqrt(x + iy) = sqrt((r + x) / 2) +/- i*sqrt((r - x) / 2)

where the sign of the imaginary part of the root is taken to be same as the sign
 of the imaginary part of the original number, and

r = abs(x + iy) = sqrt(x^2 + y^2)

is the absolute value or modulus of the original number. The real part of the
principal value is always non-negative.
The other square root is simply –1 times the principal square root; in other
words, the two square roots of a number sum to 0.
 */
