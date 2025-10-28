/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"

#include "include/core/SkArc.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTDArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkFloatBits.h"
#include "src/base/SkVx.h"
#include "src/core/SkCubicClipper.h"
#include "src/core/SkEdgeClipper.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRawShapes.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkStringUtils.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits.h>
#include <utility>

class SkAutoAddSimpleShape {
public:
    SkAutoAddSimpleShape(SkPath* path, SkPathDirection dir)
        : fPath(path)
        , fDirection(dir)
    {
        fIsEffectivelyEmpty = SkPathPriv::IsEffectivelyEmpty(*path);
    }

    ~SkAutoAddSimpleShape() {
        fPath->setConvexity(fIsEffectivelyEmpty ? SkPathDirection_ToConvexity(fDirection)
                                                : SkPathConvexity::kUnknown);
    }

private:
    SkPath*         fPath;
    bool            fIsEffectivelyEmpty;
    SkPathDirection fDirection;
};

////////////////////////////////////////////////////////////////////////////

#ifndef SK_HIDE_PATH_EDIT_METHODS

SkPath& SkPath::rewind() {
    SkDEBUGCODE(this->validate();)

    SkPathRef::Rewind(&fPathRef);
    this->resetFields();
    return *this;
}

/*
    Stores the verbs and points as they are given to us, with exceptions:
    - we only record "Close" if it was immediately preceeded by Move | Line | Quad | Cubic
    - we insert a Move(0,0) if Line | Quad | Cubic is our first command

    The iterator does more cleanup, especially if forceClose == true
    1. If we encounter degenerate segments, remove them
    2. if we encounter Close, return a cons'd up Line() first (if the curr-pt != start-pt)
    3. if we encounter Move without a preceeding Close, and forceClose is true, goto #2
    4. if we encounter Line | Quad | Cubic after Close, cons up a Move
*/

SkPath& SkPath::moveTo(SkScalar x, SkScalar y) {
    SkDEBUGCODE(this->validate();)

    SkPathRef::Editor ed(&fPathRef);

    if (!fPathRef->fVerbs.empty() && fPathRef->fVerbs.back() == SkPathVerb::kMove) {
        fPathRef->fPoints.back() = {x, y};
    } else {
        // remember our index
        fLastMoveToIndex = fPathRef->countPoints();

        ed.growForVerb(SkPathVerb::kMove)->set(x, y);
    }

    return this->dirtyAfterEdit();
}

SkPath& SkPath::rMoveTo(SkScalar x, SkScalar y) {
    SkPoint pt = {0,0};
    int count = fPathRef->countPoints();
    if (count > 0) {
        if (fLastMoveToIndex >= 0) {
            pt = fPathRef->atPoint(count - 1);
        } else {
            pt = fPathRef->atPoint(~fLastMoveToIndex);
        }
    }
    return this->moveTo(pt.fX + x, pt.fY + y);
}

void SkPath::injectMoveToIfNeeded() {
    if (fLastMoveToIndex < 0) {
        SkScalar x, y;
        if (fPathRef->countVerbs() == 0) {
            x = y = 0;
        } else {
            const SkPoint& pt = fPathRef->atPoint(~fLastMoveToIndex);
            x = pt.fX;
            y = pt.fY;
        }
        this->moveTo(x, y);
    }
}

SkPath& SkPath::lineTo(SkScalar x, SkScalar y) {
    SkDEBUGCODE(this->validate();)

    this->injectMoveToIfNeeded();

    SkPathRef::Editor ed(&fPathRef);
    ed.growForVerb(SkPathVerb::kLine)->set(x, y);

    return this->dirtyAfterEdit();
}

SkPath& SkPath::rLineTo(SkScalar x, SkScalar y) {
    this->injectMoveToIfNeeded();  // This can change the result of this->getLastPt().
    SkPoint pt = this->getLastPt().value_or(SkPoint{0, 0});
    return this->lineTo(pt.fX + x, pt.fY + y);
}

SkPath& SkPath::quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
    SkDEBUGCODE(this->validate();)

    this->injectMoveToIfNeeded();

    SkPathRef::Editor ed(&fPathRef);
    SkPoint* pts = ed.growForVerb(SkPathVerb::kQuad);
    pts[0].set(x1, y1);
    pts[1].set(x2, y2);

    return this->dirtyAfterEdit();
}

SkPath& SkPath::rQuadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2) {
    this->injectMoveToIfNeeded();  // This can change the result of this->getLastPt().
    SkPoint pt = this->getLastPt().value_or(SkPoint{0, 0});
    return this->quadTo(pt.fX + x1, pt.fY + y1, pt.fX + x2, pt.fY + y2);
}

SkPath& SkPath::conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                        SkScalar w) {
    // check for <= 0 or NaN with this test
    if (!(w > 0)) {
        this->lineTo(x2, y2);
    } else if (!SkIsFinite(w)) {
        this->lineTo(x1, y1);
        this->lineTo(x2, y2);
    } else if (SK_Scalar1 == w) {
        this->quadTo(x1, y1, x2, y2);
    } else {
        SkDEBUGCODE(this->validate();)

        this->injectMoveToIfNeeded();

        SkPathRef::Editor ed(&fPathRef);
        SkPoint* pts = ed.growForVerb(SkPathVerb::kConic, w);
        pts[0].set(x1, y1);
        pts[1].set(x2, y2);

        (void)this->dirtyAfterEdit();
    }
    return *this;
}

SkPath& SkPath::rConicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2,
                         SkScalar w) {
    this->injectMoveToIfNeeded();  // This can change the result of this->getLastPt().
    SkPoint pt = this->getLastPt().value_or(SkPoint{0, 0});
    return this->conicTo(pt.fX + dx1, pt.fY + dy1, pt.fX + dx2, pt.fY + dy2, w);
}

SkPath& SkPath::cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                        SkScalar x3, SkScalar y3) {
    SkDEBUGCODE(this->validate();)

    this->injectMoveToIfNeeded();

    SkPathRef::Editor ed(&fPathRef);
    SkPoint* pts = ed.growForVerb(SkPathVerb::kCubic);
    pts[0].set(x1, y1);
    pts[1].set(x2, y2);
    pts[2].set(x3, y3);

    return this->dirtyAfterEdit();
}

SkPath& SkPath::rCubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2,
                         SkScalar x3, SkScalar y3) {
    this->injectMoveToIfNeeded();  // This can change the result of this->getLastPt().
    SkPoint pt = this->getLastPt().value_or(SkPoint{0, 0});
    return this->cubicTo(pt.fX + x1, pt.fY + y1, pt.fX + x2, pt.fY + y2,
                         pt.fX + x3, pt.fY + y3);
}

SkPath& SkPath::close() {
    SkDEBUGCODE(this->validate();)

    if (!fPathRef->verbs().empty()) {
        switch (fPathRef->verbs().back()) {
            case SkPathVerb::kLine:
            case SkPathVerb::kQuad:
            case SkPathVerb::kConic:
            case SkPathVerb::kCubic:
            case SkPathVerb::kMove: {
                SkPathRef::Editor ed(&fPathRef);
                ed.growForVerb(SkPathVerb::kClose);
                break;
            }
            case SkPathVerb::kClose:
                // don't add a close if it's the first verb or a repeat
                break;
        }
    }

    // signal that we need a moveTo to follow us (unless we're done)
#if 0
    if (fLastMoveToIndex >= 0) {
        fLastMoveToIndex = ~fLastMoveToIndex;
    }
#else
    fLastMoveToIndex ^= ~fLastMoveToIndex >> (8 * sizeof(fLastMoveToIndex) - 1);
#endif
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

static void assert_known_direction(SkPathDirection dir) {
    SkASSERT(SkPathDirection::kCW == dir || SkPathDirection::kCCW == dir);
}

SkPath& SkPath::addRect(const SkRect &rect, SkPathDirection dir, unsigned startIndex) {
    assert_known_direction(dir);

    SkAutoAddSimpleShape addc(this, dir);

    this->addRaw(SkPathRawShapes::Rect(rect, dir, startIndex));

    return *this;
}

SkPath& SkPath::addPoly(SkSpan<const SkPoint> pts, bool close) {
    SkDEBUGCODE(this->validate();)
    if (pts.empty()) {
        return *this;
    }
    const int count = SkToInt(pts.size());

    fLastMoveToIndex = fPathRef->countPoints();

    // +close makes room for the extra SkPathVerb::kClose
    SkPathRef::Editor ed(&fPathRef, count+close, count);

    ed.growForVerb(SkPathVerb::kMove)->set(pts[0].fX, pts[0].fY);
    if (count > 1) {
        SkPoint* p = ed.growForRepeatedVerb(SkPathVerb::kLine, count - 1);
        memcpy(p, &pts[1], (count-1) * sizeof(SkPoint));
    }

    if (close) {
        ed.growForVerb(SkPathVerb::kClose);
        fLastMoveToIndex ^= ~fLastMoveToIndex >> (8 * sizeof(fLastMoveToIndex) - 1);
    }

    (void)this->dirtyAfterEdit();
    SkDEBUGCODE(this->validate();)
    return *this;
}

void SkPath::addRaw(const SkPathRaw& raw) {
    this->incReserve(raw.points().size(), raw.verbs().size());

    for (auto iter = raw.iter(); auto rec = iter.next();) {
        const auto pts = rec->fPoints;
        switch (rec->fVerb) {
            case SkPathVerb::kMove:  this->moveTo( pts[0]); break;
            case SkPathVerb::kLine:  this->lineTo( pts[1]); break;
            case SkPathVerb::kQuad:  this->quadTo( pts[1], pts[2]); break;
            case SkPathVerb::kConic: this->conicTo(pts[1], pts[2], rec->fConicWeight); break;
            case SkPathVerb::kCubic: this->cubicTo(pts[1], pts[2], pts[3]); break;
            case SkPathVerb::kClose: this->close(); break;
        }
    }
}

static bool arc_is_lone_point(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                              SkPoint* pt) {
    if (0 == sweepAngle && (0 == startAngle || SkIntToScalar(360) == startAngle)) {
        // Chrome uses this path to move into and out of ovals. If not
        // treated as a special case the moves can distort the oval's
        // bounding box (and break the circle special case).
        pt->set(oval.fRight, oval.centerY());
        return true;
    } else if (0 == oval.width() && 0 == oval.height()) {
        // Chrome will sometimes create 0 radius round rects. Having degenerate
        // quad segments in the path prevents the path from being recognized as
        // a rect.
        // TODO: optimizing the case where only one of width or height is zero
        // should also be considered. This case, however, doesn't seem to be
        // as common as the single point case.
        pt->set(oval.fRight, oval.fTop);
        return true;
    }
    return false;
}

// Return the unit vectors pointing at the start/stop points for the given start/sweep angles
//
static void angles_to_unit_vectors(SkScalar startAngle, SkScalar sweepAngle,
                                   SkVector* startV, SkVector* stopV, SkPathDirection* dir) {
    SkScalar startRad = SkDegreesToRadians(startAngle),
             stopRad  = SkDegreesToRadians(startAngle + sweepAngle);

    startV->fY = SkScalarSinSnapToZero(startRad);
    startV->fX = SkScalarCosSnapToZero(startRad);
    stopV->fY = SkScalarSinSnapToZero(stopRad);
    stopV->fX = SkScalarCosSnapToZero(stopRad);

    /*  If the sweep angle is nearly (but less than) 360, then due to precision
     loss in radians-conversion and/or sin/cos, we may end up with coincident
     vectors, which will fool SkBuildQuadArc into doing nothing (bad) instead
     of drawing a nearly complete circle (good).
     e.g. canvas.drawArc(0, 359.99, ...)
     -vs- canvas.drawArc(0, 359.9, ...)
     We try to detect this edge case, and tweak the stop vector
     */
    if (*startV == *stopV) {
        SkScalar sw = SkScalarAbs(sweepAngle);
        if (sw < SkIntToScalar(360) && sw > SkIntToScalar(359)) {
            // make a guess at a tiny angle (in radians) to tweak by
            SkScalar deltaRad = SkScalarCopySign(SK_Scalar1/512, sweepAngle);
            // not sure how much will be enough, so we use a loop
            do {
                stopRad -= deltaRad;
                stopV->fY = SkScalarSinSnapToZero(stopRad);
                stopV->fX = SkScalarCosSnapToZero(stopRad);
            } while (*startV == *stopV);
        }
    }
    *dir = sweepAngle > 0 ? SkPathDirection::kCW : SkPathDirection::kCCW;
}

/**
 *  If this returns 0, then the caller should just line-to the singlePt, else it should
 *  ignore singlePt and append the specified number of conics.
 */
static int build_arc_conics(const SkRect& oval, const SkVector& start, const SkVector& stop,
                            SkPathDirection dir, SkConic conics[SkConic::kMaxConicsForArc],
                            SkPoint* singlePt) {
    SkMatrix    matrix;

    matrix.setScale(SkScalarHalf(oval.width()), SkScalarHalf(oval.height()));
    matrix.postTranslate(oval.centerX(), oval.centerY());

    int count = SkConic::BuildUnitArc(start, stop, dir, &matrix, conics);
    if (0 == count) {
        *singlePt = matrix.mapPoint(stop);
    }
    return count;
}

SkPath& SkPath::addRoundRect(const SkRect& rect, SkSpan<const SkScalar> radii,
                             SkPathDirection dir) {
    SkASSERT(radii.size() >= 8);
    SkRRect rrect;
    rrect.setRectRadii(rect, (const SkVector*) radii.data());
    return this->addRRect(rrect, dir);
}

SkPath& SkPath::addRRect(const SkRRect& rrect, SkPathDirection dir) {
    // legacy start indices: 6 (CW) and 7(CCW)
    return this->addRRect(rrect, dir, dir == SkPathDirection::kCW ? 6 : 7);
}

SkPath& SkPath::addRRect(const SkRRect &rrect, SkPathDirection dir, unsigned startIndex) {
    assert_known_direction(dir);

    bool isRRect = hasOnlyMoveTos();
    const SkRect& bounds = rrect.getBounds();

    if (rrect.isRect() || rrect.isEmpty()) {
        // degenerate(rect) => radii points are collapsing
        this->addRect(bounds, dir, (startIndex + 1) / 2);
    } else if (rrect.isOval()) {
        // degenerate(oval) => line points are collapsing
        this->addOval(bounds, dir, startIndex / 2);
    } else {
        SkAutoAddSimpleShape addc(this, dir);

        this->addRaw(SkPathRawShapes::RRect(rrect, dir, startIndex));

        if (isRRect) {
            SkPathRef::Editor ed(&fPathRef);
            ed.setIsRRect(dir, startIndex % 8);
        }
    }

    SkDEBUGCODE(fPathRef->validate();)
    return *this;
}

SkPath& SkPath::addRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry,
                             SkPathDirection dir) {
    assert_known_direction(dir);

    if (rx < 0 || ry < 0) {
        return *this;
    }

    SkRRect rrect;
    rrect.setRectXY(rect, rx, ry);
    return this->addRRect(rrect, dir);
}

SkPath& SkPath::addOval(const SkRect& oval, SkPathDirection dir) {
    // legacy start index: 1
    return this->addOval(oval, dir, 1);
}

SkPath& SkPath::addOval(const SkRect &oval, SkPathDirection dir, unsigned startPointIndex) {
    assert_known_direction(dir);

    /* If addOval() is called after previous moveTo(),
       this path is still marked as an oval. This is used to
       fit into WebKit's calling sequences.
       We can't simply check isEmpty() in this case, as additional
       moveTo() would mark the path non empty.
     */
    bool isOval = hasOnlyMoveTos();

    SkAutoAddSimpleShape addc(this, dir);

    this->addRaw(SkPathRawShapes::Oval(oval, dir, startPointIndex));

    if (isOval) {
        SkPathRef::Editor ed(&fPathRef);
        ed.setIsOval(dir, startPointIndex % 4);
    }
    return *this;
}

SkPath& SkPath::addCircle(SkScalar x, SkScalar y, SkScalar r, SkPathDirection dir) {
    if (r > 0) {
        this->addOval(SkRect::MakeLTRB(x - r, y - r, x + r, y + r), dir);
    }
    return *this;
}

SkPath& SkPath::arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                      bool forceMoveTo) {
    if (oval.width() < 0 || oval.height() < 0) {
        return *this;
    }

    startAngle = SkScalarMod(startAngle, 360.0f);

    if (fPathRef->countVerbs() == 0) {
        forceMoveTo = true;
    }

    SkPoint lonePt;
    if (arc_is_lone_point(oval, startAngle, sweepAngle, &lonePt)) {
        return forceMoveTo ? this->moveTo(lonePt) : this->lineTo(lonePt);
    }

    SkVector startV, stopV;
    SkPathDirection dir;
    angles_to_unit_vectors(startAngle, sweepAngle, &startV, &stopV, &dir);

    SkPoint singlePt;

    // Adds a move-to to 'pt' if forceMoveTo is true. Otherwise a lineTo unless we're sufficiently
    // close to 'pt' currently. This prevents spurious lineTos when adding a series of contiguous
    // arcs from the same oval.
    auto addPt = [&forceMoveTo, this](const SkPoint& pt) {
        if (forceMoveTo) {
            this->moveTo(pt);
        } else {
            auto lastPt = this->getLastPt();
            if (!lastPt ||
                !SkScalarNearlyEqual(lastPt->fX, pt.fX) ||
                !SkScalarNearlyEqual(lastPt->fY, pt.fY)) {
                this->lineTo(pt);
            }
        }
    };

    // At this point, we know that the arc is not a lone point, but startV == stopV
    // indicates that the sweepAngle is too small such that angles_to_unit_vectors
    // cannot handle it.
    if (startV == stopV) {
        SkScalar endAngle = SkDegreesToRadians(startAngle + sweepAngle);
        SkScalar radiusX = oval.width() / 2;
        SkScalar radiusY = oval.height() / 2;
        // We do not use SkScalar[Sin|Cos]SnapToZero here. When sin(startAngle) is 0 and sweepAngle
        // is very small and radius is huge, the expected behavior here is to draw a line. But
        // calling SkScalarSinSnapToZero will make sin(endAngle) be 0 which will then draw a dot.
        singlePt.set(oval.centerX() + radiusX * SkScalarCos(endAngle),
                     oval.centerY() + radiusY * SkScalarSin(endAngle));
        addPt(singlePt);
        return *this;
    }

    SkConic conics[SkConic::kMaxConicsForArc];
    int count = build_arc_conics(oval, startV, stopV, dir, conics, &singlePt);
    if (count) {
        // Conics take two points. Add one to the verb in case there is a moveto.
        this->incReserve(count * 2 + 1, count + 1, count);
        const SkPoint& pt = conics[0].fPts[0];
        addPt(pt);
        for (int i = 0; i < count; ++i) {
            this->conicTo(conics[i].fPts[1], conics[i].fPts[2], conics[i].fW);
        }
    } else {
        addPt(singlePt);
    }
    return *this;
}

// This converts the SVG arc to conics.
// Partly adapted from Niko's code in kdelibs/kdecore/svgicons.
// Then transcribed from webkit/chrome's SVGPathNormalizer::decomposeArcToCubic()
// See also SVG implementation notes:
// http://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
// Note that arcSweep bool value is flipped from the original implementation.
SkPath& SkPath::arcTo(SkScalar rx, SkScalar ry, SkScalar angle, SkPath::ArcSize arcLarge,
                      SkPathDirection arcSweep, SkScalar x, SkScalar y) {
    this->injectMoveToIfNeeded();
    SkPoint srcPts[2];
    srcPts[0] = *this->getLastPt();
    // If rx = 0 or ry = 0 then this arc is treated as a straight line segment (a "lineto")
    // joining the endpoints.
    // http://www.w3.org/TR/SVG/implnote.html#ArcOutOfRangeParameters
    if (!rx || !ry) {
        return this->lineTo(x, y);
    }
    // If the current point and target point for the arc are identical, it should be treated as a
    // zero length path. This ensures continuity in animations.
    srcPts[1].set(x, y);
    if (srcPts[0] == srcPts[1]) {
        return this->lineTo(x, y);
    }
    rx = SkScalarAbs(rx);
    ry = SkScalarAbs(ry);
    SkVector midPointDistance = srcPts[0] - srcPts[1];
    midPointDistance *= 0.5f;

    SkMatrix pointTransform;
    pointTransform.setRotate(-angle);

    SkPoint transformedMidPoint = pointTransform.mapPoint(midPointDistance);
    SkScalar squareRx = rx * rx;
    SkScalar squareRy = ry * ry;
    SkScalar squareX = transformedMidPoint.fX * transformedMidPoint.fX;
    SkScalar squareY = transformedMidPoint.fY * transformedMidPoint.fY;

    // Check if the radii are big enough to draw the arc, scale radii if not.
    // http://www.w3.org/TR/SVG/implnote.html#ArcCorrectionOutOfRangeRadii
    SkScalar radiiScale = squareX / squareRx + squareY / squareRy;
    if (radiiScale > 1) {
        radiiScale = SkScalarSqrt(radiiScale);
        rx *= radiiScale;
        ry *= radiiScale;
    }

    pointTransform.setScale(1 / rx, 1 / ry);
    pointTransform.preRotate(-angle);

    SkPoint unitPts[2];
    pointTransform.mapPoints(unitPts, srcPts);
    SkVector delta = unitPts[1] - unitPts[0];

    SkScalar d = delta.fX * delta.fX + delta.fY * delta.fY;
    SkScalar scaleFactorSquared = std::max(1 / d - 0.25f, 0.f);

    SkScalar scaleFactor = SkScalarSqrt(scaleFactorSquared);
    if ((arcSweep == SkPathDirection::kCCW) != SkToBool(arcLarge)) {  // flipped from the original implementation
        scaleFactor = -scaleFactor;
    }
    delta.scale(scaleFactor);
    SkPoint centerPoint = unitPts[0] + unitPts[1];
    centerPoint *= 0.5f;
    centerPoint.offset(-delta.fY, delta.fX);
    unitPts[0] -= centerPoint;
    unitPts[1] -= centerPoint;
    SkScalar theta1 = SkScalarATan2(unitPts[0].fY, unitPts[0].fX);
    SkScalar theta2 = SkScalarATan2(unitPts[1].fY, unitPts[1].fX);
    SkScalar thetaArc = theta2 - theta1;
    if (thetaArc < 0 && (arcSweep == SkPathDirection::kCW)) {  // arcSweep flipped from the original implementation
        thetaArc += SK_ScalarPI * 2;
    } else if (thetaArc > 0 && (arcSweep != SkPathDirection::kCW)) {  // arcSweep flipped from the original implementation
        thetaArc -= SK_ScalarPI * 2;
    }

    // Very tiny angles cause our subsequent math to go wonky (skbug.com/40040578)
    // so we do a quick check here. The precise tolerance amount is just made up.
    // PI/million happens to fix the bug in 9272, but a larger value is probably
    // ok too.
    if (SkScalarAbs(thetaArc) < (SK_ScalarPI / (1000 * 1000))) {
        return this->lineTo(x, y);
    }

    pointTransform.setRotate(angle);
    pointTransform.preScale(rx, ry);

    // the arc may be slightly bigger than 1/4 circle, so allow up to 1/3rd
    int segments = SkScalarCeilToInt(SkScalarAbs(thetaArc / (2 * SK_ScalarPI / 3)));
    SkScalar thetaWidth = thetaArc / segments;
    SkScalar t = SkScalarTan(0.5f * thetaWidth);
    if (!SkIsFinite(t)) {
        return *this;
    }
    SkScalar startTheta = theta1;
    SkScalar w = SkScalarSqrt(SK_ScalarHalf + SkScalarCos(thetaWidth) * SK_ScalarHalf);
    auto scalar_is_integer = [](SkScalar scalar) -> bool {
        return scalar == SkScalarFloorToScalar(scalar);
    };
    bool expectIntegers = SkScalarNearlyZero(SK_ScalarPI/2 - SkScalarAbs(thetaWidth)) &&
        scalar_is_integer(rx) && scalar_is_integer(ry) &&
        scalar_is_integer(x) && scalar_is_integer(y);

    for (int i = 0; i < segments; ++i) {
        SkScalar endTheta    = startTheta + thetaWidth,
                 sinEndTheta = SkScalarSinSnapToZero(endTheta),
                 cosEndTheta = SkScalarCosSnapToZero(endTheta);

        unitPts[1].set(cosEndTheta, sinEndTheta);
        unitPts[1] += centerPoint;
        unitPts[0] = unitPts[1];
        unitPts[0].offset(t * sinEndTheta, -t * cosEndTheta);
        SkPoint mapped[2];
        pointTransform.mapPoints(mapped, unitPts);
        /*
        Computing the arc width introduces rounding errors that cause arcs to start
        outside their marks. A round rect may lose convexity as a result. If the input
        values are on integers, place the conic on integers as well.
         */
        if (expectIntegers) {
            for (SkPoint& point : mapped) {
                point.fX = SkScalarRoundToScalar(point.fX);
                point.fY = SkScalarRoundToScalar(point.fY);
            }
        }
        this->conicTo(mapped[0], mapped[1], w);
        startTheta = endTheta;
    }

    // The final point should match the input point (by definition); replace it to
    // ensure that rounding errors in the above math don't cause any problems.
    this->setLastPt(x, y);
    return *this;
}

SkPath& SkPath::rArcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, SkPath::ArcSize largeArc,
                       SkPathDirection sweep, SkScalar dx, SkScalar dy) {
    SkPoint currentPoint = this->getLastPt().value_or(SkPoint{0, 0});
    return this->arcTo(rx, ry, xAxisRotate, largeArc, sweep,
                       currentPoint.fX + dx, currentPoint.fY + dy);
}

SkPath& SkPath::addArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle) {
    if (oval.isEmpty() || 0 == sweepAngle) {
        return *this;
    }

    const SkScalar kFullCircleAngle = SkIntToScalar(360);

    if (sweepAngle >= kFullCircleAngle || sweepAngle <= -kFullCircleAngle) {
        // We can treat the arc as an oval if it begins at one of our legal starting positions.
        // See SkPath::addOval() docs.
        SkScalar startOver90 = startAngle / 90.f;
        SkScalar startOver90I = SkScalarRoundToScalar(startOver90);
        SkScalar error = startOver90 - startOver90I;
        if (SkScalarNearlyEqual(error, 0)) {
            // Index 1 is at startAngle == 0.
            SkScalar startIndex = std::fmod(startOver90I + 1.f, 4.f);
            startIndex = startIndex < 0 ? startIndex + 4.f : startIndex;
            return this->addOval(oval, sweepAngle > 0 ? SkPathDirection::kCW : SkPathDirection::kCCW,
                                 (unsigned) startIndex);
        }
    }
    return this->arcTo(oval, startAngle, sweepAngle, true);
}

/*
    Need to handle the case when the angle is sharp, and our computed end-points
    for the arc go behind pt1 and/or p2...
*/
SkPath& SkPath::arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius) {
    this->injectMoveToIfNeeded();

    if (radius == 0) {
        return this->lineTo(x1, y1);
    }

    // need to know our prev pt so we can construct tangent vectors
    SkPoint start = this->getLastPt().value_or(SkPoint{0, 0});

    // need double precision for these calcs.
    skvx::double2 befored = normalize(skvx::double2{x1 - start.fX, y1 - start.fY});
    skvx::double2 afterd = normalize(skvx::double2{x2 - x1, y2 - y1});
    double cosh = dot(befored, afterd);
    double sinh = cross(befored, afterd);

    // If the previous point equals the first point, befored will be denormalized.
    // If the two points equal, afterd will be denormalized.
    // If the second point equals the first point, sinh will be zero.
    // In all these cases, we cannot construct an arc, so we construct a line to the first point.
    if (!isfinite(befored) || !isfinite(afterd) || SkScalarNearlyZero(SkDoubleToScalar(sinh))) {
        return this->lineTo(x1, y1);
    }

    // safe to convert back to floats now
    SkScalar dist = SkScalarAbs(SkDoubleToScalar(radius * (1 - cosh) / sinh));
    SkScalar xx = x1 - dist * befored[0];
    SkScalar yy = y1 - dist * befored[1];

    SkVector after = SkVector::Make(afterd[0], afterd[1]);
    after.setLength(dist);
    this->lineTo(xx, yy);
    SkScalar weight = SkScalarSqrt(SkDoubleToScalar(SK_ScalarHalf + cosh * 0.5));
    return this->conicTo(x1, y1, x1 + after.fX, y1 + after.fY, weight);
}

///////////////////////////////////////////////////////////////////////////////

SkPath& SkPath::addPath(const SkPath& path, SkScalar dx, SkScalar dy, AddPathMode mode) {
    SkMatrix matrix;

    matrix.setTranslate(dx, dy);
    return this->addPath(path, matrix, mode);
}

SkPath& SkPath::addPath(const SkPath& srcPath, const SkMatrix& matrix, AddPathMode mode) {
    if (srcPath.isEmpty()) {
        return *this;
    }

    const bool canReplaceThis = (mode == kAppend_AddPathMode &&
                                 SkPathPriv::IsEffectivelyEmpty(*this))
                              || this->countVerbs() == 0;
    if (canReplaceThis && matrix.isIdentity()) {
        const SkPathFillType fillType = fFillType;
        *this = srcPath;
        fFillType = fillType;
        return *this;
    }

    // Detect if we're trying to add ourself
    const SkPath* src = &srcPath;
    std::optional<SkPath> tmp;
    if (this == src) {
        tmp = srcPath;
        src = &tmp.value();
    }

    if (kAppend_AddPathMode == mode && !matrix.hasPerspective()) {
        if (src->fLastMoveToIndex >= 0) {
            fLastMoveToIndex = src->fLastMoveToIndex + this->countPoints();
        } else {
            fLastMoveToIndex = src->fLastMoveToIndex - this->countPoints();
        }
        SkPathRef::Editor ed(&fPathRef);
        auto [newPts, newWeights] = ed.growForVerbsInPath(*src->fPathRef);
        const size_t N = src->countPoints();
        matrix.mapPoints({newPts, N}, {src->fPathRef->points(), N});
        if (int numWeights = src->fPathRef->countWeights()) {
            memcpy(newWeights, src->fPathRef->conicWeights(), numWeights * sizeof(newWeights[0]));
        }
        return this->dirtyAfterEdit();
    }

    SkMatrixPriv::MapPtsProc mapPtsProc = SkMatrixPriv::GetMapPtsProc(matrix);
    bool firstVerb = true;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(*src)) {
        SkPoint mappedPts[3];
        switch (verb) {
            case SkPathVerb::kMove:
                mapPtsProc(matrix, mappedPts, &pts[0], 1);
                if (firstVerb && mode == kExtend_AddPathMode && !isEmpty()) {
                    injectMoveToIfNeeded(); // In case last contour is closed
                    auto lastPt = this->getLastPt();
                    // don't add lineTo if it is degenerate
                    if (!lastPt.has_value() || *lastPt != mappedPts[0]) {
                        this->lineTo(mappedPts[0]);
                    }
                } else {
                    this->moveTo(mappedPts[0]);
                }
                break;
            case SkPathVerb::kLine:
                mapPtsProc(matrix, mappedPts, &pts[1], 1);
                this->lineTo(mappedPts[0]);
                break;
            case SkPathVerb::kQuad:
                mapPtsProc(matrix, mappedPts, &pts[1], 2);
                this->quadTo(mappedPts[0], mappedPts[1]);
                break;
            case SkPathVerb::kConic:
                mapPtsProc(matrix, mappedPts, &pts[1], 2);
                this->conicTo(mappedPts[0], mappedPts[1], *w);
                break;
            case SkPathVerb::kCubic:
                mapPtsProc(matrix, mappedPts, &pts[1], 3);
                this->cubicTo(mappedPts[0], mappedPts[1], mappedPts[2]);
                break;
            case SkPathVerb::kClose:
                this->close();
                break;
        }
        firstVerb = false;
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////

// ignore the last point of the 1st contour
SkPath& SkPath::reversePathTo(const SkPath& path) {
    if (path.fPathRef->fVerbs.empty()) {
        return *this;
    }

    const SkPathVerb* verbs = path.fPathRef->verbsEnd();
    const SkPathVerb* verbsBegin = path.fPathRef->verbsBegin();
    SkASSERT(verbsBegin[0] == SkPathVerb::kMove);
    const SkPoint*  pts = path.fPathRef->pointsEnd() - 1;
    const SkScalar* conicWeights = path.fPathRef->conicWeightsEnd();

    while (verbs > verbsBegin) {
        SkPathVerb v = *--verbs;
        pts -= SkPathPriv::PtsInVerb(v);
        switch (v) {
            case SkPathVerb::kMove:
                // if the path has multiple contours, stop after reversing the last
                return *this;
            case SkPathVerb::kLine:
                this->lineTo(pts[0]);
                break;
            case SkPathVerb::kQuad:
                this->quadTo(pts[1], pts[0]);
                break;
            case SkPathVerb::kConic:
                this->conicTo(pts[1], pts[0], *--conicWeights);
                break;
            case SkPathVerb::kCubic:
                this->cubicTo(pts[2], pts[1], pts[0]);
                break;
            case SkPathVerb::kClose:
                break;
        }
    }
    return *this;
}

SkPath& SkPath::reverseAddPath(const SkPath& srcPath) {
    // Detect if we're trying to add ourself
    const SkPath* src = &srcPath;
    std::optional<SkPath> tmp;
    if (this == src) {
        tmp = srcPath;
        src = &tmp.value();
    }

    const SkPathVerb* verbsBegin = src->fPathRef->verbsBegin();
    const SkPathVerb* verbs = src->fPathRef->verbsEnd();
    const SkPoint* pts = src->fPathRef->pointsEnd();
    const SkScalar* conicWeights = src->fPathRef->conicWeightsEnd();

    bool needMove = true;
    bool needClose = false;
    while (verbs > verbsBegin) {
        SkPathVerb v = *--verbs;
        int n = SkPathPriv::PtsInVerb(v);

        if (needMove) {
            --pts;
            this->moveTo(pts->fX, pts->fY);
            needMove = false;
        }
        pts -= n;
        switch (v) {
            case SkPathVerb::kMove:
                if (needClose) {
                    this->close();
                    needClose = false;
                }
                needMove = true;
                pts += 1;   // so we see the point in "if (needMove)" above
                break;
            case SkPathVerb::kLine:
                this->lineTo(pts[0]);
                break;
            case SkPathVerb::kQuad:
                this->quadTo(pts[1], pts[0]);
                break;
            case SkPathVerb::kConic:
                this->conicTo(pts[1], pts[0], *--conicWeights);
                break;
            case SkPathVerb::kCubic:
                this->cubicTo(pts[2], pts[1], pts[0]);
                break;
            case SkPathVerb::kClose:
                needClose = true;
                break;
        }
    }
    return *this;
}

void SkPath::setBounds(const SkRect& rect) {
    SkPathRef::Editor ed(&fPathRef);
    ed.setBounds(rect);
}

void SkPath::setPt(int index, SkScalar x, SkScalar y) {
    SkDEBUGCODE(this->validate();)

    int count = fPathRef->countPoints();
    if (count <= index) {
        return;
    } else {
        SkPathRef::Editor ed(&fPathRef);
        ed.atPoint(index)->set(x, y);
    }
}

void SkPath::setLastPt(SkScalar x, SkScalar y) {
    SkDEBUGCODE(this->validate();)

    int count = fPathRef->countPoints();
    if (count == 0) {
        this->moveTo(x, y);
    } else {
        SkPathRef::Editor ed(&fPathRef);
        ed.atPoint(count-1)->set(x, y);
    }
}

void SkPath::incReserve(int extraPtCount, int extraVerbCount, int extraConicCount) {
    SkDEBUGCODE(this->validate();)
    if (extraPtCount > 0) {
        // For compat with when this function only took a single argument, use
        // extraPtCount if extraVerbCount is 0 (default value).
        SkPathRef::Editor(&fPathRef, extraVerbCount == 0 ? extraPtCount : extraVerbCount, extraPtCount, extraConicCount);
    }
    SkDEBUGCODE(this->validate();)
}

SkPath& SkPath::dirtyAfterEdit() {
    this->setConvexity(SkPathConvexity::kUnknown);

#ifdef SK_DEBUG
    // enable this as needed for testing, but it slows down some chrome tests so much
    // that they don't complete, so we don't enable it by default
    // e.g. TEST(IdentifiabilityPaintOpDigestTest, MassiveOpSkipped)
    if (this->countVerbs() < 16) {
        SkASSERT(fPathRef->dataMatchesVerbs());
    }
#endif

    return *this;
}

#endif
