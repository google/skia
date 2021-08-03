/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/pathops/SkPathOps.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkRectPriv.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "tests/Test.h"

#include <initializer_list>
#include <functional>
#include <memory>
#include <utility>

uint32_t GrStyledShape::testingOnly_getOriginalGenerationID() const {
    if (const auto* lp = this->originalPathForListeners()) {
        return lp->getGenerationID();
    }
    return SkPath().getGenerationID();
}

bool GrStyledShape::testingOnly_isPath() const {
    return fShape.isPath();
}

bool GrStyledShape::testingOnly_isNonVolatilePath() const {
    return fShape.isPath() && !fShape.path().isVolatile();
}

using Key = SkTArray<uint32_t>;

static bool make_key(Key* key, const GrStyledShape& shape) {
    int size = shape.unstyledKeySize();
    if (size <= 0) {
        key->reset(0);
        return false;
    }
    SkASSERT(size);
    key->reset(size);
    shape.writeUnstyledKey(key->begin());
    return true;
}

static bool paths_fill_same(const SkPath& a, const SkPath& b) {
    SkPath pathXor;
    Op(a, b, SkPathOp::kXOR_SkPathOp, &pathXor);
    return pathXor.isEmpty();
}

static bool test_bounds_by_rasterizing(const SkPath& path, const SkRect& bounds) {
    // We test the bounds by rasterizing the path into a kRes by kRes grid. The bounds is
    // mapped to the range kRes/4 to 3*kRes/4 in x and y. A difference clip is used to avoid
    // rendering within the bounds (with a tolerance). Then we render the path and check that
    // everything got clipped out.
    static constexpr int kRes = 2000;
    // This tolerance is in units of 1/kRes fractions of the bounds width/height.
    static constexpr int kTol = 2;
    static_assert(kRes % 4 == 0);
    SkImageInfo info = SkImageInfo::MakeA8(kRes, kRes);
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
    surface->getCanvas()->clear(0x0);
    SkRect clip = SkRect::MakeXYWH(kRes/4, kRes/4, kRes/2, kRes/2);
    SkMatrix matrix = SkMatrix::RectToRect(bounds, clip);
    clip.outset(SkIntToScalar(kTol), SkIntToScalar(kTol));
    surface->getCanvas()->clipRect(clip, SkClipOp::kDifference);
    surface->getCanvas()->concat(matrix);
    SkPaint whitePaint;
    whitePaint.setColor(SK_ColorWHITE);
    surface->getCanvas()->drawPath(path, whitePaint);
    SkPixmap pixmap;
    surface->getCanvas()->peekPixels(&pixmap);
#if defined(SK_BUILD_FOR_WIN)
    // The static constexpr version in #else causes cl.exe to crash.
    const uint8_t* kZeros = reinterpret_cast<uint8_t*>(calloc(kRes, 1));
#else
    static constexpr uint8_t kZeros[kRes] = {0};
#endif
    for (int y = 0; y < kRes; ++y) {
        const uint8_t* row = pixmap.addr8(0, y);
        if (0 != memcmp(kZeros, row, kRes)) {
            return false;
        }
    }
#ifdef SK_BUILD_FOR_WIN
    free(const_cast<uint8_t*>(kZeros));
#endif
    return true;
}

static bool can_interchange_winding_and_even_odd_fill(const GrStyledShape& shape) {
    SkPath path;
    shape.asPath(&path);
    if (shape.style().hasNonDashPathEffect()) {
        return false;
    }
    const SkStrokeRec::Style strokeRecStyle = shape.style().strokeRec().getStyle();
    return strokeRecStyle == SkStrokeRec::kStroke_Style ||
           strokeRecStyle == SkStrokeRec::kHairline_Style ||
           (shape.style().isSimpleFill() && path.isConvex());
}

static void check_equivalence(skiatest::Reporter* r, const GrStyledShape& a, const GrStyledShape& b,
                              const Key& keyA, const Key& keyB) {
    // GrStyledShape only respects the input winding direction and start point for rrect shapes
    // when there is a path effect. Thus, if there are two GrStyledShapes representing the same
    // rrect but one has a path effect in its style and the other doesn't then asPath() and the
    // unstyled key will differ. GrStyledShape will have canonicalized the direction and start point
    // for the shape without the path effect. If *both* have path effects then they should have both
    // preserved the direction and starting point.

    // The asRRect() output params are all initialized just to silence compiler warnings about
    // uninitialized variables.
    SkRRect rrectA = SkRRect::MakeEmpty(), rrectB = SkRRect::MakeEmpty();
    SkPathDirection dirA = SkPathDirection::kCW, dirB = SkPathDirection::kCW;
    unsigned startA = ~0U, startB = ~0U;
    bool invertedA = true, invertedB = true;

    bool aIsRRect = a.asRRect(&rrectA, &dirA, &startA, &invertedA);
    bool bIsRRect = b.asRRect(&rrectB, &dirB, &startB, &invertedB);
    bool aHasPE = a.style().hasPathEffect();
    bool bHasPE = b.style().hasPathEffect();
    bool allowSameRRectButDiffStartAndDir = (aIsRRect && bIsRRect) && (aHasPE != bHasPE);
    // GrStyledShape will close paths with simple fill style.
    bool allowedClosednessDiff = (a.style().isSimpleFill() != b.style().isSimpleFill());
    SkPath pathA, pathB;
    a.asPath(&pathA);
    b.asPath(&pathB);

    // Having a dash path effect can allow 'a' but not 'b' to turn a inverse fill type into a
    // non-inverse fill type  (or vice versa).
    bool ignoreInversenessDifference = false;
    if (pathA.isInverseFillType() != pathB.isInverseFillType()) {
        const GrStyledShape* s1 = pathA.isInverseFillType() ? &a : &b;
        const GrStyledShape* s2 = pathA.isInverseFillType() ? &b : &a;
        bool canDropInverse1 = s1->style().isDashed();
        bool canDropInverse2 = s2->style().isDashed();
        ignoreInversenessDifference = (canDropInverse1 != canDropInverse2);
    }
    bool ignoreWindingVsEvenOdd = false;
    if (SkPathFillType_ConvertToNonInverse(pathA.getFillType()) !=
        SkPathFillType_ConvertToNonInverse(pathB.getFillType())) {
        bool aCanChange = can_interchange_winding_and_even_odd_fill(a);
        bool bCanChange = can_interchange_winding_and_even_odd_fill(b);
        if (aCanChange != bCanChange) {
            ignoreWindingVsEvenOdd = true;
        }
    }
    if (allowSameRRectButDiffStartAndDir) {
        REPORTER_ASSERT(r, rrectA == rrectB);
        REPORTER_ASSERT(r, paths_fill_same(pathA, pathB));
        REPORTER_ASSERT(r, ignoreInversenessDifference || invertedA == invertedB);
    } else {
        SkPath pA = pathA;
        SkPath pB = pathB;
        REPORTER_ASSERT(r, a.inverseFilled() == pA.isInverseFillType());
        REPORTER_ASSERT(r, b.inverseFilled() == pB.isInverseFillType());
        if (ignoreInversenessDifference) {
            pA.setFillType(SkPathFillType_ConvertToNonInverse(pathA.getFillType()));
            pB.setFillType(SkPathFillType_ConvertToNonInverse(pathB.getFillType()));
        }
        if (ignoreWindingVsEvenOdd) {
            pA.setFillType(pA.isInverseFillType() ? SkPathFillType::kInverseEvenOdd
                                                  : SkPathFillType::kEvenOdd);
            pB.setFillType(pB.isInverseFillType() ? SkPathFillType::kInverseEvenOdd
                                                  : SkPathFillType::kEvenOdd);
        }
        if (!ignoreInversenessDifference && !ignoreWindingVsEvenOdd) {
            REPORTER_ASSERT(r, keyA == keyB);
        } else {
            REPORTER_ASSERT(r, keyA != keyB);
        }
        if (allowedClosednessDiff) {
            // GrStyledShape will close paths with simple fill style. Make the non-filled path
            // closed so that the comparision will succeed. Make sure both are closed before
            // comparing.
            pA.close();
            pB.close();
        }
        REPORTER_ASSERT(r, pA == pB);
        REPORTER_ASSERT(r, aIsRRect == bIsRRect);
        if (aIsRRect) {
            REPORTER_ASSERT(r, rrectA == rrectB);
            REPORTER_ASSERT(r, dirA == dirB);
            REPORTER_ASSERT(r, startA == startB);
            REPORTER_ASSERT(r, ignoreInversenessDifference || invertedA == invertedB);
        }
    }
    REPORTER_ASSERT(r, a.isEmpty() == b.isEmpty());
    REPORTER_ASSERT(r, allowedClosednessDiff || a.knownToBeClosed() == b.knownToBeClosed());
    // closedness can affect convexity.
    REPORTER_ASSERT(r, allowedClosednessDiff || a.knownToBeConvex() == b.knownToBeConvex());
    if (a.knownToBeConvex()) {
        REPORTER_ASSERT(r, pathA.isConvex());
    }
    if (b.knownToBeConvex()) {
        REPORTER_ASSERT(r, pathB.isConvex());
    }
    REPORTER_ASSERT(r, a.bounds() == b.bounds());
    REPORTER_ASSERT(r, a.segmentMask() == b.segmentMask());
    // Init these to suppress warnings.
    SkPoint pts[4] {{0, 0,}, {0, 0}, {0, 0}, {0, 0}} ;
    bool invertedLine[2] {true, true};
    REPORTER_ASSERT(r, a.asLine(pts, &invertedLine[0]) == b.asLine(pts + 2, &invertedLine[1]));
    // mayBeInverseFilledAfterStyling() is allowed to differ if one has a arbitrary PE and the other
    // doesn't (since the PE can set any fill type on its output path).
    // Moreover, dash style explicitly ignores inverseness. So if one is dashed but not the other
    // then they may disagree about inverseness.
    if (a.style().hasNonDashPathEffect() == b.style().hasNonDashPathEffect() &&
        a.style().isDashed() == b.style().isDashed()) {
        REPORTER_ASSERT(r, a.mayBeInverseFilledAfterStyling() ==
                           b.mayBeInverseFilledAfterStyling());
    }
    if (a.asLine(nullptr, nullptr)) {
        REPORTER_ASSERT(r, pts[2] == pts[0] && pts[3] == pts[1]);
        REPORTER_ASSERT(r, ignoreInversenessDifference || invertedLine[0] == invertedLine[1]);
        REPORTER_ASSERT(r, invertedLine[0] == a.inverseFilled());
        REPORTER_ASSERT(r, invertedLine[1] == b.inverseFilled());
    }
    REPORTER_ASSERT(r, ignoreInversenessDifference || a.inverseFilled() == b.inverseFilled());
}

static void check_original_path_ids(skiatest::Reporter* r, const GrStyledShape& base,
                                    const GrStyledShape& pe, const GrStyledShape& peStroke,
                                    const GrStyledShape& full) {
    bool baseIsNonVolatilePath = base.testingOnly_isNonVolatilePath();
    bool peIsPath = pe.testingOnly_isPath();
    bool peStrokeIsPath = peStroke.testingOnly_isPath();
    bool fullIsPath = full.testingOnly_isPath();

    REPORTER_ASSERT(r, peStrokeIsPath == fullIsPath);

    uint32_t baseID = base.testingOnly_getOriginalGenerationID();
    uint32_t peID = pe.testingOnly_getOriginalGenerationID();
    uint32_t peStrokeID = peStroke.testingOnly_getOriginalGenerationID();
    uint32_t fullID = full.testingOnly_getOriginalGenerationID();

    // All empty paths have the same gen ID
    uint32_t emptyID = SkPath().getGenerationID();

    // If we started with a real path, then our genID should match that path's gen ID (and not be
    // empty). If we started with a simple shape or a volatile path, our original path should have
    // been reset.
    REPORTER_ASSERT(r, baseIsNonVolatilePath == (baseID != emptyID));

    // For the derived shapes, if they're simple types, their original paths should have been reset
    REPORTER_ASSERT(r, peIsPath || (peID == emptyID));
    REPORTER_ASSERT(r, peStrokeIsPath || (peStrokeID == emptyID));
    REPORTER_ASSERT(r, fullIsPath || (fullID == emptyID));

    if (!peIsPath) {
        // If the path effect produces a simple shape, then there are no unbroken chains to test
        return;
    }

    // From here on, we know that the path effect produced a shape that was a "real" path

    if (baseIsNonVolatilePath) {
        REPORTER_ASSERT(r, baseID == peID);
    }

    if (peStrokeIsPath) {
        REPORTER_ASSERT(r, peID == peStrokeID);
        REPORTER_ASSERT(r, peStrokeID == fullID);
    }

    if (baseIsNonVolatilePath && peStrokeIsPath) {
        REPORTER_ASSERT(r, baseID == peStrokeID);
        REPORTER_ASSERT(r, baseID == fullID);
    }
}

void test_inversions(skiatest::Reporter* r, const GrStyledShape& shape, const Key& shapeKey) {
    GrStyledShape preserve = GrStyledShape::MakeFilled(
            shape, GrStyledShape::FillInversion::kPreserve);
    Key preserveKey;
    make_key(&preserveKey, preserve);

    GrStyledShape flip = GrStyledShape::MakeFilled(shape, GrStyledShape::FillInversion::kFlip);
    Key flipKey;
    make_key(&flipKey, flip);

    GrStyledShape inverted = GrStyledShape::MakeFilled(
            shape, GrStyledShape::FillInversion::kForceInverted);
    Key invertedKey;
    make_key(&invertedKey, inverted);

    GrStyledShape noninverted = GrStyledShape::MakeFilled(
            shape, GrStyledShape::FillInversion::kForceNoninverted);
    Key noninvertedKey;
    make_key(&noninvertedKey, noninverted);

    if (invertedKey.count() || noninvertedKey.count()) {
        REPORTER_ASSERT(r, invertedKey != noninvertedKey);
    }
    if (shape.style().isSimpleFill()) {
        check_equivalence(r, shape, preserve, shapeKey, preserveKey);
    }
    if (shape.inverseFilled()) {
        check_equivalence(r, preserve, inverted, preserveKey, invertedKey);
        check_equivalence(r, flip, noninverted, flipKey, noninvertedKey);
    } else {
        check_equivalence(r, preserve, noninverted, preserveKey, noninvertedKey);
        check_equivalence(r, flip, inverted, flipKey, invertedKey);
    }

    GrStyledShape doubleFlip = GrStyledShape::MakeFilled(flip, GrStyledShape::FillInversion::kFlip);
    Key doubleFlipKey;
    make_key(&doubleFlipKey, doubleFlip);
    // It can be the case that the double flip has no key but preserve does. This happens when the
    // original shape has an inherited style key. That gets dropped on the first inversion flip.
    if (preserveKey.count() && !doubleFlipKey.count()) {
        preserveKey.reset();
    }
    check_equivalence(r, preserve, doubleFlip, preserveKey, doubleFlipKey);
}

namespace {
/**
 * Geo is a factory for creating a GrStyledShape from another representation. It also answers some
 * questions about expected behavior for GrStyledShape given the inputs.
 */
class Geo {
public:
    virtual ~Geo() {}
    virtual GrStyledShape makeShape(const SkPaint&) const = 0;
    virtual SkPath path() const = 0;
    // These functions allow tests to check for special cases where style gets
    // applied by GrStyledShape in its constructor (without calling GrStyledShape::applyStyle).
    // These unfortunately rely on knowing details of GrStyledShape's implementation.
    // These predicates are factored out here to avoid littering the rest of the
    // test code with GrStyledShape implementation details.
    virtual bool fillChangesGeom() const { return false; }
    virtual bool strokeIsConvertedToFill() const { return false; }
    virtual bool strokeAndFillIsConvertedToFill(const SkPaint&) const { return false; }
    // Is this something we expect GrStyledShape to recognize as something simpler than a path.
    virtual bool isNonPath(const SkPaint& paint) const { return true; }
};

class RectGeo : public Geo {
public:
    RectGeo(const SkRect& rect) : fRect(rect) {}

    SkPath path() const override {
        SkPath path;
        path.addRect(fRect);
        return path;
    }

    GrStyledShape makeShape(const SkPaint& paint) const override {
        return GrStyledShape(fRect, paint);
    }

    bool strokeAndFillIsConvertedToFill(const SkPaint& paint) const override {
        SkASSERT(paint.getStyle() == SkPaint::kStrokeAndFill_Style);
        // Converted to an outset rectangle or round rect
        return (paint.getStrokeJoin() == SkPaint::kMiter_Join &&
                paint.getStrokeMiter() >= SK_ScalarSqrt2) ||
               paint.getStrokeJoin() == SkPaint::kRound_Join;
    }

private:
    SkRect fRect;
};

class RRectGeo : public Geo {
public:
    RRectGeo(const SkRRect& rrect) : fRRect(rrect) {}

    GrStyledShape makeShape(const SkPaint& paint) const override {
        return GrStyledShape(fRRect, paint);
    }

    SkPath path() const override {
        SkPath path;
        path.addRRect(fRRect);
        return path;
    }

    bool strokeAndFillIsConvertedToFill(const SkPaint& paint) const override {
        SkASSERT(paint.getStyle() == SkPaint::kStrokeAndFill_Style);
        if (fRRect.isRect()) {
            return RectGeo(fRRect.rect()).strokeAndFillIsConvertedToFill(paint);
        }
        return false;
    }

private:
    SkRRect fRRect;
};

class ArcGeo : public Geo {
public:
    ArcGeo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool useCenter)
            : fOval(oval)
            , fStartAngle(startAngle)
            , fSweepAngle(sweepAngle)
            , fUseCenter(useCenter) {}

    SkPath path() const override {
        SkPath path;
        SkPathPriv::CreateDrawArcPath(&path, fOval, fStartAngle, fSweepAngle, fUseCenter, false);
        return path;
    }

    GrStyledShape makeShape(const SkPaint& paint) const override {
        return GrStyledShape::MakeArc(fOval, fStartAngle, fSweepAngle, fUseCenter, GrStyle(paint));
    }

    // GrStyledShape specializes when created from arc params but it doesn't recognize arcs from
    // SkPath.
    bool isNonPath(const SkPaint& paint) const override { return false; }

private:
    SkRect fOval;
    SkScalar fStartAngle;
    SkScalar fSweepAngle;
    bool fUseCenter;
};

class PathGeo : public Geo {
public:
    enum class Invert { kNo, kYes };

    PathGeo(const SkPath& path, Invert invert) : fPath(path)  {
        SkASSERT(!path.isInverseFillType());
        if (Invert::kYes == invert) {
            if (fPath.getFillType() == SkPathFillType::kEvenOdd) {
                fPath.setFillType(SkPathFillType::kInverseEvenOdd);
            } else {
                SkASSERT(fPath.getFillType() == SkPathFillType::kWinding);
                fPath.setFillType(SkPathFillType::kInverseWinding);
            }
        }
    }

    GrStyledShape makeShape(const SkPaint& paint) const override {
        return GrStyledShape(fPath, paint);
    }

    SkPath path() const override { return fPath; }

    bool fillChangesGeom() const override {
        // unclosed rects get closed. Lines get turned into empty geometry
        return this->isUnclosedRect() || fPath.isLine(nullptr);
    }

    bool strokeIsConvertedToFill() const override {
        return this->isAxisAlignedLine();
    }

    bool strokeAndFillIsConvertedToFill(const SkPaint& paint) const override {
        SkASSERT(paint.getStyle() == SkPaint::kStrokeAndFill_Style);
        if (this->isAxisAlignedLine()) {
            // The fill is ignored (zero area) and the stroke is converted to a rrect.
            return true;
        }
        SkRect rect;
        unsigned start;
        SkPathDirection dir;
        if (SkPathPriv::IsSimpleRect(fPath, false, &rect, &dir, &start)) {
            return RectGeo(rect).strokeAndFillIsConvertedToFill(paint);
        }
        return false;
    }

    bool isNonPath(const SkPaint& paint) const override {
        return fPath.isLine(nullptr) || fPath.isEmpty();
    }

private:
    bool isAxisAlignedLine() const {
        SkPoint pts[2];
        if (!fPath.isLine(pts)) {
            return false;
        }
        return pts[0].fX == pts[1].fX || pts[0].fY == pts[1].fY;
    }

    bool isUnclosedRect() const {
        bool closed;
        return fPath.isRect(nullptr, &closed, nullptr) && !closed;
    }

    SkPath fPath;
};

class RRectPathGeo : public PathGeo {
public:
    enum class RRectForStroke { kNo, kYes };

    RRectPathGeo(const SkPath& path, const SkRRect& equivalentRRect, RRectForStroke rrectForStroke,
                 Invert invert)
            : PathGeo(path, invert)
            , fRRect(equivalentRRect)
            , fRRectForStroke(rrectForStroke) {}

    RRectPathGeo(const SkPath& path, const SkRect& equivalentRect, RRectForStroke rrectForStroke,
                 Invert invert)
            : RRectPathGeo(path, SkRRect::MakeRect(equivalentRect), rrectForStroke, invert) {}

    bool isNonPath(const SkPaint& paint) const override {
        if (SkPaint::kFill_Style == paint.getStyle() || RRectForStroke::kYes == fRRectForStroke) {
            return true;
        }
        return false;
    }

    const SkRRect& rrect() const { return fRRect; }

private:
    SkRRect         fRRect;
    RRectForStroke  fRRectForStroke;
};

class TestCase {
public:
    TestCase(const Geo& geo, const SkPaint& paint, skiatest::Reporter* r,
             SkScalar scale = SK_Scalar1)
            : fBase(new GrStyledShape(geo.makeShape(paint))) {
        this->init(r, scale);
    }

    template <typename... ShapeArgs>
    TestCase(skiatest::Reporter* r, ShapeArgs... shapeArgs)
            : fBase(new GrStyledShape(shapeArgs...)) {
        this->init(r, SK_Scalar1);
    }

    TestCase(const GrStyledShape& shape, skiatest::Reporter* r, SkScalar scale = SK_Scalar1)
            : fBase(new GrStyledShape(shape)) {
        this->init(r, scale);
    }

    struct SelfExpectations {
        bool fPEHasEffect;
        bool fPEHasValidKey;
        bool fStrokeApplies;
    };

    void testExpectations(skiatest::Reporter* reporter, SelfExpectations expectations) const;

    enum ComparisonExpecation {
        kAllDifferent_ComparisonExpecation,
        kSameUpToPE_ComparisonExpecation,
        kSameUpToStroke_ComparisonExpecation,
        kAllSame_ComparisonExpecation,
    };

    void compare(skiatest::Reporter*, const TestCase& that, ComparisonExpecation) const;

    const GrStyledShape& baseShape() const { return *fBase; }
    const GrStyledShape& appliedPathEffectShape() const { return *fAppliedPE; }
    const GrStyledShape& appliedFullStyleShape() const { return *fAppliedFull; }

    // The returned array's count will be 0 if the key shape has no key.
    const Key& baseKey() const { return fBaseKey; }
    const Key& appliedPathEffectKey() const { return fAppliedPEKey; }
    const Key& appliedFullStyleKey() const { return fAppliedFullKey; }
    const Key& appliedPathEffectThenStrokeKey() const { return fAppliedPEThenStrokeKey; }

private:
    static void CheckBounds(skiatest::Reporter* r, const GrStyledShape& shape,
                            const SkRect& bounds) {
        SkPath path;
        shape.asPath(&path);
        // If the bounds are empty, the path ought to be as well.
        if (bounds.fLeft > bounds.fRight || bounds.fTop > bounds.fBottom) {
            REPORTER_ASSERT(r, path.isEmpty());
            return;
        }
        if (path.isEmpty()) {
            return;
        }
        // The bounds API explicitly calls out that it does not consider inverseness.
        SkPath p = path;
        p.setFillType(SkPathFillType_ConvertToNonInverse(path.getFillType()));
        REPORTER_ASSERT(r, test_bounds_by_rasterizing(p, bounds));
    }

    void init(skiatest::Reporter* r, SkScalar scale) {
        fAppliedPE = std::make_unique<GrStyledShape>();
        fAppliedPEThenStroke = std::make_unique<GrStyledShape>();
        fAppliedFull = std::make_unique<GrStyledShape>();

        *fAppliedPE = fBase->applyStyle(GrStyle::Apply::kPathEffectOnly, scale);
        *fAppliedPEThenStroke =
                fAppliedPE->applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, scale);
        *fAppliedFull = fBase->applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, scale);

        make_key(&fBaseKey, *fBase);
        make_key(&fAppliedPEKey, *fAppliedPE);
        make_key(&fAppliedPEThenStrokeKey, *fAppliedPEThenStroke);
        make_key(&fAppliedFullKey, *fAppliedFull);

        // All shapes should report the same "original" path, so that path renderers can get to it
        // if necessary.
        check_original_path_ids(r, *fBase, *fAppliedPE, *fAppliedPEThenStroke, *fAppliedFull);

        // Applying the path effect and then the stroke should always be the same as applying
        // both in one go.
        REPORTER_ASSERT(r, fAppliedPEThenStrokeKey == fAppliedFullKey);
        SkPath a, b;
        fAppliedPEThenStroke->asPath(&a);
        fAppliedFull->asPath(&b);
        // If the output of the path effect is a rrect then it is possible for a and b to be
        // different paths that fill identically. The reason is that fAppliedFull will do this:
        // base -> apply path effect -> rrect_as_path -> stroke -> stroked_rrect_as_path
        // fAppliedPEThenStroke will have converted the rrect_as_path back to a rrect. However,
        // now that there is no longer a path effect, the direction and starting index get
        // canonicalized before the stroke.
        if (fAppliedPE->asRRect(nullptr, nullptr, nullptr, nullptr)) {
            REPORTER_ASSERT(r, paths_fill_same(a, b));
        } else {
            REPORTER_ASSERT(r, a == b);
        }
        REPORTER_ASSERT(r, fAppliedFull->isEmpty() == fAppliedPEThenStroke->isEmpty());

        SkPath path;
        fBase->asPath(&path);
        REPORTER_ASSERT(r, path.isEmpty() == fBase->isEmpty());
        REPORTER_ASSERT(r, path.getSegmentMasks() == fBase->segmentMask());
        fAppliedPE->asPath(&path);
        REPORTER_ASSERT(r, path.isEmpty() == fAppliedPE->isEmpty());
        REPORTER_ASSERT(r, path.getSegmentMasks() == fAppliedPE->segmentMask());
        fAppliedFull->asPath(&path);
        REPORTER_ASSERT(r, path.isEmpty() == fAppliedFull->isEmpty());
        REPORTER_ASSERT(r, path.getSegmentMasks() == fAppliedFull->segmentMask());

        CheckBounds(r, *fBase, fBase->bounds());
        CheckBounds(r, *fAppliedPE, fAppliedPE->bounds());
        CheckBounds(r, *fAppliedPEThenStroke, fAppliedPEThenStroke->bounds());
        CheckBounds(r, *fAppliedFull, fAppliedFull->bounds());
        SkRect styledBounds = fBase->styledBounds();
        CheckBounds(r, *fAppliedFull, styledBounds);
        styledBounds = fAppliedPE->styledBounds();
        CheckBounds(r, *fAppliedFull, styledBounds);

        // Check that the same path is produced when style is applied by GrStyledShape and GrStyle.
        SkPath preStyle;
        SkPath postPathEffect;
        SkPath postAllStyle;

        fBase->asPath(&preStyle);
        SkStrokeRec postPEStrokeRec(SkStrokeRec::kFill_InitStyle);
        if (fBase->style().applyPathEffectToPath(&postPathEffect, &postPEStrokeRec, preStyle,
                                                 scale)) {
            // run postPathEffect through GrStyledShape to get any geometry reductions that would
            // have occurred to fAppliedPE.
            GrStyledShape(postPathEffect, GrStyle(postPEStrokeRec, nullptr))
                    .asPath(&postPathEffect);

            SkPath testPath;
            fAppliedPE->asPath(&testPath);
            REPORTER_ASSERT(r, testPath == postPathEffect);
            REPORTER_ASSERT(r, postPEStrokeRec.hasEqualEffect(fAppliedPE->style().strokeRec()));
        }
        SkStrokeRec::InitStyle fillOrHairline;
        if (fBase->style().applyToPath(&postAllStyle, &fillOrHairline, preStyle, scale)) {
            SkPath testPath;
            fAppliedFull->asPath(&testPath);
            if (fBase->style().hasPathEffect()) {
                // Because GrStyledShape always does two-stage application when there is a path
                // effect there may be a reduction/canonicalization step between the path effect and
                // strokerec not reflected in postAllStyle since it applied both the path effect
                // and strokerec without analyzing the intermediate path.
                REPORTER_ASSERT(r, paths_fill_same(postAllStyle, testPath));
            } else {
                // Make sure that postAllStyle sees any reductions/canonicalizations that
                // GrStyledShape would apply.
                GrStyledShape(postAllStyle, GrStyle(fillOrHairline)).asPath(&postAllStyle);
                REPORTER_ASSERT(r, testPath == postAllStyle);
            }

            if (fillOrHairline == SkStrokeRec::kFill_InitStyle) {
                REPORTER_ASSERT(r, fAppliedFull->style().isSimpleFill());
            } else {
                REPORTER_ASSERT(r, fAppliedFull->style().isSimpleHairline());
            }
        }
        test_inversions(r, *fBase, fBaseKey);
        test_inversions(r, *fAppliedPE, fAppliedPEKey);
        test_inversions(r, *fAppliedFull, fAppliedFullKey);
    }

    std::unique_ptr<GrStyledShape> fBase;
    std::unique_ptr<GrStyledShape> fAppliedPE;
    std::unique_ptr<GrStyledShape> fAppliedPEThenStroke;
    std::unique_ptr<GrStyledShape> fAppliedFull;

    Key fBaseKey;
    Key fAppliedPEKey;
    Key fAppliedPEThenStrokeKey;
    Key fAppliedFullKey;
};

void TestCase::testExpectations(skiatest::Reporter* reporter, SelfExpectations expectations) const {
    // The base's key should always be valid (unless the path is volatile)
    REPORTER_ASSERT(reporter, fBaseKey.count());
    if (expectations.fPEHasEffect) {
        REPORTER_ASSERT(reporter, fBaseKey != fAppliedPEKey);
        REPORTER_ASSERT(reporter, expectations.fPEHasValidKey == SkToBool(fAppliedPEKey.count()));
        REPORTER_ASSERT(reporter, fBaseKey != fAppliedFullKey);
        REPORTER_ASSERT(reporter, expectations.fPEHasValidKey == SkToBool(fAppliedFullKey.count()));
        if (expectations.fStrokeApplies && expectations.fPEHasValidKey) {
            REPORTER_ASSERT(reporter, fAppliedPEKey != fAppliedFullKey);
            REPORTER_ASSERT(reporter, SkToBool(fAppliedFullKey.count()));
        }
    } else {
        REPORTER_ASSERT(reporter, fBaseKey == fAppliedPEKey);
        SkPath a, b;
        fBase->asPath(&a);
        fAppliedPE->asPath(&b);
        REPORTER_ASSERT(reporter, a == b);
        if (expectations.fStrokeApplies) {
            REPORTER_ASSERT(reporter, fBaseKey != fAppliedFullKey);
        } else {
            REPORTER_ASSERT(reporter, fBaseKey == fAppliedFullKey);
        }
    }
}

void TestCase::compare(skiatest::Reporter* r, const TestCase& that,
                       ComparisonExpecation expectation) const {
    SkPath a, b;
    switch (expectation) {
        case kAllDifferent_ComparisonExpecation:
            REPORTER_ASSERT(r, fBaseKey != that.fBaseKey);
            REPORTER_ASSERT(r, fAppliedPEKey != that.fAppliedPEKey);
            REPORTER_ASSERT(r, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kSameUpToPE_ComparisonExpecation:
            check_equivalence(r, *fBase, *that.fBase, fBaseKey, that.fBaseKey);
            REPORTER_ASSERT(r, fAppliedPEKey != that.fAppliedPEKey);
            REPORTER_ASSERT(r, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kSameUpToStroke_ComparisonExpecation:
            check_equivalence(r, *fBase, *that.fBase, fBaseKey, that.fBaseKey);
            check_equivalence(r, *fAppliedPE, *that.fAppliedPE, fAppliedPEKey, that.fAppliedPEKey);
            REPORTER_ASSERT(r, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kAllSame_ComparisonExpecation:
            check_equivalence(r, *fBase, *that.fBase, fBaseKey, that.fBaseKey);
            check_equivalence(r, *fAppliedPE, *that.fAppliedPE, fAppliedPEKey, that.fAppliedPEKey);
            check_equivalence(r, *fAppliedFull, *that.fAppliedFull, fAppliedFullKey,
                              that.fAppliedFullKey);
            break;
    }
}
}  // namespace

static sk_sp<SkPathEffect> make_dash() {
    static const SkScalar kIntervals[] = { 0.25, 3.f, 0.5, 2.f };
    static const SkScalar kPhase = 0.75;
    return SkDashPathEffect::Make(kIntervals, SK_ARRAY_COUNT(kIntervals), kPhase);
}

static sk_sp<SkPathEffect> make_null_dash() {
    static const SkScalar kNullIntervals[] = {0, 0, 0, 0, 0, 0};
    return SkDashPathEffect::Make(kNullIntervals, SK_ARRAY_COUNT(kNullIntervals), 0.f);
}

// We make enough TestCases, and they're large enough, that on Google3 builds we exceed
// the maximum stack frame limit.  make_TestCase() moves those temporaries over to the heap.
template <typename... Args>
static std::unique_ptr<TestCase> make_TestCase(Args&&... args) {
    return std::make_unique<TestCase>( std::forward<Args>(args)... );
}

static void test_basic(skiatest::Reporter* reporter, const Geo& geo) {
    sk_sp<SkPathEffect> dashPE = make_dash();

    TestCase::SelfExpectations expectations;
    SkPaint fill;

    TestCase fillCase(geo, fill, reporter);
    expectations.fPEHasEffect = false;
    expectations.fPEHasValidKey = false;
    expectations.fStrokeApplies = false;
    fillCase.testExpectations(reporter, expectations);
    // Test that another GrStyledShape instance built from the same primitive is the same.
    make_TestCase(geo, fill, reporter)
        ->compare(reporter, fillCase, TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke2RoundBevel;
    stroke2RoundBevel.setStyle(SkPaint::kStroke_Style);
    stroke2RoundBevel.setStrokeCap(SkPaint::kRound_Cap);
    stroke2RoundBevel.setStrokeJoin(SkPaint::kBevel_Join);
    stroke2RoundBevel.setStrokeWidth(2.f);
    TestCase stroke2RoundBevelCase(geo, stroke2RoundBevel, reporter);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = false;
    expectations.fStrokeApplies = !geo.strokeIsConvertedToFill();
    stroke2RoundBevelCase.testExpectations(reporter, expectations);
    make_TestCase(geo, stroke2RoundBevel, reporter)
        ->compare(reporter, stroke2RoundBevelCase, TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke2RoundBevelDash = stroke2RoundBevel;
    stroke2RoundBevelDash.setPathEffect(make_dash());
    TestCase stroke2RoundBevelDashCase(geo, stroke2RoundBevelDash, reporter);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = true;
    expectations.fStrokeApplies = true;
    stroke2RoundBevelDashCase.testExpectations(reporter, expectations);
    make_TestCase(geo, stroke2RoundBevelDash, reporter)
        ->compare(reporter, stroke2RoundBevelDashCase, TestCase::kAllSame_ComparisonExpecation);

    if (geo.fillChangesGeom() || geo.strokeIsConvertedToFill()) {
        fillCase.compare(reporter, stroke2RoundBevelCase,
                         TestCase::kAllDifferent_ComparisonExpecation);
        fillCase.compare(reporter, stroke2RoundBevelDashCase,
                         TestCase::kAllDifferent_ComparisonExpecation);
    } else {
        fillCase.compare(reporter, stroke2RoundBevelCase,
                         TestCase::kSameUpToStroke_ComparisonExpecation);
        fillCase.compare(reporter, stroke2RoundBevelDashCase,
                         TestCase::kSameUpToPE_ComparisonExpecation);
    }
    if (geo.strokeIsConvertedToFill()) {
        stroke2RoundBevelCase.compare(reporter, stroke2RoundBevelDashCase,
                                      TestCase::kAllDifferent_ComparisonExpecation);
    } else {
        stroke2RoundBevelCase.compare(reporter, stroke2RoundBevelDashCase,
                                      TestCase::kSameUpToPE_ComparisonExpecation);
    }

    // Stroke and fill cases
    SkPaint stroke2RoundBevelAndFill = stroke2RoundBevel;
    stroke2RoundBevelAndFill.setStyle(SkPaint::kStrokeAndFill_Style);
    TestCase stroke2RoundBevelAndFillCase(geo, stroke2RoundBevelAndFill, reporter);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = false;
    expectations.fStrokeApplies = !geo.strokeIsConvertedToFill();
    stroke2RoundBevelAndFillCase.testExpectations(reporter, expectations);
    make_TestCase(geo, stroke2RoundBevelAndFill, reporter)->compare(
            reporter, stroke2RoundBevelAndFillCase, TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke2RoundBevelAndFillDash = stroke2RoundBevelDash;
    stroke2RoundBevelAndFillDash.setStyle(SkPaint::kStrokeAndFill_Style);
    TestCase stroke2RoundBevelAndFillDashCase(geo, stroke2RoundBevelAndFillDash, reporter);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = false;
    expectations.fStrokeApplies = !geo.strokeIsConvertedToFill();
    stroke2RoundBevelAndFillDashCase.testExpectations(reporter, expectations);
    make_TestCase(geo, stroke2RoundBevelAndFillDash, reporter)->compare(
        reporter, stroke2RoundBevelAndFillDashCase, TestCase::kAllSame_ComparisonExpecation);
    stroke2RoundBevelAndFillDashCase.compare(reporter, stroke2RoundBevelAndFillCase,
                                             TestCase::kAllSame_ComparisonExpecation);

    SkPaint hairline;
    hairline.setStyle(SkPaint::kStroke_Style);
    hairline.setStrokeWidth(0.f);
    TestCase hairlineCase(geo, hairline, reporter);
    // Since hairline style doesn't change the SkPath data, it is keyed identically to fill (except
    // in the line and unclosed rect cases).
    if (geo.fillChangesGeom()) {
        hairlineCase.compare(reporter, fillCase, TestCase::kAllDifferent_ComparisonExpecation);
    } else {
        hairlineCase.compare(reporter, fillCase, TestCase::kAllSame_ComparisonExpecation);
    }
    REPORTER_ASSERT(reporter, hairlineCase.baseShape().style().isSimpleHairline());
    REPORTER_ASSERT(reporter, hairlineCase.appliedFullStyleShape().style().isSimpleHairline());
    REPORTER_ASSERT(reporter, hairlineCase.appliedPathEffectShape().style().isSimpleHairline());

}

static void test_scale(skiatest::Reporter* reporter, const Geo& geo) {
    sk_sp<SkPathEffect> dashPE = make_dash();

    static const SkScalar kS1 = 1.f;
    static const SkScalar kS2 = 2.f;

    SkPaint fill;
    TestCase fillCase1(geo, fill, reporter, kS1);
    TestCase fillCase2(geo, fill, reporter, kS2);
    // Scale doesn't affect fills.
    fillCase1.compare(reporter, fillCase2, TestCase::kAllSame_ComparisonExpecation);

    SkPaint hairline;
    hairline.setStyle(SkPaint::kStroke_Style);
    hairline.setStrokeWidth(0.f);
    TestCase hairlineCase1(geo, hairline, reporter, kS1);
    TestCase hairlineCase2(geo, hairline, reporter, kS2);
    // Scale doesn't affect hairlines.
    hairlineCase1.compare(reporter, hairlineCase2, TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(2.f);
    TestCase strokeCase1(geo, stroke, reporter, kS1);
    TestCase strokeCase2(geo, stroke, reporter, kS2);
    // Scale affects the stroke
    if (geo.strokeIsConvertedToFill()) {
        REPORTER_ASSERT(reporter, !strokeCase1.baseShape().style().applies());
        strokeCase1.compare(reporter, strokeCase2, TestCase::kAllSame_ComparisonExpecation);
    } else {
        strokeCase1.compare(reporter, strokeCase2, TestCase::kSameUpToStroke_ComparisonExpecation);
    }

    SkPaint strokeDash = stroke;
    strokeDash.setPathEffect(make_dash());
    TestCase strokeDashCase1(geo, strokeDash, reporter, kS1);
    TestCase strokeDashCase2(geo, strokeDash, reporter, kS2);
    // Scale affects the dash and the stroke.
    strokeDashCase1.compare(reporter, strokeDashCase2,
                            TestCase::kSameUpToPE_ComparisonExpecation);

    // Stroke and fill cases
    SkPaint strokeAndFill = stroke;
    strokeAndFill.setStyle(SkPaint::kStrokeAndFill_Style);
    TestCase strokeAndFillCase1(geo, strokeAndFill, reporter, kS1);
    TestCase strokeAndFillCase2(geo, strokeAndFill, reporter, kS2);
    SkPaint strokeAndFillDash = strokeDash;
    strokeAndFillDash.setStyle(SkPaint::kStrokeAndFill_Style);
    // Dash is ignored for stroke and fill
    TestCase strokeAndFillDashCase1(geo, strokeAndFillDash, reporter, kS1);
    TestCase strokeAndFillDashCase2(geo, strokeAndFillDash, reporter, kS2);
    // Scale affects the stroke, but check to make sure this didn't become a simpler shape (e.g.
    // stroke-and-filled rect can become a rect), in which case the scale shouldn't matter and the
    // geometries should agree.
    if (geo.strokeAndFillIsConvertedToFill(strokeAndFillDash)) {
        REPORTER_ASSERT(reporter, !strokeAndFillCase1.baseShape().style().applies());
        strokeAndFillCase1.compare(reporter, strokeAndFillCase2,
                                   TestCase::kAllSame_ComparisonExpecation);
        strokeAndFillDashCase1.compare(reporter, strokeAndFillDashCase2,
                                       TestCase::kAllSame_ComparisonExpecation);
    } else {
        strokeAndFillCase1.compare(reporter, strokeAndFillCase2,
                                   TestCase::kSameUpToStroke_ComparisonExpecation);
    }
    strokeAndFillDashCase1.compare(reporter, strokeAndFillCase1,
                                   TestCase::kAllSame_ComparisonExpecation);
    strokeAndFillDashCase2.compare(reporter, strokeAndFillCase2,
                                   TestCase::kAllSame_ComparisonExpecation);
}

template <typename T>
static void test_stroke_param_impl(skiatest::Reporter* reporter, const Geo& geo,
                                   std::function<void(SkPaint*, T)> setter, T a, T b,
                                   bool paramAffectsStroke,
                                   bool paramAffectsDashAndStroke) {
    // Set the stroke width so that we don't get hairline. However, call the setter afterward so
    // that it can override the stroke width.
    SkPaint strokeA;
    strokeA.setStyle(SkPaint::kStroke_Style);
    strokeA.setStrokeWidth(2.f);
    setter(&strokeA, a);
    SkPaint strokeB;
    strokeB.setStyle(SkPaint::kStroke_Style);
    strokeB.setStrokeWidth(2.f);
    setter(&strokeB, b);

    TestCase strokeACase(geo, strokeA, reporter);
    TestCase strokeBCase(geo, strokeB, reporter);
    if (paramAffectsStroke) {
        // If stroking is immediately incorporated into a geometric transformation then the base
        // shapes will differ.
        if (geo.strokeIsConvertedToFill()) {
            strokeACase.compare(reporter, strokeBCase,
                                TestCase::kAllDifferent_ComparisonExpecation);
        } else {
            strokeACase.compare(reporter, strokeBCase,
                                TestCase::kSameUpToStroke_ComparisonExpecation);
        }
    } else {
        strokeACase.compare(reporter, strokeBCase, TestCase::kAllSame_ComparisonExpecation);
    }

    SkPaint strokeAndFillA = strokeA;
    SkPaint strokeAndFillB = strokeB;
    strokeAndFillA.setStyle(SkPaint::kStrokeAndFill_Style);
    strokeAndFillB.setStyle(SkPaint::kStrokeAndFill_Style);
    TestCase strokeAndFillACase(geo, strokeAndFillA, reporter);
    TestCase strokeAndFillBCase(geo, strokeAndFillB, reporter);
    if (paramAffectsStroke) {
        // If stroking is immediately incorporated into a geometric transformation then the base
        // shapes will differ.
        if (geo.strokeAndFillIsConvertedToFill(strokeAndFillA) ||
            geo.strokeAndFillIsConvertedToFill(strokeAndFillB)) {
            strokeAndFillACase.compare(reporter, strokeAndFillBCase,
                                       TestCase::kAllDifferent_ComparisonExpecation);
        } else {
            strokeAndFillACase.compare(reporter, strokeAndFillBCase,
                                       TestCase::kSameUpToStroke_ComparisonExpecation);
        }
    } else {
        strokeAndFillACase.compare(reporter, strokeAndFillBCase,
                                   TestCase::kAllSame_ComparisonExpecation);
    }

    // Make sure stroking params don't affect fill style.
    SkPaint fillA = strokeA, fillB = strokeB;
    fillA.setStyle(SkPaint::kFill_Style);
    fillB.setStyle(SkPaint::kFill_Style);
    TestCase fillACase(geo, fillA, reporter);
    TestCase fillBCase(geo, fillB, reporter);
    fillACase.compare(reporter, fillBCase, TestCase::kAllSame_ComparisonExpecation);

    // Make sure just applying the dash but not stroke gives the same key for both stroking
    // variations.
    SkPaint dashA = strokeA, dashB = strokeB;
    dashA.setPathEffect(make_dash());
    dashB.setPathEffect(make_dash());
    TestCase dashACase(geo, dashA, reporter);
    TestCase dashBCase(geo, dashB, reporter);
    if (paramAffectsDashAndStroke) {
        dashACase.compare(reporter, dashBCase, TestCase::kSameUpToStroke_ComparisonExpecation);
    } else {
        dashACase.compare(reporter, dashBCase, TestCase::kAllSame_ComparisonExpecation);
    }
}

template <typename T>
static void test_stroke_param(skiatest::Reporter* reporter, const Geo& geo,
                              std::function<void(SkPaint*, T)> setter, T a, T b) {
    test_stroke_param_impl(reporter, geo, setter, a, b, true, true);
};

static void test_stroke_cap(skiatest::Reporter* reporter, const Geo& geo) {
    SkPaint hairline;
    hairline.setStrokeWidth(0);
    hairline.setStyle(SkPaint::kStroke_Style);
    GrStyledShape shape = geo.makeShape(hairline);
    // The cap should only affect shapes that may be open.
    bool affectsStroke = !shape.knownToBeClosed();
    // Dashing adds ends that need caps.
    bool affectsDashAndStroke = true;
    test_stroke_param_impl<SkPaint::Cap>(
        reporter,
        geo,
        [](SkPaint* p, SkPaint::Cap c) { p->setStrokeCap(c);},
        SkPaint::kButt_Cap, SkPaint::kRound_Cap,
        affectsStroke,
        affectsDashAndStroke);
};

static bool shape_known_not_to_have_joins(const GrStyledShape& shape) {
    return shape.asLine(nullptr, nullptr) || shape.isEmpty();
}

static void test_stroke_join(skiatest::Reporter* reporter, const Geo& geo) {
    SkPaint hairline;
    hairline.setStrokeWidth(0);
    hairline.setStyle(SkPaint::kStroke_Style);
    GrStyledShape shape = geo.makeShape(hairline);
    // GrStyledShape recognizes certain types don't have joins and will prevent the join type from
    // affecting the style key.
    // Dashing doesn't add additional joins. However, GrStyledShape currently loses track of this
    // after applying the dash.
    bool affectsStroke = !shape_known_not_to_have_joins(shape);
    test_stroke_param_impl<SkPaint::Join>(
            reporter,
            geo,
            [](SkPaint* p, SkPaint::Join j) { p->setStrokeJoin(j);},
            SkPaint::kRound_Join, SkPaint::kBevel_Join,
            affectsStroke, true);
};

static void test_miter_limit(skiatest::Reporter* reporter, const Geo& geo) {
    auto setMiterJoinAndLimit = [](SkPaint* p, SkScalar miter) {
        p->setStrokeJoin(SkPaint::kMiter_Join);
        p->setStrokeMiter(miter);
    };

    auto setOtherJoinAndLimit = [](SkPaint* p, SkScalar miter) {
        p->setStrokeJoin(SkPaint::kRound_Join);
        p->setStrokeMiter(miter);
    };

    SkPaint hairline;
    hairline.setStrokeWidth(0);
    hairline.setStyle(SkPaint::kStroke_Style);
    GrStyledShape shape = geo.makeShape(hairline);
    bool mayHaveJoins = !shape_known_not_to_have_joins(shape);

    // The miter limit should affect stroked and dashed-stroked cases when the join type is
    // miter.
    test_stroke_param_impl<SkScalar>(
        reporter,
        geo,
        setMiterJoinAndLimit,
        0.5f, 0.75f,
        mayHaveJoins,
        true);

    // The miter limit should not affect stroked and dashed-stroked cases when the join type is
    // not miter.
    test_stroke_param_impl<SkScalar>(
        reporter,
        geo,
        setOtherJoinAndLimit,
        0.5f, 0.75f,
        false,
        false);
}

static void test_dash_fill(skiatest::Reporter* reporter, const Geo& geo) {
    // A dash with no stroke should have no effect
    using DashFactoryFn = sk_sp<SkPathEffect>(*)();
    for (DashFactoryFn md : {&make_dash, &make_null_dash}) {
        SkPaint dashFill;
        dashFill.setPathEffect((*md)());
        TestCase dashFillCase(geo, dashFill, reporter);

        TestCase fillCase(geo, SkPaint(), reporter);
        dashFillCase.compare(reporter, fillCase, TestCase::kAllSame_ComparisonExpecation);
    }
}

void test_null_dash(skiatest::Reporter* reporter, const Geo& geo) {
    SkPaint fill;
    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(1.f);
    SkPaint dash;
    dash.setStyle(SkPaint::kStroke_Style);
    dash.setStrokeWidth(1.f);
    dash.setPathEffect(make_dash());
    SkPaint nullDash;
    nullDash.setStyle(SkPaint::kStroke_Style);
    nullDash.setStrokeWidth(1.f);
    nullDash.setPathEffect(make_null_dash());

    TestCase fillCase(geo, fill, reporter);
    TestCase strokeCase(geo, stroke, reporter);
    TestCase dashCase(geo, dash, reporter);
    TestCase nullDashCase(geo, nullDash, reporter);

    // We expect the null dash to be ignored so nullDashCase should match strokeCase, always.
    nullDashCase.compare(reporter, strokeCase, TestCase::kAllSame_ComparisonExpecation);
    // Check whether the fillCase or strokeCase/nullDashCase would undergo a geometric tranformation
    // on construction in order to determine how to compare the fill and stroke.
    if (geo.fillChangesGeom() || geo.strokeIsConvertedToFill()) {
        nullDashCase.compare(reporter, fillCase, TestCase::kAllDifferent_ComparisonExpecation);
    } else {
        nullDashCase.compare(reporter, fillCase, TestCase::kSameUpToStroke_ComparisonExpecation);
    }
    // In the null dash case we may immediately convert to a fill, but not for the normal dash case.
    if (geo.strokeIsConvertedToFill()) {
        nullDashCase.compare(reporter, dashCase, TestCase::kAllDifferent_ComparisonExpecation);
    } else {
        nullDashCase.compare(reporter, dashCase, TestCase::kSameUpToPE_ComparisonExpecation);
    }
}

void test_path_effect_makes_rrect(skiatest::Reporter* reporter, const Geo& geo) {
    /**
     * This path effect takes any input path and turns it into a rrect. It passes through stroke
     * info.
     */
    class RRectPathEffect : SkPathEffectBase {
    public:
        static const SkRRect& RRect() {
            static const SkRRect kRRect = SkRRect::MakeRectXY(SkRect::MakeWH(12, 12), 3, 5);
            return kRRect;
        }

        static sk_sp<SkPathEffect> Make() { return sk_sp<SkPathEffect>(new RRectPathEffect); }
        Factory getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return nullptr; }

    protected:
        bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*,
                          const SkRect* cullR, const SkMatrix&) const override {
            dst->reset();
            dst->addRRect(RRect());
            return true;
        }

        bool computeFastBounds(SkRect* bounds) const override {
            if (bounds) {
                *bounds = RRect().getBounds();
            }
            return true;
        }

    private:
        RRectPathEffect() {}
    };

    SkPaint fill;
    TestCase fillGeoCase(geo, fill, reporter);

    SkPaint pe;
    pe.setPathEffect(RRectPathEffect::Make());
    TestCase geoPECase(geo, pe, reporter);

    SkPaint peStroke;
    peStroke.setPathEffect(RRectPathEffect::Make());
    peStroke.setStrokeWidth(2.f);
    peStroke.setStyle(SkPaint::kStroke_Style);
    TestCase geoPEStrokeCase(geo, peStroke, reporter);

    // Check whether constructing the filled case would cause the base shape to have a different
    // geometry (because of a geometric transformation upon initial GrStyledShape construction).
    if (geo.fillChangesGeom()) {
        fillGeoCase.compare(reporter, geoPECase, TestCase::kAllDifferent_ComparisonExpecation);
        fillGeoCase.compare(reporter, geoPEStrokeCase,
                            TestCase::kAllDifferent_ComparisonExpecation);
    } else {
        fillGeoCase.compare(reporter, geoPECase, TestCase::kSameUpToPE_ComparisonExpecation);
        fillGeoCase.compare(reporter, geoPEStrokeCase, TestCase::kSameUpToPE_ComparisonExpecation);
    }
    geoPECase.compare(reporter, geoPEStrokeCase,
                      TestCase::kSameUpToStroke_ComparisonExpecation);

    TestCase rrectFillCase(reporter, RRectPathEffect::RRect(), fill);
    SkPaint stroke = peStroke;
    stroke.setPathEffect(nullptr);
    TestCase rrectStrokeCase(reporter, RRectPathEffect::RRect(), stroke);

    SkRRect rrect;
    // Applying the path effect should make a SkRRect shape. There is no further stroking in the
    // geoPECase, so the full style should be the same as just the PE.
    REPORTER_ASSERT(reporter, geoPECase.appliedPathEffectShape().asRRect(&rrect, nullptr, nullptr,
                                                                         nullptr));
    REPORTER_ASSERT(reporter, rrect == RRectPathEffect::RRect());
    REPORTER_ASSERT(reporter, geoPECase.appliedPathEffectKey() == rrectFillCase.baseKey());

    REPORTER_ASSERT(reporter, geoPECase.appliedFullStyleShape().asRRect(&rrect, nullptr, nullptr,
                                                                        nullptr));
    REPORTER_ASSERT(reporter, rrect == RRectPathEffect::RRect());
    REPORTER_ASSERT(reporter, geoPECase.appliedFullStyleKey() == rrectFillCase.baseKey());

    // In the PE+stroke case applying the full style should be the same as just stroking the rrect.
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectShape().asRRect(&rrect, nullptr,
                                                                               nullptr, nullptr));
    REPORTER_ASSERT(reporter, rrect == RRectPathEffect::RRect());
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectKey() == rrectFillCase.baseKey());

    REPORTER_ASSERT(reporter, !geoPEStrokeCase.appliedFullStyleShape().asRRect(&rrect, nullptr,
                                                                               nullptr, nullptr));
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedFullStyleKey() ==
                              rrectStrokeCase.appliedFullStyleKey());
}

void test_unknown_path_effect(skiatest::Reporter* reporter, const Geo& geo) {
    /**
     * This path effect just adds two lineTos to the input path.
     */
    class AddLineTosPathEffect : SkPathEffectBase {
    public:
        static sk_sp<SkPathEffect> Make() { return sk_sp<SkPathEffect>(new AddLineTosPathEffect); }
        Factory getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return nullptr; }

    protected:
        bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*,
                          const SkRect* cullR, const SkMatrix&) const override {
            *dst = src;
            // To avoid triggering data-based keying of paths with few verbs we add many segments.
            for (int i = 0; i < 100; ++i) {
                dst->lineTo(SkIntToScalar(i), SkIntToScalar(i));
            }
            return true;
        }
        bool computeFastBounds(SkRect* bounds) const override {
            if (bounds) {
                SkRectPriv::GrowToInclude(bounds, {0, 0});
                SkRectPriv::GrowToInclude(bounds, {100, 100});
            }
            return true;
        }
    private:
        AddLineTosPathEffect() {}
    };

     // This path effect should make the keys invalid when it is applied. We only produce a path
     // effect key for dash path effects. So the only way another arbitrary path effect can produce
     // a styled result with a key is to produce a non-path shape that has a purely geometric key.
    SkPaint peStroke;
    peStroke.setPathEffect(AddLineTosPathEffect::Make());
    peStroke.setStrokeWidth(2.f);
    peStroke.setStyle(SkPaint::kStroke_Style);
    TestCase geoPEStrokeCase(geo, peStroke, reporter);
    TestCase::SelfExpectations expectations;
    expectations.fPEHasEffect = true;
    expectations.fPEHasValidKey = false;
    expectations.fStrokeApplies = true;
    geoPEStrokeCase.testExpectations(reporter, expectations);
}

void test_make_hairline_path_effect(skiatest::Reporter* reporter, const Geo& geo) {
    /**
     * This path effect just changes the stroke rec to hairline.
     */
    class MakeHairlinePathEffect : SkPathEffectBase {
    public:
        static sk_sp<SkPathEffect> Make() {
            return sk_sp<SkPathEffect>(new MakeHairlinePathEffect);
        }
        Factory getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return nullptr; }

    protected:
        bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec* strokeRec,
                          const SkRect* cullR, const SkMatrix&) const override {
            *dst = src;
            strokeRec->setHairlineStyle();
            return true;
        }
    private:
        bool computeFastBounds(SkRect* bounds) const override { return true; }

        MakeHairlinePathEffect() {}
    };

    SkPaint fill;
    SkPaint pe;
    pe.setPathEffect(MakeHairlinePathEffect::Make());

    TestCase peCase(geo, pe, reporter);

    SkPath a, b, c;
    peCase.baseShape().asPath(&a);
    peCase.appliedPathEffectShape().asPath(&b);
    peCase.appliedFullStyleShape().asPath(&c);
    if (geo.isNonPath(pe)) {
        // RRect types can have a change in start index or direction after the PE is applied. This
        // is because once the PE is applied, GrStyledShape may canonicalize the dir and index since
        // it is not germane to the styling any longer.
        // Instead we just check that the paths would fill the same both before and after styling.
        REPORTER_ASSERT(reporter, paths_fill_same(a, b));
        REPORTER_ASSERT(reporter, paths_fill_same(a, c));
    } else {
        // The base shape cannot perform canonicalization on the path's fill type because of an
        // unknown path effect. However, after the path effect is applied the resulting hairline
        // shape will canonicalize the path fill type since hairlines (and stroking in general)
        // don't distinguish between even/odd and non-zero winding.
        a.setFillType(b.getFillType());
        REPORTER_ASSERT(reporter, a == b);
        REPORTER_ASSERT(reporter, a == c);
        // If the resulting path is small enough then it will have a key.
        REPORTER_ASSERT(reporter, paths_fill_same(a, b));
        REPORTER_ASSERT(reporter, paths_fill_same(a, c));
        REPORTER_ASSERT(reporter, peCase.appliedPathEffectKey().empty());
        REPORTER_ASSERT(reporter, peCase.appliedFullStyleKey().empty());
    }
    REPORTER_ASSERT(reporter, peCase.appliedPathEffectShape().style().isSimpleHairline());
    REPORTER_ASSERT(reporter, peCase.appliedFullStyleShape().style().isSimpleHairline());
}

void test_volatile_path(skiatest::Reporter* reporter, const Geo& geo) {
    SkPath vPath = geo.path();
    vPath.setIsVolatile(true);

    SkPaint dashAndStroke;
    dashAndStroke.setPathEffect(make_dash());
    dashAndStroke.setStrokeWidth(2.f);
    dashAndStroke.setStyle(SkPaint::kStroke_Style);
    TestCase volatileCase(reporter, vPath, dashAndStroke);
    // We expect a shape made from a volatile path to have a key iff the shape is recognized
    // as a specialized geometry.
    if (geo.isNonPath(dashAndStroke)) {
        REPORTER_ASSERT(reporter, SkToBool(volatileCase.baseKey().count()));
        // In this case all the keys should be identical to the non-volatile case.
        TestCase nonVolatileCase(reporter, geo.path(), dashAndStroke);
        volatileCase.compare(reporter, nonVolatileCase, TestCase::kAllSame_ComparisonExpecation);
    } else {
        // None of the keys should be valid.
        REPORTER_ASSERT(reporter, !SkToBool(volatileCase.baseKey().count()));
        REPORTER_ASSERT(reporter, !SkToBool(volatileCase.appliedPathEffectKey().count()));
        REPORTER_ASSERT(reporter, !SkToBool(volatileCase.appliedFullStyleKey().count()));
        REPORTER_ASSERT(reporter, !SkToBool(volatileCase.appliedPathEffectThenStrokeKey().count()));
    }
}

void test_path_effect_makes_empty_shape(skiatest::Reporter* reporter, const Geo& geo) {
    /**
     * This path effect returns an empty path (possibly inverted)
     */
    class EmptyPathEffect : SkPathEffectBase {
    public:
        static sk_sp<SkPathEffect> Make(bool invert) {
            return sk_sp<SkPathEffect>(new EmptyPathEffect(invert));
        }
        Factory getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return nullptr; }
    protected:
        bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*,
                          const SkRect* cullR, const SkMatrix&) const override {
            dst->reset();
            if (fInvert) {
                dst->toggleInverseFillType();
            }
            return true;
        }
        bool computeFastBounds(SkRect* bounds) const override {
            if (bounds) {
                *bounds = { 0, 0, 0, 0 };
            }
            return true;
        }
    private:
        bool fInvert;
        EmptyPathEffect(bool invert) : fInvert(invert) {}
    };

    SkPath emptyPath;
    GrStyledShape emptyShape(emptyPath);
    Key emptyKey;
    make_key(&emptyKey, emptyShape);
    REPORTER_ASSERT(reporter, emptyShape.isEmpty());

    emptyPath.toggleInverseFillType();
    GrStyledShape invertedEmptyShape(emptyPath);
    Key invertedEmptyKey;
    make_key(&invertedEmptyKey, invertedEmptyShape);
    REPORTER_ASSERT(reporter, invertedEmptyShape.isEmpty());

    REPORTER_ASSERT(reporter, invertedEmptyKey != emptyKey);

    SkPaint pe;
    pe.setPathEffect(EmptyPathEffect::Make(false));
    TestCase geoPECase(geo, pe, reporter);
    REPORTER_ASSERT(reporter, geoPECase.appliedFullStyleKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPECase.appliedPathEffectKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPECase.appliedPathEffectThenStrokeKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPECase.appliedPathEffectShape().isEmpty());
    REPORTER_ASSERT(reporter, geoPECase.appliedFullStyleShape().isEmpty());
    REPORTER_ASSERT(reporter, !geoPECase.appliedPathEffectShape().inverseFilled());
    REPORTER_ASSERT(reporter, !geoPECase.appliedFullStyleShape().inverseFilled());

    SkPaint peStroke;
    peStroke.setPathEffect(EmptyPathEffect::Make(false));
    peStroke.setStrokeWidth(2.f);
    peStroke.setStyle(SkPaint::kStroke_Style);
    TestCase geoPEStrokeCase(geo, peStroke, reporter);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedFullStyleKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectThenStrokeKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectShape().isEmpty());
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedFullStyleShape().isEmpty());
    REPORTER_ASSERT(reporter, !geoPEStrokeCase.appliedPathEffectShape().inverseFilled());
    REPORTER_ASSERT(reporter, !geoPEStrokeCase.appliedFullStyleShape().inverseFilled());
    pe.setPathEffect(EmptyPathEffect::Make(true));

    TestCase geoPEInvertCase(geo, pe, reporter);
    REPORTER_ASSERT(reporter, geoPEInvertCase.appliedFullStyleKey() == invertedEmptyKey);
    REPORTER_ASSERT(reporter, geoPEInvertCase.appliedPathEffectKey() == invertedEmptyKey);
    REPORTER_ASSERT(reporter, geoPEInvertCase.appliedPathEffectThenStrokeKey() == invertedEmptyKey);
    REPORTER_ASSERT(reporter, geoPEInvertCase.appliedPathEffectShape().isEmpty());
    REPORTER_ASSERT(reporter, geoPEInvertCase.appliedFullStyleShape().isEmpty());
    REPORTER_ASSERT(reporter, geoPEInvertCase.appliedPathEffectShape().inverseFilled());
    REPORTER_ASSERT(reporter, geoPEInvertCase.appliedFullStyleShape().inverseFilled());

    peStroke.setPathEffect(EmptyPathEffect::Make(true));
    TestCase geoPEInvertStrokeCase(geo, peStroke, reporter);
    REPORTER_ASSERT(reporter, geoPEInvertStrokeCase.appliedFullStyleKey() == invertedEmptyKey);
    REPORTER_ASSERT(reporter, geoPEInvertStrokeCase.appliedPathEffectKey() == invertedEmptyKey);
    REPORTER_ASSERT(reporter,
                    geoPEInvertStrokeCase.appliedPathEffectThenStrokeKey() == invertedEmptyKey);
    REPORTER_ASSERT(reporter, geoPEInvertStrokeCase.appliedPathEffectShape().isEmpty());
    REPORTER_ASSERT(reporter, geoPEInvertStrokeCase.appliedFullStyleShape().isEmpty());
    REPORTER_ASSERT(reporter, geoPEInvertStrokeCase.appliedPathEffectShape().inverseFilled());
    REPORTER_ASSERT(reporter, geoPEInvertStrokeCase.appliedFullStyleShape().inverseFilled());
}

void test_path_effect_fails(skiatest::Reporter* reporter, const Geo& geo) {
    /**
     * This path effect always fails to apply.
     */
    class FailurePathEffect : SkPathEffectBase {
    public:
        static sk_sp<SkPathEffect> Make() { return sk_sp<SkPathEffect>(new FailurePathEffect); }
        Factory getFactory() const override { return nullptr; }
        const char* getTypeName() const override { return nullptr; }
    protected:
        bool onFilterPath(SkPath* dst, const SkPath& src, SkStrokeRec*,
                          const SkRect* cullR, const SkMatrix&) const override {
            return false;
        }
    private:
        bool computeFastBounds(SkRect* bounds) const override { return false; }

        FailurePathEffect() {}
    };

    SkPaint fill;
    TestCase fillCase(geo, fill, reporter);

    SkPaint pe;
    pe.setPathEffect(FailurePathEffect::Make());
    TestCase peCase(geo, pe, reporter);

    SkPaint stroke;
    stroke.setStrokeWidth(2.f);
    stroke.setStyle(SkPaint::kStroke_Style);
    TestCase strokeCase(geo, stroke, reporter);

    SkPaint peStroke = stroke;
    peStroke.setPathEffect(FailurePathEffect::Make());
    TestCase peStrokeCase(geo, peStroke, reporter);

    // In general the path effect failure can cause some of the TestCase::compare() tests to fail
    // for at least two reasons: 1) We will initially treat the shape as unkeyable because of the
    // path effect, but then when the path effect fails we can key it. 2) GrStyledShape will change
    // its mind about whether a unclosed rect is actually rect. The path effect initially bars us
    // from closing it but after the effect fails we can (for the fill+pe case). This causes
    // different routes through GrStyledShape to have equivalent but different representations of
    // the path (closed or not) but that fill the same.
    SkPath a;
    SkPath b;
    fillCase.appliedPathEffectShape().asPath(&a);
    peCase.appliedPathEffectShape().asPath(&b);
    REPORTER_ASSERT(reporter, paths_fill_same(a, b));

    fillCase.appliedFullStyleShape().asPath(&a);
    peCase.appliedFullStyleShape().asPath(&b);
    REPORTER_ASSERT(reporter, paths_fill_same(a, b));

    strokeCase.appliedPathEffectShape().asPath(&a);
    peStrokeCase.appliedPathEffectShape().asPath(&b);
    REPORTER_ASSERT(reporter, paths_fill_same(a, b));

    strokeCase.appliedFullStyleShape().asPath(&a);
    peStrokeCase.appliedFullStyleShape().asPath(&b);
    REPORTER_ASSERT(reporter, paths_fill_same(a, b));
}

DEF_TEST(GrStyledShape_empty_shape, reporter) {
    SkPath emptyPath;
    SkPath invertedEmptyPath;
    invertedEmptyPath.toggleInverseFillType();
    SkPaint fill;
    TestCase fillEmptyCase(reporter, emptyPath, fill);
    REPORTER_ASSERT(reporter, fillEmptyCase.baseShape().isEmpty());
    REPORTER_ASSERT(reporter, fillEmptyCase.appliedPathEffectShape().isEmpty());
    REPORTER_ASSERT(reporter, fillEmptyCase.appliedFullStyleShape().isEmpty());
    REPORTER_ASSERT(reporter, !fillEmptyCase.baseShape().inverseFilled());
    REPORTER_ASSERT(reporter, !fillEmptyCase.appliedPathEffectShape().inverseFilled());
    REPORTER_ASSERT(reporter, !fillEmptyCase.appliedFullStyleShape().inverseFilled());
    TestCase fillInvertedEmptyCase(reporter, invertedEmptyPath, fill);
    REPORTER_ASSERT(reporter, fillInvertedEmptyCase.baseShape().isEmpty());
    REPORTER_ASSERT(reporter, fillInvertedEmptyCase.appliedPathEffectShape().isEmpty());
    REPORTER_ASSERT(reporter, fillInvertedEmptyCase.appliedFullStyleShape().isEmpty());
    REPORTER_ASSERT(reporter, fillInvertedEmptyCase.baseShape().inverseFilled());
    REPORTER_ASSERT(reporter, fillInvertedEmptyCase.appliedPathEffectShape().inverseFilled());
    REPORTER_ASSERT(reporter, fillInvertedEmptyCase.appliedFullStyleShape().inverseFilled());

    const Key& emptyKey = fillEmptyCase.baseKey();
    REPORTER_ASSERT(reporter, emptyKey.count());
    const Key& inverseEmptyKey = fillInvertedEmptyCase.baseKey();
    REPORTER_ASSERT(reporter, inverseEmptyKey.count());
    TestCase::SelfExpectations expectations;
    expectations.fStrokeApplies = false;
    expectations.fPEHasEffect = false;
    // This will test whether applying style preserves emptiness
    fillEmptyCase.testExpectations(reporter, expectations);
    fillInvertedEmptyCase.testExpectations(reporter, expectations);

    // Stroking an empty path should have no effect
    SkPaint stroke;
    stroke.setStrokeWidth(2.f);
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeJoin(SkPaint::kRound_Join);
    stroke.setStrokeCap(SkPaint::kRound_Cap);
    TestCase strokeEmptyCase(reporter, emptyPath, stroke);
    strokeEmptyCase.compare(reporter, fillEmptyCase, TestCase::kAllSame_ComparisonExpecation);
    TestCase strokeInvertedEmptyCase(reporter, invertedEmptyPath, stroke);
    strokeInvertedEmptyCase.compare(reporter, fillInvertedEmptyCase,
                                    TestCase::kAllSame_ComparisonExpecation);

    // Dashing and stroking an empty path should have no effect
    SkPaint dashAndStroke;
    dashAndStroke.setPathEffect(make_dash());
    dashAndStroke.setStrokeWidth(2.f);
    dashAndStroke.setStyle(SkPaint::kStroke_Style);
    TestCase dashAndStrokeEmptyCase(reporter, emptyPath, dashAndStroke);
    dashAndStrokeEmptyCase.compare(reporter, fillEmptyCase,
                                   TestCase::kAllSame_ComparisonExpecation);
    TestCase dashAndStrokeInvertexEmptyCase(reporter, invertedEmptyPath, dashAndStroke);
    // Dashing ignores inverseness so this is equivalent to the non-inverted empty fill.
    dashAndStrokeInvertexEmptyCase.compare(reporter, fillEmptyCase,
                                           TestCase::kAllSame_ComparisonExpecation);

    // A shape made from an empty rrect should behave the same as an empty path when filled and
    // when stroked. The shape is closed so it does not produce caps when stroked. When dashed there
    // is no path to dash along, making it equivalent as well.
    SkRRect emptyRRect = SkRRect::MakeEmpty();
    REPORTER_ASSERT(reporter, emptyRRect.getType() == SkRRect::kEmpty_Type);

    TestCase fillEmptyRRectCase(reporter, emptyRRect, fill);
    fillEmptyRRectCase.compare(reporter, fillEmptyCase, TestCase::kAllSame_ComparisonExpecation);

    TestCase strokeEmptyRRectCase(reporter, emptyRRect, stroke);
    strokeEmptyRRectCase.compare(reporter, strokeEmptyCase,
                                 TestCase::kAllSame_ComparisonExpecation);

    TestCase dashAndStrokeEmptyRRectCase(reporter, emptyRRect, dashAndStroke);
    dashAndStrokeEmptyRRectCase.compare(reporter, fillEmptyCase,
                                        TestCase::kAllSame_ComparisonExpecation);

    static constexpr SkPathDirection kDir = SkPathDirection::kCCW;
    static constexpr int kStart = 0;

    TestCase fillInvertedEmptyRRectCase(reporter, emptyRRect, kDir, kStart, true, GrStyle(fill));
    fillInvertedEmptyRRectCase.compare(reporter, fillInvertedEmptyCase,
                                       TestCase::kAllSame_ComparisonExpecation);

    TestCase strokeInvertedEmptyRRectCase(reporter, emptyRRect, kDir, kStart, true,
                                          GrStyle(stroke));
    strokeInvertedEmptyRRectCase.compare(reporter, strokeInvertedEmptyCase,
                                         TestCase::kAllSame_ComparisonExpecation);

    TestCase dashAndStrokeEmptyInvertedRRectCase(reporter, emptyRRect, kDir, kStart, true,
                                                 GrStyle(dashAndStroke));
    dashAndStrokeEmptyInvertedRRectCase.compare(reporter, fillEmptyCase,
                                                TestCase::kAllSame_ComparisonExpecation);

    // Same for a rect.
    SkRect emptyRect = SkRect::MakeEmpty();
    TestCase fillEmptyRectCase(reporter, emptyRect, fill);
    fillEmptyRectCase.compare(reporter, fillEmptyCase, TestCase::kAllSame_ComparisonExpecation);

    TestCase dashAndStrokeEmptyRectCase(reporter, emptyRect, dashAndStroke);
    dashAndStrokeEmptyRectCase.compare(reporter, fillEmptyCase,
                                       TestCase::kAllSame_ComparisonExpecation);

    TestCase dashAndStrokeEmptyInvertedRectCase(reporter, SkRRect::MakeRect(emptyRect), kDir,
                                                kStart, true, GrStyle(dashAndStroke));
    // Dashing ignores inverseness so this is equivalent to the non-inverted empty fill.
    dashAndStrokeEmptyInvertedRectCase.compare(reporter, fillEmptyCase,
                                               TestCase::kAllSame_ComparisonExpecation);
}

// rect and oval types have rrect start indices that collapse to the same point. Here we select the
// canonical point in these cases.
unsigned canonicalize_rrect_start(int s, const SkRRect& rrect) {
    switch (rrect.getType()) {
        case SkRRect::kRect_Type:
            return (s + 1) & 0b110;
        case SkRRect::kOval_Type:
            return s & 0b110;
        default:
            return s;
    }
}

void test_rrect(skiatest::Reporter* r, const SkRRect& rrect) {
    enum Style {
        kFill,
        kStroke,
        kHairline,
        kStrokeAndFill
    };

    // SkStrokeRec has no default cons., so init with kFill before calling the setters below.
    SkStrokeRec strokeRecs[4] { SkStrokeRec::kFill_InitStyle, SkStrokeRec::kFill_InitStyle,
                                SkStrokeRec::kFill_InitStyle, SkStrokeRec::kFill_InitStyle};
    strokeRecs[kFill].setFillStyle();
    strokeRecs[kStroke].setStrokeStyle(2.f);
    strokeRecs[kHairline].setHairlineStyle();
    strokeRecs[kStrokeAndFill].setStrokeStyle(3.f, true);
    // Use a bevel join to avoid complications of stroke+filled rects becoming filled rects before
    // applyStyle() is called.
    strokeRecs[kStrokeAndFill].setStrokeParams(SkPaint::kButt_Cap, SkPaint::kBevel_Join, 1.f);
    sk_sp<SkPathEffect> dashEffect = make_dash();

    static constexpr Style kStyleCnt = static_cast<Style>(SK_ARRAY_COUNT(strokeRecs));

    auto index = [](bool inverted,
                    SkPathDirection dir,
                    unsigned start,
                    Style style,
                    bool dash) -> int {
        return inverted * (2 * 8 * kStyleCnt * 2) +
               (int)dir * (    8 * kStyleCnt * 2) +
               start    * (        kStyleCnt * 2) +
               style    * (                    2) +
               dash;
    };
    static const SkPathDirection kSecondDirection = static_cast<SkPathDirection>(1);
    const int cnt = index(true, kSecondDirection, 7, static_cast<Style>(kStyleCnt - 1), true) + 1;
    SkAutoTArray<GrStyledShape> shapes(cnt);
    for (bool inverted : {false, true}) {
        for (SkPathDirection dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
            for (unsigned start = 0; start < 8; ++start) {
                for (Style style : {kFill, kStroke, kHairline, kStrokeAndFill}) {
                    for (bool dash : {false, true}) {
                        sk_sp<SkPathEffect> pe = dash ? dashEffect : nullptr;
                        shapes[index(inverted, dir, start, style, dash)] =
                                GrStyledShape(rrect, dir, start, SkToBool(inverted),
                                        GrStyle(strokeRecs[style], std::move(pe)));
                    }
                }
            }
        }
    }

    // Get the keys for some example shape instances that we'll use for comparision against the
    // rest.
    static constexpr SkPathDirection kExamplesDir = SkPathDirection::kCW;
    static constexpr unsigned kExamplesStart = 0;
    const GrStyledShape& exampleFillCase = shapes[index(false, kExamplesDir, kExamplesStart, kFill,
                                                  false)];
    Key exampleFillCaseKey;
    make_key(&exampleFillCaseKey, exampleFillCase);

    const GrStyledShape& exampleStrokeAndFillCase = shapes[index(false, kExamplesDir,
                                                           kExamplesStart, kStrokeAndFill, false)];
    Key exampleStrokeAndFillCaseKey;
    make_key(&exampleStrokeAndFillCaseKey, exampleStrokeAndFillCase);

    const GrStyledShape& exampleInvFillCase = shapes[index(true, kExamplesDir,
                                                     kExamplesStart, kFill, false)];
    Key exampleInvFillCaseKey;
    make_key(&exampleInvFillCaseKey, exampleInvFillCase);

    const GrStyledShape& exampleInvStrokeAndFillCase = shapes[index(true, kExamplesDir,
                                                              kExamplesStart, kStrokeAndFill,
                                                              false)];
    Key exampleInvStrokeAndFillCaseKey;
    make_key(&exampleInvStrokeAndFillCaseKey, exampleInvStrokeAndFillCase);

    const GrStyledShape& exampleStrokeCase = shapes[index(false, kExamplesDir, kExamplesStart,
                                                    kStroke, false)];
    Key exampleStrokeCaseKey;
    make_key(&exampleStrokeCaseKey, exampleStrokeCase);

    const GrStyledShape& exampleInvStrokeCase = shapes[index(true, kExamplesDir, kExamplesStart,
                                                       kStroke, false)];
    Key exampleInvStrokeCaseKey;
    make_key(&exampleInvStrokeCaseKey, exampleInvStrokeCase);

    const GrStyledShape& exampleHairlineCase = shapes[index(false, kExamplesDir, kExamplesStart,
                                                      kHairline, false)];
    Key exampleHairlineCaseKey;
    make_key(&exampleHairlineCaseKey, exampleHairlineCase);

    const GrStyledShape& exampleInvHairlineCase = shapes[index(true, kExamplesDir, kExamplesStart,
                                                         kHairline, false)];
    Key exampleInvHairlineCaseKey;
    make_key(&exampleInvHairlineCaseKey, exampleInvHairlineCase);

    // These initializations suppress warnings.
    SkRRect queryRR = SkRRect::MakeEmpty();
    SkPathDirection queryDir = SkPathDirection::kCW;
    unsigned queryStart = ~0U;
    bool queryInverted = true;

    REPORTER_ASSERT(r, exampleFillCase.asRRect(&queryRR, &queryDir, &queryStart, &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPathDirection::kCW == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, !queryInverted);

    REPORTER_ASSERT(r, exampleInvFillCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                  &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPathDirection::kCW == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, queryInverted);

    REPORTER_ASSERT(r, exampleStrokeAndFillCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                        &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPathDirection::kCW == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, !queryInverted);

    REPORTER_ASSERT(r, exampleInvStrokeAndFillCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                           &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPathDirection::kCW == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, queryInverted);

    REPORTER_ASSERT(r, exampleHairlineCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                   &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPathDirection::kCW == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, !queryInverted);

    REPORTER_ASSERT(r, exampleInvHairlineCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                      &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPathDirection::kCW == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, queryInverted);

    REPORTER_ASSERT(r, exampleStrokeCase.asRRect(&queryRR, &queryDir, &queryStart, &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPathDirection::kCW == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, !queryInverted);

    REPORTER_ASSERT(r, exampleInvStrokeCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                    &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPathDirection::kCW == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, queryInverted);

    // Remember that the key reflects the geometry before styling is applied.
    REPORTER_ASSERT(r, exampleFillCaseKey != exampleInvFillCaseKey);
    REPORTER_ASSERT(r, exampleFillCaseKey == exampleStrokeAndFillCaseKey);
    REPORTER_ASSERT(r, exampleFillCaseKey != exampleInvStrokeAndFillCaseKey);
    REPORTER_ASSERT(r, exampleFillCaseKey == exampleStrokeCaseKey);
    REPORTER_ASSERT(r, exampleFillCaseKey != exampleInvStrokeCaseKey);
    REPORTER_ASSERT(r, exampleFillCaseKey == exampleHairlineCaseKey);
    REPORTER_ASSERT(r, exampleFillCaseKey != exampleInvHairlineCaseKey);
    REPORTER_ASSERT(r, exampleInvStrokeAndFillCaseKey == exampleInvFillCaseKey);
    REPORTER_ASSERT(r, exampleInvStrokeAndFillCaseKey == exampleInvStrokeCaseKey);
    REPORTER_ASSERT(r, exampleInvStrokeAndFillCaseKey == exampleInvHairlineCaseKey);

    for (bool inverted : {false, true}) {
        for (SkPathDirection dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
            for (unsigned start = 0; start < 8; ++start) {
                for (bool dash : {false, true}) {
                    const GrStyledShape& fillCase = shapes[index(inverted, dir, start, kFill,
                                                           dash)];
                    Key fillCaseKey;
                    make_key(&fillCaseKey, fillCase);

                    const GrStyledShape& strokeAndFillCase = shapes[index(inverted, dir, start,
                                                                    kStrokeAndFill, dash)];
                    Key strokeAndFillCaseKey;
                    make_key(&strokeAndFillCaseKey, strokeAndFillCase);

                    // Both fill and stroke-and-fill shapes must respect the inverseness and both
                    // ignore dashing.
                    REPORTER_ASSERT(r, !fillCase.style().pathEffect());
                    REPORTER_ASSERT(r, !strokeAndFillCase.style().pathEffect());
                    TestCase a(fillCase, r);
                    TestCase b(inverted ? exampleInvFillCase : exampleFillCase, r);
                    TestCase c(strokeAndFillCase, r);
                    TestCase d(inverted ? exampleInvStrokeAndFillCase
                                        : exampleStrokeAndFillCase, r);
                    a.compare(r, b, TestCase::kAllSame_ComparisonExpecation);
                    c.compare(r, d, TestCase::kAllSame_ComparisonExpecation);

                    const GrStyledShape& strokeCase = shapes[index(inverted, dir, start, kStroke,
                                                             dash)];
                    const GrStyledShape& hairlineCase = shapes[index(inverted, dir, start,
                                                               kHairline, dash)];

                    TestCase e(strokeCase, r);
                    TestCase g(hairlineCase, r);

                    // Both hairline and stroke shapes must respect the dashing.
                    if (dash) {
                        // Dashing always ignores the inverseness. skbug.com/5421
                        TestCase f(exampleStrokeCase, r);
                        TestCase h(exampleHairlineCase, r);
                        unsigned expectedStart = canonicalize_rrect_start(start, rrect);
                        REPORTER_ASSERT(r, strokeCase.style().pathEffect());
                        REPORTER_ASSERT(r, hairlineCase.style().pathEffect());

                        REPORTER_ASSERT(r, strokeCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                              &queryInverted));
                        REPORTER_ASSERT(r, queryRR == rrect);
                        REPORTER_ASSERT(r, queryDir == dir);
                        REPORTER_ASSERT(r, queryStart == expectedStart);
                        REPORTER_ASSERT(r, !queryInverted);
                        REPORTER_ASSERT(r, hairlineCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                                &queryInverted));
                        REPORTER_ASSERT(r, queryRR == rrect);
                        REPORTER_ASSERT(r, queryDir == dir);
                        REPORTER_ASSERT(r, queryStart == expectedStart);
                        REPORTER_ASSERT(r, !queryInverted);

                        // The pre-style case for the dash will match the non-dash example iff the
                        // dir and start match (dir=cw, start=0).
                        if (0 == expectedStart && SkPathDirection::kCW == dir) {
                            e.compare(r, f, TestCase::kSameUpToPE_ComparisonExpecation);
                            g.compare(r, h, TestCase::kSameUpToPE_ComparisonExpecation);
                        } else {
                            e.compare(r, f, TestCase::kAllDifferent_ComparisonExpecation);
                            g.compare(r, h, TestCase::kAllDifferent_ComparisonExpecation);
                        }
                    } else {
                        TestCase f(inverted ? exampleInvStrokeCase : exampleStrokeCase, r);
                        TestCase h(inverted ? exampleInvHairlineCase : exampleHairlineCase, r);
                        REPORTER_ASSERT(r, !strokeCase.style().pathEffect());
                        REPORTER_ASSERT(r, !hairlineCase.style().pathEffect());
                        e.compare(r, f, TestCase::kAllSame_ComparisonExpecation);
                        g.compare(r, h, TestCase::kAllSame_ComparisonExpecation);
                    }
                }
            }
        }
    }
}

DEF_TEST(GrStyledShape_lines, r) {
    static constexpr SkPoint kA { 1,  1};
    static constexpr SkPoint kB { 5, -9};
    static constexpr SkPoint kC {-3, 17};

    SkPath lineAB = SkPath::Line(kA, kB);
    SkPath lineBA = SkPath::Line(kB, kA);
    SkPath lineAC = SkPath::Line(kB, kC);
    SkPath invLineAB = lineAB;

    invLineAB.setFillType(SkPathFillType::kInverseEvenOdd);

    SkPaint fill;
    SkPaint stroke;
    stroke.setStyle(SkPaint::kStroke_Style);
    stroke.setStrokeWidth(2.f);
    SkPaint hairline;
    hairline.setStyle(SkPaint::kStroke_Style);
    hairline.setStrokeWidth(0.f);
    SkPaint dash = stroke;
    dash.setPathEffect(make_dash());

    TestCase fillAB(r, lineAB, fill);
    TestCase fillEmpty(r, SkPath(), fill);
    fillAB.compare(r, fillEmpty, TestCase::kAllSame_ComparisonExpecation);
    REPORTER_ASSERT(r, !fillAB.baseShape().asLine(nullptr, nullptr));

    SkPath path;
    path.toggleInverseFillType();
    TestCase fillEmptyInverted(r, path, fill);
    TestCase fillABInverted(r, invLineAB, fill);
    fillABInverted.compare(r, fillEmptyInverted, TestCase::kAllSame_ComparisonExpecation);
    REPORTER_ASSERT(r, !fillABInverted.baseShape().asLine(nullptr, nullptr));

    TestCase strokeAB(r, lineAB, stroke);
    TestCase strokeBA(r, lineBA, stroke);
    TestCase strokeAC(r, lineAC, stroke);

    TestCase hairlineAB(r, lineAB, hairline);
    TestCase hairlineBA(r, lineBA, hairline);
    TestCase hairlineAC(r, lineAC, hairline);

    TestCase dashAB(r, lineAB, dash);
    TestCase dashBA(r, lineBA, dash);
    TestCase dashAC(r, lineAC, dash);

    strokeAB.compare(r, fillAB, TestCase::kAllDifferent_ComparisonExpecation);

    strokeAB.compare(r, strokeBA, TestCase::kAllSame_ComparisonExpecation);
    strokeAB.compare(r, strokeAC, TestCase::kAllDifferent_ComparisonExpecation);

    hairlineAB.compare(r, hairlineBA, TestCase::kAllSame_ComparisonExpecation);
    hairlineAB.compare(r, hairlineAC, TestCase::kAllDifferent_ComparisonExpecation);

    dashAB.compare(r, dashBA, TestCase::kAllDifferent_ComparisonExpecation);
    dashAB.compare(r, dashAC, TestCase::kAllDifferent_ComparisonExpecation);

    strokeAB.compare(r, hairlineAB, TestCase::kSameUpToStroke_ComparisonExpecation);

    // One of dashAB or dashBA should have the same line as strokeAB. It depends upon how
    // GrStyledShape canonicalizes line endpoints (when it can, i.e. when not dashed).
    bool canonicalizeAsAB;
    SkPoint canonicalPts[2] {kA, kB};
    // Init these to suppress warnings.
    bool inverted = true;
    SkPoint pts[2] {{0, 0}, {0, 0}};
    REPORTER_ASSERT(r, strokeAB.baseShape().asLine(pts, &inverted) && !inverted);
    if (pts[0] == kA && pts[1] == kB) {
        canonicalizeAsAB = true;
    } else if (pts[1] == kA && pts[0] == kB) {
        canonicalizeAsAB = false;
        using std::swap;
        swap(canonicalPts[0], canonicalPts[1]);
    } else {
        ERRORF(r, "Should return pts (a,b) or (b, a)");
        return;
    }

    strokeAB.compare(r, canonicalizeAsAB ? dashAB : dashBA,
                     TestCase::kSameUpToPE_ComparisonExpecation);
    REPORTER_ASSERT(r, strokeAB.baseShape().asLine(pts, &inverted) && !inverted &&
                       pts[0] == canonicalPts[0] && pts[1] == canonicalPts[1]);
    REPORTER_ASSERT(r, hairlineAB.baseShape().asLine(pts, &inverted) && !inverted &&
                       pts[0] == canonicalPts[0] && pts[1] == canonicalPts[1]);
    REPORTER_ASSERT(r, dashAB.baseShape().asLine(pts, &inverted) && !inverted &&
                       pts[0] == kA && pts[1] == kB);
    REPORTER_ASSERT(r, dashBA.baseShape().asLine(pts, &inverted) && !inverted &&
                       pts[0] == kB && pts[1] == kA);


    TestCase strokeInvAB(r, invLineAB, stroke);
    TestCase hairlineInvAB(r, invLineAB, hairline);
    TestCase dashInvAB(r, invLineAB, dash);
    strokeInvAB.compare(r, strokeAB, TestCase::kAllDifferent_ComparisonExpecation);
    hairlineInvAB.compare(r, hairlineAB, TestCase::kAllDifferent_ComparisonExpecation);
    // Dashing ignores inverse.
    dashInvAB.compare(r, dashAB, TestCase::kAllSame_ComparisonExpecation);

    REPORTER_ASSERT(r, strokeInvAB.baseShape().asLine(pts, &inverted) && inverted &&
                       pts[0] == canonicalPts[0] && pts[1] == canonicalPts[1]);
    REPORTER_ASSERT(r, hairlineInvAB.baseShape().asLine(pts, &inverted) && inverted &&
                       pts[0] == canonicalPts[0] && pts[1] == canonicalPts[1]);
    // Dashing ignores inverse.
    REPORTER_ASSERT(r, dashInvAB.baseShape().asLine(pts, &inverted) && !inverted &&
                       pts[0] == kA && pts[1] == kB);

}

DEF_TEST(GrStyledShape_stroked_lines, r) {
    static constexpr SkScalar kIntervals1[] = {1.f, 0.f};
    auto dash1 = SkDashPathEffect::Make(kIntervals1, SK_ARRAY_COUNT(kIntervals1), 0.f);
    REPORTER_ASSERT(r, dash1);
    static constexpr SkScalar kIntervals2[] = {10.f, 0.f, 5.f, 0.f};
    auto dash2 = SkDashPathEffect::Make(kIntervals2, SK_ARRAY_COUNT(kIntervals2), 10.f);
    REPORTER_ASSERT(r, dash2);

    sk_sp<SkPathEffect> pathEffects[] = {nullptr, std::move(dash1), std::move(dash2)};

    for (const auto& pe : pathEffects) {
        // Paints to try
        SkPaint buttCap;
        buttCap.setStyle(SkPaint::kStroke_Style);
        buttCap.setStrokeWidth(4);
        buttCap.setStrokeCap(SkPaint::kButt_Cap);
        buttCap.setPathEffect(pe);

        SkPaint squareCap = buttCap;
        squareCap.setStrokeCap(SkPaint::kSquare_Cap);
        squareCap.setPathEffect(pe);

        SkPaint roundCap = buttCap;
        roundCap.setStrokeCap(SkPaint::kRound_Cap);
        roundCap.setPathEffect(pe);

        // vertical
        SkPath linePath;
        linePath.moveTo(4, 4);
        linePath.lineTo(4, 5);

        SkPaint fill;

        make_TestCase(r, linePath, buttCap)->compare(
                r, TestCase(r, SkRect::MakeLTRB(2, 4, 6, 5), fill),
                TestCase::kAllSame_ComparisonExpecation);

        make_TestCase(r, linePath, squareCap)->compare(
                r, TestCase(r, SkRect::MakeLTRB(2, 2, 6, 7), fill),
                TestCase::kAllSame_ComparisonExpecation);

        make_TestCase(r, linePath, roundCap)->compare(r,
                TestCase(r, SkRRect::MakeRectXY(SkRect::MakeLTRB(2, 2, 6, 7), 2, 2), fill),
                TestCase::kAllSame_ComparisonExpecation);

        // horizontal
        linePath.reset();
        linePath.moveTo(4, 4);
        linePath.lineTo(5, 4);

        make_TestCase(r, linePath, buttCap)->compare(
                r, TestCase(r, SkRect::MakeLTRB(4, 2, 5, 6), fill),
                TestCase::kAllSame_ComparisonExpecation);
        make_TestCase(r, linePath, squareCap)->compare(
                r, TestCase(r, SkRect::MakeLTRB(2, 2, 7, 6), fill),
                TestCase::kAllSame_ComparisonExpecation);
        make_TestCase(r, linePath, roundCap)->compare(
                r, TestCase(r, SkRRect::MakeRectXY(SkRect::MakeLTRB(2, 2, 7, 6), 2, 2), fill),
                TestCase::kAllSame_ComparisonExpecation);

        // point
        linePath.reset();
        linePath.moveTo(4, 4);
        linePath.lineTo(4, 4);

        make_TestCase(r, linePath, buttCap)->compare(
                r, TestCase(r, SkRect::MakeEmpty(), fill),
                TestCase::kAllSame_ComparisonExpecation);
        make_TestCase(r, linePath, squareCap)->compare(
                r, TestCase(r, SkRect::MakeLTRB(2, 2, 6, 6), fill),
                TestCase::kAllSame_ComparisonExpecation);
        make_TestCase(r, linePath, roundCap)->compare(
                r, TestCase(r, SkRRect::MakeRectXY(SkRect::MakeLTRB(2, 2, 6, 6), 2, 2), fill),
                TestCase::kAllSame_ComparisonExpecation);
    }
}

DEF_TEST(GrStyledShape_short_path_keys, r) {
    SkPaint paints[4];
    paints[1].setStyle(SkPaint::kStroke_Style);
    paints[1].setStrokeWidth(5.f);
    paints[2].setStyle(SkPaint::kStroke_Style);
    paints[2].setStrokeWidth(0.f);
    paints[3].setStyle(SkPaint::kStrokeAndFill_Style);
    paints[3].setStrokeWidth(5.f);

    auto compare = [r, &paints] (const SkPath& pathA, const SkPath& pathB,
                                 TestCase::ComparisonExpecation expectation) {
        SkPath volatileA = pathA;
        SkPath volatileB = pathB;
        volatileA.setIsVolatile(true);
        volatileB.setIsVolatile(true);
        for (const SkPaint& paint : paints) {
            REPORTER_ASSERT(r, !GrStyledShape(volatileA, paint).hasUnstyledKey());
            REPORTER_ASSERT(r, !GrStyledShape(volatileB, paint).hasUnstyledKey());
            for (PathGeo::Invert invert : {PathGeo::Invert::kNo, PathGeo::Invert::kYes}) {
                TestCase caseA(PathGeo(pathA, invert), paint, r);
                TestCase caseB(PathGeo(pathB, invert), paint, r);
                caseA.compare(r, caseB, expectation);
            }
        }
    };

    SkPath pathA;
    SkPath pathB;

    // Two identical paths
    pathA.lineTo(10.f, 10.f);
    pathA.conicTo(20.f, 20.f, 20.f, 30.f, 0.7f);

    pathB.lineTo(10.f, 10.f);
    pathB.conicTo(20.f, 20.f, 20.f, 30.f, 0.7f);
    compare(pathA, pathB, TestCase::kAllSame_ComparisonExpecation);

    // Give path b a different point
    pathB.reset();
    pathB.lineTo(10.f, 10.f);
    pathB.conicTo(21.f, 20.f, 20.f, 30.f, 0.7f);
    compare(pathA, pathB, TestCase::kAllDifferent_ComparisonExpecation);

    // Give path b a different conic weight
    pathB.reset();
    pathB.lineTo(10.f, 10.f);
    pathB.conicTo(20.f, 20.f, 20.f, 30.f, 0.6f);
    compare(pathA, pathB, TestCase::kAllDifferent_ComparisonExpecation);

    // Give path b an extra lineTo verb
    pathB.reset();
    pathB.lineTo(10.f, 10.f);
    pathB.conicTo(20.f, 20.f, 20.f, 30.f, 0.6f);
    pathB.lineTo(50.f, 50.f);
    compare(pathA, pathB, TestCase::kAllDifferent_ComparisonExpecation);

    // Give path b a close
    pathB.reset();
    pathB.lineTo(10.f, 10.f);
    pathB.conicTo(20.f, 20.f, 20.f, 30.f, 0.7f);
    pathB.close();
    compare(pathA, pathB, TestCase::kAllDifferent_ComparisonExpecation);
}

DEF_TEST(GrStyledShape, reporter) {
    SkTArray<std::unique_ptr<Geo>> geos;
    SkTArray<std::unique_ptr<RRectPathGeo>> rrectPathGeos;

    for (auto r : { SkRect::MakeWH(10, 20),
                    SkRect::MakeWH(-10, -20),
                    SkRect::MakeWH(-10, 20),
                    SkRect::MakeWH(10, -20)}) {
        geos.emplace_back(new RectGeo(r));
        SkPath rectPath;
        rectPath.addRect(r);
        geos.emplace_back(new RRectPathGeo(rectPath, r, RRectPathGeo::RRectForStroke::kYes,
                                           PathGeo::Invert::kNo));
        geos.emplace_back(new RRectPathGeo(rectPath, r, RRectPathGeo::RRectForStroke::kYes,
                                           PathGeo::Invert::kYes));
        rrectPathGeos.emplace_back(new RRectPathGeo(rectPath, r, RRectPathGeo::RRectForStroke::kYes,
                                                    PathGeo::Invert::kNo));
    }
    for (auto rr : { SkRRect::MakeRect(SkRect::MakeWH(10, 10)),
                     SkRRect::MakeRectXY(SkRect::MakeWH(10, 10), 3, 4),
                     SkRRect::MakeOval(SkRect::MakeWH(20, 20))}) {
        geos.emplace_back(new RRectGeo(rr));
        test_rrect(reporter, rr);
        SkPath rectPath;
        rectPath.addRRect(rr);
        geos.emplace_back(new RRectPathGeo(rectPath, rr, RRectPathGeo::RRectForStroke::kYes,
                                           PathGeo::Invert::kNo));
        geos.emplace_back(new RRectPathGeo(rectPath, rr, RRectPathGeo::RRectForStroke::kYes,
                                           PathGeo::Invert::kYes));
        rrectPathGeos.emplace_back(new RRectPathGeo(rectPath, rr,
                                                    RRectPathGeo::RRectForStroke::kYes,
                                                    PathGeo::Invert::kNo));
    }

    // Arcs
    geos.emplace_back(new ArcGeo(SkRect::MakeWH(200, 100), 12.f, 110.f, false));
    geos.emplace_back(new ArcGeo(SkRect::MakeWH(200, 100), 12.f, 110.f, true));

    {
        SkPath openRectPath;
        openRectPath.moveTo(0, 0);
        openRectPath.lineTo(10, 0);
        openRectPath.lineTo(10, 10);
        openRectPath.lineTo(0, 10);
        geos.emplace_back(new RRectPathGeo(
                    openRectPath, SkRect::MakeWH(10, 10),
                    RRectPathGeo::RRectForStroke::kNo, PathGeo::Invert::kNo));
        geos.emplace_back(new RRectPathGeo(
                    openRectPath, SkRect::MakeWH(10, 10),
                    RRectPathGeo::RRectForStroke::kNo, PathGeo::Invert::kYes));
        rrectPathGeos.emplace_back(new RRectPathGeo(
                    openRectPath, SkRect::MakeWH(10, 10),
                    RRectPathGeo::RRectForStroke::kNo, PathGeo::Invert::kNo));
    }

    {
        SkPath quadPath;
        quadPath.quadTo(10, 10, 5, 8);
        geos.emplace_back(new PathGeo(quadPath, PathGeo::Invert::kNo));
        geos.emplace_back(new PathGeo(quadPath, PathGeo::Invert::kYes));
    }

    {
        SkPath linePath;
        linePath.lineTo(10, 10);
        geos.emplace_back(new PathGeo(linePath, PathGeo::Invert::kNo));
        geos.emplace_back(new PathGeo(linePath, PathGeo::Invert::kYes));
    }

    // Horizontal and vertical paths become rrects when stroked.
    {
        SkPath vLinePath;
        vLinePath.lineTo(0, 10);
        geos.emplace_back(new PathGeo(vLinePath, PathGeo::Invert::kNo));
        geos.emplace_back(new PathGeo(vLinePath, PathGeo::Invert::kYes));
    }

    {
        SkPath hLinePath;
        hLinePath.lineTo(10, 0);
        geos.emplace_back(new PathGeo(hLinePath, PathGeo::Invert::kNo));
        geos.emplace_back(new PathGeo(hLinePath, PathGeo::Invert::kYes));
    }

    for (int i = 0; i < geos.count(); ++i) {
        test_basic(reporter, *geos[i]);
        test_scale(reporter, *geos[i]);
        test_dash_fill(reporter, *geos[i]);
        test_null_dash(reporter, *geos[i]);
        // Test modifying various stroke params.
        test_stroke_param<SkScalar>(
                reporter, *geos[i],
                [](SkPaint* p, SkScalar w) { p->setStrokeWidth(w);},
                SkIntToScalar(2), SkIntToScalar(4));
        test_stroke_join(reporter, *geos[i]);
        test_stroke_cap(reporter, *geos[i]);
        test_miter_limit(reporter, *geos[i]);
        test_path_effect_makes_rrect(reporter, *geos[i]);
        test_unknown_path_effect(reporter, *geos[i]);
        test_path_effect_makes_empty_shape(reporter, *geos[i]);
        test_path_effect_fails(reporter, *geos[i]);
        test_make_hairline_path_effect(reporter, *geos[i]);
        test_volatile_path(reporter, *geos[i]);
    }

    for (int i = 0; i < rrectPathGeos.count(); ++i) {
        const RRectPathGeo& rrgeo = *rrectPathGeos[i];
        SkPaint fillPaint;
        TestCase fillPathCase(reporter, rrgeo.path(), fillPaint);
        SkRRect rrect;
        REPORTER_ASSERT(reporter, rrgeo.isNonPath(fillPaint) ==
                                  fillPathCase.baseShape().asRRect(&rrect, nullptr, nullptr,
                                                                   nullptr));
        if (rrgeo.isNonPath(fillPaint)) {
            TestCase fillPathCase2(reporter, rrgeo.path(), fillPaint);
            REPORTER_ASSERT(reporter, rrect == rrgeo.rrect());
            TestCase fillRRectCase(reporter, rrect, fillPaint);
            fillPathCase2.compare(reporter, fillRRectCase,
                                  TestCase::kAllSame_ComparisonExpecation);
        }
        SkPaint strokePaint;
        strokePaint.setStrokeWidth(3.f);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        TestCase strokePathCase(reporter, rrgeo.path(), strokePaint);
        if (rrgeo.isNonPath(strokePaint)) {
            REPORTER_ASSERT(reporter, strokePathCase.baseShape().asRRect(&rrect, nullptr, nullptr,
                                                                         nullptr));
            REPORTER_ASSERT(reporter, rrect == rrgeo.rrect());
            TestCase strokeRRectCase(reporter, rrect, strokePaint);
            strokePathCase.compare(reporter, strokeRRectCase,
                                   TestCase::kAllSame_ComparisonExpecation);
        }
    }

    // Test a volatile empty path.
    test_volatile_path(reporter, PathGeo(SkPath(), PathGeo::Invert::kNo));
}

DEF_TEST(GrStyledShape_arcs, reporter) {
    SkStrokeRec roundStroke(SkStrokeRec::kFill_InitStyle);
    roundStroke.setStrokeStyle(2.f);
    roundStroke.setStrokeParams(SkPaint::kRound_Cap, SkPaint::kRound_Join, 1.f);

    SkStrokeRec squareStroke(roundStroke);
    squareStroke.setStrokeParams(SkPaint::kSquare_Cap, SkPaint::kRound_Join, 1.f);

    SkStrokeRec roundStrokeAndFill(roundStroke);
    roundStrokeAndFill.setStrokeStyle(2.f, true);

    static constexpr SkScalar kIntervals[] = {1, 2};
    auto dash = SkDashPathEffect::Make(kIntervals, SK_ARRAY_COUNT(kIntervals), 1.5f);

    SkTArray<GrStyle> styles;
    styles.push_back(GrStyle::SimpleFill());
    styles.push_back(GrStyle::SimpleHairline());
    styles.push_back(GrStyle(roundStroke, nullptr));
    styles.push_back(GrStyle(squareStroke, nullptr));
    styles.push_back(GrStyle(roundStrokeAndFill, nullptr));
    styles.push_back(GrStyle(roundStroke, dash));

    for (const auto& style : styles) {
        // An empty rect never draws anything according to SkCanvas::drawArc() docs.
        TestCase emptyArc(GrStyledShape::MakeArc(SkRect::MakeEmpty(), 0, 90.f, false, style),
                                                 reporter);
        TestCase emptyPath(reporter, SkPath(), style);
        emptyArc.compare(reporter, emptyPath, TestCase::kAllSame_ComparisonExpecation);

        static constexpr SkRect kOval1{0, 0, 50, 50};
        static constexpr SkRect kOval2{50, 0, 100, 50};
        // Test that swapping starting and ending angle doesn't change the shape unless the arc
        // has a path effect. Also test that different ovals produce different shapes.
        TestCase arc1CW(GrStyledShape::MakeArc(kOval1, 0, 90.f, false, style), reporter);
        TestCase arc1CCW(GrStyledShape::MakeArc(kOval1, 90.f, -90.f, false, style), reporter);

        TestCase arc1CWWithCenter(GrStyledShape::MakeArc(kOval1, 0, 90.f, true, style), reporter);
        TestCase arc1CCWWithCenter(GrStyledShape::MakeArc(kOval1, 90.f, -90.f, true, style),
                                   reporter);

        TestCase arc2CW(GrStyledShape::MakeArc(kOval2, 0, 90.f, false, style), reporter);
        TestCase arc2CWWithCenter(GrStyledShape::MakeArc(kOval2, 0, 90.f, true, style), reporter);

        auto reversedExepectations = style.hasPathEffect()
                                             ? TestCase::kAllDifferent_ComparisonExpecation
                                             : TestCase::kAllSame_ComparisonExpecation;
        arc1CW.compare(reporter, arc1CCW, reversedExepectations);
        arc1CWWithCenter.compare(reporter, arc1CCWWithCenter, reversedExepectations);
        arc1CW.compare(reporter, arc2CW, TestCase::kAllDifferent_ComparisonExpecation);
        arc1CW.compare(reporter, arc1CWWithCenter, TestCase::kAllDifferent_ComparisonExpecation);
        arc1CWWithCenter.compare(reporter, arc2CWWithCenter,
                                 TestCase::kAllDifferent_ComparisonExpecation);

        // Test that two arcs that start at the same angle but specified differently are equivalent.
        TestCase arc3A(GrStyledShape::MakeArc(kOval1, 224.f, 73.f, false, style), reporter);
        TestCase arc3B(GrStyledShape::MakeArc(kOval1, 224.f - 360.f, 73.f, false, style), reporter);
        arc3A.compare(reporter, arc3B, TestCase::kAllDifferent_ComparisonExpecation);

        // Test that an arc that traverses the entire oval (and then some) is equivalent to the
        // oval itself unless there is a path effect.
        TestCase ovalArc(GrStyledShape::MakeArc(kOval1, 150.f, -790.f, false, style), reporter);
        TestCase oval(GrStyledShape(SkRRect::MakeOval(kOval1)), reporter);
        auto ovalExpectations = style.hasPathEffect() ? TestCase::kAllDifferent_ComparisonExpecation
                                                      : TestCase::kAllSame_ComparisonExpecation;
        if (style.strokeRec().getWidth() >= 0 && style.strokeRec().getCap() != SkPaint::kButt_Cap) {
            ovalExpectations = TestCase::kAllDifferent_ComparisonExpecation;
        }
        ovalArc.compare(reporter, oval, ovalExpectations);

        // If the the arc starts/ends at the center then it is then equivalent to the oval only for
        // simple fills.
        TestCase ovalArcWithCenter(GrStyledShape::MakeArc(kOval1, 304.f, 1225.f, true, style),
                                   reporter);
        ovalExpectations = style.isSimpleFill() ? TestCase::kAllSame_ComparisonExpecation
                                                : TestCase::kAllDifferent_ComparisonExpecation;
        ovalArcWithCenter.compare(reporter, oval, ovalExpectations);
    }
}

DEF_TEST(GrShapeInversion, r) {
    SkPath path;
    SkScalar radii[] = {10.f, 10.f, 10.f, 10.f,
                        10.f, 10.f, 10.f, 10.f};
    path.addRoundRect(SkRect::MakeWH(50, 50), radii);
    path.toggleInverseFillType();

    GrShape inverseRRect(path);
    GrShape rrect(inverseRRect);
    rrect.setInverted(false);

    REPORTER_ASSERT(r, inverseRRect.inverted() && inverseRRect.isPath());
    REPORTER_ASSERT(r, !rrect.inverted() && rrect.isPath());

    // Invertedness should be preserved after simplification
    inverseRRect.simplify();
    rrect.simplify();

    REPORTER_ASSERT(r, inverseRRect.inverted() && inverseRRect.isRRect());
    REPORTER_ASSERT(r, !rrect.inverted() && rrect.isRRect());

    // Invertedness should be reset when calling reset().
    inverseRRect.reset();
    REPORTER_ASSERT(r, !inverseRRect.inverted() && inverseRRect.isEmpty());
    inverseRRect.setPath(path);
    inverseRRect.reset();
    REPORTER_ASSERT(r, !inverseRRect.inverted() && inverseRRect.isEmpty());
}
