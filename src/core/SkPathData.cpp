/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMalloc.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkPathData.h"
#include "src/core/SkPathEnums.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkPathRawShapes.h"
#include "src/core/SkSpanPriv.h"

#include <new>
#include <optional>
#include <type_traits>

SkPathData* SkPathData::PeekEmptySingleton() {
    static SkPathData* gEmpty = SkPathData::MakeNoCheck({}, {}, {}, {}, {}).release();
    return gEmpty;
}

static uint32_t next_pathdata_unique_id() {
    constexpr int kHighBitsToMakeRoomForFillType = 2;

    static std::atomic<int32_t> nextID{1};

    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
        // clear the high bits to make room for filltype
        id <<= kHighBitsToMakeRoomForFillType;
        id >>= kHighBitsToMakeRoomForFillType;
    } while (id == 0);
    return id;
}

class SkSafeAccumulator {
public:
    SkSafeAccumulator(size_t n = 0) : fTotal(n) {}

    bool ok() const { return fSafe.ok(); }
    explicit operator bool() const { return fSafe.ok(); }

    SkSafeAccumulator& add(size_t n) {
        fTotal = fSafe.add(fTotal, n);
        return *this;
    }

    SkSafeAccumulator& addMul(size_t a, size_t b) {
        fTotal = fSafe.add(fTotal, fSafe.mul(a, b));
        return *this;
    }

    std::optional<size_t> total() const {
        if (fSafe.ok()) {
            return fTotal;
        }
        return {};
    }
private:
    SkSafeMath fSafe;
    size_t     fTotal;
};

const uint8_t gPtsPerVerb[] = {
    1, 1, 2, 2, 3, 0,  // move, line, quad, conic, cubic, close
};

static std::pair<size_t, size_t> count_pts_cns(SkSpan<const SkPathVerb> vbs) {
    size_t pts = 0,
           cns = 0;
    for (auto v : vbs) {
        SkASSERT((unsigned)v < sizeof(gPtsPerVerb));
        pts += gPtsPerVerb[(unsigned)v];
        cns += (v == SkPathVerb::kConic);
    }
    return {pts, cns};
}

// If it detects a single trailing Move verb, remove it and it's corresponding point.
// Otherwise return leave the spans unchanged.
//
static void trim_trailing_move(SkSpan<const SkPoint>* pts, SkSpan<const SkPathVerb>* vbs) {
    if (!vbs->empty() && vbs->back() == SkPathVerb::kMove) {
        *vbs = vbs->first(vbs->size() - 1);
        *pts = pts->first(pts->size() - 1);
    }
}

static bool valid_verbs(SkSpan<const SkPathVerb> vbs) {
    if (vbs.size() == 0) {
        return true;
    }

    // We must begin with a Move (unless we're empty)
    if (vbs.front() != SkPathVerb::kMove) {
        return false;
    }
    SkPathVerb prev = SkPathVerb::kMove;

    // Check that we have a valid sequence.
    for (size_t i = 1; i < vbs.size(); ++i) {
        SkPathVerb curr = vbs[i];

        if (static_cast<unsigned>(curr) > static_cast<unsigned>(SkPathVerb::kLast_Verb)) {
            return false;
        }

        // Previous verb             Valid next verb
        // -----------------------------------------
        // Move                  --> not Move
        // Line/Quad/Conic/Cubic --> *
        // Close                 --> Move
        //
        if (prev == SkPathVerb::kMove && curr == SkPathVerb::kMove) {
            return false;
        }
        if (prev == SkPathVerb::kClose && curr != SkPathVerb::kMove) {
            return false;
        }
        prev = curr;
    }

    // A trailing Move is also illegal, since it creates an empty contour,
    // which will confuse some of our callers
    //
    // See trim_trailing_move() helper, which may be called before calling us.
    //
    return vbs.back() != SkPathVerb::kMove;
}

static inline bool valid_conic_weight(float w) {
    return w >= 0 && SkIsFinite(w);
}

// Handing in debugging, to set a break-point here if you want to know why we
// failed to create a pathdta
//
static void report_pathdata_make_failure(const char reason[]) {
//  `    SkDEBUGF("SkPathData::Make failed: %s\n", reason);
}

// This just sets-up the spans to point inside our allocation
//
SkPathData::SkPathData(size_t npts, size_t nvbs, size_t ncns)
    : fUniqueID(next_pathdata_unique_id())
    , fConvexity((uint8_t)SkPathConvexity::kUnknown)
    , fType(SkPathIsAType::kGeneral)
{
    SkASSERT((npts == 0 && nvbs == 0 && ncns == 0) ||
             (npts != 0 && nvbs != 0));

#ifdef SK_DEBUG
    {
        SkSafeAccumulator accum(sizeof(*this));
        accum.addMul(npts, sizeof(SkPoint))
             .addMul(ncns, sizeof(SkPoint))
             .addMul(nvbs, sizeof(SkPathVerb));
        SkASSERT(accum.ok());
    }
#endif

    if (nvbs == 0) {
        SkASSERT(fPoints.empty());
        SkASSERT(fVerbs.empty());
        SkASSERT(fConics.empty());
    } else {
        auto data = reinterpret_cast<std::byte*>(this + 1);

        fPoints = {reinterpret_cast<SkPoint*>(data), npts};
        data += fPoints.size_bytes();

        fConics = {reinterpret_cast<float*>(data), ncns};
        data += fConics.size_bytes();

        fVerbs = {reinterpret_cast<SkPathVerb*>(data), nvbs};
    }

    // fBounds is initialized in finishInit()
}

SkPathData::~SkPathData() {
    // We will implicitly call our IDChangeList here, notifying them that we are
    // being dstroyed.
    SkDEBUGCODE(fUniqueID = 0xEEEEEEEE;)
}

void SkPathData::operator delete(void* p) {
    ::operator delete(p);
}

void SkPathData::addGenIDChangeListener(sk_sp<SkIDChangeListener> listener) const {
    // our empty singleton is never deleted, so we don't want to add any listeners to it.
    if (this != SkPathData::PeekEmptySingleton()) {
        // this method on the list is thread-safe
        fGenIDChangeListeners.add(std::move(listener));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////

// NOTE: This only allocates and initializes the span pointers (points, verbs),
//       it does NOT set the other fields
sk_sp<SkPathData> SkPathData::Alloc(size_t npts, size_t nvbs, size_t ncns) {
    SkSafeAccumulator accum(sizeof(SkPathData));

    accum.addMul(npts, sizeof(SkPoint))
         .addMul(ncns, sizeof(SkPoint))
         .addMul(nvbs, sizeof(SkPathVerb));

    if (auto size = accum.total()) {
        // This trick allows us to just make one allocation, for us and our buffer
        // rather than allocating us and also allocating the buffer (via malloc or new[])
        // We have the corresponding operator delete() specified as well.
        void* storage = ::operator new (*size);
        sk_sp<SkPathData> path(new (storage) SkPathData(npts, nvbs, ncns));

        return path;
    }
    return nullptr;
}

bool SkPathData::finishInit(std::optional<SkRect> bounds, std::optional<uint8_t> segmentMask) {
    if (fPoints.empty()) {
        fBounds = SkRect::MakeEmpty();
        fSegmentMask = 0;
        return true;
    }

    if (segmentMask.has_value()) {
        fSegmentMask = segmentMask.value();
    } else {
        fSegmentMask = SkPathPriv::ComputeSegmentMask(fVerbs);
    }

    if (bounds.has_value()) {
        fBounds = bounds.value();
        SkASSERT(SkIsFinite(&fPoints.data()->fX, fPoints.size() * 2));
    } else {
        if (auto r = SkRect::Bounds(fPoints)) {
            fBounds = r.value();
        } else {
            report_pathdata_make_failure("non-finite bounds");
            return false;
        }
    }

    return true;
}

sk_sp<SkPathData> SkPathData::MakeTransform(const SkPathRaw& src, const SkMatrix& mx) {
    if (src.empty()) {
        return SkPathData::Empty();
    }

    if (mx.hasPerspective()) {
        SkPathBuilder bu;
        bu.addRaw(src);
        bu.transform(mx);
        return bu.detachData();
    }

    // Allocate our result, so we can map the new points directly into it
    auto result = Alloc(src.points().size(), src.verbs().size(), src.conics().size());
    mx.mapPoints(result->fPoints, src.points());
    SkSpanPriv::Copy(result->fConics, src.conics());
    SkSpanPriv::Copy(result->fVerbs,  src.verbs());

    std::optional<SkRect> transformedBounds;
    if (mx.rectStaysRect()) {
        // safe us from having to compute our transformed bounds in finishInit()
        transformedBounds = mx.mapRect(src.bounds());
        if (!transformedBounds.value().isFinite()) {
            report_pathdata_make_failure("transform created non-finite bounds");
            return nullptr;
        }
    }

    if (!result->finishInit(transformedBounds, src.fSegmentMask)) {
        return nullptr;
    }

    result->setConvexity(SkPathPriv::TransformConvexity(mx, src.fPoints, src.fConvexity));

    return result;
}

sk_sp<SkPathData> SkPathData::makeTransform(const SkMatrix& mx) const {
    if (mx.isIdentity()) {
        return sk_ref_sp(this);
    }

    // not important for transform, just need a value
    const SkPathFillType ft = SkPathFillType::kDefault;

    if (auto result = MakeTransform(this->raw(ft, SkResolveConvexity::kNo), mx)) {
        // See if we can maintian our IsA status ...
        if ((fType == SkPathIsAType::kOval || fType == SkPathIsAType::kRRect) &&
            mx.rectStaysRect() && SkPathPriv::IsAxisAligned(fPoints))
        {
            auto [dir, start] =
            SkPathPriv::TransformDirAndStart(mx, fType == SkPathIsAType::kRRect,
                                             fIsA.fDirection, fIsA.fStartIndex);
            result->setupIsA(fType, dir, start);
        }
        return result;
    }
    return nullptr;
}

sk_sp<SkPathData> SkPathData::makeOffset(SkVector v) const {
    return this->makeTransform(SkMatrix::Translate(v));
}

bool operator==(const SkPathData& a, const SkPathData& b) {
    if (&a == &b) {
        return true;
    }

    return  SkSpanPriv::EQ(a.fPoints, b.fPoints) &&
            SkSpanPriv::EQ(a.fConics, b.fConics) &&
            SkSpanPriv::EQ(a.fVerbs,  b.fVerbs);
}

/////////////////////////////////////

sk_sp<SkPathData> SkPathData::MakeNoCheck(SkSpan<const SkPoint> pts,
                                          SkSpan<const SkPathVerb> vbs,
                                          SkSpan<const float> conics,
                                          std::optional<SkRect> bounds,
                                          std::optional<unsigned> segmentMask) {
    trim_trailing_move(&pts, &vbs);
    SkASSERT(valid_verbs(vbs));

    auto path = Alloc(pts.size(), vbs.size(), conics.size());

    SkSpanPriv::Copy(path->fPoints, pts);
    SkSpanPriv::Copy(path->fConics, conics);
    SkSpanPriv::Copy(path->fVerbs,  vbs);

    return path->finishInit(bounds, segmentMask) ? path : nullptr;
}

sk_sp<SkPathData> SkPathData::MakeNoCheck(const SkPathRaw& raw) {
    return MakeNoCheck(raw.points(), raw.verbs(), raw.conics(), raw.fBounds, raw.fSegmentMask);
}

sk_sp<SkPathData> SkPathData::Empty() {
    return sk_ref_sp(PeekEmptySingleton());
}

void SkPathData::setupIsA(SkPathIsAType type, SkPathDirection dir, unsigned index) {
    this->setConvexity(SkPathDirection_ToConvexity(dir));

    SkASSERT(type == SkPathIsAType::kOval || type == SkPathIsAType::kRRect);
    fType = type;

    SkASSERT((type == SkPathIsAType::kOval && index < 4) ||
             (type == SkPathIsAType::kRRect && index < 8));

    fIsA.fDirection  = dir;
    fIsA.fStartIndex = SkTo<uint8_t>(index);
}

sk_sp<SkPathData> SkPathData::Rect(const SkRect& r, SkPathDirection dir, unsigned index) {
    if (!r.isFinite()) {
        return nullptr;
    }
    SkPathRawShapes::Rect raw(r, dir, index);
    return MakeNoCheck(raw.points(), raw.verbs(), raw.conics(), raw.fBounds, raw.fSegmentMask);
}

sk_sp<SkPathData> SkPathData::Oval(const SkRect& r, SkPathDirection dir, unsigned index) {
    if (!r.isFinite()) {
        return nullptr;
    }
    SkPathRawShapes::Oval raw(r, dir, index);
    auto path = MakeNoCheck(raw.points(), raw.verbs(), raw.conics(), raw.fBounds, raw.fSegmentMask);

    path->setupIsA(SkPathIsAType::kOval, dir, index);
    return path;
}

sk_sp<SkPathData> SkPathData::RRect(const SkRRect& r, SkPathDirection dir, unsigned index) {
    if (!r.isValid()) {
        return nullptr;
    }
    SkPathRawShapes::RRect raw(r, dir, index);
    // we use Make, not MakeNoCheck, to confirm all points an conics are finite
    if (auto path = Make(raw.points(), raw.verbs(), raw.conics())) {
        path->setupIsA(SkPathIsAType::kRRect, dir, index);
        return path;
    }
    return nullptr;
}

sk_sp<SkPathData> SkPathData::Polygon(SkSpan<const SkPoint> pts, bool isClosed) {
    if (pts.size() == 0 || (pts.size() == 1 && !isClosed)) {
        return Empty();
    }

    const size_t nverbs = pts.size() + isClosed;    // +1 for the kClose verb
    const size_t nconics = 0;
    auto path = Alloc(pts.size(), nverbs, nconics);

    SkSpanPriv::Copy(path->fPoints, pts);

    path->fVerbs[0] = SkPathVerb::kMove;
    for (size_t i = 1; i < pts.size(); ++i) {
        path->fVerbs[i] = SkPathVerb::kLine;
    }
    if (isClosed) {
        path->fVerbs.back() = SkPathVerb::kClose;
    }

    return path->finishInit({}, kLine_SkPathSegmentMask) ? path : nullptr;
}

/////////////////////////////////////

SkPathConvexity SkPathData::getConvexityOrUnknown() const {
    return static_cast<SkPathConvexity>(fConvexity.load(std::memory_order_relaxed));
}

SkPathConvexity SkPathData::getResolvedConvexity() const {
    auto convexity = this->getConvexityOrUnknown();
    if (convexity == SkPathConvexity::kUnknown) {
        convexity = SkPathPriv::ComputeConvexity(fPoints, fVerbs, fConics);
        this->setConvexity(convexity);
    }
    return convexity;
}

void SkPathData::setConvexity(SkPathConvexity convexity) const {
    fConvexity.store((uint8_t)convexity, std::memory_order_relaxed);
}

bool SkPathData::isConvex() const {
    return SkPathConvexity_IsConvex(this->getResolvedConvexity());
}

SkRect SkPathData::computeTightBounds() const {
    return SkPathPriv::ComputeTightBounds(this->points(), this->verbs(), this->conics());
}

SkPathRaw SkPathData::raw(SkPathFillType ft, SkResolveConvexity rc) const {
    return {
        fPoints,
        fVerbs,
        fConics,
        fBounds,
        ft,
        rc == SkResolveConvexity::kYes ? this->getResolvedConvexity()
                                       : this->getConvexityOrUnknown(),
        fSegmentMask,
    };
}

std::optional<std::array<SkPoint, 2>> SkPathData::asLine() const {
    if (fPoints.size() == 2 && fVerbs.size() == 2 && fConics.size() == 0 &&
        fVerbs[1] == SkPathVerb::kLine)
    {
        SkASSERT(fVerbs[0] == SkPathVerb::kMove);
        return {{ fPoints[0], fPoints[1] }};
    }
    return {};
}

std::optional<SkPathRectInfo> SkPathData::asRect() const {
    if (auto rc = SkPathPriv::IsRectContour(fPoints, fVerbs, fSegmentMask, false)) {
        SkASSERT(rc->fRect == fBounds);
        return {{
            fBounds,
            rc->fDirection,
            0, // start index???
        }};
    }
    return {};
}

std::optional<SkPathOvalInfo> SkPathData::asOval() const {
    if (fType == SkPathIsAType::kOval) {
        return {{
            fBounds,
            fIsA.fDirection,
            fIsA.fStartIndex,
        }};
    }
    return {};
}

std::optional<SkPathRRectInfo> SkPathData::asRRect() const {
    if (fType == SkPathIsAType::kRRect) {
        return {{
            SkPathPriv::DeduceRRectFromContour(fBounds, fPoints, fVerbs),
            fIsA.fDirection,
            fIsA.fStartIndex,
        }};
    }
    return {};
}

bool SkPathData::contains(SkPoint p, SkPathFillType ft) const {
    return SkPathPriv::Contains(this->raw(ft, SkResolveConvexity::kNo), p);
}

/////////////////

sk_sp<SkPathData> SkPathData::Make(SkSpan<const SkPoint> pts,
                                   SkSpan<const SkPathVerb> vbs,
                                   SkSpan<const float> conics) {
    trim_trailing_move(&pts, &vbs);
    if (!valid_verbs(vbs)) {
        report_pathdata_make_failure("invalid verb sequence");
        return nullptr;
    }

    auto [npts, ncns] = count_pts_cns(vbs);
    if (pts.size() != npts || conics.size() != ncns) {
        report_pathdata_make_failure("unexpected # points or conics");
        return nullptr;
    }

    // Now we can check for a dangling kMove verb, and just ignore it
    if (!vbs.empty() && vbs.back() == SkPathVerb::kMove) {
        SkASSERT(!pts.empty());
        vbs = vbs.first(vbs.size() - 1);
        pts = pts.first(pts.size() - 1);
    }
    if (vbs.empty()) {
        return Empty();
    }

    for (auto w : conics) {
        if (!valid_conic_weight(w)) {
            report_pathdata_make_failure("non-finite conics");
            return nullptr;
        }
    }

    // MakeNoCheck *does* compute/check bounds if we don't pass them in
    return MakeNoCheck(pts, vbs, conics, {}, {});
}
