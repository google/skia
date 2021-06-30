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

static void ButtCapper(SkPath* path, const SkPoint& pivot, const SkVector& normal,
                       const SkPoint& stop, SkPath*) {
    path->lineTo(stop.fX, stop.fY);
}

static void RoundCapper(SkPath* path, const SkPoint& pivot, const SkVector& normal,
                        const SkPoint& stop, SkPath*) {
    SkVector parallel;
    SkPointPriv::RotateCW(normal, &parallel);

    SkPoint projectedCenter = pivot + parallel;

    path->conicTo(projectedCenter + normal, projectedCenter, SK_ScalarRoot2Over2);
    path->conicTo(projectedCenter - normal, stop, SK_ScalarRoot2Over2);
}

static void SquareCapper(SkPath* path, const SkPoint& pivot, const SkVector& normal,
                         const SkPoint& stop, SkPath* otherPath) {
    SkVector parallel;
    SkPointPriv::RotateCW(normal, &parallel);

    if (otherPath) {
        path->setLastPt(pivot.fX + normal.fX + parallel.fX, pivot.fY + normal.fY + parallel.fY);
        path->lineTo(pivot.fX - normal.fX + parallel.fX, pivot.fY - normal.fY + parallel.fY);
    } else {
        path->lineTo(pivot.fX + normal.fX + parallel.fX, pivot.fY + normal.fY + parallel.fY);
        path->lineTo(pivot.fX - normal.fX + parallel.fX, pivot.fY - normal.fY + parallel.fY);
        path->lineTo(stop.fX, stop.fY);
    }
}

/////////////////////////////////////////////////////////////////////////////

static bool is_clockwise(const SkVector& before, const SkVector& after) {
    return before.fX * after.fY > before.fY * after.fX;
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
        return SkScalarNearlyZero(SK_Scalar1 - dot) ? kNearlyLine_AngleType : kShallow_AngleType;
    } else {           // sharp or 180
        return SkScalarNearlyZero(SK_Scalar1 + dot) ? kNearly180_AngleType : kSharp_AngleType;
    }
}

/*
 * Consider the two line segments:
 *
 *               s     Inside
 *      q +------+-----------------+
 *        |      |                 |
 * prevPt +------+ pivotPt---------+ nextPt
 *        |      |                 |
 *        +------+-----------------+
 *                     Outside
 *
 * If the perpendicular distance from q to the line (pivot, nextPt) is less than
 * the stroke radius, the inner join geometry is needed. Or if s, which is the first
 * point on the inner stroke of the next segment, lies to the left of line (prevPt, q),
 * we also need the inner join. Then, we also need to check the reverse configuration:
 *
 *               s     Inside      q
 *        +------+-----------------+
 *        |      |                 |
 * prevPt +------+ pivotPt---------+ nextPt
 *        |      |                 |
 *        +------+-----------------+
 *                     Outside
 *
 * In the reverse case, s is the last point on the inner stroke of the previous
 * segment (no longer the first point on the inner stroke of the next segment).
 */
static bool NeedInnerJoin(const SkPoint& beforeUnitNormal,
                          const SkPoint& afterUnitNormal,
                          const SkPoint& prevPt,
                          const SkPoint& pivot,
                          const SkPoint& nextPt,
                          float radius,
                          bool ccw) {
#ifdef SK_LEGACY_INNER_JOINS
    // Previously we would always add inner joins.
    return true;
#else
    SkPoint scaledBefore = beforeUnitNormal;
    scaledBefore.scale(radius);
    SkPoint scaledAfter = afterUnitNormal;
    scaledAfter.scale(radius);

    const int sgn = ccw ? 1 : -1;
    bool needInnerJoin = false;

    // Forward case
    {
        const SkPoint q = prevPt - scaledBefore;
        const SkPoint s = pivot - scaledAfter;

        // Perpendicular distance
        const SkPoint n = -afterUnitNormal;
        needInnerJoin |= n.dot(q - pivot) < radius;

        // Which-side-of-line
        const SkPoint v1 = s - prevPt;
        const SkPoint v2 = q - prevPt;
        needInnerJoin |= sgn * v1.cross(v2) > 0;
    }

    // Reverse
    {
        const SkPoint q = nextPt - scaledAfter;
        const SkPoint s = pivot - scaledBefore;

        // Perpendicular distance
        const SkPoint n = -beforeUnitNormal;
        needInnerJoin |= n.dot(q - pivot) < radius;

        // Which-side-of-line
        const SkPoint v1 = s - nextPt;
        const SkPoint v2 = q - nextPt;
        needInnerJoin |= sgn * v1.cross(v2) < 0;
    }

    return needInnerJoin;
#endif
}

static void HandleInnerJoin(SkPath* inner, const SkPoint& pivot, const SkVector& after) {
#if 1
    /*  In the degenerate case that the stroke radius is larger than our segments
        just connecting the two inner segments may "show through" as a funny
        diagonal. To pseudo-fix this, we go through the pivot point. This adds
        an extra point/edge, but I can't see a cheap way to know when this is
        not needed :(
    */
    inner->lineTo(pivot.fX, pivot.fY);
#endif

    inner->lineTo(pivot.fX - after.fX, pivot.fY - after.fY);
}

static void BluntJoiner(SkPath* outer, SkPath* inner, const SkVector& beforeUnitNormal,
                        const SkPoint& pivot, const SkVector& afterUnitNormal,
                        SkScalar radius, SkScalar invMiterLimit, bool, bool,
                        const SkPoint&, const SkPoint&) {
    SkVector    after;
    afterUnitNormal.scale(radius, &after);

    if (!is_clockwise(beforeUnitNormal, afterUnitNormal)) {
        using std::swap;
        swap(outer, inner);
        after.negate();
    }

    outer->lineTo(pivot.fX + after.fX, pivot.fY + after.fY);
    HandleInnerJoin(inner, pivot, after);
}

static void RoundJoiner(SkPath* outer, SkPath* inner, const SkVector& beforeUnitNormal,
                        const SkPoint& pivot, const SkVector& afterUnitNormal,
                        SkScalar radius, SkScalar invMiterLimit, bool, bool,
                        const SkPoint&, const SkPoint&) {
    SkScalar    dotProd = SkPoint::DotProduct(beforeUnitNormal, afterUnitNormal);
    AngleType   angleType = Dot2AngleType(dotProd);

    if (angleType == kNearlyLine_AngleType)
        return;

    SkVector            before = beforeUnitNormal;
    SkVector            after = afterUnitNormal;
    SkRotationDirection dir = kCW_SkRotationDirection;

    if (!is_clockwise(before, after)) {
        using std::swap;
        swap(outer, inner);
        before.negate();
        after.negate();
        dir = kCCW_SkRotationDirection;
    }

    SkMatrix    matrix;
    matrix.setScale(radius, radius);
    matrix.postTranslate(pivot.fX, pivot.fY);
    SkConic conics[SkConic::kMaxConicsForArc];
    int count = SkConic::BuildUnitArc(before, after, dir, &matrix, conics);
    if (count > 0) {
        for (int i = 0; i < count; ++i) {
            outer->conicTo(conics[i].fPts[1], conics[i].fPts[2], conics[i].fW);
        }
        after.scale(radius);
        HandleInnerJoin(inner, pivot, after);
    }
}

#define kOneOverSqrt2   (0.707106781f)

static void MiterJoiner(SkPath* outer, SkPath* inner, const SkVector& beforeUnitNormal,
                        const SkPoint& pivot, const SkVector& afterUnitNormal,
                        SkScalar radius, SkScalar invMiterLimit,
                        bool prevIsLine, bool currIsLine,
                        const SkPoint& prevPt, const SkPoint& nextPt) {
    // negate the dot since we're using normals instead of tangents
    SkScalar    dotProd = SkPoint::DotProduct(beforeUnitNormal, afterUnitNormal);
    AngleType   angleType = Dot2AngleType(dotProd);
    SkVector    before = beforeUnitNormal;
    SkVector    after = afterUnitNormal;
    SkVector    mid;
    SkScalar    sinHalfAngle;
    bool        ccw = !is_clockwise(before, after);

    if (angleType == kNearlyLine_AngleType) {
        return;
    }
    if (angleType == kNearly180_AngleType) {
        currIsLine = false;
        goto DO_BLUNT;
    }

    if (ccw) {
        using std::swap;
        swap(outer, inner);
        before.negate();
        after.negate();
    }

    /*  Before we enter the world of square-roots and divides,
        check if we're trying to join an upright right angle
        (common case for stroking rectangles). If so, special case
        that (for speed an accuracy).
        Note: we only need to check one normal if dot==0
    */
    if (0 == dotProd && invMiterLimit <= kOneOverSqrt2) {
        mid = (before + after) * radius;
        goto DO_MITER;
    }

    /*  midLength = radius / sinHalfAngle
        if (midLength > miterLimit * radius) abort
        if (radius / sinHalf > miterLimit * radius) abort
        if (1 / sinHalf > miterLimit) abort
        if (1 / miterLimit > sinHalf) abort
        My dotProd is opposite sign, since it is built from normals and not tangents
        hence 1 + dot instead of 1 - dot in the formula
    */
    sinHalfAngle = SkScalarSqrt(SkScalarHalf(SK_Scalar1 + dotProd));
    if (sinHalfAngle < invMiterLimit) {
        currIsLine = false;
        goto DO_BLUNT;
    }

    // choose the most accurate way to form the initial mid-vector
    if (angleType == kSharp_AngleType) {
        mid.set(after.fY - before.fY, before.fX - after.fX);
        if (ccw) {
            mid.negate();
        }
    } else {
        mid.set(before.fX + after.fX, before.fY + after.fY);
    }

    mid.setLength(radius / sinHalfAngle);
DO_MITER:
    if (prevIsLine) {
        outer->setLastPt(pivot.fX + mid.fX, pivot.fY + mid.fY);
    } else {
        outer->lineTo(pivot.fX + mid.fX, pivot.fY + mid.fY);
    }

DO_BLUNT:
    // Save copy of unit normal before scaling it
    const SkPoint savedAfter = after;
    after.scale(radius);
    if (!currIsLine) {
        outer->lineTo(pivot.fX + after.fX, pivot.fY + after.fY);
    }

    // With two line segments, sometimes we can omit the inner join geometry.
    if (prevIsLine && currIsLine &&
        !NeedInnerJoin(before, savedAfter, prevPt, pivot, nextPt, radius, ccw)) {
        // Skip the inner join: move last inner point to mirrored miter point.
        inner->setLastPt(pivot - mid);
    } else {
        HandleInnerJoin(inner, pivot, after);
    }
}

/////////////////////////////////////////////////////////////////////////////

SkStrokerPriv::CapProc SkStrokerPriv::CapFactory(SkPaint::Cap cap) {
    const SkStrokerPriv::CapProc gCappers[] = {
        ButtCapper, RoundCapper, SquareCapper
    };

    SkASSERT((unsigned)cap < SkPaint::kCapCount);
    return gCappers[cap];
}

SkStrokerPriv::JoinProc SkStrokerPriv::JoinFactory(SkPaint::Join join) {
    const SkStrokerPriv::JoinProc gJoiners[] = {
        MiterJoiner, RoundJoiner, BluntJoiner
    };

    SkASSERT((unsigned)join < SkPaint::kJoinCount);
    return gJoiners[join];
}
