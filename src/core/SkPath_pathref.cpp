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
#include "include/private/SkPathRef.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits.h>
#include <utility>

#ifndef SK_PATH_USES_PATHDATA

/*  Contains path methods that require the legacy fields:
 *  - fPathRef
 *  - fConvexity
 *  - fLastMoveToIndex
 *
 *  ... these are encompaed by SkPathData
 *
 *  The remaining fields:
 *  - fFillType
 *  - fIsVolatile
 *
 *  ... are shared in both implemtations.
 */

// flag to require a moveTo if we begin with something else, like lineTo etc.
// This will also be the value of lastMoveToIndex for a single contour
// ending with close, so countVerbs needs to be checked against 0.
#define INITIAL_LASTMOVETOINDEX_VALUE   ~0

SkPath::SkPath(sk_sp<SkPathRef> pr, SkPathFillType ft, bool isVolatile, SkPathConvexity ct)
    : fPathRef(std::move(pr))
    , fLastMoveToIndex(INITIAL_LASTMOVETOINDEX_VALUE)
    , fConvexity((uint8_t)ct)
    , fFillType(ft)
    , fIsVolatile(isVolatile)
{}

SkPath::SkPath(SkPathFillType ft)
    : fPathRef(SkPathRef::CreateEmpty())
    , fLastMoveToIndex(INITIAL_LASTMOVETOINDEX_VALUE)
    , fConvexity((uint8_t)SkPathConvexity::kUnknown)
    , fFillType(ft)
    , fIsVolatile(false)
{}

void SkPath::resetFields() {
    //fPathRef is assumed to have been emptied by the caller.
    fLastMoveToIndex = INITIAL_LASTMOVETOINDEX_VALUE;
    fFillType = SkPathFillType::kDefault;
    this->setConvexity(SkPathConvexity::kUnknown);
}

SkPath::SkPath(const SkPath& that)
    : fPathRef(SkRef(that.fPathRef.get())) {
    this->copyFields(that);
    SkDEBUGCODE(that.validate();)
}

void SkPath::setConvexity(SkPathConvexity c) const {
    fConvexity.store((uint8_t)c, std::memory_order_relaxed);
}

SkPathConvexity SkPath::getConvexityOrUnknown() const {
    return (SkPathConvexity)fConvexity.load(std::memory_order_relaxed);
}

void SkPath::copyFields(const SkPath& that) {
    //fPathRef is assumed to have been set by the caller.
    fLastMoveToIndex = that.fLastMoveToIndex;
    fFillType        = that.fFillType;
    fIsVolatile      = that.fIsVolatile;

    // Non-atomic assignment of atomic values.
    this->setConvexity(that.getConvexityOrUnknown());
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
        std::swap(fFillType, that.fFillType);
        std::swap(fIsVolatile, that.fIsVolatile);

        // Non-atomic swaps of atomic values.
        SkPathConvexity c = this->getConvexityOrUnknown();
        this->setConvexity(that.getConvexityOrUnknown());
        that.setConvexity(c);
    }
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

bool SkPath::isFinite() const {
    SkDEBUGCODE(this->validate();)
    return fPathRef->isFinite();
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

#ifdef SK_DEBUG
void SkPath::validate() const {
    SkASSERT(this->isValidImpl());
}

void SkPath::validateRef() const {
    // This will SkASSERT if not valid.
    fPathRef->validate();
}
#endif

std::optional<SkPathOvalInfo> SkPath::getOvalInfo() const { return fPathRef->isOval(); }
std::optional<SkPathRRectInfo> SkPath::getRRectInfo() const { return fPathRef->isRRect(); }

SkSpan<const SkPoint> SkPath::points() const {
    return fPathRef->pointSpan();
}
SkSpan<const SkPathVerb> SkPath::verbs() const {
    return fPathRef->verbs();
}
SkSpan<const float> SkPath::conicWeights() const {
    return fPathRef->conicSpan();
}

///////////////////////////////////////////////////////////////////////////////

bool SkPath::isValidImpl() const {
    if ((static_cast<int>(fFillType) & ~3) != 0) {
        return false;
    }

#ifdef SK_DEBUG_PATH
    if (!fBoundsIsDirty) {
        SkRect bounds;

        bool isFinite = compute_pt_bounds(&bounds, *fPathRef.get());
        if (SkToBool(fIsFinite) != isFinite) {
            return false;
        }

        if (this->countPoints() <= 1) {
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

SkPath SkPath::Oval(const SkRect& r, SkPathDirection dir, unsigned startIndex) {
    return SkPathBuilder().addOval(r, dir, startIndex).detach();
}

SkPath SkPath::RRect(const SkRRect& rr, SkPathDirection dir, unsigned startIndex) {
    return SkPathBuilder().addRRect(rr, dir, startIndex).detach();
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

        dst->setConvexity(SkPathPriv::TransformConvexity(matrix, fPathRef->pointSpan(), convexity));

        SkDEBUGCODE(dst->validate();)
    }
}

// TODO: evolve this one to the source of truth (when we have SkPathData),
//       and have makeTransform() call it and mark the non-finite flag if it fails.
std::optional<SkPath> SkPath::tryMakeTransform(const SkMatrix& matrix) const {
    auto path = this->makeTransform(matrix);
    if (path.isFinite()) {
        return path;
    }
    return {};
}

std::optional<SkPathRaw> SkPath::raw(SkResolveConvexity rc) const {
    const SkPathRef* ref = fPathRef.get();
    SkASSERT(ref);
    if (!ref->isFinite()) {
        return {};
    }

    return SkPathRaw{
        ref->pointSpan(),
        ref->verbs(),
        ref->conicSpan(),
        ref->getBounds(),
        this->getFillType(),
        rc == SkResolveConvexity::kYes ? this->getConvexity() : this->getConvexityOrUnknown(),
        SkTo<uint8_t>(ref->getSegmentMasks()),
    };
}

int SkPathPriv::GenIDChangeListenersCount(const SkPath& path) {
    return path.fPathRef->genIDChangeListenerCount();
}

bool SkPathPriv::TestingOnly_unique(const SkPath& path) {
    return path.fPathRef->unique();
}

void SkPathPriv::AddGenIDChangeListener(const SkPath& path, sk_sp<SkIDChangeListener> listener) {
    path.fPathRef->addGenIDChangeListener(std::move(listener));
}

/////////////////////////////////////////////////////////////////////////////////////

SkPathBuilder& SkPathBuilder::operator=(const SkPath& src) {
    this->reset().setFillType(src.getFillType());
    this->setIsVolatile(src.isVolatile());

    const sk_sp<SkPathRef>& ref = src.fPathRef;
    fVerbs        = ref->fVerbs;
    fPts          = ref->fPoints;
    fConicWeights = ref->fConicWeights;

    fSegmentMask   = ref->fSegmentMask;
    fLastMoveIndex = src.fLastMoveToIndex < 0 ? ~src.fLastMoveToIndex : src.fLastMoveToIndex;

    fType = ref->fType;
    fIsA  = ref->fIsA;

    fConvexity = src.getConvexityOrUnknown();

    return *this;
}

SkPath SkPathBuilder::make(sk_sp<SkPathRef> pr) const {
    switch (fType) {
        case SkPathIsAType::kGeneral:
            break;
        case SkPathIsAType::kOval:
            pr->setIsOval(fIsA.fDirection, fIsA.fStartIndex);
            SkASSERT(SkPathConvexity_IsConvex(fConvexity));
            break;
        case SkPathIsAType::kRRect:
            pr->setIsRRect(fIsA.fDirection, fIsA.fStartIndex);
            SkASSERT(SkPathConvexity_IsConvex(fConvexity));
            break;
    }

    // Wonder if we can combine convexity and dir internally...
    //  unknown, convex_cw, convex_ccw, concave
    // Do we ever have direction w/o convexity, or viceversa (inside path)?
    //
    auto path = SkPath(std::move(pr), fFillType, fIsVolatile, fConvexity);

    // This hopefully can go away in the future when Paths are immutable,
    // but if while they are still editable, we need to correctly set this.
    SkSpan<const SkPathVerb> verbs = path.fPathRef->verbs();
    if (!verbs.empty()) {
        SkASSERT(fLastMoveIndex >= 0);
        // peek at the last verb, to know if our last contour is closed
        const bool isClosed = (verbs.back() == SkPathVerb::kClose);
        path.fLastMoveToIndex = isClosed ? ~fLastMoveIndex : fLastMoveIndex;
    }

    return path;
}

SkPath SkPathBuilder::snapshot(const SkMatrix* mx) const {
    return this->make(sk_sp<SkPathRef>(new SkPathRef(fPts,
                                                     fVerbs,
                                                     fConicWeights,
                                                     fSegmentMask,
                                                     mx)));
}

#endif
