/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <initializer_list>
#include <functional>
#include "Test.h"
#if SK_SUPPORT_GPU
#include "GrShape.h"
#include "SkCanvas.h"
#include "SkDashPathEffect.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkSurface.h"

using Key = SkTArray<uint32_t>;

static bool make_key(Key* key, const GrShape& shape) {
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
    static constexpr int kRes = 2000;
    // This tolerance is in units of 1/kRes fractions of the bounds width/height.
    static constexpr int kTol = 0;
    GR_STATIC_ASSERT(kRes % 4 == 0);
    SkImageInfo info = SkImageInfo::MakeA8(kRes, kRes);
    sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
    surface->getCanvas()->clear(0x0);
    SkRect clip = SkRect::MakeXYWH(kRes/4, kRes/4, kRes/2, kRes/2);
    SkMatrix matrix;
    matrix.setRectToRect(bounds, clip, SkMatrix::kFill_ScaleToFit);
    clip.outset(SkIntToScalar(kTol), SkIntToScalar(kTol));
    surface->getCanvas()->clipRect(clip, SkRegion::kDifference_Op);
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
    for (int y = 0; y < kRes/4; ++y) {
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

namespace {
class TestCase {
public:
    template <typename GEO>
    TestCase(const GEO& geo, const SkPaint& paint, skiatest::Reporter* r,
             SkScalar scale = SK_Scalar1) : fBase(geo, paint) {
        this->init(r, scale);
    }

    TestCase(const GrShape& shape, skiatest::Reporter* r, SkScalar scale = SK_Scalar1)
        : fBase(shape) {
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

    const GrShape& baseShape() const { return fBase; }
    const GrShape& appliedPathEffectShape() const { return fAppliedPE; }
    const GrShape& appliedFullStyleShape() const { return fAppliedFull; }

    // The returned array's count will be 0 if the key shape has no key.
    const Key& baseKey() const { return fBaseKey; }
    const Key& appliedPathEffectKey() const { return fAppliedPEKey; }
    const Key& appliedFullStyleKey() const { return fAppliedFullKey; }
    const Key& appliedPathEffectThenStrokeKey() const { return fAppliedPEThenStrokeKey; }

private:
    static void CheckBounds(skiatest::Reporter* r, const GrShape& shape, const SkRect& bounds) {
        SkPath path;
        shape.asPath(&path);
        // If the bounds are empty, the path ought to be as well.
        if (bounds.isEmpty()) {
            REPORTER_ASSERT(r, path.isEmpty());
            return;
        }
        if (path.isEmpty()) {
            return;
        }
        // The bounds API explicitly calls out that it does not consider inverseness.
        SkPath p = path;
        p.setFillType(SkPath::ConvertToNonInverseFillType(path.getFillType()));
        REPORTER_ASSERT(r, test_bounds_by_rasterizing(p, bounds));
    }

    void init(skiatest::Reporter* r, SkScalar scale) {
        fAppliedPE           = fBase.applyStyle(GrStyle::Apply::kPathEffectOnly, scale);
        fAppliedPEThenStroke = fAppliedPE.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec,
                                                     scale);
        fAppliedFull         = fBase.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, scale);

        make_key(&fBaseKey, fBase);
        make_key(&fAppliedPEKey, fAppliedPE);
        make_key(&fAppliedPEThenStrokeKey, fAppliedPEThenStroke);
        make_key(&fAppliedFullKey, fAppliedFull);

        // Applying the path effect and then the stroke should always be the same as applying
        // both in one go.
        REPORTER_ASSERT(r, fAppliedPEThenStrokeKey == fAppliedFullKey);
        SkPath a, b;
        fAppliedPEThenStroke.asPath(&a);
        fAppliedFull.asPath(&b);
        // If the output of the path effect is a rrect then it is possible for a and b to be
        // different paths that fill identically. The reason is that fAppliedFull will do this:
        // base -> apply path effect -> rrect_as_path -> stroke -> stroked_rrect_as_path
        // fAppliedPEThenStroke will have converted the rrect_as_path back to a rrect. However,
        // now that there is no longer a path effect, the direction and starting index get
        // canonicalized before the stroke.
        if (fAppliedPE.asRRect(nullptr, nullptr, nullptr, nullptr)) {
            REPORTER_ASSERT(r, paths_fill_same(a, b));
        } else {
            REPORTER_ASSERT(r, a == b);
        }
        REPORTER_ASSERT(r, fAppliedFull.isEmpty() == fAppliedPEThenStroke.isEmpty());

        SkPath path;
        fBase.asPath(&path);
        REPORTER_ASSERT(r, path.isEmpty() == fBase.isEmpty());
        REPORTER_ASSERT(r, path.getSegmentMasks() == fBase.segmentMask());
        fAppliedPE.asPath(&path);
        REPORTER_ASSERT(r, path.isEmpty() == fAppliedPE.isEmpty());
        REPORTER_ASSERT(r, path.getSegmentMasks() == fAppliedPE.segmentMask());
        fAppliedFull.asPath(&path);
        REPORTER_ASSERT(r, path.isEmpty() == fAppliedFull.isEmpty());
        REPORTER_ASSERT(r, path.getSegmentMasks() == fAppliedFull.segmentMask());

        CheckBounds(r, fBase, fBase.bounds());
        CheckBounds(r, fAppliedPE, fAppliedPE.bounds());
        CheckBounds(r, fAppliedPEThenStroke, fAppliedPEThenStroke.bounds());
        CheckBounds(r, fAppliedFull, fAppliedFull.bounds());
        SkRect styledBounds;
        fBase.styledBounds(&styledBounds);
        CheckBounds(r, fAppliedFull, styledBounds);
        fAppliedPE.styledBounds(&styledBounds);
        CheckBounds(r, fAppliedFull, styledBounds);

        // Check that the same path is produced when style is applied by GrShape and GrStyle.
        SkPath preStyle;
        SkPath postPathEffect;
        SkPath postAllStyle;

        fBase.asPath(&preStyle);
        SkStrokeRec postPEStrokeRec(SkStrokeRec::kFill_InitStyle);
        if (fBase.style().applyPathEffectToPath(&postPathEffect, &postPEStrokeRec, preStyle,
                                                scale)) {
            // run postPathEffect through GrShape to get any geometry reductions that would have
            // occurred to fAppliedPE.
            GrShape(postPathEffect, GrStyle(postPEStrokeRec, nullptr)).asPath(&postPathEffect);

            SkPath testPath;
            fAppliedPE.asPath(&testPath);
            REPORTER_ASSERT(r, testPath == postPathEffect);
            REPORTER_ASSERT(r, postPEStrokeRec.hasEqualEffect(fAppliedPE.style().strokeRec()));
        }
        SkStrokeRec::InitStyle fillOrHairline;
        if (fBase.style().applyToPath(&postAllStyle, &fillOrHairline, preStyle, scale)) {
            // run postPathEffect through GrShape to get any reductions that would have occurred
            // to fAppliedFull.
            GrShape(postAllStyle, GrStyle(fillOrHairline)).asPath(&postAllStyle);

            SkPath testPath;
            fAppliedFull.asPath(&testPath);
            REPORTER_ASSERT(r, testPath == postAllStyle);
            if (fillOrHairline == SkStrokeRec::kFill_InitStyle) {
                REPORTER_ASSERT(r, fAppliedFull.style().isSimpleFill());
            } else {
                REPORTER_ASSERT(r, fAppliedFull.style().isSimpleHairline());
            }
        }
    }

    GrShape fBase;
    GrShape fAppliedPE;
    GrShape fAppliedPEThenStroke;
    GrShape fAppliedFull;

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
        fBase.asPath(&a);
        fAppliedPE.asPath(&b);
        REPORTER_ASSERT(reporter, a == b);
        if (expectations.fStrokeApplies) {
            REPORTER_ASSERT(reporter, fBaseKey != fAppliedFullKey);
        } else {
            REPORTER_ASSERT(reporter, fBaseKey == fAppliedFullKey);
        }
    }
}

void check_equivalence(skiatest::Reporter* r, const GrShape& a, const GrShape& b,
                       const Key& keyA, const Key& keyB) {
    // GrShape only respects the input winding direction and start point for rrect shapes
    // when there is a path effect. Thus, if there are two GrShapes representing the same rrect
    // but one has a path effect in its style and the other doesn't then asPath() and the unstyled
    // key will differ. GrShape will have canonicalized the direction and start point for the shape
    // without the path effect. If *both* have path effects then they should have both preserved
    // the direction and starting point.

    // The asRRect() output params are all initialized just to silence compiler warnings about
    // uninitialized variables.
    SkRRect rrectA = SkRRect::MakeEmpty(), rrectB = SkRRect::MakeEmpty();
    SkPath::Direction dirA = SkPath::kCW_Direction, dirB = SkPath::kCW_Direction;
    unsigned startA = ~0U, startB = ~0U;
    bool invertedA = true, invertedB = true;

    bool aIsRRect = a.asRRect(&rrectA, &dirA, &startA, &invertedA);
    bool bIsRRect = b.asRRect(&rrectB, &dirB, &startB, &invertedB);
    bool aHasPE = a.style().hasPathEffect();
    bool bHasPE = b.style().hasPathEffect();
    bool allowSameRRectButDiffStartAndDir = (aIsRRect && bIsRRect) && (aHasPE != bHasPE);

    SkPath pathA, pathB;
    a.asPath(&pathA);
    b.asPath(&pathB);

    // Having a fill style or non-dash path effect can prevent 'a' but not 'b' from turning an
    // inverse fill type into a non-inverse fill type.
    bool ignoreInversenessDifference = false;
    if (pathA.isInverseFillType() != pathB.isInverseFillType()) {
        const GrShape* s1 = pathA.isInverseFillType() ? &a : &b;
        const GrShape* s2 = pathA.isInverseFillType() ? &b : &a;
        SkStrokeRec::Style style1 = s1->style().strokeRec().getStyle();
        SkStrokeRec::Style style2 = s2->style().strokeRec().getStyle();
        bool canDropInverse1 = !s1->style().hasNonDashPathEffect() &&
                                (SkStrokeRec::kStroke_Style == style1 ||
                                 SkStrokeRec::kHairline_Style == style1);
        bool canDropInverse2 = !s2->style().hasNonDashPathEffect() &&
                               (SkStrokeRec::kStroke_Style == style2 ||
                                SkStrokeRec::kHairline_Style == style2);
        ignoreInversenessDifference = !canDropInverse1 && canDropInverse2;
    }

    if (allowSameRRectButDiffStartAndDir) {
        REPORTER_ASSERT(r, rrectA == rrectB);
        REPORTER_ASSERT(r, paths_fill_same(pathA, pathB));
        REPORTER_ASSERT(r, ignoreInversenessDifference || invertedA == invertedB);
    } else {
        SkPath pA = pathA;
        SkPath pB = pathB;
        if (ignoreInversenessDifference) {
            pA.setFillType(SkPath::ConvertToNonInverseFillType(pathA.getFillType()));
            pB.setFillType(SkPath::ConvertToNonInverseFillType(pathB.getFillType()));
            REPORTER_ASSERT(r, keyA != keyB);
        } else {
            REPORTER_ASSERT(r, keyA == keyB);
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
    REPORTER_ASSERT(r, a.knownToBeClosed() == b.knownToBeClosed());
    REPORTER_ASSERT(r, a.bounds() == b.bounds());
    REPORTER_ASSERT(r, a.segmentMask() == b.segmentMask());
    SkPoint pts[4];
    REPORTER_ASSERT(r, a.asLine(pts) == b.asLine(pts + 2));
    if (a.asLine(pts)) {
        REPORTER_ASSERT(r, pts[2] == pts[0] && pts[3] == pts[1]);
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
            check_equivalence(r, fBase, that.fBase, fBaseKey, that.fBaseKey);
            REPORTER_ASSERT(r, fAppliedPEKey != that.fAppliedPEKey);
            REPORTER_ASSERT(r, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kSameUpToStroke_ComparisonExpecation:
            check_equivalence(r, fBase, that.fBase, fBaseKey, that.fBaseKey);
            check_equivalence(r, fAppliedPE, that.fAppliedPE, fAppliedPEKey, that.fAppliedPEKey);
            REPORTER_ASSERT(r, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kAllSame_ComparisonExpecation:
            check_equivalence(r, fBase, that.fBase, fBaseKey, that.fBaseKey);
            check_equivalence(r, fAppliedPE, that.fAppliedPE, fAppliedPEKey, that.fAppliedPEKey);
            check_equivalence(r, fAppliedFull, that.fAppliedFull, fAppliedFullKey,
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

template<typename GEO>
static void test_basic(skiatest::Reporter* reporter, const GEO& geo) {
    sk_sp<SkPathEffect> dashPE = make_dash();

    TestCase::SelfExpectations expectations;
    SkPaint fill;

    TestCase fillCase(geo, fill, reporter);
    expectations.fPEHasEffect = false;
    expectations.fPEHasValidKey = false;
    expectations.fStrokeApplies = false;
    fillCase.testExpectations(reporter, expectations);
    // Test that another GrShape instance built from the same primitive is the same.
    TestCase(geo, fill, reporter).compare(reporter, fillCase,
                                          TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke2RoundBevel;
    stroke2RoundBevel.setStyle(SkPaint::kStroke_Style);
    stroke2RoundBevel.setStrokeCap(SkPaint::kRound_Cap);
    stroke2RoundBevel.setStrokeJoin(SkPaint::kBevel_Join);
    stroke2RoundBevel.setStrokeWidth(2.f);
    TestCase stroke2RoundBevelCase(geo, stroke2RoundBevel, reporter);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = false;
    expectations.fStrokeApplies = true;
    stroke2RoundBevelCase.testExpectations(reporter, expectations);
    TestCase(geo, stroke2RoundBevel, reporter).compare(reporter, stroke2RoundBevelCase,
                                                       TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke2RoundBevelDash = stroke2RoundBevel;
    stroke2RoundBevelDash.setPathEffect(make_dash());
    TestCase stroke2RoundBevelDashCase(geo, stroke2RoundBevelDash, reporter);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = true;
    expectations.fStrokeApplies = true;
    stroke2RoundBevelDashCase.testExpectations(reporter, expectations);
    TestCase(geo, stroke2RoundBevelDash, reporter).compare(reporter, stroke2RoundBevelDashCase,
                                                           TestCase::kAllSame_ComparisonExpecation);

    fillCase.compare(reporter, stroke2RoundBevelCase,
                     TestCase::kSameUpToStroke_ComparisonExpecation);
    fillCase.compare(reporter, stroke2RoundBevelDashCase,
                     TestCase::kSameUpToPE_ComparisonExpecation);
    stroke2RoundBevelCase.compare(reporter, stroke2RoundBevelDashCase,
                                  TestCase::kSameUpToPE_ComparisonExpecation);

    // Stroke and fill cases
    SkPaint stroke2RoundBevelAndFill = stroke2RoundBevel;
    stroke2RoundBevelAndFill.setStyle(SkPaint::kStrokeAndFill_Style);
    TestCase stroke2RoundBevelAndFillCase(geo, stroke2RoundBevelAndFill, reporter);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = false;
    expectations.fStrokeApplies = true;
    stroke2RoundBevelAndFillCase.testExpectations(reporter, expectations);
    TestCase(geo, stroke2RoundBevelAndFill, reporter).compare(reporter,
            stroke2RoundBevelAndFillCase, TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke2RoundBevelAndFillDash = stroke2RoundBevelDash;
    stroke2RoundBevelAndFillDash.setStyle(SkPaint::kStrokeAndFill_Style);
    TestCase stroke2RoundBevelAndFillDashCase(geo, stroke2RoundBevelAndFillDash, reporter);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = false;
    expectations.fStrokeApplies = true;
    stroke2RoundBevelAndFillDashCase.testExpectations(reporter, expectations);
    TestCase(geo, stroke2RoundBevelAndFillDash, reporter).compare(
        reporter, stroke2RoundBevelAndFillDashCase, TestCase::kAllSame_ComparisonExpecation);
    stroke2RoundBevelAndFillDashCase.compare(reporter, stroke2RoundBevelAndFillCase,
                                             TestCase::kAllSame_ComparisonExpecation);

    SkPaint hairline;
    hairline.setStyle(SkPaint::kStroke_Style);
    hairline.setStrokeWidth(0.f);
    TestCase hairlineCase(geo, hairline, reporter);
    // Since hairline style doesn't change the SkPath data, it is keyed identically to fill.
    hairlineCase.compare(reporter, fillCase, TestCase::kAllSame_ComparisonExpecation);
    REPORTER_ASSERT(reporter, hairlineCase.baseShape().style().isSimpleHairline());
    REPORTER_ASSERT(reporter, hairlineCase.appliedFullStyleShape().style().isSimpleHairline());
    REPORTER_ASSERT(reporter, hairlineCase.appliedPathEffectShape().style().isSimpleHairline());
}

template<typename GEO>
static void test_scale(skiatest::Reporter* reporter, const GEO& geo) {
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
    // Scale affects the stroke.
    strokeCase1.compare(reporter, strokeCase2, TestCase::kSameUpToStroke_ComparisonExpecation);

    SkPaint strokeDash = stroke;
    strokeDash.setPathEffect(make_dash());
    TestCase strokeDashCase1(geo, strokeDash, reporter, kS1);
    TestCase strokeDashCase2(geo, strokeDash, reporter, kS2);
    // Scale affects the dash and the stroke.
    strokeDashCase1.compare(reporter, strokeDashCase2,  TestCase::kSameUpToPE_ComparisonExpecation);

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
    // Scale affects the stroke. Though, this can wind up creating a rect when the input is a rect.
    // In that case we wind up with a pure geometry key and the geometries are the same.
    SkRRect rrect;
    if (strokeAndFillCase1.appliedFullStyleShape().asRRect(&rrect, nullptr, nullptr, nullptr)) {
        // We currently only expect to get here in the rect->rect case.
        REPORTER_ASSERT(reporter, rrect.isRect());
        REPORTER_ASSERT(reporter,
                        strokeAndFillCase1.baseShape().asRRect(&rrect, nullptr, nullptr, nullptr) &&
                        rrect.isRect());
        strokeAndFillCase1.compare(reporter, strokeAndFillCase2,
                                   TestCase::kAllSame_ComparisonExpecation);
    } else {
        strokeAndFillCase1.compare(reporter, strokeAndFillCase2,
                                   TestCase::kSameUpToStroke_ComparisonExpecation);
        strokeAndFillDashCase1.compare(reporter, strokeAndFillDashCase2,
                                       TestCase::kSameUpToStroke_ComparisonExpecation);
    }
    strokeAndFillDashCase1.compare(reporter, strokeAndFillCase1,
                                   TestCase::kAllSame_ComparisonExpecation);
    strokeAndFillDashCase2.compare(reporter, strokeAndFillCase2,
                                   TestCase::kAllSame_ComparisonExpecation);
}

template <typename GEO, typename T>
static void test_stroke_param_impl(skiatest::Reporter* reporter, const GEO& geo,
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
        strokeACase.compare(reporter, strokeBCase, TestCase::kSameUpToStroke_ComparisonExpecation);
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
        strokeAndFillACase.compare(reporter, strokeAndFillBCase,
                                   TestCase::kSameUpToStroke_ComparisonExpecation);
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

template <typename GEO, typename T>
static void test_stroke_param(skiatest::Reporter* reporter, const GEO& geo,
                              std::function<void(SkPaint*, T)> setter, T a, T b) {
    test_stroke_param_impl(reporter, geo, setter, a, b, true, true);
};

template <typename GEO>
static void test_stroke_cap(skiatest::Reporter* reporter, const GEO& geo) {
    GrShape shape(geo, GrStyle(SkStrokeRec::kHairline_InitStyle));
    // The cap should only affect shapes that may be open.
    bool affectsStroke = !shape.knownToBeClosed();
    // Dashing adds ends that need caps.
    bool affectsDashAndStroke = true;
    test_stroke_param_impl<GEO, SkPaint::Cap>(
        reporter,
        geo,
        [](SkPaint* p, SkPaint::Cap c) { p->setStrokeCap(c);},
        SkPaint::kButt_Cap, SkPaint::kRound_Cap,
        affectsStroke,
        affectsDashAndStroke);
};

template <typename GEO>
static void test_miter_limit(skiatest::Reporter* reporter, const GEO& geo) {
    auto setMiterJoinAndLimit = [](SkPaint* p, SkScalar miter) {
        p->setStrokeJoin(SkPaint::kMiter_Join);
        p->setStrokeMiter(miter);
    };

    auto setOtherJoinAndLimit = [](SkPaint* p, SkScalar miter) {
        p->setStrokeJoin(SkPaint::kRound_Join);
        p->setStrokeMiter(miter);
    };

    // The miter limit should affect stroked and dashed-stroked cases when the join type is
    // miter.
    test_stroke_param_impl<GEO, SkScalar>(
        reporter,
        geo,
        setMiterJoinAndLimit,
        0.5f, 0.75f,
        true,
        true);

    // The miter limit should not affect stroked and dashed-stroked cases when the join type is
    // not miter.
    test_stroke_param_impl<GEO, SkScalar>(
        reporter,
        geo,
        setOtherJoinAndLimit,
        0.5f, 0.75f,
        false,
        false);
}

template<typename GEO>
static void test_dash_fill(skiatest::Reporter* reporter, const GEO& geo) {
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

template<typename GEO>
void test_null_dash(skiatest::Reporter* reporter, const GEO& geo) {
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

    nullDashCase.compare(reporter, fillCase, TestCase::kSameUpToStroke_ComparisonExpecation);
    nullDashCase.compare(reporter, strokeCase, TestCase::kAllSame_ComparisonExpecation);
    nullDashCase.compare(reporter, dashCase, TestCase::kSameUpToPE_ComparisonExpecation);
}

template <typename GEO>
void test_path_effect_makes_rrect(skiatest::Reporter* reporter, const GEO& geo) {
    /**
     * This path effect takes any input path and turns it into a rrect. It passes through stroke
     * info.
     */
    class RRectPathEffect : SkPathEffect {
    public:
        static const SkRRect& RRect() {
            static const SkRRect kRRect = SkRRect::MakeRectXY(SkRect::MakeWH(12, 12), 3, 5);
            return kRRect;
        }

        bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*,
                        const SkRect* cullR) const override {
            dst->reset();
            dst->addRRect(RRect());
            return true;
        }
        void computeFastBounds(SkRect* dst, const SkRect& src) const override {
            *dst = RRect().getBounds();
        }
        static sk_sp<SkPathEffect> Make() { return sk_sp<SkPathEffect>(new RRectPathEffect); }
        Factory getFactory() const override { return nullptr; }
        void toString(SkString*) const override {}
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

    fillGeoCase.compare(reporter, geoPECase, TestCase::kSameUpToPE_ComparisonExpecation);
    fillGeoCase.compare(reporter, geoPEStrokeCase, TestCase::kSameUpToPE_ComparisonExpecation);
    geoPECase.compare(reporter, geoPEStrokeCase,
                      TestCase::kSameUpToStroke_ComparisonExpecation);

    TestCase rrectFillCase(RRectPathEffect::RRect(), fill, reporter);
    SkPaint stroke = peStroke;
    stroke.setPathEffect(nullptr);
    TestCase rrectStrokeCase(RRectPathEffect::RRect(), stroke, reporter);

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

template <typename GEO>
void test_unknown_path_effect(skiatest::Reporter* reporter, const GEO& geo) {
    /**
     * This path effect just adds two lineTos to the input path.
     */
    class AddLineTosPathEffect : SkPathEffect {
    public:
        bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*,
                        const SkRect* cullR) const override {
            *dst = src;
            dst->lineTo(0, 0);
            dst->lineTo(10, 10);
            return true;
        }
        void computeFastBounds(SkRect* dst, const SkRect& src) const override {
            *dst = src;
            dst->growToInclude(0, 0);
            dst->growToInclude(10, 10);
        }
        static sk_sp<SkPathEffect> Make() { return sk_sp<SkPathEffect>(new AddLineTosPathEffect); }
        Factory getFactory() const override { return nullptr; }
        void toString(SkString*) const override {}
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

template <typename GEO>
void test_make_hairline_path_effect(skiatest::Reporter* reporter, const GEO& geo, bool isNonPath) {
    /**
     * This path effect just changes the stroke rec to hairline.
     */
    class MakeHairlinePathEffect : SkPathEffect {
    public:
        bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec* strokeRec,
                        const SkRect* cullR) const override {
            *dst = src;
            strokeRec->setHairlineStyle();
            return true;
        }
        void computeFastBounds(SkRect* dst, const SkRect& src) const override { *dst = src; }
        static sk_sp<SkPathEffect> Make() {
            return sk_sp<SkPathEffect>(new MakeHairlinePathEffect);
        }
        Factory getFactory() const override { return nullptr; }
        void toString(SkString*) const override {}
    private:
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
    if (isNonPath) {
        // RRect types can have a change in start index or direction after the PE is applied. This
        // is because once the PE is applied, GrShape may canonicalize the dir and index since it
        // is not germane to the styling any longer.
        // Instead we just check that the paths would fill the same both before and after styling.
        REPORTER_ASSERT(reporter, paths_fill_same(a, b));
        REPORTER_ASSERT(reporter, paths_fill_same(a, c));
    } else {
        REPORTER_ASSERT(reporter, a == b);
        REPORTER_ASSERT(reporter, a == c);
        REPORTER_ASSERT(reporter, peCase.appliedPathEffectKey().empty());
        REPORTER_ASSERT(reporter, peCase.appliedFullStyleKey().empty());
    }
    REPORTER_ASSERT(reporter, peCase.appliedPathEffectShape().style().isSimpleHairline());
    REPORTER_ASSERT(reporter, peCase.appliedFullStyleShape().style().isSimpleHairline());
}

/**
 * isNonPath indicates whether the initial shape made from the path is expected to be recognized
 * as a simpler shape type (e.g. rrect)
 */
void test_volatile_path(skiatest::Reporter* reporter, const SkPath& path,
                        bool isNonPath) {
    SkPath vPath(path);
    vPath.setIsVolatile(true);

    SkPaint dashAndStroke;
    dashAndStroke.setPathEffect(make_dash());
    dashAndStroke.setStrokeWidth(2.f);
    dashAndStroke.setStyle(SkPaint::kStroke_Style);
    TestCase volatileCase(vPath, dashAndStroke, reporter);
    // We expect a shape made from a volatile path to have a key iff the shape is recognized
    // as a specialized geometry.
    if (isNonPath) {
        REPORTER_ASSERT(reporter, SkToBool(volatileCase.baseKey().count()));
        // In this case all the keys should be identical to the non-volatile case.
        TestCase nonVolatileCase(path, dashAndStroke, reporter);
        volatileCase.compare(reporter, nonVolatileCase, TestCase::kAllSame_ComparisonExpecation);
    } else {
        // None of the keys should be valid.
        REPORTER_ASSERT(reporter, !SkToBool(volatileCase.baseKey().count()));
        REPORTER_ASSERT(reporter, !SkToBool(volatileCase.appliedPathEffectKey().count()));
        REPORTER_ASSERT(reporter, !SkToBool(volatileCase.appliedFullStyleKey().count()));
        REPORTER_ASSERT(reporter, !SkToBool(volatileCase.appliedPathEffectThenStrokeKey().count()));
    }
}

template <typename GEO>
void test_path_effect_makes_empty_shape(skiatest::Reporter* reporter, const GEO& geo) {
    /**
     * This path effect returns an empty path.
     */
    class EmptyPathEffect : SkPathEffect {
    public:
        bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*,
                        const SkRect* cullR) const override {
            dst->reset();
            return true;
        }
        void computeFastBounds(SkRect* dst, const SkRect& src) const override {
            dst->setEmpty();
        }
        static sk_sp<SkPathEffect> Make() { return sk_sp<SkPathEffect>(new EmptyPathEffect); }
        Factory getFactory() const override { return nullptr; }
        void toString(SkString*) const override {}
    private:
        EmptyPathEffect() {}
    };

    SkPath emptyPath;
    GrShape emptyShape(emptyPath);
    Key emptyKey;
    make_key(&emptyKey, emptyShape);
    REPORTER_ASSERT(reporter, emptyShape.isEmpty());

    SkPaint pe;
    pe.setPathEffect(EmptyPathEffect::Make());
    TestCase geoCase(geo, pe, reporter);
    REPORTER_ASSERT(reporter, geoCase.appliedFullStyleKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoCase.appliedPathEffectKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoCase.appliedPathEffectThenStrokeKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoCase.appliedPathEffectShape().isEmpty());
    REPORTER_ASSERT(reporter, geoCase.appliedFullStyleShape().isEmpty());

    SkPaint peStroke;
    peStroke.setPathEffect(EmptyPathEffect::Make());
    peStroke.setStrokeWidth(2.f);
    peStroke.setStyle(SkPaint::kStroke_Style);
    TestCase geoPEStrokeCase(geo, peStroke, reporter);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedFullStyleKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectThenStrokeKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectShape().isEmpty());
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedFullStyleShape().isEmpty());
}

template <typename GEO>
void test_path_effect_fails(skiatest::Reporter* reporter, const GEO& geo) {
    /**
     * This path effect returns an empty path.
     */
    class FailurePathEffect : SkPathEffect {
    public:
        bool filterPath(SkPath* dst, const SkPath& src, SkStrokeRec*,
                        const SkRect* cullR) const override {
            return false;
        }
        void computeFastBounds(SkRect* dst, const SkRect& src) const override {
            *dst = src;
        }
        static sk_sp<SkPathEffect> Make() { return sk_sp<SkPathEffect>(new FailurePathEffect); }
        Factory getFactory() const override { return nullptr; }
        void toString(SkString*) const override {}
    private:
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
    // path effect, but then when the path effect fails we can key it. 2) GrShape will change its
    // mind about whether a unclosed rect is actually rect. The path effect initially bars us from
    // closing it but after the effect fails we can (for the fill+pe case). This causes different
    // routes through GrShape to have equivalent but different representations of the path (closed
    // or not) but that fill the same.
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

void test_empty_shape(skiatest::Reporter* reporter) {
    SkPath emptyPath;
    SkPaint fill;
    TestCase fillEmptyCase(emptyPath, fill, reporter);
    REPORTER_ASSERT(reporter, fillEmptyCase.baseShape().isEmpty());
    REPORTER_ASSERT(reporter, fillEmptyCase.appliedPathEffectShape().isEmpty());
    REPORTER_ASSERT(reporter, fillEmptyCase.appliedFullStyleShape().isEmpty());

    Key emptyKey(fillEmptyCase.baseKey());
    REPORTER_ASSERT(reporter, emptyKey.count());
    TestCase::SelfExpectations expectations;
    expectations.fStrokeApplies = false;
    expectations.fPEHasEffect = false;
    // This will test whether applying style preserves emptiness
    fillEmptyCase.testExpectations(reporter, expectations);

    // Stroking an empty path should have no effect
    SkPath emptyPath2;
    SkPaint stroke;
    stroke.setStrokeWidth(2.f);
    stroke.setStyle(SkPaint::kStroke_Style);
    TestCase strokeEmptyCase(emptyPath2, stroke, reporter);
    strokeEmptyCase.compare(reporter, fillEmptyCase, TestCase::kAllSame_ComparisonExpecation);

    // Dashing and stroking an empty path should have no effect
    SkPath emptyPath3;
    SkPaint dashAndStroke;
    dashAndStroke.setPathEffect(make_dash());
    dashAndStroke.setStrokeWidth(2.f);
    dashAndStroke.setStyle(SkPaint::kStroke_Style);
    TestCase dashAndStrokeEmptyCase(emptyPath3, dashAndStroke, reporter);
    dashAndStrokeEmptyCase.compare(reporter, fillEmptyCase,
                                   TestCase::kAllSame_ComparisonExpecation);

    // A shape made from an empty rrect should behave the same as an empty path.
    SkRRect emptyRRect = SkRRect::MakeRect(SkRect::MakeEmpty());
    REPORTER_ASSERT(reporter, emptyRRect.getType() == SkRRect::kEmpty_Type);
    TestCase dashAndStrokeEmptyRRectCase(emptyRRect, dashAndStroke, reporter);
    dashAndStrokeEmptyRRectCase.compare(reporter, fillEmptyCase,
                                        TestCase::kAllSame_ComparisonExpecation);

    // Same for a rect.
    SkRect emptyRect = SkRect::MakeEmpty();
    TestCase dashAndStrokeEmptyRectCase(emptyRect, dashAndStroke, reporter);
    dashAndStrokeEmptyRectCase.compare(reporter, fillEmptyCase,
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
    sk_sp<SkPathEffect> dashEffect = make_dash();

    static constexpr Style kStyleCnt = static_cast<Style>(SK_ARRAY_COUNT(strokeRecs));

    auto index = [](bool inverted,
                    SkPath::Direction dir,
                    unsigned start,
                    Style style,
                    bool dash) -> int {
        return inverted * (2 * 8 * kStyleCnt * 2) +
               dir      * (    8 * kStyleCnt * 2) +
               start    * (        kStyleCnt * 2) +
               style    * (                    2) +
               dash;
    };
    static const SkPath::Direction kSecondDirection = static_cast<SkPath::Direction>(1);
    const int cnt = index(true, kSecondDirection, 7, static_cast<Style>(kStyleCnt - 1), true) + 1;
    SkAutoTArray<GrShape> shapes(cnt);
    for (bool inverted : {false, true}) {
        for (SkPath::Direction dir : {SkPath::kCW_Direction, SkPath::kCCW_Direction}) {
            for (unsigned start = 0; start < 8; ++start) {
                for (Style style : {kFill, kStroke, kHairline, kStrokeAndFill}) {
                    for (bool dash : {false, true}) {
                        SkPathEffect* pe = dash ? dashEffect.get() : nullptr;
                        shapes[index(inverted, dir, start, style, dash)] =
                                GrShape(rrect, dir, start, SkToBool(inverted),
                                        GrStyle(strokeRecs[style], pe));
                    }
                }
            }
        }
    }

    static const SkPath::Direction kDir = SkPath::kCW_Direction; // arbitrary
    const GrShape& exampleFillCase = shapes[index(false, kDir, 0, kFill, false)];
    Key exampleFillCaseKey;
    make_key(&exampleFillCaseKey, exampleFillCase);

    const GrShape& exampleStrokeAndFillCase = shapes[index(false, kDir, 0, kStrokeAndFill, false)];
    Key exampleStrokeAndFillCaseKey;
    make_key(&exampleStrokeAndFillCaseKey, exampleStrokeAndFillCase);

    const GrShape& exampleInvFillCase = shapes[index(true, kDir, 0, kFill, false)];
    Key exampleInvFillCaseKey;
    make_key(&exampleInvFillCaseKey, exampleInvFillCase);

    const GrShape& exampleInvStrokeAndFillCase =
            shapes[index(true, kDir, 0, kStrokeAndFill, false)];
    Key exampleInvStrokeAndFillCaseKey;
    make_key(&exampleInvStrokeAndFillCaseKey, exampleInvStrokeAndFillCase);

    const GrShape& exampleStrokeCase = shapes[index(false, kDir, 0, kStroke, false)];
    Key exampleStrokeCaseKey;
    make_key(&exampleStrokeCaseKey, exampleStrokeCase);

    const GrShape& exampleHairlineCase = shapes[index(false, kDir, 0, kHairline, false)];
    Key exampleHairlineCaseKey;
    make_key(&exampleHairlineCaseKey, exampleHairlineCase);

    // These are dummy initializations to suppress warnings.
    SkRRect queryRR = SkRRect::MakeEmpty();
    SkPath::Direction queryDir = SkPath::kCW_Direction;
    unsigned queryStart = ~0U;
    bool queryInverted = true;

    REPORTER_ASSERT(r, exampleFillCase.asRRect(&queryRR, &queryDir, &queryStart, &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPath::kCW_Direction == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, !queryInverted);

    REPORTER_ASSERT(r, exampleInvFillCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                  &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPath::kCW_Direction == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, queryInverted);

    REPORTER_ASSERT(r, exampleStrokeAndFillCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                        &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPath::kCW_Direction == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, !queryInverted);

    REPORTER_ASSERT(r, exampleInvStrokeAndFillCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                           &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPath::kCW_Direction == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, queryInverted);

    REPORTER_ASSERT(r, exampleHairlineCase.asRRect(&queryRR, &queryDir, &queryStart,
                                                   &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPath::kCW_Direction == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, !queryInverted);

    REPORTER_ASSERT(r, exampleStrokeCase.asRRect(&queryRR, &queryDir, &queryStart, &queryInverted));
    REPORTER_ASSERT(r, queryRR == rrect);
    REPORTER_ASSERT(r, SkPath::kCW_Direction == queryDir);
    REPORTER_ASSERT(r, 0 == queryStart);
    REPORTER_ASSERT(r, !queryInverted);

    // Remember that the key reflects the geometry before styling is applied.
    REPORTER_ASSERT(r, exampleFillCaseKey != exampleInvFillCaseKey);
    REPORTER_ASSERT(r, exampleFillCaseKey == exampleStrokeAndFillCaseKey);
    REPORTER_ASSERT(r, exampleFillCaseKey != exampleInvStrokeAndFillCaseKey);
    REPORTER_ASSERT(r, exampleFillCaseKey == exampleStrokeCaseKey);
    REPORTER_ASSERT(r, exampleFillCaseKey == exampleHairlineCaseKey);
    REPORTER_ASSERT(r, exampleInvStrokeAndFillCaseKey == exampleInvFillCaseKey);

    for (bool inverted : {false, true}) {
        for (SkPath::Direction dir : {SkPath::kCW_Direction, SkPath::kCCW_Direction}) {
            for (unsigned start = 0; start < 8; ++start) {
                for (bool dash : {false, true}) {
                    const GrShape& fillCase = shapes[index(inverted, dir, start, kFill, dash)];
                    Key fillCaseKey;
                    make_key(&fillCaseKey, fillCase);

                    const GrShape& strokeAndFillCase = shapes[index(inverted, dir, start,
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

                    const GrShape& strokeCase = shapes[index(inverted, dir, start, kStroke, dash)];
                    const GrShape& hairlineCase = shapes[index(inverted, dir, start, kHairline,
                                                               dash)];

                    TestCase e(strokeCase, r);
                    TestCase f(exampleStrokeCase, r);
                    TestCase g(hairlineCase, r);
                    TestCase h(exampleHairlineCase, r);

                    // Both hairline and stroke shapes must respect the dashing and both
                    // ignore inverseness.
                    if (dash) {
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
                        if (0 == expectedStart && SkPath::kCW_Direction == dir) {
                            e.compare(r, f, TestCase::kSameUpToPE_ComparisonExpecation);
                            g.compare(r, h, TestCase::kSameUpToPE_ComparisonExpecation);
                        } else {
                            e.compare(r, f, TestCase::kAllDifferent_ComparisonExpecation);
                            g.compare(r, h, TestCase::kAllDifferent_ComparisonExpecation);
                        }
                    } else {
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

DEF_TEST(GrShape, reporter) {
    for (auto r : { SkRect::MakeWH(10, 20),
                    SkRect::MakeWH(-10, -20),
                    SkRect::MakeWH(-10, 20),
                    SkRect::MakeWH(10, -20)}) {
        test_basic(reporter, r);
        test_scale(reporter, r);
        test_dash_fill(reporter, r);
        test_null_dash(reporter, r);
        // Test modifying various stroke params.
        test_stroke_param<SkRect, SkScalar>(
                reporter, r,
                [](SkPaint* p, SkScalar w) { p->setStrokeWidth(w);},
                SkIntToScalar(2), SkIntToScalar(4));
        test_stroke_param<SkRect, SkPaint::Join>(
                reporter, r,
                [](SkPaint* p, SkPaint::Join j) { p->setStrokeJoin(j);},
                SkPaint::kMiter_Join, SkPaint::kRound_Join);
        test_stroke_cap(reporter, r);
        test_miter_limit(reporter, r);
        test_path_effect_makes_rrect(reporter, r);
        test_unknown_path_effect(reporter, r);
        test_path_effect_makes_empty_shape(reporter, r);
        test_path_effect_fails(reporter, r);
        test_make_hairline_path_effect(reporter, r, true);
        GrShape shape(r);
        REPORTER_ASSERT(reporter, !shape.asLine(nullptr));
    }

    for (auto rr : { SkRRect::MakeRect(SkRect::MakeWH(10, 10)),
                     SkRRect::MakeRectXY(SkRect::MakeWH(10, 10), 3, 4),
                     SkRRect::MakeOval(SkRect::MakeWH(20, 20))}) {
        test_basic(reporter, rr);
        test_rrect(reporter, rr);
        test_scale(reporter, rr);
        test_dash_fill(reporter, rr);
        test_null_dash(reporter, rr);
        // Test modifying various stroke params.
        test_stroke_param<SkRRect, SkScalar>(
                          reporter, rr,
                          [](SkPaint* p, SkScalar w) { p->setStrokeWidth(w);},
                          SkIntToScalar(2), SkIntToScalar(4));
        test_stroke_param<SkRRect, SkPaint::Join>(
                          reporter, rr,
                          [](SkPaint* p, SkPaint::Join j) { p->setStrokeJoin(j);},
                          SkPaint::kMiter_Join, SkPaint::kRound_Join);
        test_stroke_cap(reporter, rr);
        test_miter_limit(reporter, rr);
        test_path_effect_makes_rrect(reporter, rr);
        test_unknown_path_effect(reporter, rr);
        test_path_effect_makes_empty_shape(reporter, rr);
        test_path_effect_fails(reporter, rr);
        test_make_hairline_path_effect(reporter, rr, true);
        GrShape shape(rr);
        REPORTER_ASSERT(reporter, !shape.asLine(nullptr));
    }

    struct TestPath {
        TestPath(const SkPath& path, bool isRRectFill, bool isRRectStroke, bool isLine, const SkRRect& rrect)
            : fPath(path)
            , fIsRRectForFill(isRRectFill)
            , fIsRRectForStroke(isRRectStroke)
            , fIsLine(isLine)
            , fRRect(rrect) {}
        SkPath  fPath;
        bool    fIsRRectForFill;
        bool    fIsRRectForStroke;
        bool    fIsLine;
        SkRRect fRRect;
    };
    SkTArray<TestPath> paths;

    SkPath circlePath;
    circlePath.addCircle(10, 10, 10);
    paths.emplace_back(circlePath, true, true, false, SkRRect::MakeOval(SkRect::MakeWH(20,20)));

    SkPath rectPath;
    rectPath.addRect(SkRect::MakeWH(10, 10));
    paths.emplace_back(rectPath, true, true, false, SkRRect::MakeRect(SkRect::MakeWH(10, 10)));

    SkPath openRectPath;
    openRectPath.moveTo(0, 0);
    openRectPath.lineTo(10, 0);
    openRectPath.lineTo(10, 10);
    openRectPath.lineTo(0, 10);
    paths.emplace_back(openRectPath, true, false, false, SkRRect::MakeRect(SkRect::MakeWH(10, 10)));

    SkPath quadPath;
    quadPath.quadTo(10, 10, 5, 8);
    paths.emplace_back(quadPath, false, false, false, SkRRect());

    SkPath linePath;
    linePath.lineTo(10, 10);
    paths.emplace_back(linePath, false, false, true, SkRRect());

    for (auto testPath : paths) {
        for (bool inverseFill : {false, true}) {
            if (inverseFill) {
                if (testPath.fPath.getFillType() == SkPath::kEvenOdd_FillType) {
                    testPath.fPath.setFillType(SkPath::kInverseEvenOdd_FillType);
                } else {
                    SkASSERT(testPath.fPath.getFillType() == SkPath::kWinding_FillType);
                    testPath.fPath.setFillType(SkPath::kInverseWinding_FillType);
                }
            }
            const SkPath& path = testPath.fPath;
            // These tests all assume that the original GrShape for fill and stroke will be the
            // same.
            // However, that is not the case in special cases (e.g. an unclosed rect becomes a RRect
            // GrShape with a fill style but becomes a Path GrShape when stroked).
            if (testPath.fIsRRectForFill == testPath.fIsRRectForStroke) {
                test_basic(reporter, path);
                test_null_dash(reporter, path);
                test_path_effect_makes_rrect(reporter, path);
            }
            test_scale(reporter, path);
            // This test uses a stroking paint, hence use of fIsRRectForStroke
            test_volatile_path(reporter, path, testPath.fIsRRectForStroke);
            test_dash_fill(reporter, path);
            // Test modifying various stroke params.
            test_stroke_param<SkPath, SkScalar>(
                reporter, path,
                [](SkPaint* p, SkScalar w) { p->setStrokeWidth(w);},
                SkIntToScalar(2), SkIntToScalar(4));
            test_stroke_param<SkPath, SkPaint::Join>(
                reporter, path,
                [](SkPaint* p, SkPaint::Join j) { p->setStrokeJoin(j);},
                SkPaint::kMiter_Join, SkPaint::kRound_Join);
            test_stroke_cap(reporter, path);
            test_miter_limit(reporter, path);
            test_unknown_path_effect(reporter, path);
            test_path_effect_makes_empty_shape(reporter, path);
            test_path_effect_fails(reporter, path);
            test_make_hairline_path_effect(reporter, path, testPath.fIsRRectForStroke);
        }
    }
    for (auto testPath : paths) {
        const SkPath& path = testPath.fPath;

        SkPaint fillPaint;
        TestCase fillPathCase(path, fillPaint, reporter);
        SkRRect rrect;
        REPORTER_ASSERT(reporter, testPath.fIsRRectForFill ==
                                  fillPathCase.baseShape().asRRect(&rrect, nullptr, nullptr,
                                                                   nullptr));
        if (testPath.fIsRRectForFill) {
            TestCase fillPathCase2(testPath.fPath, fillPaint, reporter);
            REPORTER_ASSERT(reporter, rrect == testPath.fRRect);
            TestCase fillRRectCase(rrect, fillPaint, reporter);
            fillPathCase2.compare(reporter, fillRRectCase,
                                  TestCase::kAllSame_ComparisonExpecation);
        }
        SkPaint strokePaint;
        strokePaint.setStrokeWidth(3.f);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        TestCase strokePathCase(path, strokePaint, reporter);
        REPORTER_ASSERT(reporter, testPath.fIsRRectForStroke ==
                                  strokePathCase.baseShape().asRRect(&rrect, nullptr, nullptr,
                                                                     nullptr));
        if (testPath.fIsRRectForStroke) {
            REPORTER_ASSERT(reporter, rrect == testPath.fRRect);
            TestCase strokeRRectCase(rrect, strokePaint, reporter);
            strokePathCase.compare(reporter, strokeRRectCase,
                                   TestCase::kAllSame_ComparisonExpecation);
        }
        REPORTER_ASSERT(reporter, testPath.fIsLine == fillPathCase.baseShape().asLine(nullptr));
        REPORTER_ASSERT(reporter, testPath.fIsLine == strokePathCase.baseShape().asLine(nullptr));
    }

    // Test a volatile empty path.
    test_volatile_path(reporter, SkPath(), true);

    test_empty_shape(reporter);
}

#endif
