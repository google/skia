/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathBuilder.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/core/SkTypes.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkAssert.h" // IWYU pragma: keep
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkSafe32.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkVx.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkPathData.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRawShapes.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <utility>

namespace {

void subdivide_cubic_to(SkPathBuilder* path, const SkPoint pts[4], int level = 2) {
    if (--level >= 0) {
        SkPoint tmp[7];

        SkChopCubicAtHalf(pts, tmp);
        subdivide_cubic_to(path, &tmp[0], level);
        subdivide_cubic_to(path, &tmp[3], level);
    } else {
        path->cubicTo(pts[1], pts[2], pts[3]);
    }
}

}  // namespace

SkPathBuilder::SkPathBuilder() {
    this->reset();
}

SkPathBuilder::SkPathBuilder(SkPathFillType ft) {
    this->reset();
    fFillType = ft;
}

SkPathBuilder::SkPathBuilder(const SkPath& src) {
    *this = src;
}

SkPathBuilder::~SkPathBuilder() {
}

SkPathBuilder& SkPathBuilder::reset() {
    fPts.clear();
    fVerbs.clear();
    fConicWeights.clear();
    fFillType = SkPathFillType::kDefault;
    fIsVolatile = false;

    // these are internal state

    fSegmentMask = 0;
    fLastMoveIndex = -1;        // illegal

    fType      = SkPathIsAType::kGeneral;
    fConvexity = SkPathConvexity::kUnknown;

    return *this;
}

bool SkPathBuilder::operator==(const SkPathBuilder& o) const {
    // quick-accept
    if (this == &o) {
        return true;
    }
    // quick-reject
    if (fSegmentMask != o.fSegmentMask || fFillType != o.fFillType) {
        return false;
    }
    // deep compare
    return fVerbs == o.fVerbs && fPts == o.fPts && fConicWeights == o.fConicWeights;
}

void SkPathBuilder::incReserve(int extraPtCount, int extraVbCount, int extraCnCount) {
    fPts.reserve_exact(Sk32_sat_add(fPts.size(), extraPtCount));
    fVerbs.reserve_exact(Sk32_sat_add(fVerbs.size(), extraVbCount));
    fConicWeights.reserve_exact(Sk32_sat_add(fConicWeights.size(), extraCnCount));
}

std::tuple<SkPoint*, SkScalar*> SkPathBuilder::growForVerbsInPath(const SkPath& path) {
    fSegmentMask |= path.getSegmentMasks();

    const SkSpan<const SkPathVerb> verbs = path.verbs();
    if (!verbs.empty()) {
         // TODO(borenet): If the current builder is empty or JustMoves, we can use the type of the
         // path. If the path is empty, we can keep the current type.
        fType = SkPathIsAType::kGeneral;
        fVerbs.push_back_n(verbs.size(), verbs.data());
    }

    SkPoint* pts = nullptr;
    if (int numPts = path.countPoints()) {
        pts = fPts.push_back_n(numPts);
    }

    SkScalar* weights = nullptr;
    if (int numConics = path.conicWeights().size()) {
        weights = fConicWeights.push_back_n(numConics);
    }

    return {pts, weights};
}

/*
 *  Some old behavior in SkPath -- should we keep it?
 *
 *  After each edit (i.e. adding a verb)
        this->setConvexityType(SkPathConvexity::kUnknown);
        this->setFirstDirection(SkPathPriv::kUnknown_FirstDirection);
 */

SkPathBuilder& SkPathBuilder::moveTo(SkPoint pt) {
    if (!fVerbs.empty() && fVerbs.back() == SkPathVerb::kMove) {
        fPts.back() = pt;

        SkASSERT(fType != SkPathIsAType::kOval && fType != SkPathIsAType::kRRect);
        SkASSERT(fConvexity == SkPathConvexity::kUnknown);
        SkASSERT(fLastMoveIndex == SkToInt(fPts.size()) - 1);
    } else {
        fLastMoveIndex = SkToInt(fPts.size());

        fPts.push_back(pt);
        fVerbs.push_back(SkPathVerb::kMove);

        if (fType == SkPathIsAType::kOval || fType == SkPathIsAType::kRRect) {
            fType = SkPathIsAType::kGeneral;
        }
        fConvexity = SkPathConvexity::kUnknown;
    }

    return *this;
}

SkPathBuilder& SkPathBuilder::lineTo(SkPoint pt) {
    this->ensureMove();

    fPts.push_back(pt);
    fVerbs.push_back(SkPathVerb::kLine);

    fSegmentMask |= kLine_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::quadTo(SkPoint pt1, SkPoint pt2) {
    this->ensureMove();

    SkPoint* p = fPts.push_back_n(2);
    p[0] = pt1;
    p[1] = pt2;
    fVerbs.push_back(SkPathVerb::kQuad);

    fSegmentMask |= kQuad_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::conicTo(SkPoint pt1, SkPoint pt2, SkScalar w) {
    this->ensureMove();

    SkPoint* p = fPts.push_back_n(2);
    p[0] = pt1;
    p[1] = pt2;
    if (w == 1) {
        fVerbs.push_back(SkPathVerb::kQuad);
        fSegmentMask |= kQuad_SkPathSegmentMask;
    } else {
        fVerbs.push_back(SkPathVerb::kConic);
        fConicWeights.push_back(w);
        fSegmentMask |= kConic_SkPathSegmentMask;
    }

    return *this;
}

SkPathBuilder& SkPathBuilder::cubicTo(SkPoint pt1, SkPoint pt2, SkPoint pt3) {
    this->ensureMove();

    SkPoint* p = fPts.push_back_n(3);
    p[0] = pt1;
    p[1] = pt2;
    p[2] = pt3;
    fVerbs.push_back(SkPathVerb::kCubic);

    fSegmentMask |= kCubic_SkPathSegmentMask;
    return *this;
}

SkPathBuilder& SkPathBuilder::close() {
    // If this is a 2nd 'close', we just ignore it
    if (!fVerbs.empty() && fVerbs.back() != SkPathVerb::kClose) {
        this->ensureMove();
        fVerbs.push_back(SkPathVerb::kClose);
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////

SkPathBuilder& SkPathBuilder::rMoveTo(SkVector pt) {
    SkPoint lastPt = {0,0}; // in case we're empty
    if (!fPts.empty()) {
        if (fVerbs.back() == SkPathVerb::kClose) {
            lastPt = fPts[fLastMoveIndex];
        } else {
            lastPt = fPts.back();
        }
    }
    return this->moveTo(lastPt + pt);
}

SkPathBuilder& SkPathBuilder::rLineTo(SkVector p1) {
    this->ensureMove();
    return this->lineTo(fPts.back() + p1);
}

SkPathBuilder& SkPathBuilder::rQuadTo(SkVector p1, SkVector p2) {
    this->ensureMove();
    SkPoint base = fPts.back();
    return this->quadTo(base + p1, base + p2);
}

SkPathBuilder& SkPathBuilder::rConicTo(SkVector p1, SkVector p2, SkScalar w) {
    this->ensureMove();
    SkPoint base = fPts.back();
    return this->conicTo(base + p1, base + p2, w);
}

SkPathBuilder& SkPathBuilder::rCubicTo(SkVector p1, SkVector p2, SkVector p3) {
    this->ensureMove();
    SkPoint base = fPts.back();
    return this->cubicTo(base + p1, base + p2, base + p3);
}

///////////////////////////////////////////////////////////////////////////////////////////

SkPath SkPathBuilder::detach(const SkMatrix* mx) {
    auto path = this->snapshot(mx);
    this->reset();
    return path;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkPathData> SkPathBuilder::snapshotData() const {
    if (fVerbs.size() <= 1) {
        return SkPathData::Empty();
    }

    switch (fType) {
        case SkPathIsAType::kGeneral:
            break;
        case SkPathIsAType::kOval:
            if (auto r = SkRect::Bounds(fPts)) {
                return SkPathData::Oval(*r, fIsA.fDirection, fIsA.fStartIndex);
            }
            return nullptr;
        case SkPathIsAType::kRRect:
            if (auto r = SkRect::Bounds(fPts)) {
                return SkPathData::RRect(SkPathPriv::DeduceRRectFromContour(*r, fPts, fVerbs),
                                         fIsA.fDirection, fIsA.fStartIndex);
            }
            return nullptr;
    }
    SkASSERT(fType == SkPathIsAType::kGeneral);

    return SkPathData::Make(fPts, fVerbs, fConicWeights);
}

sk_sp<SkPathData> SkPathBuilder::detachData() {
    auto data = this->snapshotData();
    this->reset();
    return data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

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

static bool nearly_equal(const SkPoint& a, const SkPoint& b) {
    return SkScalarNearlyEqual(a.fX, b.fX)
        && SkScalarNearlyEqual(a.fY, b.fY);
}

SkPathBuilder& SkPathBuilder::arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
                                    bool forceMoveTo) {
    if (oval.width() < 0 || oval.height() < 0) {
        return *this;
    }

    startAngle = SkScalarMod(startAngle, 360.0f);

    if (fVerbs.empty()) {
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
    auto addPt = [forceMoveTo, this](const SkPoint& pt) {
        if (forceMoveTo) {
            this->moveTo(pt);
        } else if (!nearly_equal(fPts.back(), pt)) {
            this->lineTo(pt);
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
        this->incReserve(count * 2 + 1);
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

SkPathBuilder& SkPathBuilder::rArcTo(SkPoint r, SkScalar xAxisRotate, ArcSize largeArc,
                                     SkPathDirection sweep, SkVector dxdy) {
    const SkPoint currentPoint = this->getLastPt().value_or(SkPoint{0, 0});
    return this->arcTo(r, xAxisRotate, largeArc, sweep, currentPoint + dxdy);
}

SkPathBuilder& SkPathBuilder::addArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle) {
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

SkPathBuilder& SkPathBuilder::arcTo(SkPoint p1, SkPoint p2, SkScalar radius) {
    this->ensureMove();

    if (radius == 0) {
        return this->lineTo(p1);
    }

    // need to know our prev pt so we can construct tangent vectors
    SkPoint start = fPts.back();

    // need double precision for these calcs.
    skvx::double2 befored = normalize(skvx::double2{p1.fX - start.fX, p1.fY - start.fY});
    skvx::double2 afterd = normalize(skvx::double2{p2.fX - p1.fX, p2.fY - p1.fY});
    double cosh = dot(befored, afterd);
    double sinh = cross(befored, afterd);

    // If the previous point equals the first point, befored will be denormalized.
    // If the two points equal, afterd will be denormalized.
    // If the second point equals the first point, sinh will be zero.
    // In all these cases, we cannot construct an arc, so we construct a line to the first point.
    if (!isfinite(befored) || !isfinite(afterd) || SkScalarNearlyZero(SkDoubleToScalar(sinh))) {
        return this->lineTo(p1);
    }

    // safe to convert back to floats now
    SkScalar dist = SkScalarAbs(SkDoubleToScalar(radius * (1 - cosh) / sinh));
    SkScalar xx = p1.fX - dist * befored[0];
    SkScalar yy = p1.fY - dist * befored[1];

    SkVector after = SkVector::Make(afterd[0], afterd[1]);
    after.setLength(dist);
    this->lineTo(xx, yy);
    SkScalar weight = SkScalarSqrt(SkDoubleToScalar(SK_ScalarHalf + cosh * 0.5));
    return this->conicTo(p1, p1 + after, weight);
}

// This converts the SVG arc to conics.
// Partly adapted from Niko's code in kdelibs/kdecore/svgicons.
// Then transcribed from webkit/chrome's SVGPathNormalizer::decomposeArcToCubic()
// See also SVG implementation notes:
// http://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter
// Note that arcSweep bool value is flipped from the original implementation.
SkPathBuilder& SkPathBuilder::arcTo(SkPoint rad, SkScalar angle, SkPathBuilder::ArcSize arcLarge,
                                    SkPathDirection arcSweep, SkPoint endPt) {
    this->ensureMove();

    const SkPoint srcPts[2] = { fPts.back(), endPt };

    // If rx = 0 or ry = 0 then this arc is treated as a straight line segment (a "lineto")
    // joining the endpoints.
    // http://www.w3.org/TR/SVG/implnote.html#ArcOutOfRangeParameters
    if (!rad.fX || !rad.fY) {
        return this->lineTo(endPt);
    }
    // If the current point and target point for the arc are identical, it should be treated as a
    // zero length path. This ensures continuity in animations.
    if (srcPts[0] == srcPts[1]) {
        return this->lineTo(endPt);
    }
    SkScalar rx = SkScalarAbs(rad.fX);
    SkScalar ry = SkScalarAbs(rad.fY);
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
        return this->lineTo(endPt);
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
        scalar_is_integer(endPt.fX) && scalar_is_integer(endPt.fY);

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
    fPts.back() = endPt;
    return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////

SkPathIter SkPathBuilder::iter() const {
    return SkPathIter(fPts, fVerbs, fConicWeights);
}

SkPathBuilder& SkPathBuilder::addRaw(const SkPathRaw& raw) {
    this->incReserve(raw.points().size(), raw.verbs().size(), raw.conics().size());

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
    return *this;
}

SkPathBuilder& SkPathBuilder::addRect(const SkRect& rect, SkPathDirection dir, unsigned index) {
    const bool wasEmpty = (fSegmentMask == 0);

    this->addRaw(SkPathRawShapes::Rect(rect, dir, index));

    if (wasEmpty) {
        // now we're a rect
        fConvexity = SkPathDirection_ToConvexity(dir);
    }
    return *this;
}

SkPathBuilder& SkPathBuilder::addOval(const SkRect& oval, SkPathDirection dir, unsigned index) {
    const bool wasEmpty = (fSegmentMask == 0);

    this->addRaw(SkPathRawShapes::Oval(oval, dir, index));

    if (wasEmpty) {
        fType            = SkPathIsAType::kOval;
        fIsA.fDirection  = dir;
        fIsA.fStartIndex = index % 4;
        fConvexity = SkPathDirection_ToConvexity(dir);
    }

    return *this;
}

SkPathBuilder& SkPathBuilder::addRRect(const SkRRect& rrect, SkPathDirection dir, unsigned index) {
    const SkRect& bounds = rrect.getBounds();

    auto [asType, newIndex] = SkPathPriv::SimplifyRRect(rrect, index);
    switch (asType) {
        case SkPathPriv::RRectAsEnum::kRect:
            return this->addRect(bounds, dir, newIndex);
        case SkPathPriv::RRectAsEnum::kOval:
            return this->addOval(bounds, dir, newIndex);
        case SkPathPriv::RRectAsEnum::kRRect:
            // fall through ...
            break;
    }

    const bool wasEmpty = (fSegmentMask == 0);

    this->addRaw(SkPathRawShapes::RRect(rrect, dir, index));

    if (wasEmpty) {
        fType            = SkPathIsAType::kRRect;
        fIsA.fDirection  = dir;
        fIsA.fStartIndex = index % 8;
        fConvexity = SkPathDirection_ToConvexity(dir);
    }
    return *this;
}

SkPathBuilder& SkPathBuilder::addCircle(SkScalar x, SkScalar y, SkScalar r, SkPathDirection dir) {
    if (r >= 0) {
        this->addOval(SkRect::MakeLTRB(x - r, y - r, x + r, y + r), dir);
    }
    return *this;
}

SkPathBuilder& SkPathBuilder::addPolygon(SkSpan<const SkPoint> pts, bool isClosed) {
    if (pts.empty()) {
        return *this;
    }

    this->moveTo(pts[0]);
    this->polylineTo(pts.last(pts.size() - 1));
    if (isClosed) {
        this->close();
    }
    return *this;
}

SkPathBuilder& SkPathBuilder::polylineTo(SkSpan<const SkPoint> pts) {
    if (!pts.empty()) {
        this->ensureMove();

        const auto count = pts.size();
        this->incReserve(count, count, 0);
        memcpy(fPts.push_back_n(count), pts.data(), count * sizeof(SkPoint));
        memset(fVerbs.push_back_n(count), (uint8_t)SkPathVerb::kLine, count);
        fSegmentMask |= kLine_SkPathSegmentMask;
    }
    return *this;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SkPathBuilder& SkPathBuilder::offset(SkScalar dx, SkScalar dy) {
    for (auto& p : fPts) {
        p += {dx, dy};
    }
    return *this;
}

SkPathBuilder& SkPathBuilder::addPath(const SkPath& path, SkScalar dx, SkScalar dy,
                                      SkPath::AddPathMode mode) {
    SkMatrix matrix = SkMatrix::Translate(dx, dy);
    return this->addPath(path, matrix, mode);
}

SkPathBuilder& SkPathBuilder::addPath(const SkPath& src, const SkMatrix& matrix,
                                      SkPath::AddPathMode mode) {
    if (src.isEmpty()) {
        return *this;
    }

    const bool canReplaceThis = (mode == SkPath::AddPathMode::kAppend_AddPathMode &&
                                 SkPathPriv::IsEffectivelyEmpty(*this))
                              || this->verbs().empty();
    if (canReplaceThis && matrix.isIdentity()) {
        const SkPathFillType fillType = fFillType;
        *this = src;
        fFillType = fillType;
        return *this;
    }

    // We're about to append - clear convexity.
    fConvexity = SkPathConvexity::kUnknown;

    if (SkPath::AddPathMode::kAppend_AddPathMode == mode && !matrix.hasPerspective()) {
        const int lastMoveToIndex = SkPathPriv::FindLastMoveToIndex(src.verbs(), src.points().size());
        SkASSERT(lastMoveToIndex >= 0);
        fLastMoveIndex = lastMoveToIndex + this->countPoints();

        auto [newPts, newWeights] = this->growForVerbsInPath(src);
        const size_t count = src.points().size();
        matrix.mapPoints({newPts, count}, src.points());
        if (auto conics = src.conicWeights(); !conics.empty()) {
            memcpy(newWeights, conics.data(), conics.size_bytes());
        }
        return *this;
    }

    SkMatrixPriv::MapPtsProc mapPtsProc = SkMatrixPriv::GetMapPtsProc(matrix);
    bool firstVerb = true;
    for (auto [verb, pts, w] : SkPathPriv::Iterate(src)) {
        SkPoint mappedPts[3];
        switch (verb) {
            case SkPathVerb::kMove:
                mapPtsProc(matrix, mappedPts, &pts[0], 1);
                if (firstVerb && mode == SkPath::kExtend_AddPathMode && !isEmpty()) {
                    this->ensureMove(); // In case last contour is closed
                    std::optional<SkPoint> lastPt = this->getLastPt();
                    // don't add lineTo if it is degenerate
                    if (!lastPt.has_value() || lastPt.value() != mappedPts[0]) {
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

// ignore the last point of the 1st contour
SkPathBuilder& SkPathBuilder::privateReversePathTo(const SkPath& path) {
    auto verbSpan = path.verbs();
    if (verbSpan.empty()) {
        return *this;
    }

    auto verbs = verbSpan.end();
    auto verbsBegin = verbSpan.begin();
    auto pts = path.points().end() - 1;
    auto conicWeights = path.conicWeights().end();

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

SkPathBuilder& SkPathBuilder::privateReverseAddPath(const SkPath& src) {
    auto verbSpan = src.verbs();
    if (verbSpan.empty()) {
        return *this;
    }

    auto verbs = verbSpan.end();
    auto verbsBegin = verbSpan.begin();
    auto pts = src.points().end();
    auto conicWeights = src.conicWeights().end();

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
        switch ((SkPathVerb)v) {
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

std::optional<SkPoint> SkPathBuilder::getLastPt() const {
    int count = this->fPts.size();
    if (count > 0) {
        return this->fPts.at(count - 1);
    }
    return std::nullopt;
}

void SkPathBuilder::setLastPt(SkScalar x, SkScalar y) {
    int count = fPts.size();
    if (count == 0) {
        this->moveTo(x, y);
    } else {
        fPts.at(count-1).set(x, y);
        fType = SkPathIsAType::kGeneral;
    }
}

void SkPathBuilder::setPoint(size_t index, SkPoint p) {
    if (index < (size_t)fPts.size()) {
        fPts[index] = p;
        fType = SkPathIsAType::kGeneral;
    }
}

SkPathBuilder& SkPathBuilder::transform(const SkMatrix& matrix) {
    if (matrix.isIdentity() || this->isEmpty()) {
        return *this;
    }

    if (matrix.hasPerspective()) {
        SkPath src = this->detach();

        // remember this from before the detach()
        this->setFillType(src.getFillType());

        SkPath clipped;
        if (SkPathPriv::PerspectiveClip(src, matrix, &clipped)) {
            src = std::move(clipped);
        }

        for (auto [verb, pts, wt] : SkPathPriv::Iterate(src)) {
            switch (verb) {
                case SkPathVerb::kMove:
                    this->moveTo(pts[0]);
                    break;
                case SkPathVerb::kLine:
                    this->lineTo(pts[1]);
                    break;
                case SkPathVerb::kQuad:
                    // promote the quad to a conic
                    this->conicTo(pts[1], pts[2], SkConic::TransformW(pts, SK_Scalar1, matrix));
                    break;
                case SkPathVerb::kConic:
                    this->conicTo(pts[1], pts[2], SkConic::TransformW(pts, wt[0], matrix));
                    break;
                case SkPathVerb::kCubic:
                    subdivide_cubic_to(this, pts);
                    break;
                case SkPathVerb::kClose:
                    this->close();
                    break;
            }
        }
    } else {

        // Can we maintain our special case shape?
        if (!matrix.rectStaysRect() || !SkPathPriv::IsAxisAligned(fPts)) {
            fType = SkPathIsAType::kGeneral;
            // lose convexity (just to be numerically safe)
            if (SkPathConvexity_IsConvex(fConvexity)) {
                fConvexity = SkPathConvexity::kUnknown;
            }
        }

        // If we're still a special case, check if we need to reverse our winding
        if (fType == SkPathIsAType::kOval || fType == SkPathIsAType::kRRect) {
            auto [dir, start] =
            SkPathPriv::TransformDirAndStart(matrix, fType == SkPathIsAType::kRRect,
                                             fIsA.fDirection, fIsA.fStartIndex);
            fIsA.fDirection  = dir;
            fIsA.fStartIndex = start;
        }

    }
    matrix.mapPoints(fPts);

    return *this;
}

std::optional<SkRect> SkPathBuilder::computeTightBounds() const {
    if (!this->isFinite()) {
        return {};
    }
    return SkPathPriv::ComputeTightBounds(this->points(), this->verbs(), this->conicWeights());
}

bool SkPathBuilder::isFinite() const {
    for (auto p : fPts) {
        if (!p.isFinite()) {
            return false;
        }
    }
    return true;
}

bool SkPathBuilder::isZeroLengthSincePoint(int startPtIndex) const {
    int count = fPts.size() - startPtIndex;
    if (count < 2) {
        return true;
    }
    const SkPoint* pts = fPts.begin() + startPtIndex;
    const SkPoint& first = *pts;
    for (int index = 1; index < count; ++index) {
        if (first != pts[index]) {
            return false;
        }
    }
    return true;
}

bool SkPathBuilder::contains(SkPoint p) const {
    const auto raw = SkPathPriv::Raw(*this, SkResolveConvexity::kNo);
    return raw.has_value() && SkPathPriv::Contains(*raw, p);
}
