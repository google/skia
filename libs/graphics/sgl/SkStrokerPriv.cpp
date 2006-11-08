/* libs/graphics/sgl/SkStrokerPriv.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkStrokerPriv.h"
#include "SkGeometry.h"
#include "SkPath.h"

static void ButtCapper(SkPath* path, const SkPoint& pivot,
                       const SkVector& normal, const SkPoint& stop,
                       SkPath*)
{
    path->lineTo(stop.fX, stop.fY);
}

static void RoundCapper(SkPath* path, const SkPoint& pivot,
                        const SkVector& normal, const SkPoint& stop,
                        SkPath*)
{
    SkScalar    px = pivot.fX;
    SkScalar    py = pivot.fY;
    SkScalar    nx = normal.fX;
    SkScalar    ny = normal.fY;
    SkScalar    sx = SkScalarMul(nx, CUBIC_ARC_FACTOR);
    SkScalar    sy = SkScalarMul(ny, CUBIC_ARC_FACTOR);

    path->cubicTo(px + nx + CWX(sx, sy), py + ny + CWY(sx, sy),
                  px + CWX(nx, ny) + sx, py + CWY(nx, ny) + sy,
                  px + CWX(nx, ny), py + CWY(nx, ny));
    path->cubicTo(px + CWX(nx, ny) - sx, py + CWY(nx, ny) - sy,
                  px - nx + CWX(sx, sy), py - ny + CWY(sx, sy),
                  stop.fX, stop.fY);
}

static void SquareCapper(SkPath* path, const SkPoint& pivot,
                         const SkVector& normal, const SkPoint& stop,
                         SkPath* otherPath)
{
    SkVector parallel;
    normal.rotateCW(&parallel);

    if (otherPath)
    {
        path->setLastPt(pivot.fX + normal.fX + parallel.fX, pivot.fY + normal.fY + parallel.fY);
        path->lineTo(pivot.fX - normal.fX + parallel.fX, pivot.fY - normal.fY + parallel.fY);
    }
    else
    {
        path->lineTo(pivot.fX + normal.fX + parallel.fX, pivot.fY + normal.fY + parallel.fY);
        path->lineTo(pivot.fX - normal.fX + parallel.fX, pivot.fY - normal.fY + parallel.fY);
        path->lineTo(stop.fX, stop.fY);
    }
}

/////////////////////////////////////////////////////////////////////////////

static bool is_clockwise(const SkVector& before, const SkVector& after)
{
    return SkScalarMul(before.fX, after.fY) - SkScalarMul(before.fY, after.fX) > 0;
}

enum AngleType {
    kNearly180_AngleType,
    kSharp_AngleType,
    kShallow_AngleType,
    kNearlyLine_AngleType
};

static AngleType Dot2AngleType(SkScalar dot)
{
// need more precise fixed normalization
//  SkASSERT(SkScalarAbs(dot) <= SK_Scalar1 + SK_ScalarNearlyZero);

    if (dot >= 0)   // shallow or line
        return SkScalarNearlyZero(SK_Scalar1 - dot) ? kNearlyLine_AngleType : kShallow_AngleType;
    else            // sharp or 180
        return SkScalarNearlyZero(SK_Scalar1 + dot) ? kNearly180_AngleType : kSharp_AngleType;
}

static void BluntJoiner(SkPath* outer, SkPath* inner, const SkVector& beforeUnitNormal,
                        const SkPoint& pivot, const SkVector& afterUnitNormal,
                        SkScalar radius, SkScalar invMiterLimit, bool, bool)
{
    SkVector    after;
    afterUnitNormal.scale(radius, &after);

    outer->lineTo(pivot.fX + after.fX, pivot.fY + after.fY);
    inner->lineTo(pivot.fX - after.fX, pivot.fY - after.fY);
}

static void RoundJoiner(SkPath* outer, SkPath* inner, const SkVector& beforeUnitNormal,
                        const SkPoint& pivot, const SkVector& afterUnitNormal,
                        SkScalar radius, SkScalar invMiterLimit, bool, bool)
{
    SkScalar    dotProd = SkPoint::DotProduct(beforeUnitNormal, afterUnitNormal);
    AngleType   angleType = Dot2AngleType(dotProd);

    if (angleType == kNearlyLine_AngleType)
        return;

    SkVector            before = beforeUnitNormal;
    SkVector            after = afterUnitNormal;
    SkRotationDirection dir = kCW_SkRotationDirection;

    if (!is_clockwise(before, after))
    {
        SkTSwap<SkPath*>(outer, inner);
        before.negate();
        after.negate();
        dir = kCCW_SkRotationDirection;
    }

    SkPoint     pts[kSkBuildQuadArcStorage];
    SkMatrix    matrix;
    matrix.setScale(radius, radius);
    matrix.postTranslate(pivot.fX, pivot.fY);
    int count = SkBuildQuadArc(before, after, dir, &matrix, pts);

    if (count > 0)
    {
        for (int i = 1; i < count; i += 2)
            outer->quadTo(pts[i].fX, pts[i].fY, pts[i+1].fX, pts[i+1].fY);

        after.scale(radius);
        inner->lineTo(pivot.fX - after.fX, pivot.fY - after.fY);
    }
}

static void MiterJoiner(SkPath* outer, SkPath* inner, const SkVector& beforeUnitNormal,
                        const SkPoint& pivot, const SkVector& afterUnitNormal,
                        SkScalar radius, SkScalar invMiterLimit,
                        bool prevIsLine, bool currIsLine)
{
    // negate the dot since we're using normals instead of tangents
    SkScalar    dotProd = SkPoint::DotProduct(beforeUnitNormal, afterUnitNormal);
    AngleType   angleType = Dot2AngleType(dotProd);
    SkVector    before = beforeUnitNormal;
    SkVector    after = afterUnitNormal;
    SkVector    mid;
    SkScalar    sinHalfAngle;
    bool        ccw;

    if (angleType == kNearlyLine_AngleType)
        return;
    if (angleType == kNearly180_AngleType)
    {
        currIsLine = false;
        goto DO_BLUNT;
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
    if (sinHalfAngle < invMiterLimit)
    {
        currIsLine = false;
        goto DO_BLUNT;
    }

    ccw = !is_clockwise(before, after);
    if (ccw)
    {
        SkTSwap<SkPath*>(outer, inner);
        before.negate();
        after.negate();
    }

    // choose the most accurate way to form the initial mid-vector
    if (angleType == kSharp_AngleType)
    {
        mid.set(after.fY - before.fY, before.fX - after.fX);
        if (ccw)
            mid.negate();
    }
    else
        mid.set(before.fX + after.fX, before.fY + after.fY);

    mid.setLength(SkScalarDiv(radius, sinHalfAngle));
    if (prevIsLine)
        outer->setLastPt(pivot.fX + mid.fX, pivot.fY + mid.fY);
    else
        outer->lineTo(pivot.fX + mid.fX, pivot.fY + mid.fY);

DO_BLUNT:
    after.scale(radius);
    if (!currIsLine)
        outer->lineTo(pivot.fX + after.fX, pivot.fY + after.fY);
    inner->lineTo(pivot.fX - after.fX, pivot.fY - after.fY);
}

/////////////////////////////////////////////////////////////////////////////

SkStrokerPriv::CapProc SkStrokerPriv::CapFactory(SkPaint::Cap cap)
{
    static const SkStrokerPriv::CapProc gCappers[] = {
        ButtCapper, RoundCapper, SquareCapper
    };

    SkASSERT((unsigned)cap < SkPaint::kCapCount);
    return gCappers[cap];
}

SkStrokerPriv::JoinProc SkStrokerPriv::JoinFactory(SkPaint::Join join)
{
    static const SkStrokerPriv::JoinProc gJoiners[] = {
        MiterJoiner, RoundJoiner, BluntJoiner
    };

    SkASSERT((unsigned)join < SkPaint::kJoinCount);
    return gJoiners[join];
}



