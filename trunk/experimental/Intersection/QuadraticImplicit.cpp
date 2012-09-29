// Another approach is to start with the implicit form of one curve and solve
// (seek implicit coefficients in QuadraticParameter.cpp
// by substituting in the parametric form of the other.
// The downside of this approach is that early rejects are difficult to come by.
// http://planetmath.org/encyclopedia/GaloisTheoreticDerivationOfTheQuarticFormula.html#step


#include "CurveIntersection.h"
#include "Intersections.h"
#include "QuadraticParameterization.h"
#include "QuarticRoot.h"
#include "QuadraticUtilities.h"

/* given the implicit form 0 = Ax^2 + Bxy + Cy^2 + Dx + Ey + F
 * and given x = at^2 + bt + c  (the parameterized form)
 *           y = dt^2 + et + f
 * then
 * 0 = A(at^2+bt+c)(at^2+bt+c)+B(at^2+bt+c)(dt^2+et+f)+C(dt^2+et+f)(dt^2+et+f)+D(at^2+bt+c)+E(dt^2+et+f)+F
 */

static int findRoots(const QuadImplicitForm& i, const Quadratic& q2, double roots[4]) {
    double a, b, c;
    set_abc(&q2[0].x, a, b, c);
    double d, e, f;
    set_abc(&q2[0].y, d, e, f);
    const double t4 =     i.x2() *  a * a
                    +     i.xy() *  a * d
                    +     i.y2() *  d * d;
    const double t3 = 2 * i.x2() *  a * b
                    +     i.xy() * (a * e +     b * d)
                    + 2 * i.y2() *  d * e;
    const double t2 =     i.x2() * (b * b + 2 * a * c)
                    +     i.xy() * (c * d +     b * e + a * f)
                    +     i.y2() * (e * e + 2 * d * f)
                    +     i.x()  *  a
                    +     i.y()  *  d;
    const double t1 = 2 * i.x2() *  b * c
                    +     i.xy() * (c * e + b * f)
                    + 2 * i.y2() *  e * f
                    +     i.x()  *  b
                    +     i.y()  *  e;
    const double t0 =     i.x2() *  c * c
                    +     i.xy() *  c * f
                    +     i.y2() *  f * f
                    +     i.x()  *  c
                    +     i.y()  *  f
                    +     i.c();
    return quarticRoots(t4, t3, t2, t1, t0, roots);
}

static void addValidRoots(const double roots[4], const int count, const int side, Intersections& i) {
    int index;
    for (index = 0; index < count; ++index) {
        if (!approximately_zero_or_more(roots[index]) || !approximately_one_or_less(roots[index])) {
            continue;
        }
        double t = 1 - roots[index];
        if (approximately_less_than_zero(t)) {
            t = 0;
        } else if (approximately_greater_than_one(t)) {
            t = 1;
        }
        i.insertOne(t, side);
    }
}

bool intersect2(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
    QuadImplicitForm i1(q1);
    QuadImplicitForm i2(q2);
    if (i1.implicit_match(i2)) {
        // FIXME: compute T values
        // compute the intersections of the ends to find the coincident span
        bool useVertical = fabs(q1[0].x - q1[2].x) < fabs(q1[0].y - q1[2].y);
        double t;
        if ((t = axialIntersect(q1, q2[0], useVertical)) >= 0) {
            i.addCoincident(t, 0);
        }
        if ((t = axialIntersect(q1, q2[2], useVertical)) >= 0) {
            i.addCoincident(t, 1);
        }
        useVertical = fabs(q2[0].x - q2[2].x) < fabs(q2[0].y - q2[2].y);
        if ((t = axialIntersect(q2, q1[0], useVertical)) >= 0) {
            i.addCoincident(0, t);
        }
        if ((t = axialIntersect(q2, q1[2], useVertical)) >= 0) {
            i.addCoincident(1, t);
        }
        assert(i.fCoincidentUsed <= 2);
        return i.fCoincidentUsed > 0;
    }
    double roots1[4], roots2[4];
    int rootCount = findRoots(i2, q1, roots1);
    // OPTIMIZATION: could short circuit here if all roots are < 0 or > 1
    int rootCount2 = findRoots(i1, q2, roots2);
    assert(rootCount == rootCount2);
    addValidRoots(roots1, rootCount, 0, i);
    addValidRoots(roots2, rootCount, 1, i);
    _Point pts[4];
    bool matches[4];
    int index;
    for (index = 0; index < i.fUsed2; ++index) {
        xy_at_t(q2, i.fT[1][index], pts[index].x, pts[index].y);
        matches[index] = false;
    }
    for (index = 0; index < i.fUsed; ) {
        _Point xy;
        xy_at_t(q1, i.fT[0][index], xy.x, xy.y);
        for (int inner = 0; inner < i.fUsed2; ++inner) {
             if (approximately_equal(pts[inner].x, xy.x) && approximately_equal(pts[inner].y, xy.y)) {
                matches[index] = true;
                goto next;
             }
        }
        if (--i.fUsed > index) {
            memmove(&i.fT[0][index], &i.fT[0][index + 1], (i.fUsed - index) * sizeof(i.fT[0][0]));
            continue;
        }
    next:
        ++index;
    }
    for (index = 0; index < i.fUsed2; ) {
        if (!matches[index]) {
             if (--i.fUsed2 > index) {
                memmove(&i.fT[1][index], &i.fT[1][index + 1], (i.fUsed2 - index) * sizeof(i.fT[1][0]));
                continue;
             }
        }
        ++index;
    }
    assert(i.insertBalanced());
    return i.intersected();
}
