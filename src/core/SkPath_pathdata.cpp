/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * This file contains private enums related to paths. See also skbug.com/40042016
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkSpan.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits.h>
#include <utility>

/*  Contains path methods that require SkPathData -- not SkPathRef:
 *
 *  The remaining fields:
 *  - fFillType
 *  - fIsVolatile
 *
 *  ... are shared in both implemtations.
 */

#ifdef SK_PATH_USES_PATHDATA

/*
 *  This returns a singleton instance which SkPath uses to signify that its pathdata is in error:
 *  either because the inputs were invalid (e.g. bad verbs), or its coordintes were non-finite
 *  (either from the client, or after a makeTransform() call).
 */
SkPathData* SkPath::PeekErrorSingleton() {
    static SkPathData* gErrorSingleton = SkPathData::MakeNoCheck({}, {}, {}, {}, {}).release();

    // Make sure MakeNoCheck() didn't alias us to the standard Empty instance. We want our
    // pointer to be distinct from that one.
    SkASSERT(gErrorSingleton != SkPathData::Empty().get());

    return gErrorSingleton;
}

SkPath SkPath::MakeNullCheck(sk_sp<SkPathData> pdata, SkPathFillType ft, bool isVolatile) {
    if (!pdata) {
        pdata = sk_ref_sp(PeekErrorSingleton());
    }
    return SkPath(std::move(pdata), ft, isVolatile);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

SkPath::SkPath(sk_sp<SkPathData> pd, SkPathFillType ft, bool isVolatile)
    : fPathData(std::move(pd))
    , fFillType(ft)
    , fIsVolatile(isVolatile)
{
    SkASSERT(fPathData);
}

SkPath::SkPath(SkPathFillType ft)
    : fPathData(SkPathData::Empty())
    , fFillType(ft)
    , fIsVolatile(false)
{}

SkPath::SkPath(const SkPath& that)
    : fPathData(that.fPathData)
    , fFillType(that.fFillType)
    , fIsVolatile(that.fIsVolatile)
{}

SkPath& SkPath::operator=(const SkPath& o) {
    if (this != &o) {
        fPathData   = o.fPathData;
        fFillType   = o.fFillType;
        fIsVolatile = o.fIsVolatile;
    }
    return *this;
}

void SkPath::setConvexity(SkPathConvexity c) const {
    fPathData->setConvexity(c);
}

SkPathConvexity SkPath::getConvexityOrUnknown() const {
    return fPathData->getConvexityOrUnknown();
}

bool operator==(const SkPath& a, const SkPath& b) {
    return &a == &b ||
          (a.fFillType == b.fFillType && *a.fPathData == *b.fPathData);
}

void SkPath::swap(SkPath& that) {
    if (this != &that) {
        fPathData.swap(that.fPathData);
        std::swap(fFillType, that.fFillType);
        std::swap(fIsVolatile, that.fIsVolatile);
    }
}

SkPath& SkPath::reset() {
    *this = SkPath();
    return *this;
}

const SkRect& SkPath::getBounds() const {
    return fPathData->bounds();
}

uint32_t SkPath::getSegmentMasks() const {
    return fPathData->segmentMask();
}

bool SkPath::isFinite() const {
    return fPathData.get() != PeekErrorSingleton();
}

bool SkPath::isValid() const { return this->isFinite(); }

bool SkPath::hasComputedBounds() const { return true; }

uint32_t SkPath::getGenerationID() const { return fPathData->uniqueID(); }

#ifdef SK_DEBUG
void SkPath::validate() const {}

void SkPath::validateRef() const {}
#endif

std::optional<SkPathOvalInfo> SkPath::getOvalInfo() const { return fPathData->asOval(); }
std::optional<SkPathRRectInfo> SkPath::getRRectInfo() const { return fPathData->asRRect(); }

SkSpan<const SkPoint> SkPath::points() const { return fPathData->points(); }
SkSpan<const SkPathVerb> SkPath::verbs() const { return fPathData->verbs(); }
SkSpan<const float> SkPath::conicWeights() const { return fPathData->conics(); }

//////////////////////////////////////////////////////////////////////////////////////////////////

SkPath SkPath::Raw(SkSpan<const SkPoint> pts, SkSpan<const SkPathVerb> vbs,
                   SkSpan<const float> ws, SkPathFillType ft, bool isVolatile) {
    return MakeNullCheck(SkPathData::Make(pts, vbs, ws), ft, isVolatile);
}

SkPath SkPath::Rect(const SkRect& r, SkPathFillType ft, SkPathDirection dir, unsigned startIndex) {
    startIndex &= 3;    // keep it legal
    return MakeNullCheck(SkPathData::Rect(r, dir, startIndex), ft, false);
}

SkPath SkPath::Oval(const SkRect& r, SkPathDirection dir, unsigned startIndex) {
    startIndex &= 3;    // keep it legal
    return MakeNullCheck(SkPathData::Oval(r, dir, startIndex), SkPathFillType::kDefault, false);
}

SkPath SkPath::RRect(const SkRRect& rr, SkPathDirection dir, unsigned startIndex) {
    startIndex &= 7;    // keep it legal
    // To be backwards compatible with the old impl for building a rrect path, we
    // first check to see if the rrect itself can be simplified...
    const SkRect& bounds = rr.getBounds();
    auto [asType, newIndex] = SkPathPriv::SimplifyRRect(rr, startIndex);
    switch (asType) {
        case SkPathPriv::RRectAsEnum::kRect:
            return SkPath::Rect(bounds, SkPathFillType::kDefault, dir, newIndex);

        case SkPathPriv::RRectAsEnum::kOval:
            return SkPath::Oval(bounds, dir, newIndex);

        case SkPathPriv::RRectAsEnum::kRRect:
            // fall through
            break;
    }
    return MakeNullCheck(SkPathData::RRect(rr, dir, newIndex), SkPathFillType::kDefault, false);
}

SkPath SkPath::Polygon(SkSpan<const SkPoint> pts, bool isClosed,
                       SkPathFillType ft, bool isVolatile) {
    return MakeNullCheck(SkPathData::Polygon(pts, isClosed), ft, isVolatile);
}

std::optional<SkPath> SkPath::tryMakeTransform(const SkMatrix& matrix) const {
    if (auto pdata = fPathData->makeTransform(matrix)) {
        return SkPath(std::move(pdata), fFillType, fIsVolatile);
    }
    return {};
}

SkPath SkPath::makeTransform(const SkMatrix& matrix) const {
    if (!this->isFinite()) {
        return *this;
    }
    if (auto newpath = this->tryMakeTransform(matrix)) {
        return *newpath;
    }
    return SkPath(sk_ref_sp(PeekErrorSingleton()), fFillType, false);
}

std::optional<SkPathRaw> SkPath::raw(SkResolveConvexity rc) const {
    return fPathData->raw(fFillType, rc);
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool SkPathPriv::TestingOnly_unique(const SkPath& path) {
    return path.fPathData->unique();
}

int SkPathPriv::GenIDChangeListenersCount(const SkPath& path) {
    return path.fPathData->genIDChangeListenerCount();
}

void SkPathPriv::AddGenIDChangeListener(const SkPath& path, sk_sp<SkIDChangeListener> listener) {
    auto pdata = path.fPathData.get();
    // SkPath's error-singleton is never deleted, so we don't want to add any listeners to it.
    if (pdata != SkPath::PeekErrorSingleton()) {
        pdata->addGenIDChangeListener(std::move(listener));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////

SkPathBuilder& SkPathBuilder::operator=(const SkPath& src) {
    this->reset().setFillType(src.getFillType());
    this->setIsVolatile(src.isVolatile());

    if (src.isEmpty()) {
        return *this;
    }

    const SkPathData& pdata = *src.fPathData;

    this->addRaw(pdata.raw(src.getFillType(), SkResolveConvexity::kYes));

    // These are not part of SkPathRaw, so we set them separately

    fLastMoveIndex = SkPathPriv::FindLastMoveToIndex(pdata.verbs(), pdata.points().size());
    fType = pdata.fType;
    fIsA  = pdata.fIsA;

    return *this;
}

SkPath SkPathBuilder::snapshot(const SkMatrix* mx) const {
    if (!mx) {
        mx = &SkMatrix::I();
    }

    sk_sp<SkPathData> pdata;
    if (auto raw = SkPathPriv::Raw(*this, SkResolveConvexity::kNo)) {
        pdata = SkPathData::MakeTransform(*raw, *mx);
    }
    if (pdata && fType != SkPathIsAType::kGeneral) {
        pdata->setupIsA(fType, fIsA.fDirection, fIsA.fStartIndex);
    }
    return SkPath::MakeNullCheck(std::move(pdata), fFillType, fIsVolatile);
}

#endif
