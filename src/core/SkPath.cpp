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
#include "include/private/SkPathRef.h"
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

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits.h>
#include <utility>

struct SkPath_Storage_Equivalent {
    void*    fPtr;
    int32_t  fIndex;
    uint32_t fFlags;
};

static_assert(sizeof(SkPath) == sizeof(SkPath_Storage_Equivalent),
              "Please keep an eye on SkPath packing.");

static float poly_eval(float A, float B, float C, float t) {
    return (A * t + B) * t + C;
}

static float poly_eval(float A, float B, float C, float D, float t) {
    return ((A * t + B) * t + C) * t + D;
}

////////////////////////////////////////////////////////////////////////////

// flag to require a moveTo if we begin with something else, like lineTo etc.
// This will also be the value of lastMoveToIndex for a single contour
// ending with close, so countVerbs needs to be checked against 0.
#define INITIAL_LASTMOVETOINDEX_VALUE   ~0

SkPath::SkPath(sk_sp<SkPathRef> pr, SkPathFillType ft, bool isVolatile, SkPathConvexity ct)
    : fPathRef(std::move(pr))
    , fLastMoveToIndex(INITIAL_LASTMOVETOINDEX_VALUE)
    , fConvexity((uint8_t)ct)
    , fFillType((unsigned)ft)
    , fIsVolatile(isVolatile)
{}

SkPath::SkPath(SkPathFillType ft)
    : fPathRef(SkPathRef::CreateEmpty())
    , fLastMoveToIndex(INITIAL_LASTMOVETOINDEX_VALUE)
    , fConvexity((uint8_t)SkPathConvexity::kUnknown)
    , fFillType((unsigned)ft)
    , fIsVolatile(false)
{}

void SkPath::resetFields() {
    //fPathRef is assumed to have been emptied by the caller.
    fLastMoveToIndex = INITIAL_LASTMOVETOINDEX_VALUE;
    fFillType = SkToU8(SkPathFillType::kDefault);
    this->setConvexity(SkPathConvexity::kUnknown);
}

SkPath::SkPath(const SkPath& that)
    : fPathRef(SkRef(that.fPathRef.get())) {
    this->copyFields(that);
    SkDEBUGCODE(that.validate();)
}

SkPath::~SkPath() {
    SkDEBUGCODE(this->validate();)
}

SkPath& SkPath::operator=(const SkPath& that) {
    SkDEBUGCODE(that.validate();)

    if (this != &that) {
        fPathRef.reset(SkRef(that.fPathRef.get()));
        this->copyFields(that);
    }
    SkDEBUGCODE(this->validate();)
    return *this;
}

void SkPath::copyFields(const SkPath& that) {
    //fPathRef is assumed to have been set by the caller.
    fLastMoveToIndex = that.fLastMoveToIndex;
    fFillType        = that.fFillType;
    fIsVolatile      = that.fIsVolatile;

    // Non-atomic assignment of atomic values.
    this->setConvexity(that.getConvexityOrUnknown());
}

bool operator==(const SkPath& a, const SkPath& b) {
    // note: don't need to look at isConvex or bounds, since just comparing the
    // raw data is sufficient.
    return &a == &b ||
        (a.fFillType == b.fFillType && *a.fPathRef == *b.fPathRef);
}

void SkPath::swap(SkPath& that) {
    if (this != &that) {
        fPathRef.swap(that.fPathRef);
        std::swap(fLastMoveToIndex, that.fLastMoveToIndex);

        const auto ft = fFillType;
        fFillType = that.fFillType;
        that.fFillType = ft;

        const auto iv = fIsVolatile;
        fIsVolatile = that.fIsVolatile;
        that.fIsVolatile = iv;

        // Non-atomic swaps of atomic values.
        SkPathConvexity c = this->getConvexityOrUnknown();
        this->setConvexity(that.getConvexityOrUnknown());
        that.setConvexity(c);
    }
}

// This is the public-facing non-const setConvexity().
void SkPath::setConvexity(SkPathConvexity c) {
    fConvexity.store((uint8_t)c, std::memory_order_relaxed);
}

// Const hooks for working with fConvexity and fFirstDirection from const methods.
void SkPath::setConvexity(SkPathConvexity c) const {
    fConvexity.store((uint8_t)c, std::memory_order_relaxed);
}

bool SkPath::isInterpolatable(const SkPath& compare) const {
    // need the same structure (verbs, conicweights) and same point-count
    return fPathRef->fPoints.size() == compare.fPathRef->fPoints.size() &&
           fPathRef->fVerbs == compare.fPathRef->fVerbs &&
           fPathRef->fConicWeights == compare.fPathRef->fConicWeights;
}

bool SkPath::interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const {
    int pointCount = fPathRef->countPoints();
    if (pointCount != ending.fPathRef->countPoints()) {
        return false;
    }
    if (!pointCount) {
        return true;
    }
    *out = *this;
    SkPathRef::Editor editor(&(out->fPathRef));
    fPathRef->interpolate(*ending.fPathRef, weight, out->fPathRef.get());
    return true;
}

SkPath SkPath::makeInterpolate(const SkPath& ending, SkScalar weight) const {
    SkPath out;
    this->interpolate(ending, weight, &out);
    return out;
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

            if (!SkPathPriv::AllPointsEq({pts, pointCount + 1})) {
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

uint32_t SkPath::getGenerationID() const {
    return fPathRef->genID(fFillType);
}

SkPath& SkPath::reset() {
    SkDEBUGCODE(this->validate();)

    if (fPathRef->unique()) {
        fPathRef->reset();
    } else {
        fPathRef.reset(SkPathRef::CreateEmpty());
    }
    this->resetFields();
    return *this;
}

bool SkPath::isLastContourClosed() const {
    int verbCount = fPathRef->countVerbs();
    if (0 == verbCount) {
        return false;
    }
    return SkPathVerb::kClose == fPathRef->atVerb(verbCount - 1);
}

bool SkPath::isLine(SkPoint line[2]) const {
    int verbCount = fPathRef->countVerbs();

    if (2 == verbCount) {
        SkASSERT(SkPathVerb::kMove == fPathRef->verbs()[0]);
        if (SkPathVerb::kLine == fPathRef->verbs()[1]) {
            SkASSERT(2 == fPathRef->countPoints());
            if (line) {
                const SkPoint* pts = fPathRef->points();
                line[0] = pts[0];
                line[1] = pts[1];
            }
            return true;
        }
    }
    return false;
}

bool SkPath::isEmpty() const {
    SkDEBUGCODE(this->validate();)
    return 0 == fPathRef->countVerbs();
}

bool SkPath::isFinite() const {
    SkDEBUGCODE(this->validate();)
    return fPathRef->isFinite();
}

bool SkPath::isConvex() const {
    return SkPathConvexity_IsConvex(this->getConvexity());
}

const SkRect& SkPath::getBounds() const {
    return fPathRef->getBounds();
}

uint32_t SkPath::getSegmentMasks() const {
    return fPathRef->getSegmentMasks();
}

bool SkPath::isValid() const {
    return this->isValidImpl() && fPathRef->isValid();
}

bool SkPath::hasComputedBounds() const {
    SkDEBUGCODE(this->validate();)
    return fPathRef->hasComputedBounds();
}

SkPathConvexity SkPath::getConvexityOrUnknown() const {
    return (SkPathConvexity)fConvexity.load(std::memory_order_relaxed);
}

SkPath SkPath::makeFillType(SkPathFillType ft) const {
    return SkPath(fPathRef,
                  ft,
                  fIsVolatile,
                  this->getConvexityOrUnknown());
}

SkPath SkPath::makeToggleInverseFillType() const {
    return SkPath(fPathRef,
                  static_cast<SkPathFillType>(fFillType ^ 2),
                  fIsVolatile,
                  this->getConvexityOrUnknown());
}

SkPath SkPath::makeIsVolatile(bool v) const {
    return SkPath(fPathRef,
                  static_cast<SkPathFillType>(fFillType),
                  v,
                  this->getConvexityOrUnknown());
}

#ifdef SK_DEBUG
void SkPath::validate() const {
    SkASSERT(this->isValidImpl());
}

void SkPath::validateRef() const {
    // This will SkASSERT if not valid.
    fPathRef->validate();
}
#endif
bool SkPath::isRect(SkRect* rect, bool* isClosed, SkPathDirection* direction) const {
    SkDEBUGCODE(this->validate();)
    SkSpan<const SkPoint> pts = {fPathRef->points(), fPathRef->countPoints()};
    SkSpan<const SkPathVerb> vbs = fPathRef->verbs();
    if (auto rc = SkPathPriv::IsRectContour(pts, vbs, false)) {
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
    if (auto info = fPathRef->isOval()) {
        if (bounds) {
            *bounds = info->fBounds;
        }
        return true;
    }
    return false;
}

bool SkPath::isRRect(SkRRect* rrect) const {
    if (auto info = fPathRef->isRRect()) {
        if (rrect) {
            *rrect = info->fRRect;
        }
        return true;
    }
    return false;
}

int SkPath::countPoints() const {
    return fPathRef->countPoints();
}

size_t SkPath::getPoints(SkSpan<SkPoint> dst) const {
    SkDEBUGCODE(this->validate();)

    const size_t ptCount = fPathRef->countPoints();
    const size_t n = std::min(dst.size(), ptCount);
    sk_careful_memcpy(dst.data(), fPathRef->points(), n * sizeof(SkPoint));
    return ptCount;
}

SkPoint SkPath::getPoint(int index) const {
    if ((unsigned)index < (unsigned)fPathRef->countPoints()) {
        return fPathRef->atPoint(index);
    }
    return SkPoint::Make(0, 0);
}

int SkPath::countVerbs() const {
    return fPathRef->countVerbs();
}

size_t SkPath::getVerbs(SkSpan<uint8_t> dst) const {
    SkDEBUGCODE(this->validate();)

    const size_t vbCount = fPathRef->countVerbs();
    const size_t n = std::min(dst.size(), vbCount);
    sk_careful_memcpy(dst.data(), fPathRef->verbsBegin(), n);
    return vbCount;
}

size_t SkPath::approximateBytesUsed() const {
    size_t size = sizeof (SkPath);
    if (fPathRef != nullptr) {
        size += fPathRef->approximateBytesUsed();
    }
    return size;
}

std::optional<SkPoint> SkPath::getLastPt() const {
    SkDEBUGCODE(this->validate();)

    if (const int count = fPathRef->countPoints()) {
        return fPathRef->atPoint(count - 1);
    }
    return {};
}

bool SkPath::isConvexityAccurate() const {
    SkPathConvexity convexity = this->getConvexityOrUnknown();
    if (convexity != SkPathConvexity::kUnknown) {
        auto conv = this->computeConvexity();
        if (conv != convexity) {
            SkASSERT(false);
            return false;
        }
    }
    return true;
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
    return { fPathRef->fPoints, fPathRef->verbs(), fPathRef->fConicWeights };
}

bool SkPath::isZeroLengthSincePoint(int startPtIndex) const {
    int count = fPathRef->countPoints() - startPtIndex;
    if (count < 2) {
        return true;
    }
    const SkPoint* pts = fPathRef->points() + startPtIndex;
    const SkPoint& first = *pts;
    for (int index = 1; index < count; ++index) {
        if (first != pts[index]) {
            return false;
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
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
    fPts = path.fPathRef->points();
    fVerbs = path.fPathRef->verbsBegin();
    fVerbStop = path.fPathRef->verbsEnd();
    fConicWeights = path.fPathRef->conicWeights();
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

bool SkPath::isValidImpl() const {
    if ((fFillType & ~3) != 0) {
        return false;
    }

#ifdef SK_DEBUG_PATH
    if (!fBoundsIsDirty) {
        SkRect bounds;

        bool isFinite = compute_pt_bounds(&bounds, *fPathRef.get());
        if (SkToBool(fIsFinite) != isFinite) {
            return false;
        }

        if (fPathRef->countPoints() <= 1) {
            // if we're empty, fBounds may be empty but translated, so we can't
            // necessarily compare to bounds directly
            // try path.addOval(2, 2, 2, 2) which is empty, but the bounds will
            // be [2, 2, 2, 2]
            if (!bounds.isEmpty() || !fBounds.isEmpty()) {
                return false;
            }
        } else {
            if (bounds.isEmpty()) {
                if (!fBounds.isEmpty()) {
                    return false;
                }
            } else {
                if (!fBounds.isEmpty()) {
                    if (!fBounds.contains(bounds)) {
                        return false;
                    }
                }
            }
        }
    }
#endif // SK_DEBUG_PATH
    return true;
}

///////////////////////////////////////////////////////////////////////////////


SkPathConvexity SkPath::computeConvexity() const {
    if (auto c = this->getConvexityOrUnknown(); c != SkPathConvexity::kUnknown) {
        return c;
    }

    SkPathConvexity convexity = SkPathConvexity::kConcave;

    if (this->isFinite()) {
        convexity = SkPathPriv::ComputeConvexity(fPathRef->pointSpan(),
                                                 fPathRef->verbs(),
                                                 fPathRef->conicSpan());
    }

    SkASSERT(convexity != SkPathConvexity::kUnknown);
    this->setConvexity(convexity);
    return convexity;
}

///////////////////////////////////////////////////////////////////////////////

static bool between(SkScalar a, SkScalar b, SkScalar c) {
    SkASSERT(((a <= b && b <= c) || (a >= b && b >= c)) == ((a - b) * (c - b) <= 0)
            || (SkScalarNearlyZero(a) && SkScalarNearlyZero(b) && SkScalarNearlyZero(c)));
    return (a - b) * (c - b) <= 0;
}

static SkScalar eval_cubic_pts(SkScalar c0, SkScalar c1, SkScalar c2, SkScalar c3,
                               SkScalar t) {
    SkScalar A = c3 + 3*(c1 - c2) - c0;
    SkScalar B = 3*(c2 - c1 - c1 + c0);
    SkScalar C = 3*(c1 - c0);
    SkScalar D = c0;
    return poly_eval(A, B, C, D, t);
}

template <size_t N> static void find_minmax(const SkPoint pts[],
                                            SkScalar* minPtr, SkScalar* maxPtr) {
    SkScalar min, max;
    min = max = pts[0].fX;
    for (size_t i = 1; i < N; ++i) {
        min = std::min(min, pts[i].fX);
        max = std::max(max, pts[i].fX);
    }
    *minPtr = min;
    *maxPtr = max;
}

static bool checkOnCurve(SkScalar x, SkScalar y, const SkPoint& start, const SkPoint& end) {
    if (start.fY == end.fY) {
        return between(start.fX, x, end.fX) && x != end.fX;
    } else {
        return x == start.fX && y == start.fY;
    }
}

static int winding_mono_cubic(const SkPoint pts[], SkScalar x, SkScalar y, int* onCurveCount) {
    SkScalar y0 = pts[0].fY;
    SkScalar y3 = pts[3].fY;

    int dir = 1;
    if (y0 > y3) {
        using std::swap;
        swap(y0, y3);
        dir = -1;
    }
    if (y < y0 || y > y3) {
        return 0;
    }
    if (checkOnCurve(x, y, pts[0], pts[3])) {
        *onCurveCount += 1;
        return 0;
    }
    if (y == y3) {
        return 0;
    }

    // quickreject or quickaccept
    SkScalar min, max;
    find_minmax<4>(pts, &min, &max);
    if (x < min) {
        return 0;
    }
    if (x > max) {
        return dir;
    }

    // compute the actual x(t) value
    SkScalar t;
    if (!SkCubicClipper::ChopMonoAtY(pts, y, &t)) {
        return 0;
    }
    SkScalar xt = eval_cubic_pts(pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX, t);
    if (SkScalarNearlyEqual(xt, x)) {
        if (x != pts[3].fX || y != pts[3].fY) {  // don't test end points; they're start points
            *onCurveCount += 1;
            return 0;
        }
    }
    return xt < x ? dir : 0;
}

static int winding_cubic(SkSpan<const SkPoint> pts, SkScalar x, SkScalar y, int* onCurveCount) {
    SkPoint dst[10];
    int n = SkChopCubicAtYExtrema(pts.data(), dst);
    int w = 0;
    for (int i = 0; i <= n; ++i) {
        w += winding_mono_cubic(&dst[i * 3], x, y, onCurveCount);
    }
    return w;
}

static double conic_eval_numerator(const SkScalar src[], SkScalar w, SkScalar t) {
    SkASSERT(src);
    SkASSERT(t >= 0 && t <= 1);
    SkScalar src2w = src[2] * w;
    SkScalar C = src[0];
    SkScalar A = src[4] - 2 * src2w + C;
    SkScalar B = 2 * (src2w - C);
    return poly_eval(A, B, C, t);
}


static double conic_eval_denominator(SkScalar w, SkScalar t) {
    SkScalar B = 2 * (w - 1);
    SkScalar C = 1;
    SkScalar A = -B;
    return poly_eval(A, B, C, t);
}

static int winding_mono_conic(const SkConic& conic, SkScalar x, SkScalar y, int* onCurveCount) {
    const SkPoint* pts = conic.fPts;
    SkScalar y0 = pts[0].fY;
    SkScalar y2 = pts[2].fY;

    int dir = 1;
    if (y0 > y2) {
        using std::swap;
        swap(y0, y2);
        dir = -1;
    }
    if (y < y0 || y > y2) {
        return 0;
    }
    if (checkOnCurve(x, y, pts[0], pts[2])) {
        *onCurveCount += 1;
        return 0;
    }
    if (y == y2) {
        return 0;
    }

    SkScalar roots[2];
    SkScalar A = pts[2].fY;
    SkScalar B = pts[1].fY * conic.fW - y * conic.fW + y;
    SkScalar C = pts[0].fY;
    A += C - 2 * B;  // A = a + c - 2*(b*w - yCept*w + yCept)
    B -= C;  // B = b*w - w * yCept + yCept - a
    C -= y;
    int n = SkFindUnitQuadRoots(A, 2 * B, C, roots);
    SkASSERT(n <= 1);
    SkScalar xt;
    if (0 == n) {
        // zero roots are returned only when y0 == y
        // Need [0] if dir == 1
        // and  [2] if dir == -1
        xt = pts[1 - dir].fX;
    } else {
        SkScalar t = roots[0];
        xt = conic_eval_numerator(&pts[0].fX, conic.fW, t) / conic_eval_denominator(conic.fW, t);
    }
    if (SkScalarNearlyEqual(xt, x)) {
        if (x != pts[2].fX || y != pts[2].fY) {  // don't test end points; they're start points
            *onCurveCount += 1;
            return 0;
        }
    }
    return xt < x ? dir : 0;
}

static bool is_mono_quad(SkScalar y0, SkScalar y1, SkScalar y2) {
    //    return SkScalarSignAsInt(y0 - y1) + SkScalarSignAsInt(y1 - y2) != 0;
    if (y0 == y1) {
        return true;
    }
    if (y0 < y1) {
        return y1 <= y2;
    } else {
        return y1 >= y2;
    }
}

static int winding_conic(SkSpan<const SkPoint> pts, SkScalar x, SkScalar y, SkScalar weight,
                         int* onCurveCount) {
    SkConic conic(pts.data(), weight);
    SkConic chopped[2];
    // If the data points are very large, the conic may not be monotonic but may also
    // fail to chop. Then, the chopper does not split the original conic in two.
    bool isMono = is_mono_quad(pts[0].fY, pts[1].fY, pts[2].fY) || !conic.chopAtYExtrema(chopped);
    int w = winding_mono_conic(isMono ? conic : chopped[0], x, y, onCurveCount);
    if (!isMono) {
        w += winding_mono_conic(chopped[1], x, y, onCurveCount);
    }
    return w;
}

static int winding_mono_quad(SkSpan<const SkPoint> pts, SkScalar x, SkScalar y, int* onCurveCount) {
    SkScalar y0 = pts[0].fY;
    SkScalar y2 = pts[2].fY;

    int dir = 1;
    if (y0 > y2) {
        using std::swap;
        swap(y0, y2);
        dir = -1;
    }
    if (y < y0 || y > y2) {
        return 0;
    }
    if (checkOnCurve(x, y, pts[0], pts[2])) {
        *onCurveCount += 1;
        return 0;
    }
    if (y == y2) {
        return 0;
    }
    // bounds check on X (not required. is it faster?)
#if 0
    if (pts[0].fX > x && pts[1].fX > x && pts[2].fX > x) {
        return 0;
    }
#endif

    SkScalar roots[2];
    int n = SkFindUnitQuadRoots(pts[0].fY - 2 * pts[1].fY + pts[2].fY,
                                2 * (pts[1].fY - pts[0].fY),
                                pts[0].fY - y,
                                roots);
    SkASSERT(n <= 1);
    SkScalar xt;
    if (0 == n) {
        // zero roots are returned only when y0 == y
        // Need [0] if dir == 1
        // and  [2] if dir == -1
        xt = pts[1 - dir].fX;
    } else {
        SkScalar t = roots[0];
        SkScalar C = pts[0].fX;
        SkScalar A = pts[2].fX - 2 * pts[1].fX + C;
        SkScalar B = 2 * (pts[1].fX - C);
        xt = poly_eval(A, B, C, t);
    }
    if (SkScalarNearlyEqual(xt, x)) {
        if (x != pts[2].fX || y != pts[2].fY) {  // don't test end points; they're start points
            *onCurveCount += 1;
            return 0;
        }
    }
    return xt < x ? dir : 0;
}

static int winding_quad(SkSpan<const SkPoint> pts, SkScalar x, SkScalar y, int* onCurveCount) {
    SkPoint               spanStorage[5];
    SkSpan<const SkPoint> span = pts;
    int                   n = 0;

    if (!is_mono_quad(pts[0].fY, pts[1].fY, pts[2].fY)) {
        n = SkChopQuadAtYExtrema(pts.data(), spanStorage);
        span = spanStorage;
    }
    int w = winding_mono_quad(span, x, y, onCurveCount);
    if (n > 0) {
        w += winding_mono_quad(span.subspan(2), x, y, onCurveCount);
    }
    return w;
}

static int winding_line(SkSpan<const SkPoint> pts, SkScalar x, SkScalar y, int* onCurveCount) {
    SkScalar x0 = pts[0].fX;
    SkScalar y0 = pts[0].fY;
    SkScalar x1 = pts[1].fX;
    SkScalar y1 = pts[1].fY;

    SkScalar dy = y1 - y0;

    int dir = 1;
    if (y0 > y1) {
        using std::swap;
        swap(y0, y1);
        dir = -1;
    }
    if (y < y0 || y > y1) {
        return 0;
    }
    if (checkOnCurve(x, y, pts[0], pts[1])) {
        *onCurveCount += 1;
        return 0;
    }
    if (y == y1) {
        return 0;
    }
    SkScalar cross = (x1 - x0) * (y - pts[0].fY) - dy * (x - x0);

    if (!cross) {
        // zero cross means the point is on the line, and since the case where
        // y of the query point is at the end point is handled above, we can be
        // sure that we're on the line (excluding the end point) here
        if (x != x1 || y != pts[1].fY) {
            *onCurveCount += 1;
        }
        dir = 0;
    } else if (SkScalarSignAsInt(cross) == dir) {
        dir = 0;
    }
    return dir;
}

static void tangent_cubic(SkSpan<const SkPoint> pts, SkScalar x, SkScalar y,
        SkTDArray<SkVector>* tangents) {
    if (!between(pts[0].fY, y, pts[1].fY) && !between(pts[1].fY, y, pts[2].fY)
             && !between(pts[2].fY, y, pts[3].fY)) {
        return;
    }
    if (!between(pts[0].fX, x, pts[1].fX) && !between(pts[1].fX, x, pts[2].fX)
             && !between(pts[2].fX, x, pts[3].fX)) {
        return;
    }
    SkPoint dst[10];
    int n = SkChopCubicAtYExtrema(pts.data(), dst);
    for (int i = 0; i <= n; ++i) {
        SkPoint* c = &dst[i * 3];
        SkScalar t;
        if (!SkCubicClipper::ChopMonoAtY(c, y, &t)) {
            continue;
        }
        SkScalar xt = eval_cubic_pts(c[0].fX, c[1].fX, c[2].fX, c[3].fX, t);
        if (!SkScalarNearlyEqual(x, xt)) {
            continue;
        }
        SkVector tangent;
        SkEvalCubicAt(c, t, nullptr, &tangent, nullptr);
        tangents->push_back(tangent);
    }
}

static void tangent_conic(SkSpan<const SkPoint> pts, SkScalar x, SkScalar y, SkScalar w,
            SkTDArray<SkVector>* tangents) {
    if (!between(pts[0].fY, y, pts[1].fY) && !between(pts[1].fY, y, pts[2].fY)) {
        return;
    }
    if (!between(pts[0].fX, x, pts[1].fX) && !between(pts[1].fX, x, pts[2].fX)) {
        return;
    }
    SkScalar roots[2];
    SkScalar A = pts[2].fY;
    SkScalar B = pts[1].fY * w - y * w + y;
    SkScalar C = pts[0].fY;
    A += C - 2 * B;  // A = a + c - 2*(b*w - yCept*w + yCept)
    B -= C;  // B = b*w - w * yCept + yCept - a
    C -= y;
    int n = SkFindUnitQuadRoots(A, 2 * B, C, roots);
    for (int index = 0; index < n; ++index) {
        SkScalar t = roots[index];
        SkScalar xt = conic_eval_numerator(&pts[0].fX, w, t) / conic_eval_denominator(w, t);
        if (!SkScalarNearlyEqual(x, xt)) {
            continue;
        }
        SkConic conic(pts.data(), w);
        tangents->push_back(conic.evalTangentAt(t));
    }
}

static void tangent_quad(SkSpan<const SkPoint> pts, SkScalar x, SkScalar y,
        SkTDArray<SkVector>* tangents) {
    if (!between(pts[0].fY, y, pts[1].fY) && !between(pts[1].fY, y, pts[2].fY)) {
        return;
    }
    if (!between(pts[0].fX, x, pts[1].fX) && !between(pts[1].fX, x, pts[2].fX)) {
        return;
    }
    SkScalar roots[2];
    int n = SkFindUnitQuadRoots(pts[0].fY - 2 * pts[1].fY + pts[2].fY,
                                2 * (pts[1].fY - pts[0].fY),
                                pts[0].fY - y,
                                roots);
    for (int index = 0; index < n; ++index) {
        SkScalar t = roots[index];
        SkScalar C = pts[0].fX;
        SkScalar A = pts[2].fX - 2 * pts[1].fX + C;
        SkScalar B = 2 * (pts[1].fX - C);
        SkScalar xt = poly_eval(A, B, C, t);
        if (!SkScalarNearlyEqual(x, xt)) {
            continue;
        }
        tangents->push_back(SkEvalQuadTangentAt(pts.data(), t));
    }
}

static void tangent_line(SkSpan<const SkPoint> pts, SkScalar x, SkScalar y,
        SkTDArray<SkVector>* tangents) {
    SkScalar y0 = pts[0].fY;
    SkScalar y1 = pts[1].fY;
    if (!between(y0, y, y1)) {
        return;
    }
    SkScalar x0 = pts[0].fX;
    SkScalar x1 = pts[1].fX;
    if (!between(x0, x, x1)) {
        return;
    }
    SkScalar dx = x1 - x0;
    SkScalar dy = y1 - y0;
    if (!SkScalarNearlyEqual((x - x0) * dy, dx * (y - y0))) {
        return;
    }
    SkVector v;
    v.set(dx, dy);
    tangents->push_back(v);
}

static bool contains_inclusive(const SkRect& r, SkScalar x, SkScalar y) {
    return r.fLeft <= x && x <= r.fRight && r.fTop <= y && y <= r.fBottom;
}

bool SkPath::contains(SkScalar x, SkScalar y) const {
    bool isInverse = this->isInverseFillType();
    if (this->isEmpty()) {
        return isInverse;
    }

    if (!contains_inclusive(this->getBounds(), x, y)) {
        return isInverse;
    }

    SkPath::Iter iter(*this, true);
    int w = 0;
    int onCurveCount = 0;
    while (auto rec = iter.next()) {
        switch (rec->fVerb) {
            case SkPathVerb::kMove:
            case SkPathVerb::kClose:
                break;
            case SkPathVerb::kLine:
                w += winding_line(rec->fPoints, x, y, &onCurveCount);
                break;
            case SkPathVerb::kQuad:
                w += winding_quad(rec->fPoints, x, y, &onCurveCount);
                break;
            case SkPathVerb::kConic:
                w += winding_conic(rec->fPoints, x, y, rec->conicWeight(), &onCurveCount);
                break;
            case SkPathVerb::kCubic:
                w += winding_cubic(rec->fPoints, x, y, &onCurveCount);
                break;
       }
    }
    bool evenOddFill = SkPathFillType::kEvenOdd        == this->getFillType()
                    || SkPathFillType::kInverseEvenOdd == this->getFillType();
    if (evenOddFill) {
        w &= 1;
    }
    if (w) {
        return !isInverse;
    }
    if (onCurveCount <= 1) {
        return SkToBool(onCurveCount) ^ isInverse;
    }
    if ((onCurveCount & 1) || evenOddFill) {
        return SkToBool(onCurveCount & 1) ^ isInverse;
    }
    // If the point touches an even number of curves, and the fill is winding, check for
    // coincidence. Count coincidence as places where the on curve points have identical tangents.
    iter.setPath(*this, true);
    SkTDArray<SkVector> tangents;
    while (auto rec = iter.next()) {
        int oldCount = tangents.size();
        switch (rec->fVerb) {
            case SkPathVerb::kMove:
            case SkPathVerb::kClose:
                break;
            case SkPathVerb::kLine:
                tangent_line(rec->fPoints, x, y, &tangents);
                break;
            case SkPathVerb::kQuad:
                tangent_quad(rec->fPoints, x, y, &tangents);
                break;
            case SkPathVerb::kConic:
                tangent_conic(rec->fPoints, x, y, rec->conicWeight(), &tangents);
                break;
            case SkPathVerb::kCubic:
                tangent_cubic(rec->fPoints, x, y, &tangents);
                break;
       }
       if (tangents.size() > oldCount) {
            int last = tangents.size() - 1;
            const SkVector& tangent = tangents[last];
            if (SkScalarNearlyZero(SkPointPriv::LengthSqd(tangent))) {
                tangents.remove(last);
            } else {
                for (int index = 0; index < last; ++index) {
                    const SkVector& test = tangents[index];
                    if (SkScalarNearlyZero(test.cross(tangent))
                            && SkScalarSignAsInt(tangent.fX * test.fX) <= 0
                            && SkScalarSignAsInt(tangent.fY * test.fY) <= 0) {
                        tangents.remove(last);
                        tangents.removeShuffle(index);
                        break;
                    }
                }
            }
        }
    }
    return SkToBool(tangents.size()) ^ isInverse;
}

int SkPath::ConvertConicToQuads(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                                SkScalar w, SkPoint pts[], int pow2) {
    const SkConic conic(p0, p1, p2, w);
    return conic.chopIntoQuadsPOW2(pts, pow2);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static int compute_quad_extremas(const SkPoint src[3], SkPoint extremas[3]) {
    SkScalar ts[2];
    int n  = SkFindQuadExtrema(src[0].fX, src[1].fX, src[2].fX, ts);
        n += SkFindQuadExtrema(src[0].fY, src[1].fY, src[2].fY, &ts[n]);
    SkASSERT(n >= 0 && n <= 2);
    for (int i = 0; i < n; ++i) {
        extremas[i] = SkEvalQuadAt(src, ts[i]);
    }
    extremas[n] = src[2];
    return n + 1;
}

static int compute_conic_extremas(const SkPoint src[3], SkScalar w, SkPoint extremas[3]) {
    SkConic conic(src[0], src[1], src[2], w);
    SkScalar ts[2];
    int n  = conic.findXExtrema(ts);
        n += conic.findYExtrema(&ts[n]);
    SkASSERT(n >= 0 && n <= 2);
    for (int i = 0; i < n; ++i) {
        extremas[i] = conic.evalAt(ts[i]);
    }
    extremas[n] = src[2];
    return n + 1;
}

static int compute_cubic_extremas(const SkPoint src[4], SkPoint extremas[5]) {
    SkScalar ts[4];
    int n  = SkFindCubicExtrema(src[0].fX, src[1].fX, src[2].fX, src[3].fX, ts);
        n += SkFindCubicExtrema(src[0].fY, src[1].fY, src[2].fY, src[3].fY, &ts[n]);
    SkASSERT(n >= 0 && n <= 4);
    for (int i = 0; i < n; ++i) {
        SkEvalCubicAt(src, ts[i], &extremas[i], nullptr, nullptr);
    }
    extremas[n] = src[3];
    return n + 1;
}

SkRect SkPath::computeTightBounds() const {
    if (0 == this->countVerbs()) {
        return SkRect::MakeEmpty();
    }

    if (this->getSegmentMasks() == SkPath::kLine_SegmentMask) {
        return this->getBounds();
    }

    SkPoint extremas[5]; // big enough to hold worst-case curve type (cubic) extremas + 1

    // initial with the first MoveTo, so we don't have to check inside the switch
    skvx::float2 min, max;
    min = max = from_point(this->getPoint(0));
    for (auto [verb, pts, w] : SkPathPriv::Iterate(*this)) {
        int count = 0;
        switch (verb) {
            case SkPathVerb::kMove:
                extremas[0] = pts[0];
                count = 1;
                break;
            case SkPathVerb::kLine:
                extremas[0] = pts[1];
                count = 1;
                break;
            case SkPathVerb::kQuad:
                count = compute_quad_extremas(pts, extremas);
                break;
            case SkPathVerb::kConic:
                count = compute_conic_extremas(pts, *w, extremas);
                break;
            case SkPathVerb::kCubic:
                count = compute_cubic_extremas(pts, extremas);
                break;
            case SkPathVerb::kClose:
                break;
        }
        for (int i = 0; i < count; ++i) {
            skvx::float2 tmp = from_point(extremas[i]);
            min = skvx::min(min, tmp);
            max = skvx::max(max, tmp);
        }
    }
    SkRect bounds;
    min.store((SkPoint*)&bounds.fLeft);
    max.store((SkPoint*)&bounds.fRight);
    return bounds;
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

//////////////////////////////////////////////////////////////////////////////////////////////////

SkPath SkPath::Raw(SkSpan<const SkPoint> pts, SkSpan<const SkPathVerb> vbs,
                   SkSpan<const float> ws, SkPathFillType ft, bool isVolatile) {
    if (vbs.empty()) {
        return SkPath();
    }

    const auto info = SkPathPriv::AnalyzeVerbs(vbs);
    if (!info.valid || info.points > pts.size() || info.weights > ws.size()) {
        SkDEBUGFAIL("invalid verbs and number of points/weights");
        return SkPath();
    }

    return MakeInternal(info, pts.data(), vbs, ws.data(), ft, isVolatile);
}

SkPath SkPath::Rect(const SkRect& r, SkPathFillType ft, SkPathDirection dir, unsigned startIndex) {
    return SkPathBuilder(ft).addRect(r, dir, startIndex).detach();
}

SkPath SkPath::Oval(const SkRect& r, SkPathDirection dir) {
    return SkPathBuilder().addOval(r, dir).detach();
}

SkPath SkPath::Oval(const SkRect& r, SkPathDirection dir, unsigned startIndex) {
    return SkPathBuilder().addOval(r, dir, startIndex).detach();
}

SkPath SkPath::Circle(SkScalar x, SkScalar y, SkScalar r, SkPathDirection dir) {
    return SkPathBuilder().addCircle(x, y, r, dir).detach();
}

SkPath SkPath::RRect(const SkRRect& rr, SkPathDirection dir) {
    return SkPathBuilder().addRRect(rr, dir).detach();
}

SkPath SkPath::RRect(const SkRRect& rr, SkPathDirection dir, unsigned startIndex) {
    return SkPathBuilder().addRRect(rr, dir, startIndex).detach();
}

SkPath SkPath::RRect(const SkRect& r, SkScalar rx, SkScalar ry, SkPathDirection dir) {
    return SkPathBuilder().addRRect(SkRRect::MakeRectXY(r, rx, ry), dir).detach();
}

SkPath SkPath::Polygon(SkSpan<const SkPoint> pts, bool isClosed,
                       SkPathFillType ft, bool isVolatile) {
    return SkPathBuilder().addPolygon(pts, isClosed)
                          .setFillType(ft)
                          .setIsVolatile(isVolatile)
                          .detach();
}

SkPath SkPath::MakeInternal(const SkPathVerbAnalysis& analysis,
                            const SkPoint points[],
                            SkSpan<const SkPathVerb> verbs,
                            const SkScalar conics[],
                            SkPathFillType fillType,
                            bool isVolatile) {
  return SkPath(sk_sp<SkPathRef>(new SkPathRef(
                                     SkSpan(points, analysis.points),
                                     verbs,
                                     SkSpan(conics, analysis.weights),
                                     analysis.segmentMask,
                                     nullptr)),
                fillType, isVolatile, SkPathConvexity::kUnknown);
}

SkPath SkPath::makeTransform(const SkMatrix& matrix) const {
    SkPath dst;
    this->transform(matrix, &dst);
    return dst;
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
    if (auto raw = SkPathPriv::Raw(path)) {
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

    SkPath rotated = path.makeTransform(*inv);
    auto raw = SkPathPriv::Raw(rotated);
    if (!raw) {
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

int SkPathPriv::GenIDChangeListenersCount(const SkPath& path) {
    return path.fPathRef->genIDChangeListenerCount();
}

bool SkPathPriv::IsAxisAligned(const SkPath& path) {
    return IsAxisAligned(path.fPathRef->pointSpan());
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
    fMoveToPtr = fPts = raw.fPoints.begin();
    fVerbs = raw.fVerbs.begin();
    fVerbsStop = raw.fVerbs.end();
    fConicWeights = raw.fConics.begin();
    if (fConicWeights) {
        fConicWeights -= 1;  // begin one behind
    }

    fNeedsCloseLine = false;
    fNextIsNewContour = false;
    SkDEBUGCODE(fIsConic = false;)
}

SkPathEdgeIter::SkPathEdgeIter(const SkPath& path)
    : SkPathEdgeIter(SkPathPriv::Raw(path).value_or(SkPathRaw::Empty()))
{}
