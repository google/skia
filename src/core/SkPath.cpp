/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkFloatBits.h"
#include "src/core/SkEdgeClipper.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRawShapes.h"
#include "src/core/SkPointPriv.h"
#include "src/core/SkSpanPriv.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits.h>
#include <utility>

/*  Contains path methods that are common between fPathRef and fPathData implementations:
 *  - fFillType
 *  - fIsVolatile
 *
 *  Any methods that refer to these fields should be in SkPath_pathref.cpp
 *  - fPathRef
 *  - fConvexity
 *  - fLastMoveToIndex
 *
 *  ... with the plan being to create a parallel file that implements those methods on SkPathData
 */


SkPath::~SkPath() {
    SkDEBUGCODE(this->validate();)
}

static inline bool check_edge_against_rect(const SkPoint& p0,
                                           const SkPoint& p1,
                                           const SkRect& rect,
                                           SkPathDirection dir) {
    const SkPoint* edgeBegin;
    SkVector v;
    if (SkPathDirection::kCW == dir) {
        v = p1 - p0;
        edgeBegin = &p0;
    } else {
        v = p0 - p1;
        edgeBegin = &p1;
    }
    if (v.fX || v.fY) {
        // check the cross product of v with the vec from edgeBegin to each rect corner
        SkScalar yL = v.fY * (rect.fLeft - edgeBegin->fX);
        SkScalar xT = v.fX * (rect.fTop - edgeBegin->fY);
        SkScalar yR = v.fY * (rect.fRight - edgeBegin->fX);
        SkScalar xB = v.fX * (rect.fBottom - edgeBegin->fY);
        if ((xT < yL) || (xT < yR) || (xB < yL) || (xB < yR)) {
            return false;
        }
    }
    return true;
}

bool SkPath::conservativelyContainsRect(const SkRect& rect) const {
    const SkPathConvexity convexity = this->getConvexity();
    if (!SkPathConvexity_IsConvex(convexity)) {
        return false;
    }

    const auto direction = SkPathConvexity_ToDirection(convexity);
    if (!direction) {
        return false;
    }

    SkPoint firstPt;
    SkPoint prevPt;
    int segmentCount = 0;
    SkDEBUGCODE(int moveCnt = 0;)

    for (auto [verb, pts, weight] : SkPathPriv::Iterate(*this)) {
        if (verb == SkPathVerb::kClose || (segmentCount > 0 && verb == SkPathVerb::kMove)) {
            // Closing the current contour; but since convexity is a precondition, it's the only
            // contour that matters.
            SkASSERT(moveCnt);
            segmentCount++;
            break;
        } else if (verb == SkPathVerb::kMove) {
            // A move at the start of the contour (or multiple leading moves, in which case we
            // keep the last one before a non-move verb).
            SkASSERT(!segmentCount);
            SkDEBUGCODE(++moveCnt);
            firstPt = prevPt = pts[0];
        } else {
            int pointCount = SkPathPriv::PtsInVerb((unsigned) verb);
            SkASSERT(pointCount > 0);

            if (!SkPathPriv::AllPointsEq({pts, (size_t)pointCount + 1})) {
                SkASSERT(moveCnt);
                int nextPt = pointCount;
                segmentCount++;

                if (prevPt == pts[nextPt]) {
                    // A pre-condition to getting here is that the path is convex, so if a
                    // verb's start and end points are the same, it means it's the only
                    // verb in the contour (and the only contour). While it's possible for
                    // such a single verb to be a convex curve, we do not have any non-zero
                    // length edges to conservatively test against without splitting or
                    // evaluating the curve. For simplicity, just reject the rectangle.
                    return false;
                } else if (SkPathVerb::kConic == verb) {
                    SkConic orig;
                    orig.set(pts, *weight);
                    SkPoint quadPts[5];
                    int count = orig.chopIntoQuadsPOW2(quadPts, 1);
                    SkASSERT_RELEASE(2 == count);

                    if (!check_edge_against_rect(quadPts[0], quadPts[2], rect, *direction)) {
                        return false;
                    }
                    if (!check_edge_against_rect(quadPts[2], quadPts[4], rect, *direction)) {
                        return false;
                    }
                } else {
                    if (!check_edge_against_rect(prevPt, pts[nextPt], rect, *direction)) {
                        return false;
                    }
                }
                prevPt = pts[nextPt];
            }
        }
    }

    if (segmentCount) {
        return check_edge_against_rect(prevPt, firstPt, rect, *direction);
    }
    return false;
}

bool SkPath::isLastContourClosed() const {
    SkSpan<const SkPathVerb> verbs = this->verbs();
    return !verbs.empty() && verbs.back() == SkPathVerb::kClose;
}

bool SkPath::isLine(SkPoint line[2]) const {
    SkSpan<const SkPathVerb> verbs = this->verbs();
    if (verbs.size() == 2 && verbs[1] == SkPathVerb::kLine) {
        SkASSERT(verbs[0] == SkPathVerb::kMove);
        SkSpan<const SkPoint> pts = this->points();
        SkASSERT(pts.size() == 2);
        if (line) {
            line[0] = pts[0];
            line[1] = pts[1];
        }
        return true;
    }
    return false;
}

bool SkPath::isEmpty() const {
    SkDEBUGCODE(this->validate();)
    return this->verbs().empty();
}

bool SkPath::isConvex() const {
    return SkPathConvexity_IsConvex(this->getConvexity());
}

bool SkPath::isRect(SkRect* rect, bool* isClosed, SkPathDirection* direction) const {
    SkDEBUGCODE(this->validate();)
    SkSpan<const SkPoint> pts = this->points();
    SkSpan<const SkPathVerb> vbs = this->verbs();
    if (auto rc = SkPathPriv::IsRectContour(pts, vbs, this->getSegmentMasks(), false)) {
        if (rect) {
            *rect = rc->fRect;
        }
        if (isClosed) {
            *isClosed = rc->fIsClosed;
        }
        if (direction) {
            *direction = rc->fDirection;
        }
        return true;
    }
    return false;
}

bool SkPath::isOval(SkRect* bounds) const {
    if (auto info = this->getOvalInfo()) {
        if (bounds) {
            *bounds = info->fBounds;
        }
        return true;
    }
    return false;
}

bool SkPath::isRRect(SkRRect* rrect) const {
    if (auto info = this->getRRectInfo()) {
        if (rrect) {
            *rrect = info->fRRect;
        }
        return true;
    }
    return false;
}

#ifdef SK_LEGACY_PATH_ACCESSORS
size_t SkPath::getPoints(SkSpan<SkPoint> dst) const {
    SkDEBUGCODE(this->validate();)
    SkSpan<const SkPoint> src = this->points();

    const size_t n = std::min(dst.size(), src.size());
    sk_careful_memcpy(dst.data(), src.data(), n * sizeof(SkPoint));
    return src.size();
}

SkPoint SkPath::getPoint(int index) const {
    SkSpan<const SkPoint> pts = this->points();
    if ((unsigned)index < (unsigned)pts.size()) {
        return pts[index];
    }
    return SkPoint::Make(0, 0);
}

size_t SkPath::getVerbs(SkSpan<uint8_t> dst) const {
    SkDEBUGCODE(this->validate();)
    SkSpan<const SkPathVerb> src = this->verbs();

    const size_t n = std::min(dst.size(), src.size());
    sk_careful_memcpy(dst.data(), src.data(), n);
    return src.size();
}
#endif

size_t SkPath::approximateBytesUsed() const {
    return sizeof(SkPath)
         + this->points().size_bytes()
         + this->verbs().size_bytes()
         + this->conicWeights().size_bytes();
}

std::optional<SkPoint> SkPath::getLastPt() const {
    SkDEBUGCODE(this->validate();)
    SkSpan<const SkPoint> pts = this->points();
    if (!pts.empty()) {
        return pts.back();
    }
    return {};
}

SkPathConvexity SkPath::getConvexity() const {
// Enable once we fix all the bugs
//    SkDEBUGCODE(this->isConvexityAccurate());
    SkPathConvexity convexity = this->getConvexityOrUnknown();
    if (convexity == SkPathConvexity::kUnknown) {
        convexity = this->computeConvexity();
    }
    SkASSERT(convexity != SkPathConvexity::kUnknown);
    return convexity;
}

SkPathIter SkPath::iter() const {
    return { this->points(), this->verbs(), this->conicWeights() };
}

bool SkPath::isZeroLengthSincePoint(int startPtIndex) const {
    SkSpan<const SkPoint> span = this->points();

    int count = (int)span.size() - startPtIndex;
    if (count < 2) {
        return true;
    }
    const SkPoint* pts = span.data() + startPtIndex;
    const SkPoint& first = *pts;
    for (int index = 1; index < count; ++index) {
        if (first != pts[index]) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkPath::Iter::Iter() {
#ifdef SK_DEBUG
    fPts = nullptr;
    fConicWeights = nullptr;
    fMoveTo.fX = fMoveTo.fY = fLastPt.fX = fLastPt.fY = 0;
    fForceClose = fCloseLine = false;
#endif
    // need to init enough to make next() harmlessly return kDone_Verb
    fVerbs = nullptr;
    fVerbStop = nullptr;
    fNeedClose = false;
}

SkPath::Iter::Iter(const SkPath& path, bool forceClose) {
    this->setPath(path, forceClose);
}

void SkPath::Iter::setPath(const SkPath& path, bool forceClose) {
    fPts = path.points().data();
    fVerbs = path.verbs().data();
    fVerbStop = fVerbs + path.verbs().size();
    fConicWeights = path.conicWeights().data();
    if (fConicWeights) {
      fConicWeights -= 1;  // begin one behind
    }
    fLastPt.fX = fLastPt.fY = 0;
    fMoveTo.fX = fMoveTo.fY = 0;
    fForceClose = SkToU8(forceClose);
    fNeedClose = false;
}

bool SkPath::Iter::isClosedContour() const {
    if (fVerbs == nullptr || fVerbs == fVerbStop) {
        return false;
    }
    if (fForceClose) {
        return true;
    }

    const SkPathVerb* verbs = fVerbs;
    const SkPathVerb* stop = fVerbStop;

    if (SkPathVerb::kMove == *verbs) {
        verbs += 1; // skip the initial moveto
    }

    while (verbs < stop) {
        // verbs points one beyond the current verb, decrement first.
        SkPathVerb v = *verbs++;
        if (SkPathVerb::kMove == v) {
            break;
        }
        if (SkPathVerb::kClose == v) {
            return true;
        }
    }
    return false;
}

SkPathVerb SkPath::Iter::autoClose(SkPoint pts[2]) {
    SkASSERT(pts);
    if (fLastPt != fMoveTo) {
        // A special case: if both points are NaN, SkPoint::operation== returns
        // false, but the iterator expects that they are treated as the same.
        // (consider SkPoint is a 2-dimension float point).
        if (SkIsNaN(fLastPt.fX) || SkIsNaN(fLastPt.fY) ||
            SkIsNaN(fMoveTo.fX) || SkIsNaN(fMoveTo.fY)) {
            return SkPathVerb::kClose;
        }

        pts[0] = fLastPt;
        pts[1] = fMoveTo;
        fLastPt = fMoveTo;
        fCloseLine = true;
        return SkPathVerb::kLine;
    }
    pts[0] = fMoveTo;
    return SkPathVerb::kClose;
}

SkPath::Verb SkPath::Iter::next(SkPoint ptsParam[4]) {
    SkASSERT(ptsParam);

    if (fVerbs == fVerbStop) {
        // Close the curve if requested and if there is some curve to close
        if (fNeedClose) {
            if (SkPathVerb::kLine == this->autoClose(ptsParam)) {
                return kLine_Verb;
            }
            fNeedClose = false;
            return kClose_Verb;
        }
        return kDone_Verb;
    }

    SkPathVerb verb = *fVerbs++;
    const SkPoint* SK_RESTRICT srcPts = fPts;
    SkPoint* SK_RESTRICT       pts = ptsParam;

    switch (verb) {
        case SkPathVerb::kMove:
            if (fNeedClose) {
                fVerbs--; // move back one verb
                verb = this->autoClose(pts);
                if (verb == SkPathVerb::kClose) {
                    fNeedClose = false;
                }
                return (Verb)verb;
            }
            if (fVerbs == fVerbStop) {    // might be a trailing moveto
                return kDone_Verb;
            }
            fMoveTo = *srcPts;
            pts[0] = *srcPts;
            srcPts += 1;
            fLastPt = fMoveTo;
            fNeedClose = fForceClose;
            break;
        case SkPathVerb::kLine:
            pts[0] = fLastPt;
            pts[1] = srcPts[0];
            fLastPt = srcPts[0];
            fCloseLine = false;
            srcPts += 1;
            break;
        case SkPathVerb::kConic:
            fConicWeights += 1;
            [[fallthrough]];
        case SkPathVerb::kQuad:
            pts[0] = fLastPt;
            memcpy(&pts[1], srcPts, 2 * sizeof(SkPoint));
            fLastPt = srcPts[1];
            srcPts += 2;
            break;
        case SkPathVerb::kCubic:
            pts[0] = fLastPt;
            memcpy(&pts[1], srcPts, 3 * sizeof(SkPoint));
            fLastPt = srcPts[2];
            srcPts += 3;
            break;
        case SkPathVerb::kClose:
            verb = this->autoClose(pts);
            if (verb == SkPathVerb::kLine) {
                fVerbs--; // move back one verb
            } else {
                fNeedClose = false;
            }
            fLastPt = fMoveTo;
            break;
    }
    fPts = srcPts;
    return (Verb)verb;
}

static inline uint8_t SkPathIterPointsPerVerb(SkPathVerb verb) {
    static const uint8_t gCounts[] = { 1, 2, 3, 3, 4, 0 };
    unsigned index = static_cast<unsigned>(verb);
    SkASSERT(index < std::size(gCounts));
    return gCounts[index];
}

std::optional<SkPath::IterRec> SkPath::Iter::next() {
    auto legacyVerb = this->next(fStorage.data());
    if (legacyVerb == kDone_Verb) {
        return {};
    }

    SkPathVerb verb = static_cast<SkPathVerb>(legacyVerb);
    return {{
        verb,
        {fStorage.data(), SkPathIterPointsPerVerb(verb)},
        verb == SkPathVerb::kConic ? *fConicWeights : 1,
    }};
}

void SkPath::RawIter::setPath(const SkPath& path) {
    SkPathPriv::Iterate iterate(path);
    fIter = iterate.begin();
    fEnd = iterate.end();
}

SkPath::Verb SkPath::RawIter::next(SkPoint pts[4]) {
    if (!(fIter != fEnd)) {
        return kDone_Verb;
    }
    auto [verb, iterPts, weights] = *fIter;
    int numPts;
    switch (verb) {
        case SkPathVerb::kMove: numPts = 1; break;
        case SkPathVerb::kLine: numPts = 2; break;
        case SkPathVerb::kQuad: numPts = 3; break;
        case SkPathVerb::kConic:
            numPts = 3;
            fConicWeight = *weights;
            break;
        case SkPathVerb::kCubic: numPts = 4; break;
        case SkPathVerb::kClose: numPts = 0; break;
    }
    memcpy(pts, iterPts, sizeof(SkPoint) * numPts);
    ++fIter;
    return (Verb) verb;
}

std::optional<SkPath::IterRec> SkPath::RawIter::next() {
    if (fIter == fEnd) {
        return {};
    }

    auto [verb, iterPts, weights] = *fIter++;
    return {{
        verb,
        {iterPts, SkPathIterPointsPerVerb(verb)},
        verb == SkPathVerb::kConic ? *weights : 1
    }};
}

///////////////////////////////////////////////////////////////////////////////

SkPath SkPath::makeFillType(SkPathFillType ft) const {
    SkPath copy = *this;
    copy.setFillType(ft);
    return copy;
}

SkPath SkPath::makeToggleInverseFillType() const {
    return this->makeFillType(SkPathFillType_ToggleInverse(fFillType));
}

SkPath SkPath::makeIsVolatile(bool v) const {
    SkPath copy = *this;
    copy.fIsVolatile = v;
    return copy;
}

SkPathConvexity SkPath::computeConvexity() const {
    if (auto c = this->getConvexityOrUnknown(); c != SkPathConvexity::kUnknown) {
        return c;
    }

    SkPathConvexity convexity = SkPathConvexity::kConcave;

    if (this->isFinite()) {
        convexity = SkPathPriv::ComputeConvexity(this->points(),
                                                 this->verbs(),
                                                 this->conicWeights());
    }

    SkASSERT(convexity != SkPathConvexity::kUnknown);
    this->setConvexity(convexity);
    return convexity;
}

bool SkPath::contains(SkPoint p) const {
    const auto raw = SkPathPriv::Raw(*this, SkResolveConvexity::kNo);
    return raw.has_value() && SkPathPriv::Contains(*raw, p);
}

int SkPath::ConvertConicToQuads(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                                SkScalar w, SkPoint pts[], int pow2) {
    const SkConic conic(p0, p1, p2, w);
    return conic.chopIntoQuadsPOW2(pts, pow2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRect SkPath::computeTightBounds() const {
    // If we're only lines, then our (quick) bounds is also tight.
    if (this->getSegmentMasks() == SkPath::kLine_SegmentMask) {
        return this->getBounds();
    }

    return SkPathPriv::ComputeTightBounds(this->points(),
                                          this->verbs(),
                                          this->conicWeights());
}

bool SkPath::IsLineDegenerate(const SkPoint& p1, const SkPoint& p2, bool exact) {
    return exact ? p1 == p2 : SkPointPriv::EqualsWithinTolerance(p1, p2);
}

bool SkPath::IsQuadDegenerate(const SkPoint& p1, const SkPoint& p2,
                                const SkPoint& p3, bool exact) {
    return exact ? p1 == p2 && p2 == p3 : SkPointPriv::EqualsWithinTolerance(p1, p2) &&
            SkPointPriv::EqualsWithinTolerance(p2, p3);
}

bool SkPath::IsCubicDegenerate(const SkPoint& p1, const SkPoint& p2,
                                const SkPoint& p3, const SkPoint& p4, bool exact) {
    return exact ? p1 == p2 && p2 == p3 && p3 == p4 :
            SkPointPriv::EqualsWithinTolerance(p1, p2) &&
            SkPointPriv::EqualsWithinTolerance(p2, p3) &&
            SkPointPriv::EqualsWithinTolerance(p3, p4);
}

SkPath SkPath::RRect(const SkRRect& rr, SkPathDirection dir) {
    // legacy start indices: 6 (CW) and 7 (CCW)
    return RRect(rr, dir, dir == SkPathDirection::kCW ? 6 : 7);
}

SkPath SkPath::Oval(const SkRect& r, SkPathDirection dir) {
    // legacy start index: 1
    return Oval(r, dir, 1);
}

SkPath SkPath::Circle(SkScalar x, SkScalar y, SkScalar r, SkPathDirection dir) {
    if (r >= 0) {
        return Oval(SkRect::MakeLTRB(x - r, y - r, x + r, y + r), dir);
    } else {
        return SkPath();
    }
}

SkPath SkPath::RRect(const SkRect& r, SkScalar rx, SkScalar ry, SkPathDirection dir) {
    return RRect(SkRRect::MakeRectXY(r, rx, ry), dir);
}

SkPathFirstDirection SkPathPriv::ComputeFirstDirection(const SkPath& path) {
    auto convexity = path.getConvexityOrUnknown();
    if (SkPathConvexity_IsConvex(convexity)) {
        // Note, this can return kUnknown. That is valid. If we've determined that the
        // path is convex, then we've already tried to compute its first-direction. If
        // that failed, then kUnknown is the right answer.
        return SkPathConvexity_ToFirstDirection(convexity);
    }

    // Note, this can compute a 'first' direction, even for non-convex shapes.
    if (auto raw = SkPathPriv::Raw(path, SkResolveConvexity::kNo)) {
        return ComputeFirstDirection(*raw);
    } else {
        return SkPathFirstDirection::kUnknown;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

struct SkHalfPlane {
    SkScalar fA, fB, fC;

    SkScalar eval(SkScalar x, SkScalar y) const {
        return fA * x + fB * y + fC;
    }
    SkScalar operator()(SkScalar x, SkScalar y) const { return this->eval(x, y); }

    bool normalize() {
        double a = fA;
        double b = fB;
        double c = fC;
        double dmag = sqrt(a * a + b * b);
        // length of initial plane normal is zero
        if (dmag == 0) {
           fA = fB = 0;
           fC = SK_Scalar1;
           return true;
        }
        double dscale = sk_ieee_double_divide(1.0, dmag);
        a *= dscale;
        b *= dscale;
        c *= dscale;
        // check if we're not finite, or normal is zero-length
        if (!SkIsFinite(a, b, c) ||
            (a == 0 && b == 0)) {
            fA = fB = 0;
            fC = SK_Scalar1;
            return false;
        }
        fA = a;
        fB = b;
        fC = c;
        return true;
    }

    enum Result {
        kAllNegative,
        kAllPositive,
        kMixed
    };
    Result test(const SkRect& bounds) const {
        // check whether the diagonal aligned with the normal crosses the plane
        SkPoint diagMin, diagMax;
        if (fA >= 0) {
            diagMin.fX = bounds.fLeft;
            diagMax.fX = bounds.fRight;
        } else {
            diagMin.fX = bounds.fRight;
            diagMax.fX = bounds.fLeft;
        }
        if (fB >= 0) {
            diagMin.fY = bounds.fTop;
            diagMax.fY = bounds.fBottom;
        } else {
            diagMin.fY = bounds.fBottom;
            diagMax.fY = bounds.fTop;
        }
        SkScalar test = this->eval(diagMin.fX, diagMin.fY);
        SkScalar sign = test*this->eval(diagMax.fX, diagMax.fY);
        if (sign > 0) {
            // the path is either all on one side of the half-plane or the other
            if (test < 0) {
                return kAllNegative;
            } else {
                return kAllPositive;
            }
        }
        return kMixed;
    }
};

// assumes plane is pre-normalized
static std::optional<SkPath> clip(const SkPath& path, const SkHalfPlane& plane) {
    SkMatrix mx;
    SkPoint p0 = { -plane.fA*plane.fC, -plane.fB*plane.fC };
    mx.setAll( plane.fB, plane.fA, p0.fX,
              -plane.fA, plane.fB, p0.fY,
                      0,        0,     1);
    auto inv = mx.invert();
    if (!inv) {
        return {};
    }

    auto rotated = path.tryMakeTransform(*inv);
    if (!rotated) {
        return {};
    }
    auto raw = SkPathPriv::Raw(*rotated, SkResolveConvexity::kNo);
    if (!raw) {
        SkASSERT(false);    // if rotated was valid, so should the raw
        return {};
    }

    SkScalar big = SK_ScalarMax;
    SkRect clip = {-big, 0, big, big };

    struct Rec {
        SkPathBuilder fResult;
        SkPoint       fPrev = {0,0};
    } rec;

    SkEdgeClipper::ClipPath(*raw, clip, false,
                            [](SkEdgeClipper* clipper, bool newCtr, void* ctx) {
        Rec* rec = (Rec*)ctx;

        bool addLineTo = false;
        SkPoint      pts[4];
        while (auto verb = clipper->next(pts)) {
            if (newCtr) {
                rec->fResult.moveTo(pts[0]);
                rec->fPrev = pts[0];
                newCtr = false;
            }

            if (addLineTo || pts[0] != rec->fPrev) {
                rec->fResult.lineTo(pts[0]);
            }

            switch (*verb) {
                case SkPathVerb::kLine:
                    rec->fResult.lineTo(pts[1]);
                    rec->fPrev = pts[1];
                    break;
                case SkPathVerb::kQuad:
                    rec->fResult.quadTo(pts[1], pts[2]);
                    rec->fPrev = pts[2];
                    break;
                case SkPathVerb::kCubic:
                    rec->fResult.cubicTo(pts[1], pts[2], pts[3]);
                    rec->fPrev = pts[3];
                    break;
                default: break;
            }
            addLineTo = true;
        }
    }, &rec);

    rec.fResult.setFillType(path.getFillType());
    SkPath result = rec.fResult.detach(&mx);
    if (!result.isFinite()) {
        return {};
    }
    return result;
}

// true means we have written to clippedPath
bool SkPathPriv::PerspectiveClip(const SkPath& path, const SkMatrix& matrix, SkPath* clippedPath) {
    if (!matrix.hasPerspective()) {
        return false;
    }

    SkHalfPlane plane {
        matrix[SkMatrix::kMPersp0],
        matrix[SkMatrix::kMPersp1],
        matrix[SkMatrix::kMPersp2] - kW0PlaneDistance
    };
    if (plane.normalize()) {
        switch (plane.test(path.getBounds())) {
            case SkHalfPlane::kAllPositive:
                return false;
            case SkHalfPlane::kMixed: {
                if (auto result = clip(path, plane)) {
                    *clippedPath = *result;
                } else {
                    *clippedPath = SkPath(); // clipped out (or failed)
                }
                return true;
            }
            default: break; // handled outside of the switch
        }
    }
    // clipped out (or failed)
    *clippedPath = SkPath();
    return true;
}

bool SkPathPriv::IsAxisAligned(const SkPath& path) {
    return IsAxisAligned(path.points());
}

std::optional<SkPathRectInfo> SkPathPriv::IsSimpleRect(const SkPath& path, bool isSimpleFill) {
    if (path.getSegmentMasks() != SkPath::kLine_SegmentMask) {
        return {};
    }
    SkPoint rectPts[5];
    int rectPtCnt = 0;
    bool needsClose = !isSimpleFill;
    for (auto [v, verbPts, w] : SkPathPriv::Iterate(path)) {
        switch (v) {
            case SkPathVerb::kMove:
                if (0 != rectPtCnt) {
                    return {};
                }
                rectPts[0] = verbPts[0];
                ++rectPtCnt;
                break;
            case SkPathVerb::kLine:
                if (5 == rectPtCnt) {
                    return {};
                }
                rectPts[rectPtCnt] = verbPts[1];
                ++rectPtCnt;
                break;
            case SkPathVerb::kClose:
                if (4 == rectPtCnt) {
                    rectPts[4] = rectPts[0];
                    rectPtCnt = 5;
                }
                needsClose = false;
                break;
            case SkPathVerb::kQuad:
            case SkPathVerb::kConic:
            case SkPathVerb::kCubic:
                return {};
        }
    }
    if (needsClose) {
        return {};
    }
    if (rectPtCnt < 5) {
        return {};
    }
    if (rectPts[0] != rectPts[4]) {
        return {};
    }
    // Check for two cases of rectangles: pts 0 and 3 form a vertical edge or a horizontal edge (
    // and pts 1 and 2 the opposite vertical or horizontal edge).
    bool vec03IsVertical;
    if (rectPts[0].fX == rectPts[3].fX && rectPts[1].fX == rectPts[2].fX &&
        rectPts[0].fY == rectPts[1].fY && rectPts[3].fY == rectPts[2].fY) {
        // Make sure it has non-zero width and height
        if (rectPts[0].fX == rectPts[1].fX || rectPts[0].fY == rectPts[3].fY) {
            return {};
        }
        vec03IsVertical = true;
    } else if (rectPts[0].fY == rectPts[3].fY && rectPts[1].fY == rectPts[2].fY &&
               rectPts[0].fX == rectPts[1].fX && rectPts[3].fX == rectPts[2].fX) {
        // Make sure it has non-zero width and height
        if (rectPts[0].fY == rectPts[1].fY || rectPts[0].fX == rectPts[3].fX) {
            return {};
        }
        vec03IsVertical = false;
    } else {
        return {};
    }

    SkPathRectInfo info;

    // Set sortFlags so that it has the low bit set if pt index 0 is on right edge and second bit
    // set if it is on the bottom edge.
    unsigned sortFlags =
            ((rectPts[0].fX < rectPts[2].fX) ? 0b00 : 0b01) |
            ((rectPts[0].fY < rectPts[2].fY) ? 0b00 : 0b10);
    switch (sortFlags) {
        case 0b00:
            info.fRect.setLTRB(rectPts[0].fX, rectPts[0].fY, rectPts[2].fX, rectPts[2].fY);
            info.fDirection = vec03IsVertical ? SkPathDirection::kCW : SkPathDirection::kCCW;
            info.fStartIndex = 0;
            break;
        case 0b01:
            info.fRect.setLTRB(rectPts[2].fX, rectPts[0].fY, rectPts[0].fX, rectPts[2].fY);
            info.fDirection = vec03IsVertical ? SkPathDirection::kCCW : SkPathDirection::kCW;
            info.fStartIndex = 1;
            break;
        case 0b10:
            info.fRect.setLTRB(rectPts[0].fX, rectPts[2].fY, rectPts[2].fX, rectPts[0].fY);
            info.fDirection = vec03IsVertical ? SkPathDirection::kCCW : SkPathDirection::kCW;
            info.fStartIndex = 3;
            break;
        case 0b11:
            info.fRect.setLTRB(rectPts[2].fX, rectPts[2].fY, rectPts[0].fX, rectPts[0].fY);
            info.fDirection = vec03IsVertical ? SkPathDirection::kCW : SkPathDirection::kCCW;
            info.fStartIndex = 2;
            break;
    }
    return info;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SkPathEdgeIter::SkPathEdgeIter(const SkPathRaw& raw) {
    fMoveToPtr = fPts = raw.fPoints.data();
    fVerbs = raw.fVerbs.data();
    fVerbsStop = fVerbs + raw.fVerbs.size();
    fConicWeights = raw.fConics.data();
    if (fConicWeights) {
        fConicWeights -= 1;  // begin one behind
    }

    fNeedsCloseLine = false;
    fNextIsNewContour = false;
    SkDEBUGCODE(fIsConic = false;)
}

SkPathEdgeIter::SkPathEdgeIter(const SkPath& path)
    : SkPathEdgeIter(SkPathPriv::Raw(path, SkResolveConvexity::kNo).value_or(SkPathRaw::Empty()))
{}
