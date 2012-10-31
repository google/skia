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

static bool onlyEndPtsInCommon(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
// the idea here is to see at minimum do a quick reject by rotating all points
// to either side of the line formed by connecting the endpoints
// if the opposite curves points are on the line or on the other side, the
// curves at most intersect at the endpoints
    for (int oddMan = 0; oddMan < 3; ++oddMan) {
        const _Point* endPt[2];
        for (int opp = 1; opp < 3; ++opp) {
            int end = oddMan ^ opp;
            if (end == 3) {
                end = opp;
            }
            endPt[opp - 1] = &q1[end];
        }
        double origX = endPt[0]->x;
        double origY = endPt[0]->y;
        double adj = endPt[1]->x - origX;
        double opp = endPt[1]->y - origY;
        double sign = (q1[oddMan].y - origY) * adj - (q1[oddMan].x - origX) * opp;
        assert(!approximately_zero(sign));
        for (int n = 0; n < 3; ++n) {
            double test = (q2[n].y - origY) * adj - (q2[n].x - origX) * opp;
            if (test * sign > 0) {
                goto tryNextHalfPlane;
            }
        }
        for (int i1 = 0; i1 < 3; i1 += 2) {
            for (int i2 = 0; i2 < 3; i2 += 2) {
                if (q1[i1] == q2[i2]) {
                    i.insert(i1 >> 1, i2 >> 1);
                }
            }
        }
        assert(i.fUsed < 3);
        return true;
tryNextHalfPlane:
        ;
    }
    return false;
}

bool intersect2(const Quadratic& q1, const Quadratic& q2, Intersections& i) {
    // if the quads share an end point, check to see if they overlap

    if (onlyEndPtsInCommon(q1, q2, i)) {
        return i.intersected();
    }
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
#ifndef NDEBUG
    int rootCount2 =
#endif
        findRoots(i1, q2, roots2);
    assert(rootCount == rootCount2);
    addValidRoots(roots1, rootCount, 0, i);
    addValidRoots(roots2, rootCount, 1, i);
    if (i.insertBalanced() && i.fUsed <= 1) {
        if (i.fUsed == 1) {
            _Point xy1, xy2;
            xy_at_t(q1, i.fT[0][0], xy1.x, xy1.y);
            xy_at_t(q2, i.fT[1][0], xy2.x, xy2.y);
            if (!xy1.approximatelyEqual(xy2)) {
                --i.fUsed;
                --i.fUsed2;
            }
        }
        return i.intersected();
    }
    _Point pts[4];
    bool matches[4];
    int flipCheck[4];
    int closest[4];
    double dist[4];
    int index, ndex2;
    int flipIndex = 0;
    for (ndex2 = 0; ndex2 < i.fUsed2; ++ndex2) {
        xy_at_t(q2, i.fT[1][ndex2], pts[ndex2].x, pts[ndex2].y);
        matches[ndex2] = false;
    }
    for (index = 0; index < i.fUsed; ++index) {
        _Point xy;
        xy_at_t(q1, i.fT[0][index], xy.x, xy.y);
        dist[index] = DBL_MAX;
        closest[index] = -1;
        for (ndex2 = 0; ndex2 < i.fUsed2; ++ndex2) {
            if (!pts[ndex2].approximatelyEqual(xy)) {
                continue;
            }
            double dx = pts[ndex2].x - xy.x;
            double dy = pts[ndex2].y - xy.y;
            double distance = dx * dx + dy * dy;
            if (dist[index] <= distance) {
                continue;
            }
            for (int outer = 0; outer < index; ++outer) {
                if (closest[outer] != ndex2) {
                    continue;
                }
                if (dist[outer] < distance) {
                    goto next;
                }
                closest[outer] = -1;
            }
            dist[index] = distance;
            closest[index] = ndex2;
        next:
            ;
        }
    }
    for (index = 0; index < i.fUsed; ) {
        for (ndex2 = 0; ndex2 < i.fUsed2; ++ndex2) {
             if (closest[index] == ndex2) {
                assert(flipIndex < 4);
                flipCheck[flipIndex++] = ndex2;
                matches[ndex2] = true;
                goto next2;
             }
        }
        if (--i.fUsed > index) {
            memmove(&i.fT[0][index], &i.fT[0][index + 1], (i.fUsed - index) * sizeof(i.fT[0][0]));
            memmove(&closest[index], &closest[index + 1], (i.fUsed - index) * sizeof(closest[0]));
            continue;
        }
    next2:
        ++index;
    }
    for (ndex2 = 0; ndex2 < i.fUsed2; ) {
        if (!matches[ndex2]) {
             if (--i.fUsed2 > ndex2) {
                memmove(&i.fT[1][ndex2], &i.fT[1][ndex2 + 1], (i.fUsed2 - ndex2) * sizeof(i.fT[1][0]));
                memmove(&matches[ndex2], &matches[ndex2 + 1], (i.fUsed2 - ndex2) * sizeof(matches[0]));
                continue;
             }
        }
        ++ndex2;
    }
    i.fFlip = i.fUsed >= 2 && flipCheck[0] > flipCheck[1];
    assert(i.insertBalanced());
    return i.intersected();
}
