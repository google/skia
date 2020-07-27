/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkStrokerPriv.h"

#include <utility>

SkStrokerPriv::SkStrokerPriv(uint8_t cap, uint8_t join, SkScalar radius,
        SkScalar joinLimit) : fTempJoin(0) {
    this->setCap(cap);
    this->setJoin(join);
    this->setRadius(radius);
    this->setJoinLimit(joinLimit);
}

bool SkStrokerPriv::calcUnitNormal(SkScalar x, SkScalar y, SkVector* un,
        SkVector* r) {
    bool ret = true;

    if (!un->setNormalize(x, y)) {
        /* Square caps and round caps draw even if the segment length
           is zero so set "upright" as the default orientation. */
        un->set(1, 0);
        ret = false;
    } else {
        SkPointPriv::RotateCCW(un);
    }

    if (r != NULL) {
        *r = *un;
    }
    return ret;
}

/* Some of the following are partially duplicated in SkPointPriv.h
   but with lines specified by two points rather than a point and
   a unit vector. */

static inline void project_point_on_line(const SkPoint& l, const SkVector& uv,
        const SkPoint& p, SkPoint* pp) {
    *pp = l + uv * uv.dot(p - l);
}

static inline SkScalar line_point_distance(const SkPoint& l,
        const SkVector& uv, const SkPoint& p) {
    SkPoint pp;
    project_point_on_line(l, uv, p, &pp);
    return SkPoint::Distance(p, pp);
}

/* The line including the segment l1 <-> l2 defines two half-planes and the
   set of points that are (approximately) colinear.  line_side() returns
   -1, 1, or 0 depending on which area p falls on (0 indicates colinearity).

   While the signed value is not aribrary this function is intended to be
   called first with a reference point and that return value compared with
   the value for the point in question. */
static inline int line_side(const SkPoint& l1, const SkPoint& l2,
    const SkPoint& p) {
    SkScalar t = (l2.fX - l1.fX) * (p.fY - l1.fY) -
                 (p.fX - l1.fX) * (l2.fY - l1.fY);
    if (SkScalarAbs(t) < 1e-5) {
        return 0;
    }
    return std::copysign(1, t);
}

/* Output parameter cp is assigned the value of p1 or p2 depending
   on which is closer to point r. */
static inline void closer_point(const SkPoint &r, const SkPoint& p1,
        const SkPoint &p2, SkPoint *cp) {
    *cp = SkPointPriv::DistanceToSqd(r, p1) > SkPointPriv::DistanceToSqd(r, p2) ? p2 : p1;
}

static const SkScalar SK_Scalar2PI = SkIntToScalar(2) * SK_ScalarPI;

/* Normalizes a radian-valued angle a between 0 and 2*PI. */
static inline SkScalar norm_radians_0_2PI(SkScalar a) {

    SkScalar b = a - SK_Scalar2PI * SkScalarFloorToScalar(a / SK_Scalar2PI);
    return b;
}

/* i is assigned the value of i1 or i2 depending on which point is
      1) on the same side of the line l1 and l2 as point r, and
      2) closer to point l1
    When l1 and l2 are the offset end and start points (on either side of
    the bevel line segment) this is a good heuristic for picking the closer
    of two circle intersections for an Arcs join. */
void SkStrokerPriv::closerIntersection(const SkPoint& l1, const SkPoint& l2,
        const SkPoint& r, const SkPoint& i1, const SkPoint& i2, SkPoint* i) {
    int lsr = line_side(l1, l2, r);
    int ls1 = line_side(l1, l2, i1);
    int ls2 = line_side(l1, l2, i2);
    SkASSERT(lsr != 0);
    if (ls1 == lsr) {
        if (ls2 == lsr) {
            closer_point(l1, i1, i2, i);
        } else {
            *i = i1;
        }
    } else {
        SkASSERT(ls2 == lsr);
        *i = i2;
    }
}

/* Given a line defined by point l and unit vector uv, and a circle
   defined by center c and radius r, returns the number of intersections
   of the line and circle. */
int SkStrokerPriv::lineCircleTest(const SkPoint &l, const SkVector& uv,
         const SkPoint& c, const SkScalar r) {
    SkScalar clDist = line_point_distance(l, uv, c);
    if (SkScalarAbs(clDist - r) < 1e-2) {
        return 1;
    } else if (clDist > r) {
        return 0;
    }
    return 2;
}

/* Given two circles defined by centers c1 and c2 and radii r1 and r2
   respectively, returns the number of intersections of the two circles.

   If there are no intersections and the circles do not overlap, returns 0

   If r2 is inside r1 returns -1 and if r1 is inside r2 returns -2. */
int SkStrokerPriv::circlesTest(const SkPoint& c1, SkScalar r1, const SkPoint& c2,
        SkScalar r2) {
    SkScalar cDist = SkPoint::Distance(c1, c2);
    if (SkScalarAbs(cDist - r1 - r2) < 1e-2) {
        return 1;
    } else if (cDist > r1 + r2) {
        return 0;
    } else if (cDist < SkScalarAbs(r1 - r2)) {
        return (r1 > r2) ? -1 : -2;
    }
    return 2;
}

bool SkStrokerPriv::intersectLines(const SkPoint& l1, const SkVector& uv1,
        const SkPoint& l2, const SkVector& uv2, SkPoint* i) {
    SkScalar cl1 = uv1.cross(l1), cl2 = uv2.cross(l2), cd = uv1.cross(uv2);
    /* XXX Could be too conservative -- depends on lower-end of typical scale
       of calculation being in the neighborhood of .1 and the size of a
       canvas being limited to say 100000 units or so. */
    if (SkScalarAbs(cd) < 1e-8) {
        return false;
    }
    i->set((cl1*uv2.fX - cl2*uv1.fX)/cd, (cl1*uv2.fY - cl2*uv1.fY)/cd);
    return true;
}

/* Given a line defined by point l and unit vector uv, and a circle
   defined by center c and radius r, i1 and i2 are set to the intersections
   between the line and circle. When there is one intersection both are
   assigned the same value.

   The values are only meaningful when there is an intersection so
   line_circle_test() should be called first. */
void SkStrokerPriv::intersectLineCircle(const SkPoint& l, const SkVector& uv,
        const SkPoint& c, SkScalar r, SkPoint* i1, SkPoint* i2) {
    SkPoint pp;
    project_point_on_line(l, uv, c, &pp);
    /* Right triangle with points center, closest point on line, and one
       intersection. "b" is the segment connecting the intersection and
       the projected point. */
    SkScalar bLenSq = r * r - SkPointPriv::DistanceToSqd(c, pp);
    if (bLenSq < 1e-4) {
        *i1 = *i2 = pp;
    } else {
        SkVector v;
        uv.scale(SkScalarSqrt(bLenSq), &v);
        *i1 = pp + v;
        *i2 = pp - v;
    }
}

/* Given two circles defined by centers c1 and c2 and radii r1 and r2
   respectively, i1 and i2 are set to the intersections between the circles.
   When there is one intersection both are assigned the same value.

   The values are only meaningful when there is an intersection so
   circles_test() should be called first.

   Derviation: Two right triangles r1, a1, b and r2, a2, b, where
   segments a1 and a2 make up the line between the centers and b
   is perpendicular and connects that line to one intersection.

     cDist = dist(c1, c2)
     a2Len = cDist - a1Len

     a1Len^2 + bLen^2 == r1^2
     a2Len^2 + bLen^2 == r2^2

     r1^2 - a1Len^2 == r2^2 - a2Len^2
     r1^2 - a1Len^2 == r2^2 - cDist^2 - 2*cDist*a1Len + a1Len^2

     a1Len = (r1^2 - r2^2 - cDist^2)/(2*cDist)
 */

void SkStrokerPriv::intersectCircles(const SkPoint& c1, SkScalar r1,
        const SkPoint& c2, SkScalar r2, SkPoint* i1, SkPoint* i2) {
    SkVector cDiff(c2 - c1), v1, iV;
    SkScalar cDistSq = SkPointPriv::LengthSqd(cDiff);
    SkScalar cDist = SkScalarSqrt(cDistSq), r1Sq = r1*r1;
    cDiff.scale(SkScalarInvert(cDist), &v1);
    SkPointPriv::RotateCCW(v1, &iV);
    SkScalar a1Len = SK_ScalarHalf*(r1Sq - r2*r2 + cDistSq)*SkScalarInvert(cDist);
    // Point separating segments a1 and a2
    SkPoint iP(c1 + v1*a1Len);
    SkScalar bLenSq = r1Sq - a1Len*a1Len, bLen;
    if (bLenSq < 1e-8) {
        bLen = 0;
    } else {
        bLen = SkScalarSqrt(bLenSq);
    }
    // Offset to intersection
    iV.scale(bLen);
    *i1 = iP + iV;
    *i2 = iP - iV;
}

/* Given a line defined by point l and unit vector uv, and a set of potential
   circles intersecting point p centered on the ray defined by p and cuv,
   returns the radius of the circle just touching the line. */
SkScalar SkStrokerPriv::adjustLineCircle(const SkPoint& l, const SkVector& uv,
        const SkPoint& p, const SkVector& cuv) {
    SkScalar b = uv.cross(l - p), a = cuv.cross(uv);
    SkScalar r = b/(SK_Scalar1 - a);
    if (!SkScalarIsFinite(r) || r < 0) {
        return -b/(SK_Scalar1 + a);
    }
    return r;
}

/* Given circles respectively intersecting points p1 and p2, with centers
   p1 + uv1*r1 and p2 + uv2*r2, adjustCircles() modifies the values of
   r1 and r2 by the same amount so that the circles are just touching.

   The circles are assumed to not already intersect, so circlesTest should
   be called first. When the circles do not overlap inside must be false.
   If they do inside must be true and r2 must be inside r1.

   Derivations:
     No overlap:
       c1 == p1 + (r1 + e) * uv1
       c2 == p2 + (r2 + e) * uv2
       c1 - c2 == p1 - p2 + r1*uv1 - r2*uv2 + (uv1 - uv2)*e
       A == p1 - p2 + r1*uv1 - r2*uv2
       B == uv1 - uv2
       c1 - c2 == A + B*e

       norm(c1 - c2) == (r1 + e) + (r2 + e)
       C = r1 + r2
       D = 2
       norm(c1 - c2) == C + D*e

      First encloses second:
       c1 == p1 + (r1 - e) * uv1
       c2 == p2 + (r2 + e) * uv2
       c1 - c2 == p1 - p2 + r1*uv1 - r2*uv2 + (-uv1 - uv2)*e
       A == p1 - p2 + r1*uv1 - r2*uv2
       B == -uv1 - uv2
       c1 - c2 == A + B*e

       norm(c1 - c2) == (r1 - e) - (r2 + e)
       C = r1 - r2
       D = -2
       norm(c1 - c2) == C + D*e

      Factorization:
       norm(c1 - c2)^2 == (C + D*e)^2
       (A + B*e).x^2 + (A + B*e).y^2 == C^2 + 2*C*D*e + D^2*e^2
       (B.x^2 + B.y^2 - D^2)*e^2 + 2*(A.x*B.x + A.y*B.y - C*D)*e + (A.x^2 + A.y^2 - C^2) == 0
 */
bool SkStrokerPriv::adjustCircles(const SkPoint& p1, const SkVector& uv1,
        SkScalar *r1, const SkPoint& p2, const SkVector& uv2, SkScalar *r2,
        bool inside) {
    SkScalar fac = inside ? -SK_Scalar1 : SK_Scalar1;
    SkVector A(p1 - p2 + uv1**r1 - uv2**r2), B(uv1*fac - uv2);
    SkScalar C = *r1 + fac**r2, D = fac*SkIntToScalar(2);
    SkScalar c = SkPointPriv::LengthSqd(A) - C*C;
    SkScalar b = SkIntToScalar(2)*(A.dot(B) - C*D);
    SkScalar a = SkPointPriv::LengthSqd(B) - D*D, e;
    if (SkScalarNearlyZero(a, 5e-5)) {
        e = -c/b;
    } else {
        SkScalar d = b*b - SkIntToScalar(4)*a*c;
        if (d < 0) {
            return false;
        }
        d = SkScalarSqrt(d);
        e = SK_ScalarHalf*(-b + d)*SkScalarInvert(a);
        if (e < 0) {
            e = SK_ScalarHalf*(-b - d)*SkScalarInvert(a);
        }
    }
    // Limit the expansion of the inside circle within reason
    if (inside && *r2 * SkIntToScalar(8) < e) {
        return false;
    }
    *r1 = *r1 + fac*e;
    *r2 = *r2 + e;
    return true;
}

/* Calculates the center c and radius r of a circle intersecting p1 and p2
   and tangent to v1 and p1. The signed curve value tracks the direction of
   curvature relative to v1.

   Explanation:  The center of the circle lies along the line through p1 in
   the direction perpendicular to v1. It also lies along the line through
   the center of secant p1,p2 perpendicular to that secant. */
int SkStrokerPriv::PVPCircle(const SkPoint& p1, const SkVector& v1,
        const SkPoint& p2, SkPoint* c, SkScalar* r) {
    SkVector uv1, uv3;
    SkPointPriv::RotateCCW(v1, &uv1);
    uv1.normalize();
    SkPointPriv::RotateCCW(p2-p1, &uv3);
    uv3.normalize();
    SkPoint p3((p1 + p2)*SK_ScalarHalf);
    (void)SkStrokerPriv::intersectLines(p1, uv1, p3, uv3, c);
    SkVector cv(p1 - *c);
    *r = cv.length();
    return std::copysign(1, uv1.dot(cv));
}

enum AngleType {
    kNearly180_AngleType,
    kSharp_AngleType,
    kShallow_AngleType,
    kNearlyLine_AngleType
};

static AngleType Dot2AngleType(SkScalar dot) {
// need more precise fixed normalization
//  SkASSERT(SkScalarAbs(dot) <= SK_Scalar1 + SK_ScalarNearlyZero);

    if (dot >= 0) { // shallow or line
        return SkScalarNearlyZero(SK_Scalar1 - dot, 5e-5) ? kNearlyLine_AngleType : kShallow_AngleType;
    } else {           // sharp or 180
        return SkScalarNearlyZero(SK_Scalar1 + dot, 5e-5) ? kNearly180_AngleType : kSharp_AngleType;
    }
}

/* Calculates the geometry of an SVG 2 Arcs join. This is split out
   into a static function in case other back-ends want to make use
   of its (non-trivial) calculations.

   An Arcs join can fall back to any of a bevel, round, or miter-clip
   join in certain cases, although here a bevel fallback is represented
   by two degenerate "point conics" at the ends of the bevel segment.
   Round and miter-clip join fallbacks are represented by return values
   other than SkPaint::kArcs_Join.

   The geometry is returned in the form of two conic arrays of the sort
   returned by BuildUnitArc(), but with some special cases:

     1) One of the conics may be a line, represented by a single degenerate
        SkConic with weight 0 and colinear control points.

     2) Either or both of the conics may be a point, represented by a
        single degenerate SkConic with weight 0 and all control points equal
        to the point.

   Note that if the last control point of the first conic array is not
   equal to the first control point of the second conic array, the two
   points should be joined with a line. This represents a clipped Arcs join
   or, in extreme cases, a bevel join. See addArcsJoin() in this file
   for a reference on using the return values.

   Also note that the SkConics are not divided so as to be non-overlapping.
   For example the middle control point of a firstConics entry may be on the
   "other side" of the geometrically corresponding secondConic entry.
   Therefore the conics cannot simply be rendered directly by, for example,
   the ccpr backend. */
SkPaint::Join SkStrokerPriv::calcArcsJoin(const SkVector& fromUnitNormal,
        SkScalar fromCurvature, const SkPoint& pivot,
        const SkVector& toUnitNormal, SkScalar toCurvature,
        SkScalar radius, SkScalar joinLimit, bool swapped,
        SkConic firstConics[SkConic::kMaxConicsForArc],
        SkConic secondConics[SkConic::kMaxConicsForArc],
        int* firstCount, int* secondCount) {
    SkScalar    dotProd = SkPoint::DotProduct(fromUnitNormal, toUnitNormal);
    AngleType   angleType = Dot2AngleType(dotProd);
    SkVector    fromNormal = fromUnitNormal, toNormal = toUnitNormal;

    *firstCount = *secondCount = 0;

    if (angleType == kNearlyLine_AngleType) {
        // Do nothing
        return SkPaint::kArcs_Join;
    }

    // Correct parameters for join direction
    if (swapped) {
        fromNormal.negate();
        toNormal.negate();
    } else {
        fromCurvature = -fromCurvature;
        toCurvature = -toCurvature;
    }
    SkPoint fromP(pivot + fromNormal * radius);
    SkPoint toP(pivot + toNormal * radius);

    // A point known to be on the "far" side of the bevel line, and the
    // "direction" of the join at the origin (e.g. the miter direction).
    SkPoint bevelLineRef, clipv1;
    if (angleType == kShallow_AngleType) {
        clipv1 = fromNormal + toNormal;
    } else {
        clipv1.set(toNormal.fY-fromNormal.fY, fromNormal.fX-toNormal.fX);
        if (swapped) {
            clipv1.negate();
        }
    }
    bevelLineRef = fromP + clipv1;

    // Adjust curvatures for offset
    fromCurvature = fromCurvature / (SK_Scalar1 - fromCurvature * radius);
    toCurvature = toCurvature / (SK_Scalar1 - toCurvature * radius);

    if (SkScalarAbs(fromCurvature) < 1e-5) {
        fromCurvature = 0;
    }
    if (SkScalarAbs(toCurvature) < 1e-5) {
        toCurvature = 0;
    }
    // Fall back to Miter-clip if both incoming and outgoing are lines
    // or for double-back joins.
    if (fromCurvature == 0 && toCurvature == 0) {
        return SkPaint::kMiterClip_Join;
    }

    // Calculate osculating circles
    SkScalar fromR, toR;
    SkPoint fromC, toC;
    if (fromCurvature != 0) {
        fromR = SkScalarInvert(fromCurvature);
        if (fromR < 0) {
            fromNormal.negate();
            fromR = -fromR;
        }
        fromC = fromP + fromNormal*fromR;
    } else {
        fromR = SK_ScalarInfinity;
    }
    if (toCurvature != 0) {
        toR = SkScalarInvert(toCurvature);
        if (toR < 0) {
            toNormal.negate();
            toR = -toR;
        }
        toC = toP + toNormal*toR;
    } else {
        toR = SK_ScalarInfinity;
    }

    // Current SVG 2 fallback for tight radii of curvature
    if (fromR < radius || toR < radius) {
        return SkPaint::kRound_Join;
    }

    // Calculate points of intersection, adjusting the
    // osculating circles as necessary.
    int cnt;
    SkPoint i1, i2, i;
    SkPoint t, t2;
    if (fromCurvature == 0) {
        SkASSERT(toCurvature != 0);
        SkPointPriv::RotateCCW(fromNormal, &t);
        cnt = SkStrokerPriv::lineCircleTest(fromP, t, toC, toR);
        if (cnt > 0) {
            SkStrokerPriv::intersectLineCircle(fromP, t, toC, toR, &i1, &i2);
        } else {
            if (angleType == kNearly180_AngleType) {
                return SkPaint::kMiterClip_Join;
            }
            toR = SkStrokerPriv::adjustLineCircle(fromP, t, toP, toCurvature > 0 ? toNormal : -toNormal);
            toC = toP + toNormal*toR;
            project_point_on_line(fromP, t, toC, &i1);
            cnt = 1;
        }
    } else if (toCurvature == 0) {
        SkASSERT(fromCurvature != 0);
        SkPointPriv::RotateCCW(toNormal, &t);
        cnt = SkStrokerPriv::lineCircleTest(toP, t, fromC, fromR);
        if (cnt > 0) {
            SkStrokerPriv::intersectLineCircle(toP, t, fromC, fromR, &i1, &i2);
        } else {
            if (angleType == kNearly180_AngleType) {
                return SkPaint::kMiterClip_Join;
            }
            fromR = SkStrokerPriv::adjustLineCircle(toP, t, fromP, fromCurvature > 0 ? fromNormal : -fromNormal);
            fromC = fromP + fromNormal*fromR;
            project_point_on_line(toP, t, fromC, &i1);
            cnt = 1;
        }
    } else {
        cnt = SkStrokerPriv::circlesTest(fromC, fromR, toC, toR);
        if (cnt <= 0) {
            if (cnt == 0 && angleType == kNearly180_AngleType) {
                return SkPaint::kMiterClip_Join;
            }
            if (cnt == -2) {
                if (!SkStrokerPriv::adjustCircles(toP, toNormal, &toR, fromP, fromNormal, &fromR, true)) {
                    return SkPaint::kMiterClip_Join;
                }
            } else {
                if (!SkStrokerPriv::adjustCircles(fromP, fromNormal, &fromR, toP, toNormal, &toR, cnt < 0)) {
                    return SkPaint::kMiterClip_Join;
                }
            }
            fromC = fromP + fromNormal*fromR;
            toC = toP + toNormal*toR;
            if (cnt == -2) {
                t = fromC - toC;
                t.setLength(toR);
                i1 = toC + t;
            } else {
                t = toC - fromC;
                t.setLength(fromR);
                i1 = fromC + t;
            }
            cnt = 1;
        } else {
            SkStrokerPriv::intersectCircles(fromC, fromR, toC, toR, &i1, &i2);
        }
    }
    // Use the closer of the two intersections that is beyond the
    // bevel line
    if (cnt > 1) {
        SkStrokerPriv::closerIntersection(fromP, toP, bevelLineRef, i1, i2, &i);
    } else {
        i = i1;
    }

    // Handle clipping (if necessary)
    bool clipped = false;
    SkPoint clipv2 = i-pivot;
    SkPoint cRefP, cRefV;
    SkScalar lengthCv2 = clipv2.length(), limitLength = joinLimit * radius;
    clipv2.scale(1/lengthCv2);
    if (SkScalarAbs(clipv1.cross(clipv2)) < 1e-4) {
        // Colinear clipping corner case
        if (limitLength < lengthCv2) {
            clipped = true;
            cRefP = pivot + clipv2 * limitLength;
            SkPointPriv::RotateCW(clipv2, &cRefV);
        }
    } else if (limitLength < lengthCv2*SK_ScalarHalf*SK_ScalarPI) { // Conservative check
        SkPoint jlc;
        SkScalar jlr, maxAngleDiff, startAngle, endAngle, angleDiff, maxAngle;
        int jls = SkStrokerPriv::PVPCircle(pivot, clipv1, i, &jlc, &jlr);
        maxAngleDiff = limitLength/jlr;
        startAngle = SkScalarATan2(pivot.fY-jlc.fY, pivot.fX-jlc.fX);
        endAngle = SkScalarATan2(i.fY-jlc.fY, i.fX-jlc.fX);
        angleDiff = norm_radians_0_2PI(jls*(endAngle - startAngle));
        if (angleDiff > maxAngleDiff) {
            clipped = true;
            maxAngle = norm_radians_0_2PI(startAngle + jls*maxAngleDiff);
            cRefV.fX = SkScalarCos(maxAngle);
            cRefV.fY = SkScalarSin(maxAngle);
            cRefP = jlc + cRefV * jlr;
        }
    }

    SkPoint fromI, toI;
    if (clipped) {
        int bevelLineSide = line_side(fromP, toP, bevelLineRef);
        // Trim the From arc or line at the clip line
        if (fromCurvature != 0) {
            SkStrokerPriv::intersectLineCircle(cRefP, cRefV, fromC, fromR, &t, &t2);
            closer_point(cRefP, t, t2, &fromI);
        } else {
            SkPointPriv::RotateCCW(fromNormal, &t);
            SkStrokerPriv::intersectLines(cRefP, cRefV, fromP, t, &fromI);
        }
        // If the trim point is behind the bevel line, just use the bevel point.
        if (line_side(fromP, toP, fromI) != bevelLineSide) {
            fromI = fromP;
        }
        // Same with the To side.
        if (toCurvature != 0) {
            SkStrokerPriv::intersectLineCircle(cRefP, cRefV, toC, toR, &t, &t2);
            closer_point(cRefP, t, t2, &toI);
        } else {
            SkPointPriv::RotateCCW(toNormal, &t);
            SkStrokerPriv::intersectLines(cRefP, cRefV, toP, t, &toI);
        }
        if (line_side(fromP, toP, toI) != bevelLineSide) {
            toI = toP;
        }
    } else {
        fromI = toI = i;
    }

    bool done = false;
    SkMatrix    matrix;
    SkRotationDirection dir;
    if (fromCurvature != 0 && fromI != fromP) {
        matrix.setScale(fromR, fromR);
        matrix.postTranslate(fromC.fX, fromC.fY);
        dir = (fromCurvature > 0) == !swapped ? kCCW_SkRotationDirection : kCW_SkRotationDirection;
        t = fromP-fromC;
        t.normalize();
        t2 = fromI-fromC;
        t2.normalize();
        *firstCount = SkConic::BuildUnitArc(t, t2, dir, &matrix, firstConics);
        if (*firstCount > 0) {
            if (!clipped) {
                toI = firstConics[*firstCount-1].fPts[2];
            }
            done = true;
        }
    }
    /* Either From is a line or BuildUnitArc failed so fall back to a line
       (or a point if fromP == fromI) */
    if (!done) {
        *firstCount = 1;
        firstConics[0].fPts[0] = fromP;
        firstConics[0].fPts[2] = fromI;
        firstConics[0].fPts[1] = (fromI + fromP) * SK_ScalarHalf;
        firstConics[0].fW = 0;
    }
    done = false;
    if (toCurvature != 0 && toI != toP) {
        matrix.reset();
        matrix.setScale(toR, toR);
        matrix.postTranslate(toC.fX, toC.fY);
        dir = (toCurvature > 0) == !swapped ? kCCW_SkRotationDirection : kCW_SkRotationDirection;
        t = toI-toC;
        t.normalize();
        t2 = toP-toC;
        t2.normalize();
        *secondCount = SkConic::BuildUnitArc(t, t2, dir, &matrix, secondConics);
        if (*secondCount > 0) {
            done = true;
        }
    }
    /* Either To is a line or BuildUnitArc failed so fall back to a line
       (or a point if toP == toI) */
    if (!done) {
        *secondCount = 1;
        secondConics[0].fPts[0] = toI;
        secondConics[0].fPts[2] = toP;
        secondConics[0].fPts[1] = (toI + toP) * SK_ScalarHalf;
        secondConics[0].fW = 0;
    }
    return SkPaint::kArcs_Join;
}

/////////////////////////////////////////////////////////////////////////////

void SkStrokerPriv::addButtCap(SkPath* path) const {
    path->lineTo(fStop.fX, fStop.fY);
}

void SkStrokerPriv::addRoundCap(SkPath* path) const {
    SkVector normal, parallel;

    fFromUnitNormal.scale(fRadius, &normal);
    SkPointPriv::RotateCW(normal, &parallel);

    SkPoint projectedCenter = fPivot + parallel;

    path->conicTo(projectedCenter + normal, projectedCenter, SK_ScalarRoot2Over2);
    path->conicTo(projectedCenter - normal, fStop, SK_ScalarRoot2Over2);
}

void SkStrokerPriv::addSquareCap(SkPath* path) const {
    SkVector normal, parallel;

    fFromUnitNormal.scale(fRadius, &normal);
    SkPointPriv::RotateCW(normal, &parallel);

    if (fFromIsLine) {
        path->setLastPt(fPivot.fX + normal.fX + parallel.fX, fPivot.fY + normal.fY + parallel.fY);
        path->lineTo(fPivot.fX - normal.fX + parallel.fX, fPivot.fY - normal.fY + parallel.fY);
    } else {
        path->lineTo(fPivot.fX + normal.fX + parallel.fX, fPivot.fY + normal.fY + parallel.fY);
        path->lineTo(fPivot.fX - normal.fX + parallel.fX, fPivot.fY - normal.fY + parallel.fY);
        path->lineTo(fStop.fX, fStop.fY);
    }
}

void SkStrokerPriv::addCap(SkPath *path) const {
    switch (fCap) {
        case SkPaint::kButt_Cap:
            this->addButtCap(path);
            break;
        case SkPaint::kRound_Cap:
            this->addRoundCap(path);
            break;
        case SkPaint::kSquare_Cap:
            this->addSquareCap(path);
            break;
    }
}

/////////////////////////////////////////////////////////////////////////////

static inline bool is_clockwise(const SkVector& before, const SkVector& after) {
    /* The small negative margin is a work-around for line-degenerate curves
       where the lineTo fallback alters the tangent and therefore the normal.
       The positions and normals in such cases are analogous but the curvature
       directions flip. By moving the outer/inner switch point a little to one
       side of zero the fallback doesn't flip the direction. */
    return SkPoint::CrossProduct(before, after) >= -7e-3;
}

void SkStrokerPriv::addBevelJoin(SkPath* outer, const SkVector& after) const {
    outer->lineTo(fPivot.fX + after.fX, fPivot.fY + after.fY);
}

bool SkStrokerPriv::addRoundJoin(SkPath* outer, bool swapped) const {
    SkScalar    dotProd = SkPoint::DotProduct(fFromUnitNormal, fToUnitNormal);
    AngleType   angleType = Dot2AngleType(dotProd);

    if (angleType == kNearlyLine_AngleType)
        return false;

    SkVector            before = fFromUnitNormal;
    SkVector            after = fToUnitNormal;
    SkRotationDirection dir = kCW_SkRotationDirection;

    if (swapped) {
        before.negate();
        after.negate();
        dir = kCCW_SkRotationDirection;
    }

    SkMatrix    matrix;
    matrix.setScale(fRadius, fRadius);
    matrix.postTranslate(fPivot.fX, fPivot.fY);
    SkConic conics[SkConic::kMaxConicsForArc];
    int count = SkConic::BuildUnitArc(before, after, dir, &matrix, conics);
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            outer->conicTo(conics[i].fPts[1], conics[i].fPts[2], conics[i].fW);
        }
        return true;
    }
    return false;
}

bool SkStrokerPriv::addMiterJoin(SkPath* outer, const SkVector& scaledAfter,
        bool swapped) const {
    SkScalar    dotProd = SkPoint::DotProduct(fFromUnitNormal, fToUnitNormal);
    AngleType   angleType = Dot2AngleType(dotProd);
    SkVector    before = fFromUnitNormal, after = fToUnitNormal, mid, parallel;
    SkPoint     iPt, tPt, oPt;
    SkScalar    invSinHalfAngle, clipRatio = SK_Scalar1;

    if (angleType == kNearlyLine_AngleType) {
        return false;
    }

    if (swapped) {
        before.negate();
        after.negate();
    }

    if (angleType == kNearly180_AngleType) {
        if (this->joinIs(SkPaint::kMiter_Join)) {
            this->addBevelJoin(outer, scaledAfter);
        } else {
            // Make a rectangle the length of the join limit
            if (swapped) {
                SkPointPriv::RotateCCW(before, &parallel);
            } else {
                SkPointPriv::RotateCW(before, &parallel);
            }
            parallel.setLength(fRadius * fJoinLimit);

            if (fFromIsLine) {
                outer->setLastPt(fPivot.fX - scaledAfter.fX + parallel.fX,
                                 fPivot.fY - scaledAfter.fY + parallel.fY);
            } else {
                outer->lineTo(fPivot.fX - scaledAfter.fX + parallel.fX,
                              fPivot.fY - scaledAfter.fY + parallel.fY);
            }
            outer->lineTo(fPivot.fX + scaledAfter.fX + parallel.fX,
                          fPivot.fY + scaledAfter.fY + parallel.fY);
            if (!fToIsLine) {
                outer->lineTo(fPivot.fX + scaledAfter.fX, fPivot.fY + scaledAfter.fY);
            }
        }
        return true;
    }

    /*  Before we enter the world of square-roots and divides,
        check if we're trying to join an upright right angle
        (common case for stroking rectangles). If so, special case
        that (for speed an accuracy). Because the limit is checked
        this works for kMiter_Join and kMiterClip_Join.
        Note: we only need to check one normal if dot==0
    */
    if (0 == dotProd && fJoinLimit >= SK_ScalarSqrt2) {
        mid = (before + after) * fRadius;
    } else {
        /*  midLength = radius / sinHalfAngle
            if (midLength > miterLimit * radius) abort
            if (radius / sinHalf > miterLimit * radius) abort
            if (1 / sinHalf > miterLimit) abort
            My dotProd is opposite sign, since it is built from normals
            and not tangents, hence 1 + dot instead of 1 - dot in the formula
        */
        invSinHalfAngle = SkScalarInvert(SkScalarSqrt(SkScalarHalf(SK_Scalar1 + dotProd)));
        if (invSinHalfAngle > fJoinLimit) {
            if (this->joinIs(SkPaint::kMiter_Join)) {
                this->addBevelJoin(outer, scaledAfter);
                return true;
            } else {
                SkPoint bevelMid = (before + after) * SK_ScalarHalf;
                SkScalar bevelMidLength = bevelMid.length();
                if (bevelMidLength >= fJoinLimit) {
                    this->addBevelJoin(outer, scaledAfter);
                    return true;
                }
                clipRatio = (fJoinLimit - bevelMidLength) /
                            (invSinHalfAngle - bevelMidLength);
            }
        }

        // choose the most accurate way to form the initial mid-vector
        if (angleType == kSharp_AngleType) {
            mid.set(after.fY - before.fY, before.fX - after.fX);
            if (swapped) {
                mid.negate();
            }
        } else {
            mid.set(before.fX + after.fX, before.fY + after.fY);
        }
        mid.setLength(fRadius * invSinHalfAngle);
    }

    if (clipRatio == SK_Scalar1) {
        if (fFromIsLine) {
            outer->setLastPt(fPivot.fX + mid.fX, fPivot.fY + mid.fY);
        } else {
            outer->lineTo(fPivot.fX + mid.fX, fPivot.fY + mid.fY);
        }

        if (!fToIsLine) {
            outer->lineTo(fPivot.fX + scaledAfter.fX, fPivot.fY + scaledAfter.fY);
        }
    } else {
        iPt = fPivot + mid;
        outer->getLastPt(&oPt);
        tPt = oPt * (SK_Scalar1-clipRatio) + iPt * clipRatio;
        if (fFromIsLine) {
            outer->setLastPt(tPt);
        } else {
            outer->lineTo(tPt);
        }
        oPt = fPivot + scaledAfter;
        tPt = oPt * (SK_Scalar1-clipRatio) + iPt * clipRatio;
        outer->lineTo(tPt);
        if (!fToIsLine) {
            outer->lineTo(oPt);
        }
    }
    return true;
}

bool SkStrokerPriv::addArcsJoin(SkPath* outer, const SkVector& after,
        bool swapped) const {
    SkConic firstConics[SkConic::kMaxConicsForArc];
    SkConic secondConics[SkConic::kMaxConicsForArc];
    SkPaint::Join join;
    int firstCnt, secondCnt, i;

    join = SkStrokerPriv::calcArcsJoin(fFromUnitNormal, fFromCurvature,
                                       fPivot, fToUnitNormal, fToCurvature,
                                       fRadius, fJoinLimit, swapped,
                                       firstConics, secondConics,
                                       &firstCnt, &secondCnt);

    if (join != SkPaint::kArcs_Join) {
        // This is safe as long as there is no circular delegation pattern
        return this->joinSwitch(join, outer, after, swapped);
    }
    // Signal to do nothing because angle is too slight
    if (firstCnt == 0 && secondCnt == 0) {
        return false;
    }
    SkASSERT(firstCnt > 0 && firstCnt <= SkConic::kMaxConicsForArc &&
             secondCnt > 0 && secondCnt <= SkConic::kMaxConicsForArc);
    if (firstConics[0].fW == 0) {
        SkASSERT(firstCnt == 1);
        if (firstConics[0].fPts[0] != firstConics[0].fPts[2]) {
            if (fFromIsLine) {
                outer->setLastPt(firstConics[0].fPts[2]);
            } else {
                outer->lineTo(firstConics[0].fPts[2]);
            }
        }
    } else {
        for (i = 0; i < firstCnt; ++i) {
            outer->conicTo(firstConics[i].fPts[1], firstConics[i].fPts[2],
                           firstConics[i].fW);
        }
    }
    if (firstConics[firstCnt-1].fPts[2] != secondConics[0].fPts[0]) {
        outer->lineTo(secondConics[0].fPts[0]);
    }
    if (secondConics[0].fW == 0) {
        SkASSERT(secondCnt == 1);
        if (!fToIsLine && secondConics[0].fPts[0] != secondConics[0].fPts[2]) {
            outer->lineTo(secondConics[0].fPts[2]);
        }
    } else {
        for (i = 0; i < secondCnt; ++i) {
            outer->conicTo(secondConics[i].fPts[1], secondConics[i].fPts[2],
                           secondConics[i].fW);
        }
    }
    return true;
}

bool SkStrokerPriv::joinSwitch(SkPaint::Join join, SkPath* outer,
        const SkVector& after, bool swapped) const {
    switch (join) {
        case SkPaint::kBevel_Join:
            this->addBevelJoin(outer, after);
            return true;
        case SkPaint::kRound_Join:
            return this->addRoundJoin(outer, swapped);
        case SkPaint::kMiter_Join:
        case SkPaint::kMiterClip_Join:
            return this->addMiterJoin(outer, after, swapped);
        case SkPaint::kArcs_Join:
            return this->addArcsJoin(outer, after, swapped);
    }
}

void SkStrokerPriv::addJoin(SkPath *outer, SkPath *inner) const {
    bool swapped = false;
    SkVector after;

    fToUnitNormal.scale(fRadius, &after);

    if (!is_clockwise(fFromUnitNormal, fToUnitNormal)) {
        using std::swap;
        swap(outer, inner);
        after.negate();
        swapped = true;
    }

    if (this->joinSwitch((SkPaint::Join)fJoin, outer, after, swapped)) {
#if 1
    /*  In the degenerate case that the stroke radius is larger than our segments
        just connecting the two inner segments may "show through" as a funny
        diagonal. To pseudo-fix this, we go through the pivot point. This adds
        an extra point/edge, but I can't see a cheap way to know when this is
        not needed :(
    */
        inner->lineTo(fPivot.fX, fPivot.fY);
#endif
        inner->lineTo(fPivot.fX - after.fX, fPivot.fY - after.fY);
    }
}

