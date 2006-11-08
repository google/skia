/* libs/graphics/sgl/SkPath.cpp
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

#include "SkPath.h"
#include "SkMath.h"

/*
    Stores the verbs and points as they are given to us, with exceptions:
    - we only record "Close" if it was immediately preceeded by Line | Quad | Cubic
    - we insert a Move(0,0) if Line | Quad | Cubic is our first command

    The iterator does more cleanup, especially if forceClose == true
    1. if we encounter Close, return a cons'd up Line() first (if the curr-pt != start-pt)
    2. if we encounter Move without a preceeding Close, and forceClose is true, goto #1
    3. if we encounter Line | Quad | Cubic after Close, cons up a Move
*/

////////////////////////////////////////////////////////////////////////////

SkPath::SkPath() : fFillType(kWinding_FillType)
{
}

SkPath::SkPath(const SkPath& src)
{
    *this = src;
}

SkPath::~SkPath()
{
}

SkPath& SkPath::operator=(const SkPath& src)
{
    if (this != &src)
    {
        fPts        = src.fPts;
        fVerbs      = src.fVerbs;
        fFillType   = src.fFillType;
    }
    return *this;
}

void SkPath::swap(SkPath& other)
{
    SkASSERT(&other != nil);

    if (this != &other)
    {
        fPts.swap(other.fPts);
        fVerbs.swap(other.fVerbs);
        SkTSwap<U8>(fFillType, other.fFillType);
    }
}

void SkPath::reset()
{
    fPts.reset();
    fVerbs.reset();
}

bool SkPath::isEmpty() const
{
    int count = fVerbs.count();
    return count == 0 || (count == 1 && fVerbs[0] == kMove_Verb);
}

bool SkPath::isRect(SkRect*) const
{
    SkASSERT(!"unimplemented");
    return false;
}

int SkPath::getPoints(SkPoint copy[], int max) const
{
    SkASSERT(max >= 0);
    int count = fPts.count();
    if (copy && max > 0 && count > 0)
        memcpy(copy, fPts.begin(), sizeof(SkPoint) * SkMin32(max, count));
    return count;
}

void SkPath::getLastPt(SkPoint* lastPt) const
{
    if (lastPt)
    {
        int count = fPts.count();
        if (count == 0)
            lastPt->set(0, 0);
        else
            *lastPt = fPts[count - 1];
    }
}

void SkPath::setLastPt(SkScalar x, SkScalar y)
{
    int count = fPts.count();
    if (count == 0)
        this->moveTo(x, y);
    else
        fPts[count - 1].set(x, y);
}

void SkPath::computeBounds(SkRect* bounds, BoundsType bt) const
{
    SkASSERT(bounds);

    if (fPts.count() <= 1)
        bounds->set(0, 0, 0, 0);
    else if (true || bt == kFast_BoundsType)
        bounds->set(fPts.begin(), fPts.count());
    else
    {
        SkASSERT(!"unimplemented");
        Iter    iter(*this, false);
        SkPoint pts[4];
        Verb    verb;

        while ((verb = iter.next(pts)) != kDone_Verb)
        {
            switch (verb) {
            case kLine_Verb:
            case kQuad_Verb:
            case kCubic_Verb:
                break;
            default:
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//  Construction methods

void SkPath::incReserve(U16CPU inc)
{
    fVerbs.setReserve(fVerbs.count() + inc);
    fPts.setReserve(fPts.count() + inc);
}

void SkPath::moveTo(SkScalar x, SkScalar y)
{
    int      vc = fVerbs.count();
    SkPoint* pt;

    if (vc > 0 && fVerbs[vc - 1] == kMove_Verb)
    {
        pt = &fPts[fPts.count() - 1];
    }
    else
    {
        pt = fPts.append();
        *fVerbs.append() = kMove_Verb;
    }
    pt->set(x, y);
}

void SkPath::rMoveTo(SkScalar x, SkScalar y)
{
    SkPoint pt;
    this->getLastPt(&pt);
    this->moveTo(pt.fX + x, pt.fY + y);
}

void SkPath::lineTo(SkScalar x, SkScalar y)
{
    if (fVerbs.count() == 0)
    {
        fPts.append()->set(0, 0);
        *fVerbs.append() = kMove_Verb;
    }
    fPts.append()->set(x, y);
    *fVerbs.append() = kLine_Verb;
}

void SkPath::rLineTo(SkScalar x, SkScalar y)
{
    SkPoint pt;
    this->getLastPt(&pt);
    this->lineTo(pt.fX + x, pt.fY + y);
}

void SkPath::quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2)
{
    if (fVerbs.count() == 0)
    {
        fPts.append()->set(0, 0);
        *fVerbs.append() = kMove_Verb;
    }

    SkPoint* pts = fPts.append(2);
    pts[0].set(x1, y1);
    pts[1].set(x2, y2);
    *fVerbs.append() = kQuad_Verb;
}

void SkPath::rQuadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2)
{
    SkPoint pt;
    this->getLastPt(&pt);
    this->quadTo(pt.fX + x1, pt.fY + y1, pt.fX + x2, pt.fY + y2);
}

void SkPath::cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3)
{
    if (fVerbs.count() == 0)
    {
        fPts.append()->set(0, 0);
        *fVerbs.append() = kMove_Verb;
    }
    SkPoint* pts = fPts.append(3);
    pts[0].set(x1, y1);
    pts[1].set(x2, y2);
    pts[2].set(x3, y3);
    *fVerbs.append() = kCubic_Verb;
}

void SkPath::rCubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3)
{
    SkPoint pt;
    this->getLastPt(&pt);
    this->cubicTo(pt.fX + x1, pt.fY + y1, pt.fX + x2, pt.fY + y2, pt.fX + x3, pt.fY + y3);
}

void SkPath::close()
{
    int count = fVerbs.count();
    if (count > 0)
    {
        switch (fVerbs[count - 1]) {
        case kLine_Verb:
        case kQuad_Verb:
        case kCubic_Verb:
            *fVerbs.append() = kClose_Verb;
            break;
        default:
            // don't add a close if the prev wasn't a primitive
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////

void SkPath::addRect(const SkRect& rect, Direction dir)
{
    this->addRect(rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, dir);
}

void SkPath::addRect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom, Direction dir)
{
    this->moveTo(left, top);
    if (dir == kCCW_Direction)
    {
        this->lineTo(left, bottom);
        this->lineTo(right, bottom);
        this->lineTo(right, top);
    }
    else
    {
        this->lineTo(right, top);
        this->lineTo(right, bottom);
        this->lineTo(left, bottom);
    }
    this->close();
}

#define CUBIC_ARC_FACTOR    ((SK_ScalarSqrt2 - SK_Scalar1) * 4 / 3)

void SkPath::addRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry, Direction dir)
{
    SkScalar    w = rect.width();
    SkScalar    halfW = SkScalarHalf(w);
    SkScalar    h = rect.height();
    SkScalar    halfH = SkScalarHalf(h);

    if (halfW <= 0 || halfH <= 0)
        return;

    bool    skip_hori = rx >= halfW;
    bool    skip_vert = ry >= halfH;

    if (skip_hori && skip_vert)
    {
        this->addOval(rect, dir);
        return;
    }
    if (skip_hori)
        rx = halfW;
    else if (skip_vert)
        ry = halfH;

    SkScalar    sx = SkScalarMul(rx, CUBIC_ARC_FACTOR);
    SkScalar    sy = SkScalarMul(ry, CUBIC_ARC_FACTOR);

    this->incReserve(17);
    this->moveTo(rect.fRight - rx, rect.fTop);
    if (dir == kCCW_Direction)
    {
        if (!skip_hori)
            this->lineTo(rect.fLeft + rx, rect.fTop);       // top
        this->cubicTo(rect.fLeft + rx - sx, rect.fTop,
                      rect.fLeft, rect.fTop + ry - sy,
                      rect.fLeft, rect.fTop + ry);          // top-left
        if (!skip_vert)
            this->lineTo(rect.fLeft, rect.fBottom - ry);        // left
        this->cubicTo(rect.fLeft, rect.fBottom - ry + sy,
                      rect.fLeft + rx - sx, rect.fBottom,
                      rect.fLeft + rx, rect.fBottom);       // bot-left
        if (!skip_hori)
            this->lineTo(rect.fRight - rx, rect.fBottom);   // bottom
        this->cubicTo(rect.fRight - rx + sx, rect.fBottom,
                      rect.fRight, rect.fBottom - ry + sy,
                      rect.fRight, rect.fBottom - ry);      // bot-right
        if (!skip_vert)
            this->lineTo(rect.fRight, rect.fTop + ry);
        this->cubicTo(rect.fRight, rect.fTop + ry - sy,
                      rect.fRight - rx + sx, rect.fTop,
                      rect.fRight - rx, rect.fTop);         // top-right
    }
    else
    {
        this->cubicTo(rect.fRight - rx + sx, rect.fTop,
                      rect.fRight, rect.fTop + ry - sy,
                      rect.fRight, rect.fTop + ry);         // top-right
        if (!skip_vert)
            this->lineTo(rect.fRight, rect.fBottom - ry);
        this->cubicTo(rect.fRight, rect.fBottom - ry + sy,
                      rect.fRight - rx + sx, rect.fBottom,
                      rect.fRight - rx, rect.fBottom);      // bot-right
        if (!skip_hori)
            this->lineTo(rect.fLeft + rx, rect.fBottom);    // bottom
        this->cubicTo(rect.fLeft + rx - sx, rect.fBottom,
                      rect.fLeft, rect.fBottom - ry + sy,
                      rect.fLeft, rect.fBottom - ry);       // bot-left
        if (!skip_vert)
            this->lineTo(rect.fLeft, rect.fTop + ry);       // left
        this->cubicTo(rect.fLeft, rect.fTop + ry - sy,
                      rect.fLeft + rx - sx, rect.fTop,
                      rect.fLeft + rx, rect.fTop);          // top-left
        if (!skip_hori)
            this->lineTo(rect.fRight - rx, rect.fTop);      // top
    }
    this->close();
}

void SkPath::addOval(const SkRect& oval, Direction dir)
{
    SkScalar    cx = oval.centerX();
    SkScalar    cy = oval.centerY();
    SkScalar    rx = SkScalarHalf(oval.width());
    SkScalar    ry = SkScalarHalf(oval.height());
#if 1   // these seem faster than using quads (1/2 the number of edges to process)
    SkScalar    sx = SkScalarMul(rx, CUBIC_ARC_FACTOR);
    SkScalar    sy = SkScalarMul(ry, CUBIC_ARC_FACTOR);

    this->incReserve(13);
    this->moveTo(cx + rx, cy);
    if (dir == kCCW_Direction)
    {
        this->cubicTo(cx + rx, cy - sy, cx + sx, cy - ry, cx, cy - ry);
        this->cubicTo(cx - sx, cy - ry, cx - rx, cy - sy, cx - rx, cy);
        this->cubicTo(cx - rx, cy + sy, cx - sx, cy + ry, cx, cy + ry);
        this->cubicTo(cx + sx, cy + ry, cx + rx, cy + sy, cx + rx, cy);
    }
    else
    {
        this->cubicTo(cx + rx, cy + sy, cx + sx, cy + ry, cx, cy + ry);
        this->cubicTo(cx - sx, cy + ry, cx - rx, cy + sy, cx - rx, cy);
        this->cubicTo(cx - rx, cy - sy, cx - sx, cy - ry, cx, cy - ry);
        this->cubicTo(cx + sx, cy - ry, cx + rx, cy - sy, cx + rx, cy);
    }
#else
    SkScalar    sx = SkScalarMul(rx, SK_ScalarTanPIOver8);
    SkScalar    sy = SkScalarMul(ry, SK_ScalarTanPIOver8);
    SkScalar    mx = SkScalarMul(rx, SK_ScalarRoot2Over2);
    SkScalar    my = SkScalarMul(ry, SK_ScalarRoot2Over2);

    this->incReserve(16);
    this->moveTo(cx + rx, cy);
    if (dir == kCCW_Direction)
    {
        this->quadTo(cx + rx, cy - sy, cx + mx, cy - my);
        this->quadTo(cx + sx, cy - ry, cx +  0, cy - ry);
        this->quadTo(cx - sx, cy - ry, cx - mx, cy - my);
        this->quadTo(cx - rx, cy - sy, cx - rx, cy -  0);
        this->quadTo(cx - rx, cy + sy, cx - mx, cy + my);
        this->quadTo(cx - sx, cy + ry, cx -  0, cy + ry);
        this->quadTo(cx + sx, cy + ry, cx + mx, cy + my);
        this->quadTo(cx + rx, cy + sy, cx + rx, cy +  0);
    }
    else
    {
        this->quadTo(cx + rx, cy + sy, cx + mx, cy + my);
        this->quadTo(cx + sx, cy + ry, cx -  0, cy + ry);
        this->quadTo(cx - sx, cy + ry, cx - mx, cy + my);
        this->quadTo(cx - rx, cy + sy, cx - rx, cy -  0);
        this->quadTo(cx - rx, cy - sy, cx - mx, cy - my);
        this->quadTo(cx - sx, cy - ry, cx +  0, cy - ry);
        this->quadTo(cx + sx, cy - ry, cx + mx, cy - my);
        this->quadTo(cx + rx, cy - sy, cx + rx, cy +  0);
    }
#endif
    this->close();
}

void SkPath::addCircle(SkScalar x, SkScalar y, SkScalar r, Direction dir)
{
    if (r > 0)
    {
        SkRect  rect;
        rect.set(x - r, y - r, x + r, y + r);
        this->addOval(rect, dir);
    }
}

#include "SkGeometry.h"

static int build_arc_points(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                            SkPoint pts[kSkBuildQuadArcStorage])
{
    SkVector start, stop;

    start.fY = SkScalarSinCos(SkDegreesToRadians(startAngle), &start.fX);
    stop.fY = SkScalarSinCos(SkDegreesToRadians(startAngle + sweepAngle), &stop.fX);
        
    SkMatrix    matrix;
    
    matrix.setScale(SkScalarHalf(oval.width()), SkScalarHalf(oval.height()));
    matrix.postTranslate(oval.centerX(), oval.centerY());
    
    return SkBuildQuadArc(start, stop,
                          sweepAngle > 0 ? kCW_SkRotationDirection : kCCW_SkRotationDirection,
                          &matrix, pts);
}

void SkPath::arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo)
{
    SkPoint pts[kSkBuildQuadArcStorage];
    int count = build_arc_points(oval, startAngle, sweepAngle, pts);

    if (fVerbs.count() == 0)
        forceMoveTo = true;

    this->incReserve(count);
    forceMoveTo ? this->moveTo(pts[0]) : this->lineTo(pts[0]);
    for (int i = 1; i < count; i += 2)
        this->quadTo(pts[i], pts[i+1]);
}

void SkPath::addArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle)
{
    if (oval.isEmpty() || 0 == sweepAngle)
        return;
    
    const SkScalar kFullCircleAngle = SkIntToScalar(360);

    if (sweepAngle >= kFullCircleAngle || sweepAngle <= -kFullCircleAngle)
    {
        this->addOval(oval, sweepAngle > 0 ? kCW_Direction : kCCW_Direction);
        return;
    }

    SkPoint pts[kSkBuildQuadArcStorage];
    int count = build_arc_points(oval, startAngle, sweepAngle, pts);

    this->incReserve(count);
    this->moveTo(pts[0]);
    for (int i = 1; i < count; i += 2)
        this->quadTo(pts[i], pts[i+1]);
}

void SkPath::addPath(const SkPath& path, SkScalar dx, SkScalar dy)
{
    SkMatrix matrix;

    matrix.setTranslate(dx, dy);
    this->addPath(path, matrix);
}

void SkPath::addPath(const SkPath& path, const SkMatrix& matrix)
{
    this->incReserve(path.fPts.count());

    Iter    iter(path, false);
    SkPoint pts[4];
    Verb    verb;

    SkMatrix::TypeMask mask = matrix.getType();

    while ((verb = iter.next(pts)) != kDone_Verb)
    {
        switch (verb) {
        case kMove_Verb:
            matrix.mapPoints(&pts[0], &pts[0], 1, mask);
            this->moveTo(pts[0]);
            break;
        case kLine_Verb:
            matrix.mapPoints(&pts[1], &pts[1], 1, mask);
            this->lineTo(pts[1]);
            break;
        case kQuad_Verb:
            matrix.mapPoints(&pts[1], &pts[1], 2, mask);
            this->quadTo(pts[1], pts[2]);
            break;
        case kCubic_Verb:
            matrix.mapPoints(&pts[1], &pts[1], 3, mask);
            this->cubicTo(pts[1], pts[2], pts[3]);
            break;
        case kClose_Verb:
            this->close();
            break;
        default:
            SkASSERT(!"unknown verb");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////

static const U8 gPtsInVerb[] = {
    1,  // kMove
    1,  // kLine
    2,  // kQuad
    3,  // kCubic
    0,  // kClose
    0   // kDone
};

// ignore the initial moveto, and stop when the 1st contour ends
void SkPath::pathTo(const SkPath& path)
{
    int i, vcount = path.fVerbs.count();
    if (vcount == 0)
        return;

    const U8*       verbs = path.fVerbs.begin();
    const SkPoint*  pts = path.fPts.begin() + 1;    // 1 for the initial moveTo

    SkASSERT(verbs[0] == kMove_Verb);
    for (i = 1; i < vcount; i++)
    {
        switch (verbs[i]) {
        case kLine_Verb:
            this->lineTo(pts[0].fX, pts[0].fY);
            break;
        case kQuad_Verb:
            this->quadTo(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY);
            break;
        case kCubic_Verb:
            this->cubicTo(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
            break;
        case kClose_Verb:
            return;
        }
        pts += gPtsInVerb[verbs[i]];
    }
}

// ignore the last point of the 1st contour
void SkPath::reversePathTo(const SkPath& path)
{
    int i, vcount = path.fVerbs.count();
    if (vcount == 0)
        return;

    const U8*       verbs = path.fVerbs.begin();
    const SkPoint*  pts = path.fPts.begin();

    SkASSERT(verbs[0] == kMove_Verb);
    for (i = 1; i < vcount; i++)
    {
        int n = gPtsInVerb[verbs[i]];
        if (n == 0)
            break;
        pts += n;
    }

    while (--i > 0)
    {
        switch (verbs[i]) {
        case kLine_Verb:
            this->lineTo(pts[-1].fX, pts[-1].fY);
            break;
        case kQuad_Verb:
            this->quadTo(pts[-1].fX, pts[-1].fY, pts[-2].fX, pts[-2].fY);
            break;
        case kCubic_Verb:
            this->cubicTo(pts[-1].fX, pts[-1].fY, pts[-2].fX, pts[-2].fY, pts[-3].fX, pts[-3].fY);
            break;
        default:
            SkASSERT(!"bad verb");
            break;
        }
        pts -= gPtsInVerb[verbs[i]];
    }
}

////////////////////////////////////////////////////////////////////////////////////

bool SkPath::offset(SkScalar dx, SkScalar dy, SkPath* dst) const
{
    SkMatrix    matrix;

    matrix.setTranslate(dx, dy);
    return this->transform(matrix, dst);
}

#include "SkGeometry.h"

static void subdivide_quad_to(SkPath* path, const SkPoint pts[3], int level = 2)
{
    if (--level >= 0)
    {
        SkPoint tmp[5];

        SkChopQuadAtHalf(pts, tmp);
        subdivide_quad_to(path, &tmp[0], level);
        subdivide_quad_to(path, &tmp[2], level);
    }
    else
        path->quadTo(pts[1], pts[2]);
}

static void subdivide_cubic_to(SkPath* path, const SkPoint pts[4], int level = 2)
{
    if (--level >= 0)
    {
        SkPoint tmp[7];

        SkChopCubicAtHalf(pts, tmp);
        subdivide_cubic_to(path, &tmp[0], level);
        subdivide_cubic_to(path, &tmp[3], level);
    }
    else
        path->cubicTo(pts[1], pts[2], pts[3]);
}

bool SkPath::transform(const SkMatrix& matrix, SkPath* dst) const
{
    if (dst == nil)
        dst = (SkPath*)this;

    if (matrix.getType() & SkMatrix::kPerspective_Mask)
    {
        SkPath  tmp;
        tmp.fFillType = fFillType;

        SkPath::Iter    iter(*this, false);
        SkPoint         pts[4];
        SkPath::Verb    verb;

        while ((verb = iter.next(pts)) != kDone_Verb)
        {
            switch (verb) {
            case kMove_Verb:
                tmp.moveTo(pts[0]);
                break;
            case kLine_Verb:
                tmp.lineTo(pts[1]);
                break;
            case kQuad_Verb:
                subdivide_quad_to(&tmp, pts);
                break;
            case kCubic_Verb:
                subdivide_cubic_to(&tmp, pts);
                break;
            case kClose_Verb:
                tmp.close();
                break;
            default:
                SkASSERT(!"unknown verb");
                break;
            }
        }

        dst->swap(tmp);
        return matrix.mapPoints(dst->fPts.begin(), dst->fPts.count());
    }

    else
    {
        if (this != dst)
        {
            dst->fVerbs = fVerbs;
            dst->fPts.setCount(fPts.count());
            dst->fFillType = fFillType;
        }
        return matrix.mapPoints(dst->fPts.begin(), fPts.begin(), fPts.count());
    }
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

enum NeedMoveToState {
    kAfterClose_NeedMoveToState,
    kAfterCons_NeedMoveToState,
    kAfterPrefix_NeedMoveToState
};

SkPath::Iter::Iter()
{
#ifdef SK_DEBUG
    fPts = nil;
    fMoveTo.fX = fMoveTo.fY = fLastPt.fX = fLastPt.fY = 0;
    fForceClose = fNeedMoveTo = fCloseLine = false;
#endif
    // need to init enough to make next() harmlessly return kDone_Verb
    fVerbs = nil;
    fVerbStop = nil;
    fNeedClose = false;
}

SkPath::Iter::Iter(const SkPath& path, bool forceClose)
{
    this->setPath(path, forceClose);
}

void SkPath::Iter::setPath(const SkPath& path, bool forceClose)
{
    fPts = path.fPts.begin();
    fVerbs = path.fVerbs.begin();
    fVerbStop = path.fVerbs.end();
    fForceClose = SkToU8(forceClose);
    fNeedClose = false;
    fNeedMoveTo = kAfterPrefix_NeedMoveToState;
}

bool SkPath::Iter::isClosedContour() const
{
    if (fVerbs == nil || fVerbs == fVerbStop)
        return false;
    if (fForceClose)
        return true;

    const uint8_t* verbs = fVerbs;
    const uint8_t* stop = fVerbStop;
    
    if (kMove_Verb == *verbs)
        verbs += 1; // skip the initial moveto

    while (verbs < stop)
    {
        unsigned v = *verbs++;        
        if (kMove_Verb == v)
            break;
        if (kClose_Verb == v)
            return true;
    }
    return false;
}

SkPath::Verb SkPath::Iter::autoClose(SkPoint pts[2])
{
    if (fLastPt != fMoveTo)
    {
        if (pts)
        {
            pts[0] = fLastPt;
            pts[1] = fMoveTo;
        }
        fLastPt = fMoveTo;
        fCloseLine = true;
        return kLine_Verb;
    }
    return kClose_Verb;
}

bool SkPath::Iter::cons_moveTo(SkPoint pts[1])
{
    if (fNeedMoveTo == kAfterClose_NeedMoveToState)
    {
        if (pts)
            *pts = fMoveTo;
        fNeedClose = fForceClose;
        fNeedMoveTo = kAfterCons_NeedMoveToState;
        fVerbs -= 1;
        return true;
    }

    if (fNeedMoveTo == kAfterCons_NeedMoveToState)
    {
        if (pts)
            *pts = fMoveTo;
        fNeedMoveTo = kAfterPrefix_NeedMoveToState;
    }
    else
    {
        SkASSERT(fNeedMoveTo == kAfterPrefix_NeedMoveToState);
        if (pts)
            *pts = fPts[-1];
    }
    return false;
}

SkPath::Verb SkPath::Iter::next(SkPoint pts[4])
{
    if (fVerbs == fVerbStop)
    {
        if (fNeedClose)
        {
            if (kLine_Verb == this->autoClose(pts))
                return kLine_Verb;
            fNeedClose = false;
            return kClose_Verb;
        }
        return kDone_Verb;
    }

    unsigned        verb = *fVerbs++;
    const SkPoint*  srcPts = fPts;

    switch (verb) {
    case kMove_Verb:
        if (fNeedClose)
        {
            fVerbs -= 1;
            verb = this->autoClose(pts);
            if (verb == kClose_Verb)
                fNeedClose = false;
            return (Verb)verb;
        }
        if (fVerbs == fVerbStop)    // might be a trailing moveto
            return kDone_Verb;
        fMoveTo = *srcPts;
        if (pts)
            pts[0] = *srcPts;
        srcPts += 1;
        fNeedMoveTo = kAfterCons_NeedMoveToState;
        fNeedClose = fForceClose;
        break;
    case kLine_Verb:
        if (this->cons_moveTo(pts))
            return kMove_Verb;
        if (pts)
            pts[1] = srcPts[0];
        fLastPt = srcPts[0];
        fCloseLine = false;
        srcPts += 1;
        break;
    case kQuad_Verb:
        if (this->cons_moveTo(pts))
            return kMove_Verb;
        if (pts)
            memcpy(&pts[1], srcPts, 2 * sizeof(SkPoint));
        fLastPt = srcPts[1];
        srcPts += 2;
        break;
    case kCubic_Verb:
        if (this->cons_moveTo(pts))
            return kMove_Verb;
        if (pts)
            memcpy(&pts[1], srcPts, 3 * sizeof(SkPoint));
        fLastPt = srcPts[2];
        srcPts += 3;
        break;
    case kClose_Verb:
        verb = this->autoClose(pts);
        if (verb == kLine_Verb)
            fVerbs -= 1;
        else
            fNeedClose = false;
        fNeedMoveTo = kAfterClose_NeedMoveToState;
        break;
    }
    fPts = srcPts;
    return (Verb)verb;
}

///////////////////////////////////////////////////////////////////////

static bool exceeds_dist(const SkScalar p[], const SkScalar q[], SkScalar dist, int count)
{
    SkASSERT(dist > 0);

    count *= 2;
    for (int i = 0; i < count; i++)
        if (SkScalarAbs(p[i] - q[i]) > dist)
            return true;
    return false;
}

static void subdivide_quad(SkPath* dst, const SkPoint pts[3], SkScalar dist, int subLevel = 4)
{
    if (--subLevel >= 0 && exceeds_dist(&pts[0].fX, &pts[1].fX, dist, 4))
    {
        SkPoint tmp[5];
        SkChopQuadAtHalf(pts, tmp);

        subdivide_quad(dst, &tmp[0], dist, subLevel);
        subdivide_quad(dst, &tmp[2], dist, subLevel);
    }
    else
        dst->quadTo(pts[1], pts[2]);
}

static void subdivide_cubic(SkPath* dst, const SkPoint pts[4], SkScalar dist, int subLevel = 4)
{
    if (--subLevel >= 0 && exceeds_dist(&pts[0].fX, &pts[1].fX, dist, 6))
    {
        SkPoint tmp[7];
        SkChopCubicAtHalf(pts, tmp);

        subdivide_cubic(dst, &tmp[0], dist, subLevel);
        subdivide_cubic(dst, &tmp[3], dist, subLevel);
    }
    else
        dst->cubicTo(pts[1], pts[2], pts[3]);
}

void SkPath::subdivide(SkScalar dist, bool bendLines, SkPath* dst) const
{
    SkPath  tmpPath;
    if (nil == dst || this == dst)
        dst = &tmpPath;

    SkPath::Iter    iter(*this, false);
    SkPoint         pts[4];

    for (;;)
    {
        switch (iter.next(pts)) {
        case SkPath::kMove_Verb:
            dst->moveTo(pts[0]);
            break;
        case SkPath::kLine_Verb:
            if (!bendLines)
            {
                dst->lineTo(pts[1]);
                break;
            }
            // construct a quad from the line
            pts[2] = pts[1];
            pts[1].set(SkScalarAve(pts[0].fX, pts[2].fX), SkScalarAve(pts[0].fY, pts[2].fY));
            // fall through to the quad case
        case SkPath::kQuad_Verb:
            subdivide_quad(dst, pts, dist);
            break;
        case SkPath::kCubic_Verb:
            subdivide_cubic(dst, pts, dist);
            break;
        case SkPath::kClose_Verb:
            dst->close();
            break;
        case SkPath::kDone_Verb:
            goto DONE;
        }
    }
DONE:
    if (&tmpPath == dst)    // i.e. the dst should be us
        dst->swap(*(SkPath*)this);
}

///////////////////////////////////////////////////////////////////////
/*
    Format in flattened buffer: [ptCount, verbCount, pts[], verbs[]]
*/

#include "SkBuffer.h"

U32 SkPath::flatten(void* storage) const
{
    if (storage)
    {
        SkWBuffer   buffer(storage);
        
        buffer.write32(fPts.count());
        buffer.write32(fVerbs.count());
        buffer.write32(fFillType);
        buffer.write(fPts.begin(), sizeof(SkPoint) * fPts.count());
        buffer.write(fVerbs.begin(), fVerbs.count());
        buffer.padToAlign4();
    }
    return 3 * sizeof(int32_t) + sizeof(SkPoint) * fPts.count() + SkAlign4(fVerbs.count());
}

void SkPath::unflatten(const void* storage)
{
    SkRBuffer   buffer(storage);

    fPts.setCount(buffer.readS32());
    fVerbs.setCount(buffer.readS32());
    fFillType = buffer.readS32();
    buffer.read(fPts.begin(), sizeof(SkPoint) * fPts.count());
    buffer.read(fVerbs.begin(), fVerbs.count());
    buffer.skipToAlign4();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkString.h"
#include "SkStream.h"

static void write_scalar(SkWStream* stream, SkScalar value)
{
    char    buffer[SkStrAppendScalar_MaxSize];
    char*   stop = SkStrAppendScalar(buffer, value);
    stream->write(buffer, stop - buffer);
}

static void append_scalars(SkWStream* stream, char verb, const SkScalar data[], int count)
{
    stream->write(&verb, 1);
    write_scalar(stream, data[0]);
    for (int i = 1; i < count; i++) {
        if (data[i] >= 0)
            stream->write(" ", 1);   // can skip the separater if data[i] is negative
        write_scalar(stream, data[i]);
    }
}

void SkPath::toString(SkString* str) const
{
    SkDynamicMemoryWStream  stream;

    SkPath::Iter    iter(*this, false);
    SkPoint         pts[4];
    
    for (;;) {
        switch (iter.next(pts)) {
        case SkPath::kMove_Verb:
            append_scalars(&stream, 'M', &pts[0].fX, 2);
            break;
        case SkPath::kLine_Verb:
            append_scalars(&stream, 'L', &pts[1].fX, 2);
            break;
        case SkPath::kQuad_Verb:
            append_scalars(&stream, 'Q', &pts[1].fX, 4);
            break;
        case SkPath::kCubic_Verb:
            append_scalars(&stream, 'C', &pts[1].fX, 6);
            break;
        case SkPath::kClose_Verb:
            stream.write("Z", 1);
            break;
        case SkPath::kDone_Verb:
            str->resize(stream.getOffset());
            stream.copyTo(str->writable_str());
            return;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

#if 0   // test to ensure that the iterator returns the same data as the path
void SkPath::test() const
{
    Iter    iter(*this, false);
    SkPoint pts[4];
    Verb    verb;

    const U8*       verbs = fVerbs.begin();
    const SkPoint*  points = fPts.begin();

    while ((verb = iter.next(pts)) != kDone_Verb)
    {
        SkASSERT(*verbs == verb);
        verbs += 1;

        int count;
        switch (verb) {
        case kMove_Verb:
            count = 1;
            break;
        case kLine_Verb:
            count = 2;
            break;
        case kQuad_Verb:
            count = 3;
            break;
        case kCubic_Verb:
            count = 4;
            break;
        case kClose_Verb:
        default:
            count = 0;
            break;
        }
        if (count > 1)
            points -= 1;
        SkASSERT(memcmp(pts, points, count * sizeof(SkPoint)) == 0);
        points += count;
    }

    int vc = fVerbs.count(), pc = fPts.count();
    if (vc && fVerbs.begin()[vc-1] == kMove_Verb)
    {
        vc -= 1;
        pc -= 1;
    }
    SkASSERT(verbs - fVerbs.begin() == vc);
    SkASSERT(points - fPts.begin() == pc);
}
#endif

void SkPath::dump(bool forceClose, const char title[]) const
{
    Iter    iter(*this, forceClose);
    SkPoint pts[4];
    Verb    verb;

    SkDebugf("path: forceClose=%s %s\n", forceClose ? "true" : "false", title ? title : "");

    while ((verb = iter.next(pts)) != kDone_Verb)
    {
        switch (verb) {
        case kMove_Verb:
#ifdef SK_CAN_USE_FLOAT
            SkDebugf("  path: moveTo [%g %g]\n",
                    SkScalarToFloat(pts[0].fX), SkScalarToFloat(pts[0].fY));
#else
            SkDebugf("  path: moveTo [%x %x]\n", pts[0].fX, pts[0].fY);
#endif
            break;
        case kLine_Verb:
#ifdef SK_CAN_USE_FLOAT
            SkDebugf("  path: lineTo [%g %g]\n",
                    SkScalarToFloat(pts[1].fX), SkScalarToFloat(pts[1].fY));
#else
            SkDebugf("  path: lineTo [%x %x]\n", pts[1].fX, pts[1].fY);
#endif
            break;
        case kQuad_Verb:
#ifdef SK_CAN_USE_FLOAT
            SkDebugf("  path: quadTo [%g %g] [%g %g]\n",
                    SkScalarToFloat(pts[1].fX), SkScalarToFloat(pts[1].fY),
                    SkScalarToFloat(pts[2].fX), SkScalarToFloat(pts[2].fY));
#else
            SkDebugf("  path: quadTo [%x %x] [%x %x]\n", pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY);
#endif
            break;
        case kCubic_Verb:
#ifdef SK_CAN_USE_FLOAT
            SkDebugf("  path: cubeTo [%g %g] [%g %g] [%g %g]\n",
                    SkScalarToFloat(pts[1].fX), SkScalarToFloat(pts[1].fY),
                    SkScalarToFloat(pts[2].fX), SkScalarToFloat(pts[2].fY),
                    SkScalarToFloat(pts[3].fX), SkScalarToFloat(pts[3].fY));
#else
            SkDebugf("  path: cubeTo [%x %x] [%x %x] [%x %x]\n",
                    pts[1].fX, pts[1].fY, pts[2].fX, pts[2].fY, pts[3].fX, pts[3].fY);
#endif
            break;
        case kClose_Verb:
            SkDebugf("  path: close\n");
            break;
        default:
            SkDebugf("  path: UNKNOWN VERB %d, aborting dump...\n", verb);
            verb = kDone_Verb;  // stop the loop
            break;
        }
    }
    SkDebugf("path: done %s\n", title ? title : "");
}

#include "SkTSort.h"

void SkPath::UnitTest()
{
#ifdef SK_SUPPORT_UNITTEST
    SkPath  p;
    SkRect  r;

    r.set(0, 0, 10, 20);
    p.addRect(r);
    p.dump(false);
    p.dump(true);

    {
        int array[] = { 5, 3, 7, 2, 6, 1, 2, 9, 5, 0 };
        int i;

        for (i = 0; i < (int)SK_ARRAY_COUNT(array); i++)
            SkDebugf(" %d", array[i]);
        SkDebugf("\n");
        SkTHeapSort<int>(array, SK_ARRAY_COUNT(array));
        for (i = 0; i < (int)SK_ARRAY_COUNT(array); i++)
            SkDebugf(" %d", array[i]);
        SkDebugf("\n");
    }

    {
        SkPath  p;
        SkPoint pt;

        p.moveTo(SK_Scalar1, 0);
        p.getLastPt(&pt);
        SkASSERT(pt.fX == SK_Scalar1);
    }
#endif
}

#endif
