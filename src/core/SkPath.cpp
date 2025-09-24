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

////////////////////////////////////////////////////////////////////////////

// flag to require a moveTo if we begin with something else, like lineTo etc.
// This will also be the value of lastMoveToIndex for a single contour
// ending with close, so countVerbs needs to be checked against 0.
#define INITIAL_LASTMOVETOINDEX_VALUE   ~0

SkPath::SkPath()
    : fPathRef(SkPathRef::CreateEmpty()) {
    this->resetFields();
    fIsVolatile = false;
}

SkPath::SkPath(sk_sp<SkPathRef> pr, SkPathFillType ft, bool isVolatile, SkPathConvexity ct)
    : fPathRef(std::move(pr))
    , fLastMoveToIndex(INITIAL_LASTMOVETOINDEX_VALUE)
    , fConvexity((uint8_t)ct)
    , fFillType((unsigned)ft)
    , fIsVolatile(isVolatile)
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
    out->reset();
    out->addPath(*this);
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

SkPath& SkPath::rewind() {
    SkDEBUGCODE(this->validate();)

    SkPathRef::Rewind(&fPathRef);
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

void SkPath::setBounds(const SkRect& rect) {
    SkPathRef::Editor ed(&fPathRef);
    ed.setBounds(rect);
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

// This is the public-facing non-const setConvexity().
void SkPath::setConvexity(SkPathConvexity c) {
    fConvexity.store((uint8_t)c, std::memory_order_relaxed);
}

// Const hooks for working with fConvexity and fFirstDirection from const methods.
void SkPath::setConvexity(SkPathConvexity c) const {
    fConvexity.store((uint8_t)c, std::memory_order_relaxed);
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

//////////////////////////////////////////////////////////////////////////////
//  Construction methods

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

void SkPath::incReserve(int extraPtCount, int extraVerbCount, int extraConicCount) {
    SkDEBUGCODE(this->validate();)
    if (extraPtCount > 0) {
        // For compat with when this function only took a single argument, use
        // extraPtCount if extraVerbCount is 0 (default value).
        SkPathRef::Editor(&fPathRef, extraVerbCount == 0 ? extraPtCount : extraVerbCount, extraPtCount, extraConicCount);
    }
    SkDEBUGCODE(this->validate();)
}

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

SkPathIter SkPath::iter() const {
    return { fPathRef->fPoints, fPathRef->verbs(), fPathRef->fConicWeights };
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

bool SkPath::hasOnlyMoveTos() const { return this->getSegmentMasks() == 0; }

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
        const uint8_t fillType = fFillType;
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

///////////////////////////////////////////////////////////////////////////////

void SkPath::offset(SkScalar dx, SkScalar dy, SkPath* dst) const {
    SkMatrix    matrix;

    matrix.setTranslate(dx, dy);
    this->transform(matrix, dst);
}

static void subdivide_cubic_to(SkPathBuilder* builder, const SkPoint pts[4],
                               int level = 2) {
    if (--level >= 0) {
        SkPoint tmp[7];

        SkChopCubicAtHalf(pts, tmp);
        subdivide_cubic_to(builder, &tmp[0], level);
        subdivide_cubic_to(builder, &tmp[3], level);
    } else {
        builder->cubicTo(pts[1], pts[2], pts[3]);
    }
}

void SkPath::transform(const SkMatrix& matrix, SkPath* dst) const {
    if (matrix.isIdentity()) {
        if (dst != nullptr && dst != this) {
            *dst = *this;
        }
        return;
    }

    SkDEBUGCODE(this->validate();)
    if (dst == nullptr) {
        dst = const_cast<SkPath*>(this);
    }

    if (matrix.hasPerspective()) {

        SkPath clipped;
        const SkPath* src = this;
        if (SkPathPriv::PerspectiveClip(*this, matrix, &clipped)) {
            src = &clipped;
        }

        SkPathBuilder tmp(this->getFillType());
        SkPath::Iter iter(*src, false);
        while (auto rec = iter.next()) {
            const SkSpan<const SkPoint> pts = rec->fPoints;
            switch (rec->fVerb) {
                case SkPathVerb::kMove:
                    tmp.moveTo(pts[0]);
                    break;
                case SkPathVerb::kLine:
                    tmp.lineTo(pts[1]);
                    break;
                case SkPathVerb::kQuad:
                    // promote the quad to a conic
                    tmp.conicTo(pts[1], pts[2],
                                SkConic::TransformW(pts.data(), SK_Scalar1, matrix));
                    break;
                case SkPathVerb::kConic:
                    tmp.conicTo(pts[1], pts[2],
                                SkConic::TransformW(pts.data(), rec->conicWeight(), matrix));
                    break;
                case SkPathVerb::kCubic:
                    subdivide_cubic_to(&tmp, pts.data());
                    break;
                case SkPathVerb::kClose:
                    tmp.close();
                    break;
            }
        }
        *dst = tmp.detach(&matrix);
    } else {
        SkPathConvexity convexity = this->getConvexityOrUnknown();

        SkPathRef::CreateTransformedCopy(&dst->fPathRef, *fPathRef, matrix);

        if (this != dst) {
            dst->fLastMoveToIndex = fLastMoveToIndex;
            dst->fFillType = fFillType;
            dst->fIsVolatile = fIsVolatile;
        }

        // Due to finite/fragile float numerics, we can't assume that a convex path remains
        // convex after a transformation, so mark it as unknown here.
        // However, some transformations are thought to be safe:
        //    axis-aligned values under scale/translate.
        //
        if (SkPathConvexity_IsConvex(convexity)) {
            if (!matrix.isScaleTranslate() || !SkPathPriv::IsAxisAligned(*this)) {
                // Not safe to still assume we're convex...
                convexity = SkPathConvexity::kUnknown;
            } else {
                SkScalar det2x2 =
                    matrix.get(SkMatrix::kMScaleX) * matrix.get(SkMatrix::kMScaleY) -
                    matrix.get(SkMatrix::kMSkewX)  * matrix.get(SkMatrix::kMSkewY);
                if (det2x2 < 0) {
                    convexity = SkPathConvexity_OppositeConvexDirection(convexity);
                } else if (det2x2 > 0) {
                    // we keep our direction
                } else /* det2x == 0 */ {
                    convexity = SkPathConvexity::kConvex_Degenerate;
                }
            }
        }
        dst->setConvexity(convexity);

        SkDEBUGCODE(dst->validate();)
    }
}

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

static void append_params(SkString* str, const char label[], const SkPoint pts[],
                          int count, SkScalarAsStringType strType, SkScalar conicWeight = -12345) {
    str->append(label);
    str->append("(");

    const SkScalar* values = &pts[0].fX;
    count *= 2;

    for (int i = 0; i < count; ++i) {
        SkAppendScalar(str, values[i], strType);
        if (i < count - 1) {
            str->append(", ");
        }
    }
    if (conicWeight != -12345) {
        str->append(", ");
        SkAppendScalar(str, conicWeight, strType);
    }
    str->append(");");
    if (kHex_SkScalarAsStringType == strType) {
        str->append("  // ");
        for (int i = 0; i < count; ++i) {
            SkAppendScalarDec(str, values[i]);
            if (i < count - 1) {
                str->append(", ");
            }
        }
        if (conicWeight >= 0) {
            str->append(", ");
            SkAppendScalarDec(str, conicWeight);
        }
    }
    str->append("\n");
}

void SkPath::dump(SkWStream* wStream, bool dumpAsHex) const {
    SkScalarAsStringType asType = dumpAsHex ? kHex_SkScalarAsStringType : kDec_SkScalarAsStringType;

    SkString builder;
    char const * const gFillTypeStrs[] = {
        "Winding",
        "EvenOdd",
        "InverseWinding",
        "InverseEvenOdd",
    };
    builder.printf("path.setFillType(SkPathFillType::k%s);\n",
            gFillTypeStrs[(int) this->getFillType()]);

    Iter iter(*this, false);
    while (auto rec = iter.next()) {
        switch (rec->fVerb) {
            case SkPathVerb::kMove:
                append_params(&builder, "path.moveTo", &rec->fPoints[0], 1, asType);
                break;
            case SkPathVerb::kLine:
                append_params(&builder, "path.lineTo", &rec->fPoints[1], 1, asType);
                break;
            case SkPathVerb::kQuad:
                append_params(&builder, "path.quadTo", &rec->fPoints[1], 2, asType);
                break;
            case SkPathVerb::kConic:
                append_params(&builder, "path.conicTo", &rec->fPoints[1], 2, asType,
                              rec->conicWeight());
                break;
            case SkPathVerb::kCubic:
                append_params(&builder, "path.cubicTo", &rec->fPoints[1], 3, asType);
                break;
            case SkPathVerb::kClose:
                builder.append("path.close();\n");
                break;
        }
        if (!wStream && builder.size()) {
            SkDebugf("%s", builder.c_str());
            builder.reset();
        }
    }
    if (wStream) {
        wStream->writeText(builder.c_str());
    }
}

void SkPath::dumpArrays(SkWStream* wStream, bool dumpAsHex) const {
    SkString builder;

    auto bool_str = [](bool v) { return v ? "true" : "false"; };

    builder.appendf("// fBoundsIsDirty = %s\n", bool_str(fPathRef->fBoundsIsDirty));
    builder.appendf("// fGenerationID = %u\n", fPathRef->fGenerationID);
    builder.appendf("// fSegmentMask = %d\n", fPathRef->fSegmentMask);

    const char* gTypeStrs[] = {
        "General", "Oval", "RRect",
    };
    builder.appendf("// fType = %s\n", gTypeStrs[static_cast<int>(fPathRef->fType)]);

    auto append_scalar = [&](SkScalar v) {
        if (dumpAsHex) {
            builder.appendf("SkBits2Float(0x%08X) /* %g */", SkFloat2Bits(v), v);
        } else {
            builder.appendf("%g", v);
        }
    };

    builder.append("const SkPoint path_points[] = {\n");
    for (int i = 0; i < this->countPoints(); ++i) {
        SkPoint p = this->getPoint(i);
        builder.append("    { ");
        append_scalar(p.fX);
        builder.append(", ");
        append_scalar(p.fY);
        builder.append(" },\n");
    }
    builder.append("};\n");

    const char* gVerbStrs[] = {
        "Move", "Line", "Quad", "Conic", "Cubic", "Close"
    };
    builder.append("const uint8_t path_verbs[] = {\n    ");
    for (auto v = fPathRef->verbsBegin(); v != fPathRef->verbsEnd(); ++v) {
        builder.appendf("(uint8_t)SkPathVerb::k%s, ", gVerbStrs[(unsigned)*v]);
    }
    builder.append("\n};\n");

    const int nConics = fPathRef->conicWeightsEnd() - fPathRef->conicWeights();
    if (nConics) {
        builder.append("const SkScalar path_conics[] = {\n    ");
        for (auto c = fPathRef->conicWeights(); c != fPathRef->conicWeightsEnd(); ++c) {
            append_scalar(*c);
            builder.append(", ");
        }
        builder.append("\n};\n");
    }

    char const * const gFillTypeStrs[] = {
        "Winding",
        "EvenOdd",
        "InverseWinding",
        "InverseEvenOdd",
    };

    builder.appendf("SkPath path = SkPath::Make(path_points, %d, path_verbs, %d, %s, %d,\n",
                    this->countPoints(), this->countVerbs(),
                    nConics ? "path_conics" : "nullptr", nConics);
    builder.appendf("                           SkPathFillType::k%s, %s);\n",
                    gFillTypeStrs[(int)this->getFillType()],
                    bool_str(fIsVolatile));

    if (wStream) {
        wStream->writeText(builder.c_str());
    } else {
        SkDebugf("%s\n", builder.c_str());
    }
}

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

static int sign(SkScalar x) { return x < 0; }
#define kValueNeverReturnedBySign   2

enum DirChange {
    kUnknown_DirChange,
    kLeft_DirChange,
    kRight_DirChange,
    kStraight_DirChange,
    kBackwards_DirChange, // if double back, allow simple lines to be convex
    kInvalid_DirChange
};

// only valid for a single contour
struct Convexicator {

    /** The direction returned is only valid if the path is determined convex */
    SkPathFirstDirection getFirstDirection() const { return fFirstDirection; }

    void setMovePt(const SkPoint& pt) {
        fFirstPt = fLastPt = pt;
        fExpectedDir = kInvalid_DirChange;
    }

    bool addPt(const SkPoint& pt) {
        if (fLastPt == pt) {
            return true;
        }
        // should only be true for first non-zero vector after setMovePt was called. It is possible
        // we doubled backed at the start so need to check if fLastVec is zero or not.
        if (fFirstPt == fLastPt && fExpectedDir == kInvalid_DirChange && fLastVec.equals(0,0)) {
            fLastVec = pt - fLastPt;
            fFirstVec = fLastVec;
        } else if (!this->addVec(pt - fLastPt)) {
            return false;
        }
        fLastPt = pt;
        return true;
    }

    static bool IsConcaveBySign(const SkPoint points[], int count) {
        if (count <= 3) {
            // point, line, or triangle are always convex
            return false;
        }

        const SkPoint* last = points + count;
        SkPoint currPt = *points++;
        SkPoint firstPt = currPt;
        int dxes = 0;
        int dyes = 0;
        int lastSx = kValueNeverReturnedBySign;
        int lastSy = kValueNeverReturnedBySign;
        for (int outerLoop = 0; outerLoop < 2; ++outerLoop ) {
            while (points != last) {
                SkVector vec = *points - currPt;
                if (!vec.isZero()) {
                    // give up if vector construction failed
                    if (!vec.isFinite()) {
                        return true;    // treat as concave
                    }
                    int sx = sign(vec.fX);
                    int sy = sign(vec.fY);
                    dxes += (sx != lastSx);
                    dyes += (sy != lastSy);
                    if (dxes > 3 || dyes > 3) {
                        return true;
                    }
                    lastSx = sx;
                    lastSy = sy;
                }
                currPt = *points++;
                if (outerLoop) {
                    break;
                }
            }
            points = &firstPt;
        }
        return false;  // that is, it may be convex, don't know yet
    }

    bool close() {
        // If this was an explicit close, there was already a lineTo to fFirstPoint, so this
        // addPt() is a no-op. Otherwise, the addPt implicitly closes the contour. In either case,
        // we have to check the direction change along the first vector in case it is concave.
        return this->addPt(fFirstPt) && this->addVec(fFirstVec);
    }

    bool isFinite() const {
        return fIsFinite;
    }

    int reversals() const {
        return fReversals;
    }

private:
    DirChange directionChange(const SkVector& curVec) {
        SkScalar cross = SkPoint::CrossProduct(fLastVec, curVec);
        if (!SkIsFinite(cross)) {
            return kUnknown_DirChange;
        }
        if (cross == 0) {
            return fLastVec.dot(curVec) < 0 ? kBackwards_DirChange : kStraight_DirChange;
        }
        return 1 == SkScalarSignAsInt(cross) ? kRight_DirChange : kLeft_DirChange;
    }

    bool addVec(const SkVector& curVec) {
        DirChange dir = this->directionChange(curVec);
        switch (dir) {
            case kLeft_DirChange:       // fall through
            case kRight_DirChange:
                if (kInvalid_DirChange == fExpectedDir) {
                    fExpectedDir = dir;
                    fFirstDirection = (kRight_DirChange == dir) ? SkPathFirstDirection::kCW
                                                                : SkPathFirstDirection::kCCW;
                } else if (dir != fExpectedDir) {
                    fFirstDirection = SkPathFirstDirection::kUnknown;
                    return false;
                }
                fLastVec = curVec;
                break;
            case kStraight_DirChange:
                break;
            case kBackwards_DirChange:
                //  allow path to reverse direction twice
                //    Given path.moveTo(0, 0); path.lineTo(1, 1);
                //    - 1st reversal: direction change formed by line (0,0 1,1), line (1,1 0,0)
                //    - 2nd reversal: direction change formed by line (1,1 0,0), line (0,0 1,1)
                fLastVec = curVec;
                return ++fReversals < 3;
            case kUnknown_DirChange:
                return (fIsFinite = false);
            case kInvalid_DirChange:
                SK_ABORT("Use of invalid direction change flag");
                break;
        }
        return true;
    }

    SkPoint              fFirstPt {0, 0};  // The first point of the contour, e.g. moveTo(x,y)
    SkVector             fFirstVec {0, 0}; // The direction leaving fFirstPt to the next vertex

    SkPoint              fLastPt {0, 0};   // The last point passed to addPt()
    SkVector             fLastVec {0, 0};  // The direction that brought the path to fLastPt

    DirChange            fExpectedDir { kInvalid_DirChange };
    SkPathFirstDirection fFirstDirection { SkPathFirstDirection::kUnknown };
    int                  fReversals { 0 };
    bool                 fIsFinite { true };
};

static void trim_trailing_moves(SkSpan<const SkPoint>& pts, SkSpan<const SkPathVerb>& vbs) {
    size_t vbCount = vbs.size();
    while (vbCount > 0 && vbs[vbCount - 1] == SkPathVerb::kMove) {
        vbCount -= 1;
    }
    if (size_t delta = vbs.size() - vbCount) {
        SkASSERT(pts.size() >= delta);
        pts = {pts.data(), pts.size() - delta};
        vbs = {vbs.data(), vbs.size() - delta};
    }
}

SkPathConvexity SkPathPriv::ComputeConvexity(SkSpan<const SkPoint> points,
                                             SkSpan<const SkPathVerb> vbs,
                                             SkSpan<const float> conicWeights) {
    // callers need to give us finite values
    SkASSERT(SkRect::Bounds(points).has_value());

    trim_trailing_moves(points, vbs);

    if (vbs.empty()) {
        return SkPathConvexity::kConvex_Degenerate;
    }

    // Check to see if path changes direction more than three times as quick concave test
    if (Convexicator::IsConcaveBySign(points.data(), points.size())) {
        return SkPathConvexity::kConcave;
    }

    int contourCount = 0;
    bool needsClose = false;
    Convexicator state;

    auto iter = SkPathIter(points, vbs, conicWeights);
    while (auto rec = iter.next()) {
        auto verb = rec->fVerb;
        auto pts = rec->fPoints;

        // Looking for the last moveTo before non-move verbs start
        if (contourCount == 0) {
            if (verb == SkPathVerb::kMove) {
                state.setMovePt(pts[0]);
            } else {
                // Starting the actual contour, fall through to c=1 to add the points
                contourCount++;
                needsClose = true;
            }
        }
        // Accumulating points into the Convexicator until we hit a close or another move
        if (contourCount == 1) {
            if (verb == SkPathVerb::kClose || verb == SkPathVerb::kMove) {
                if (!state.close()) {
                    return SkPathConvexity::kConcave;
                }
                needsClose = false;
                contourCount++;
            } else {
                // lines add 1 point, cubics add 3, conics and quads add 2
                int count = SkPathPriv::PtsInVerb((unsigned) verb);
                SkASSERT(count > 0);
                for (int i = 1; i <= count; ++i) {
                    if (!state.addPt(pts[i])) {
                        return SkPathConvexity::kConcave;
                    }
                }
            }
        } else {
            // The first contour has closed and anything other than spurious trailing moves means
            // there's multiple contours and the path can't be convex
            if (verb != SkPathVerb::kMove) {
                return SkPathConvexity::kConcave;
            }
        }
    }

    // If the path isn't explicitly closed do so implicitly
    if (needsClose && !state.close()) {
        return SkPathConvexity::kConcave;
    }

    const auto firstDir = state.getFirstDirection();
    if (firstDir == SkPathFirstDirection::kUnknown && state.reversals() >= 3) {
        return SkPathConvexity::kConcave;
    }
    return SkPathFirstDirection_ToConvexity(firstDir);
}


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

class ContourIter {
public:
    ContourIter(SkSpan<const SkPoint>, SkSpan<const SkPathVerb>, SkSpan<const float> conicWeights);

    bool done() const { return fDone; }
    // if !done() then these may be called
    int count() const { return fCurrPtCount; }
    const SkPoint* pts() const { return fCurrPt; }
    void next();

private:
    int fCurrPtCount;
    const SkPoint* fCurrPt;
    const SkPathVerb* fCurrVerb;
    const SkPathVerb* fStopVerbs;
    const SkScalar* fCurrConicWeight;
    bool fDone;
    SkDEBUGCODE(int fContourCounter;)
};

ContourIter::ContourIter(SkSpan<const SkPoint> pts, SkSpan<const SkPathVerb> vbs,
                         SkSpan<const float> conicWeights) {
    fStopVerbs = vbs.end();
    fDone = false;
    fCurrPt = pts.data();
    fCurrVerb = vbs.data();
    fCurrConicWeight = conicWeights.data();
    fCurrPtCount = 0;
    SkDEBUGCODE(fContourCounter = 0;)
    this->next();
}

void ContourIter::next() {
    if (fCurrVerb >= fStopVerbs) {
        fDone = true;
    }
    if (fDone) {
        return;
    }

    // skip pts of prev contour
    fCurrPt += fCurrPtCount;

    SkASSERT(SkPathVerb::kMove == fCurrVerb[0]);
    int ptCount = 1;    // moveTo
    const SkPathVerb* verbs = fCurrVerb;

    for (verbs++; verbs < fStopVerbs; verbs++) {
        switch (*verbs) {
            case SkPathVerb::kMove:
                goto CONTOUR_END;
            case SkPathVerb::kLine:
                ptCount += 1;
                break;
            case SkPathVerb::kConic:
                fCurrConicWeight += 1;
                [[fallthrough]];
            case SkPathVerb::kQuad:
                ptCount += 2;
                break;
            case SkPathVerb::kCubic:
                ptCount += 3;
                break;
            case SkPathVerb::kClose:
                break;
        }
    }
CONTOUR_END:
    fCurrPtCount = ptCount;
    fCurrVerb = verbs;
    SkDEBUGCODE(++fContourCounter;)
}

// returns cross product of (p1 - p0) and (p2 - p0)
static SkScalar cross_prod(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2) {
    SkScalar cross = SkPoint::CrossProduct(p1 - p0, p2 - p0);
    // We may get 0 when the above subtracts underflow. We expect this to be
    // very rare and lazily promote to double.
    if (0 == cross) {
        double p0x = SkScalarToDouble(p0.fX);
        double p0y = SkScalarToDouble(p0.fY);

        double p1x = SkScalarToDouble(p1.fX);
        double p1y = SkScalarToDouble(p1.fY);

        double p2x = SkScalarToDouble(p2.fX);
        double p2y = SkScalarToDouble(p2.fY);

        cross = SkDoubleToScalar((p1x - p0x) * (p2y - p0y) -
                                 (p1y - p0y) * (p2x - p0x));

    }
    return cross;
}

// Returns the first pt with the maximum Y coordinate
static int find_max_y(const SkPoint pts[], int count) {
    SkASSERT(count > 0);
    SkScalar max = pts[0].fY;
    int firstIndex = 0;
    for (int i = 1; i < count; ++i) {
        SkScalar y = pts[i].fY;
        if (y > max) {
            max = y;
            firstIndex = i;
        }
    }
    return firstIndex;
}

static int find_diff_pt(const SkPoint pts[], int index, int n, int inc) {
    int i = index;
    for (;;) {
        i = (i + inc) % n;
        if (i == index) {   // we wrapped around, so abort
            break;
        }
        if (pts[index] != pts[i]) { // found a different point, success!
            break;
        }
    }
    return i;
}

/**
 *  Starting at index, and moving forward (incrementing), find the xmin and
 *  xmax of the contiguous points that have the same Y.
 */
static int find_min_max_x_at_y(const SkPoint pts[], int index, int n,
                               int* maxIndexPtr) {
    const SkScalar y = pts[index].fY;
    SkScalar min = pts[index].fX;
    SkScalar max = min;
    int minIndex = index;
    int maxIndex = index;
    for (int i = index + 1; i < n; ++i) {
        if (pts[i].fY != y) {
            break;
        }
        SkScalar x = pts[i].fX;
        if (x < min) {
            min = x;
            minIndex = i;
        } else if (x > max) {
            max = x;
            maxIndex = i;
        }
    }
    *maxIndexPtr = maxIndex;
    return minIndex;
}

static SkPathFirstDirection crossToDir(SkScalar cross) {
    return cross > 0 ? SkPathFirstDirection::kCW : SkPathFirstDirection::kCCW;
}

/*
 *  We loop through all contours, and keep the computed cross-product of the
 *  contour that contained the global y-max. If we just look at the first
 *  contour, we may find one that is wound the opposite way (correctly) since
 *  it is the interior of a hole (e.g. 'o'). Thus we must find the contour
 *  that is outer most (or at least has the global y-max) before we can consider
 *  its cross product.
 */
SkPathFirstDirection SkPathPriv::ComputeFirstDirection(const SkPathRaw& raw) {
    ContourIter iter(raw.points(), raw.verbs(), raw.conics());

    // initialize with our logical y-min
    SkScalar ymax = raw.bounds().fTop;
    SkScalar ymaxCross = 0;

    for (; !iter.done(); iter.next()) {
        int n = iter.count();
        if (n < 3) {
            continue;
        }

        const SkPoint* pts = iter.pts();
        SkScalar cross = 0;
        int index = find_max_y(pts, n);
        if (pts[index].fY < ymax) {
            continue;
        }

        // If there is more than 1 distinct point at the y-max, we take the
        // x-min and x-max of them and just subtract to compute the dir.
        if (pts[(index + 1) % n].fY == pts[index].fY) {
            int maxIndex;
            int minIndex = find_min_max_x_at_y(pts, index, n, &maxIndex);
            if (minIndex == maxIndex) {
                goto TRY_CROSSPROD;
            }
            SkASSERT(pts[minIndex].fY == pts[index].fY);
            SkASSERT(pts[maxIndex].fY == pts[index].fY);
            SkASSERT(pts[minIndex].fX <= pts[maxIndex].fX);
            // we just subtract the indices, and let that auto-convert to
            // SkScalar, since we just want - or + to signal the direction.
            cross = minIndex - maxIndex;
        } else {
            TRY_CROSSPROD:
            // Find a next and prev index to use for the cross-product test,
            // but we try to find pts that form non-zero vectors from pts[index]
            //
            // Its possible that we can't find two non-degenerate vectors, so
            // we have to guard our search (e.g. all the pts could be in the
            // same place).

            // we pass n - 1 instead of -1 so we don't foul up % operator by
            // passing it a negative LH argument.
            int prev = find_diff_pt(pts, index, n, n - 1);
            if (prev == index) {
                // completely degenerate, skip to next contour
                continue;
            }
            int next = find_diff_pt(pts, index, n, 1);
            SkASSERT(next != index);
            cross = cross_prod(pts[prev], pts[index], pts[next]);
            // if we get a zero and the points are horizontal, then we look at the spread in
            // x-direction. We really should continue to walk away from the degeneracy until
            // there is a divergence.
            if (0 == cross && pts[prev].fY == pts[index].fY && pts[next].fY == pts[index].fY) {
                // construct the subtract so we get the correct Direction below
                cross = pts[index].fX - pts[next].fX;
            }
        }

        if (cross) {
            // record our best guess so far
            ymax = pts[index].fY;
            ymaxCross = cross;
        }
    }

    return ymaxCross ? crossToDir(ymaxCross) : SkPathFirstDirection::kUnknown;
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
    return ComputeFirstDirection(SkPathPriv::Raw(path));
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

// Sort of like makeSpace(0) but the the additional requirement that we actively shrink the
// allocations to just fit the current needs. makeSpace() will only grow, but never shrinks.
//
void SkPath::shrinkToFit() {
    // Since this can relocate the allocated arrays, we have to defensively copy ourselves if
    // we're not the only owner of the pathref... since relocating the arrays will invalidate
    // any existing iterators.
    if (!fPathRef->unique()) {
        SkPathRef* pr = new SkPathRef;
        pr->copy(*fPathRef, 0, 0, 0);
        fPathRef.reset(pr);
    }
    fPathRef->fPoints.shrink_to_fit();
    fPathRef->fVerbs.shrink_to_fit();
    fPathRef->fConicWeights.shrink_to_fit();
    SkDEBUGCODE(fPathRef->validate();)
}


int SkPath::ConvertConicToQuads(const SkPoint& p0, const SkPoint& p1, const SkPoint& p2,
                                SkScalar w, SkPoint pts[], int pow2) {
    const SkConic conic(p0, p1, p2, w);
    return conic.chopIntoQuadsPOW2(pts, pow2);
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

bool SkPathPriv::DrawArcIsConvex(SkScalar sweepAngle,
                                 SkArc::Type arcType,
                                 bool isFillNoPathEffect) {
    if (isFillNoPathEffect && SkScalarAbs(sweepAngle) >= 360.f) {
        // This gets converted to an oval.
        return true;
    }
    if (arcType == SkArc::Type::kWedge) {
        // This is a pie wedge. It's convex if the angle is <= 180.
        return SkScalarAbs(sweepAngle) <= 180.f;
    }
    // When the angle exceeds 360 this wraps back on top of itself. Otherwise it is a circle clipped
    // to a secant, i.e. convex.
    return SkScalarAbs(sweepAngle) <= 360.f;
}

SkPath SkPathPriv::CreateDrawArcPath(const SkArc& arc, bool isFillNoPathEffect) {
    SkRect oval = arc.fOval;
    SkScalar startAngle = arc.fStartAngle, sweepAngle = arc.fSweepAngle;
    SkASSERT(!oval.isEmpty());
    SkASSERT(sweepAngle);
    // We cap the number of total rotations. This keeps the resulting paths simpler. More important,
    // it prevents values so large that the loops below never terminate (once ULP > 360).
    if (SkScalarAbs(sweepAngle) > 3600.0f) {
        sweepAngle = std::copysign(3600.0f, sweepAngle) + std::fmod(sweepAngle, 360.0f);
    }

    SkPathBuilder builder(SkPathFillType::kWinding);
    builder.setIsVolatile(true);

    if (isFillNoPathEffect && SkScalarAbs(sweepAngle) >= 360.f) {
        builder.addOval(oval);
        SkASSERT(DrawArcIsConvex(sweepAngle, SkArc::Type::kArc, isFillNoPathEffect));
        return builder.detach();
    }

    if (arc.isWedge()) {
        builder.moveTo(oval.centerX(), oval.centerY());
    }
    auto firstDir =
            sweepAngle > 0 ? SkPathFirstDirection::kCW : SkPathFirstDirection::kCCW;
    bool convex = DrawArcIsConvex(sweepAngle, arc.fType, isFillNoPathEffect);
    // Arc to mods at 360 and drawArc is not supposed to.
    bool forceMoveTo = !arc.isWedge();
    while (sweepAngle <= -360.f) {
        builder.arcTo(oval, startAngle, -180.f, forceMoveTo);
        startAngle -= 180.f;
        builder.arcTo(oval, startAngle, -180.f, false);
        startAngle -= 180.f;
        forceMoveTo = false;
        sweepAngle += 360.f;
    }
    while (sweepAngle >= 360.f) {
        builder.arcTo(oval, startAngle, 180.f, forceMoveTo);
        startAngle += 180.f;
        builder.arcTo(oval, startAngle, 180.f, false);
        startAngle += 180.f;
        forceMoveTo = false;
        sweepAngle -= 360.f;
    }
    builder.arcTo(oval, startAngle, sweepAngle, forceMoveTo);
    if (arc.isWedge()) {
        builder.close();
    }

    auto path = builder.detach();
    const auto convexity = convex ? SkPathFirstDirection_ToConvexity(firstDir)
                                  : SkPathConvexity::kConcave;
    path.setConvexity(convexity);
    return path;
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

SkPathVerbAnalysis SkPathPriv::AnalyzeVerbs(SkSpan<const SkPathVerb> vbs) {
    SkPathVerbAnalysis info = {false, 0, 0, 0};
    bool needMove = true;
    bool invalid = false;

    if (vbs.size() >= (INT_MAX / 3)) SK_UNLIKELY {
        // A path with an extremely high number of quad, conic or cubic verbs could cause
        // `info.points` to overflow. To prevent against this, we reject extremely large paths. This
        // check is conservative and assumes the worst case (in particular, it assumes that every
        // verb consumes 3 points, which would only happen for a path composed entirely of cubics).
        // This limits us to 700 million verbs, which is large enough for any reasonable use case.
        invalid = true;
    } else {
        for (auto v : vbs) {
            switch (v) {
                case SkPathVerb::kMove:
                    needMove = false;
                    info.points += 1;
                    break;
                case SkPathVerb::kLine:
                    invalid |= needMove;
                    info.segmentMask |= kLine_SkPathSegmentMask;
                    info.points += 1;
                    break;
                case SkPathVerb::kQuad:
                    invalid |= needMove;
                    info.segmentMask |= kQuad_SkPathSegmentMask;
                    info.points += 2;
                    break;
                case SkPathVerb::kConic:
                    invalid |= needMove;
                    info.segmentMask |= kConic_SkPathSegmentMask;
                    info.points += 2;
                    info.weights += 1;
                    break;
                case SkPathVerb::kCubic:
                    invalid |= needMove;
                    info.segmentMask |= kCubic_SkPathSegmentMask;
                    info.points += 3;
                    break;
                case SkPathVerb::kClose:
                    invalid |= needMove;
                    needMove = true;
                    break;
                default:
                    invalid = true;
                    break;
            }
        }
    }
    info.valid = !invalid;
    return info;
}

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

SkPath SkPath::Rect(const SkRect& r, SkPathDirection dir, unsigned startIndex) {
    return SkPathBuilder().addRect(r, dir, startIndex).detach();
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
    if (!rotated.isFinite()) {
        return {};
    }

    SkScalar big = SK_ScalarMax;
    SkRect clip = {-big, 0, big, big };

    struct Rec {
        SkPathBuilder fResult;
        SkPoint       fPrev = {0,0};
    } rec;

    SkEdgeClipper::ClipPath(SkPathPriv::Raw(rotated), clip, false,
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

bool SkPathPriv::IsAxisAligned(SkSpan<const SkPoint> pts) {
    // Conservative (quick) test to see if all segments are axis-aligned.
    // Multiple contours might give a false-negative, but for speed, we ignore that
    // and just look at the raw points.

    for (size_t i = 1; i < pts.size(); ++i) {
        if (pts[i-1].fX != pts[i].fX && pts[i-1].fY != pts[i].fY) {
            return false;
        }
    }
    return true;
}

bool SkPathPriv::IsAxisAligned(const SkPath& path) {
    return IsAxisAligned(path.fPathRef->pointSpan());
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

SkPathEdgeIter::SkPathEdgeIter(const SkPath& path) : SkPathEdgeIter(SkPathPriv::Raw(path)) {}
