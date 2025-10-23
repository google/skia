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
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits.h>
#include <utility>

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

// This is the public-facing non-const setConvexity().
void SkPath::setConvexity(SkPathConvexity c) {
    fConvexity.store((uint8_t)c, std::memory_order_relaxed);
}

// Const hooks for working with fConvexity and fFirstDirection from const methods.
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

SkPath SkPath::makeFillType(SkPathFillType ft) const {
    return SkPath(fPathRef,
                  ft,
                  fIsVolatile,
                  this->getConvexityOrUnknown());
}

SkPath SkPath::makeToggleInverseFillType() const {
    return SkPath(fPathRef,
                  SkPathFillType_ToggleInverse(fFillType),
                  fIsVolatile,
                  this->getConvexityOrUnknown());
}

SkPath SkPath::makeIsVolatile(bool v) const {
    return SkPath(fPathRef,
                  fFillType,
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

int SkPathPriv::GenIDChangeListenersCount(const SkPath& path) {
    return path.fPathRef->genIDChangeListenerCount();
}

