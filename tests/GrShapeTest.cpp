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
#include "SkPath.h"
#include "SkDashPathEffect.h"

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

namespace {

class TestCase {
public:
    template <typename GEO>
    TestCase(const GEO& geo, const SkPaint& paint) : fBase(geo, paint) {
        this->init();
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
    void init() {
        fAppliedPE           = fBase.applyPathEffect();
        fAppliedPEThenStroke = fAppliedPE.applyFullStyle();
        fAppliedFull         = fBase.applyFullStyle();

        make_key(&fBaseKey, fBase);
        make_key(&fAppliedPEKey, fAppliedPE);
        make_key(&fAppliedPEThenStrokeKey, fAppliedPEThenStroke);
        make_key(&fAppliedFullKey, fAppliedFull);
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
    // Applying the path effect and then the stroke should always be the same as applying
    // both in one go.
    REPORTER_ASSERT(reporter, fAppliedPEThenStrokeKey == fAppliedFullKey);
    SkPath a, b;
    fAppliedPEThenStroke.asPath(&a);
    fAppliedFull.asPath(&b);
    REPORTER_ASSERT(reporter, a == b);
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

void TestCase::compare(skiatest::Reporter* reporter, const TestCase& that,
                       ComparisonExpecation expectation) const {
    SkPath a, b;
    switch (expectation) {
        case kAllDifferent_ComparisonExpecation:
            REPORTER_ASSERT(reporter, fBaseKey != that.fBaseKey);
            REPORTER_ASSERT(reporter, fAppliedPEKey != that.fAppliedPEKey);
            REPORTER_ASSERT(reporter, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kSameUpToPE_ComparisonExpecation:
            REPORTER_ASSERT(reporter, fBaseKey == that.fBaseKey);
            fBase.asPath(&a);
            that.fBase.asPath(&b);
            REPORTER_ASSERT(reporter, a == b);
            REPORTER_ASSERT(reporter, fAppliedPEKey != that.fAppliedPEKey);
            REPORTER_ASSERT(reporter, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kSameUpToStroke_ComparisonExpecation:
            REPORTER_ASSERT(reporter, fBaseKey == that.fBaseKey);
            fBase.asPath(&a);
            that.fBase.asPath(&b);
            REPORTER_ASSERT(reporter, a == b);
            REPORTER_ASSERT(reporter, fAppliedPEKey == that.fAppliedPEKey);
            fAppliedPE.asPath(&a);
            that.fAppliedPE.asPath(&b);
            REPORTER_ASSERT(reporter, a == b);
            REPORTER_ASSERT(reporter, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kAllSame_ComparisonExpecation:
            REPORTER_ASSERT(reporter, fBaseKey == that.fBaseKey);
            fBase.asPath(&a);
            that.fBase.asPath(&b);
            REPORTER_ASSERT(reporter, a == b);
            REPORTER_ASSERT(reporter, fAppliedPEKey == that.fAppliedPEKey);
            fAppliedPE.asPath(&a);
            that.fAppliedPE.asPath(&b);
            REPORTER_ASSERT(reporter, a == b);
            REPORTER_ASSERT(reporter, fAppliedFullKey == that.fAppliedFullKey);
            fAppliedFull.asPath(&a);
            that.fAppliedFull.asPath(&b);
            REPORTER_ASSERT(reporter, a == b);
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

    TestCase fillCase(geo, fill);
    expectations.fPEHasEffect = false;
    expectations.fPEHasValidKey = false;
    expectations.fStrokeApplies = false;
    fillCase.testExpectations(reporter, expectations);
    // Test that another GrShape instance built from the same primitive is the same.
    TestCase(geo, fill).compare(reporter, fillCase, TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke2RoundBevel;
    stroke2RoundBevel.setStyle(SkPaint::kStroke_Style);
    stroke2RoundBevel.setStrokeCap(SkPaint::kRound_Cap);
    stroke2RoundBevel.setStrokeJoin(SkPaint::kBevel_Join);
    stroke2RoundBevel.setStrokeWidth(2.f);
    TestCase stroke2RoundBevelCase(geo, stroke2RoundBevel);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = false;
    expectations.fStrokeApplies = true;
    stroke2RoundBevelCase.testExpectations(reporter, expectations);
    TestCase(geo, stroke2RoundBevel).compare(reporter, stroke2RoundBevelCase,
                                             TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke2RoundBevelDash = stroke2RoundBevel;
    stroke2RoundBevelDash.setPathEffect(make_dash());
    TestCase stroke2RoundBevelDashCase(geo, stroke2RoundBevelDash);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = true;
    expectations.fStrokeApplies = true;
    stroke2RoundBevelDashCase.testExpectations(reporter, expectations);
    TestCase(geo, stroke2RoundBevelDash).compare(reporter, stroke2RoundBevelDashCase,
                                                 TestCase::kAllSame_ComparisonExpecation);

    fillCase.compare(reporter, stroke2RoundBevelCase,
                     TestCase::kSameUpToStroke_ComparisonExpecation);
    fillCase.compare(reporter, stroke2RoundBevelDashCase,
                     TestCase::kSameUpToPE_ComparisonExpecation);
    stroke2RoundBevelCase.compare(reporter, stroke2RoundBevelDashCase,
                                  TestCase::kSameUpToPE_ComparisonExpecation);

    SkPaint hairline;
    hairline.setStyle(SkPaint::kStroke_Style);
    hairline.setStrokeWidth(0.f);
    TestCase hairlineCase(geo, hairline);
    // Since hairline style doesn't change the SkPath data, it is keyed identically to fill.
    hairlineCase.compare(reporter, fillCase, TestCase::kAllSame_ComparisonExpecation);
}

template <typename GEO, typename T>
static void test_stroke_param(skiatest::Reporter* reporter, const GEO& geo,
                              std::function<void(SkPaint*, T)> setter, T a, T b) {
    // Set the stroke width so that we don't get hairline. However, call the function second so that
    // it can override.
    SkPaint strokeA;
    strokeA.setStyle(SkPaint::kStroke_Style);
    strokeA.setStrokeWidth(2.f);
    setter(&strokeA, a);
    SkPaint strokeB;
    strokeB.setStyle(SkPaint::kStroke_Style);
    strokeB.setStrokeWidth(2.f);
    setter(&strokeB, b);

    TestCase strokeACase(geo, strokeA);
    TestCase strokeBCase(geo, strokeB);
    strokeACase.compare(reporter, strokeBCase, TestCase::kSameUpToStroke_ComparisonExpecation);

    // Make sure stroking params don't affect fill style.
    SkPaint fillA = strokeA, fillB = strokeB;
    fillA.setStyle(SkPaint::kFill_Style);
    fillB.setStyle(SkPaint::kFill_Style);
    TestCase fillACase(geo, fillA);
    TestCase fillBCase(geo, fillB);
    fillACase.compare(reporter, fillBCase, TestCase::kAllSame_ComparisonExpecation);

    // Make sure just applying the dash but not stroke gives the same key for both stroking
    // variations.
    SkPaint dashA = strokeA, dashB = strokeB;
    dashA.setPathEffect(make_dash());
    dashB.setPathEffect(make_dash());
    TestCase dashACase(geo, dashA);
    TestCase dashBCase(geo, dashB);
    dashACase.compare(reporter, dashBCase, TestCase::kSameUpToStroke_ComparisonExpecation);
}

template <typename GEO>
static void test_miter_limit(skiatest::Reporter* reporter, const GEO& geo) {
    // Miter limit should only matter when stroking with miter joins. It shouldn't affect other
    // joins or fills.
    SkPaint miterA;
    miterA.setStyle(SkPaint::kStroke_Style);
    miterA.setStrokeWidth(2.f);
    miterA.setStrokeJoin(SkPaint::kMiter_Join);
    miterA.setStrokeMiter(0.5f);
    SkPaint miterB = miterA;
    miterA.setStrokeMiter(0.6f);

    TestCase miterACase(geo, miterA);
    TestCase miterBCase(geo, miterB);
    miterACase.compare(reporter, miterBCase, TestCase::kSameUpToStroke_ComparisonExpecation);

    SkPaint noMiterA = miterA, noMiterB = miterB;
    noMiterA.setStrokeJoin(SkPaint::kRound_Join);
    noMiterB.setStrokeJoin(SkPaint::kRound_Join);
    TestCase noMiterACase(geo, noMiterA);
    TestCase noMiterBCase(geo, noMiterB);
    noMiterACase.compare(reporter, noMiterBCase, TestCase::kAllSame_ComparisonExpecation);

    SkPaint fillA = miterA, fillB = miterB;
    fillA.setStyle(SkPaint::kFill_Style);
    fillB.setStyle(SkPaint::kFill_Style);
    TestCase fillACase(geo, fillA);
    TestCase fillBCase(geo, fillB);
    fillACase.compare(reporter, fillBCase, TestCase::kAllSame_ComparisonExpecation);
}

template<typename GEO>
static void test_dash_fill(skiatest::Reporter* reporter, const GEO& geo) {
    // A dash with no stroke should have no effect
    using DashFactoryFn = sk_sp<SkPathEffect>(*)();
    for (DashFactoryFn md : {&make_dash, &make_null_dash}) {
        SkPaint dashFill;
        dashFill.setPathEffect((*md)());
        TestCase dashFillCase(geo, dashFill);

        TestCase fillCase(geo, SkPaint());
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

    TestCase fillCase(geo, fill);
    TestCase strokeCase(geo, stroke);
    TestCase dashCase(geo, dash);
    TestCase nullDashCase(geo, nullDash);

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
    TestCase fillGeoCase(geo, fill);

    SkPaint pe;
    pe.setPathEffect(RRectPathEffect::Make());
    TestCase geoPECase(geo, pe);

    SkPaint peStroke;
    peStroke.setPathEffect(RRectPathEffect::Make());
    peStroke.setStrokeWidth(2.f);
    peStroke.setStyle(SkPaint::kStroke_Style);
    TestCase geoPEStrokeCase(geo, peStroke);

    fillGeoCase.compare(reporter, geoPECase, TestCase::kSameUpToPE_ComparisonExpecation);
    fillGeoCase.compare(reporter, geoPEStrokeCase, TestCase::kSameUpToPE_ComparisonExpecation);
    geoPECase.compare(reporter, geoPEStrokeCase,
                      TestCase::kSameUpToStroke_ComparisonExpecation);

    TestCase rrectFillCase(RRectPathEffect::RRect(), fill);
    SkPaint stroke = peStroke;
    stroke.setPathEffect(nullptr);
    TestCase rrectStrokeCase(RRectPathEffect::RRect(), stroke);

    SkRRect rrect;
    // Applying the path effect should make a SkRRect shape. There is no further stroking in the
    // geoPECase, so the full style should be the same as just the PE.
    REPORTER_ASSERT(reporter, geoPECase.appliedPathEffectShape().asRRect(&rrect));
    REPORTER_ASSERT(reporter, rrect == RRectPathEffect::RRect());
    REPORTER_ASSERT(reporter, geoPECase.appliedPathEffectKey() == rrectFillCase.baseKey());

    REPORTER_ASSERT(reporter, geoPECase.appliedFullStyleShape().asRRect(&rrect));
    REPORTER_ASSERT(reporter, rrect == RRectPathEffect::RRect());
    REPORTER_ASSERT(reporter, geoPECase.appliedFullStyleKey() == rrectFillCase.baseKey());

    // In the PE+stroke case applying the full style should be the same as just stroking the rrect.
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectShape().asRRect(&rrect));
    REPORTER_ASSERT(reporter, rrect == RRectPathEffect::RRect());
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectKey() == rrectFillCase.baseKey());

    REPORTER_ASSERT(reporter, !geoPEStrokeCase.appliedFullStyleShape().asRRect(&rrect));
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

     // This path effect should make the keys invalid when it is applied. We only produce a pathe
     // effect key for dash path effects. So the only way another arbitrary path effect can produce
     // a styled result with a key is to produce a non-path shape that has a purely geometric key.
    SkPaint peStroke;
    peStroke.setPathEffect(AddLineTosPathEffect::Make());
    peStroke.setStrokeWidth(2.f);
    peStroke.setStyle(SkPaint::kStroke_Style);
    TestCase geoPEStrokeCase(geo, peStroke);
    TestCase::SelfExpectations expectations;
    expectations.fPEHasEffect = true;
    expectations.fPEHasValidKey = false;
    expectations.fStrokeApplies = true;
    geoPEStrokeCase.testExpectations(reporter, expectations);
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

    SkPaint pe;
    pe.setPathEffect(EmptyPathEffect::Make());
    TestCase geoCase(geo, pe);
    REPORTER_ASSERT(reporter, geoCase.appliedFullStyleKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoCase.appliedPathEffectKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoCase.appliedPathEffectThenStrokeKey() == emptyKey);

    SkPaint peStroke;
    peStroke.setPathEffect(EmptyPathEffect::Make());
    peStroke.setStrokeWidth(2.f);
    peStroke.setStyle(SkPaint::kStroke_Style);
    TestCase geoPEStrokeCase(geo, peStroke);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedFullStyleKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectThenStrokeKey() == emptyKey);
}

void test_empty_shape(skiatest::Reporter* reporter) {
    SkPath emptyPath;
    SkPaint fill;
    TestCase fillEmptyCase(emptyPath, fill);

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
    TestCase strokeEmptyCase(emptyPath2, stroke);
    strokeEmptyCase.compare(reporter, fillEmptyCase, TestCase::kAllSame_ComparisonExpecation);

    // Dashing and stroking an empty path should have no effect
    SkPath emptyPath3;
    SkPaint dashAndStroke;
    dashAndStroke.setPathEffect(make_dash());
    dashAndStroke.setStrokeWidth(2.f);
    dashAndStroke.setStyle(SkPaint::kStroke_Style);
    TestCase dashAndStrokeEmptyCase(emptyPath3, stroke);
    dashAndStrokeEmptyCase.compare(reporter, fillEmptyCase,
                                   TestCase::kAllSame_ComparisonExpecation);
}

DEF_TEST(GrShape, reporter) {
    sk_sp<SkPathEffect> dashPE = make_dash();

    for (auto rr : { SkRRect::MakeRect(SkRect::MakeWH(10, 10)),
                     SkRRect::MakeRectXY(SkRect::MakeWH(10, 10), 3, 4)}) {
        test_basic(reporter, rr);
        test_dash_fill(reporter, rr);
        test_null_dash(reporter, rr);
        // Test modifying various stroke params.
        test_stroke_param<SkRRect, SkScalar>(
                          reporter, rr,
                          [](SkPaint* p, SkScalar w) { p->setStrokeWidth(w);},
                          SkIntToScalar(2), SkIntToScalar(4));
        test_stroke_param<SkRRect, SkPaint::Cap>(
                          reporter, rr,
                          [](SkPaint* p, SkPaint::Cap c) { p->setStrokeCap(c);},
                          SkPaint::kButt_Cap, SkPaint::kRound_Cap);
        test_stroke_param<SkRRect, SkPaint::Join>(
                          reporter, rr,
                          [](SkPaint* p, SkPaint::Join j) { p->setStrokeJoin(j);},
                          SkPaint::kMiter_Join, SkPaint::kRound_Join);
        test_miter_limit(reporter, rr);
        test_path_effect_makes_rrect(reporter, rr);
        test_unknown_path_effect(reporter, rr);
        test_path_effect_makes_empty_shape(reporter, rr);
    }

    struct TestPath {
        TestPath(const SkPath& path, bool isRRectFill, bool isRRectStroke ,const SkRRect& rrect)
            : fPath(path)
            , fIsRRectForFill(isRRectFill)
            , fIsRRectForStroke(isRRectStroke)
            , fRRect(rrect) {}
        SkPath  fPath;
        bool    fIsRRectForFill;
        bool    fIsRRectForStroke;
        SkRRect fRRect;
    };
    SkTArray<TestPath> paths;

    SkPath circlePath;
    circlePath.addCircle(10, 10, 10);
    paths.emplace_back(circlePath, true, true, SkRRect::MakeOval(SkRect::MakeWH(20,20)));

    SkPath rectPath;
    rectPath.addRect(SkRect::MakeWH(10, 10));
    paths.emplace_back(rectPath, true, true, SkRRect::MakeRect(SkRect::MakeWH(10, 10)));

    SkPath openRectPath;
    openRectPath.moveTo(0, 0);
    openRectPath.lineTo(10, 0);
    openRectPath.lineTo(10, 10);
    openRectPath.lineTo(0, 10);
    paths.emplace_back(openRectPath, true, false, SkRRect::MakeRect(SkRect::MakeWH(10, 10)));

    SkPath quadPath;
    quadPath.quadTo(10, 10, 5, 8);
    paths.emplace_back(quadPath, false, false, SkRRect());

    for (auto testPath : paths) {
        const SkPath& path = testPath.fPath;
        // These tests all assume that the original GrShape for fill and stroke will be the same.
        // However, that is not the case in special cases (e.g. a unclosed rect becomes a RRect
        // GrShape with a fill style but becomes a Path GrShape when stroked).
        if (testPath.fIsRRectForFill == testPath.fIsRRectForStroke) {
            test_basic(reporter, path);
            test_null_dash(reporter, path);
            test_path_effect_makes_rrect(reporter, path);
        }
        test_dash_fill(reporter, path);
        // Test modifying various stroke params.
        test_stroke_param<SkPath, SkScalar>(
            reporter, path,
            [](SkPaint* p, SkScalar w) { p->setStrokeWidth(w);},
            SkIntToScalar(2), SkIntToScalar(4));
        test_stroke_param<SkPath, SkPaint::Cap>(
            reporter, path,
            [](SkPaint* p, SkPaint::Cap c) { p->setStrokeCap(c);},
            SkPaint::kButt_Cap, SkPaint::kRound_Cap);
        test_stroke_param<SkPath, SkPaint::Join>(
            reporter, path,
            [](SkPaint* p, SkPaint::Join j) { p->setStrokeJoin(j);},
            SkPaint::kMiter_Join, SkPaint::kRound_Join);
        test_miter_limit(reporter, path);
        test_unknown_path_effect(reporter, path);
        test_path_effect_makes_empty_shape(reporter, path);

        SkPaint fillPaint;
        TestCase fillPathCase(path, fillPaint);
        SkRRect rrect;
        REPORTER_ASSERT(reporter, testPath.fIsRRectForFill ==
                                  fillPathCase.baseShape().asRRect(&rrect));
        if (testPath.fIsRRectForFill) {
            REPORTER_ASSERT(reporter, rrect == testPath.fRRect);
            TestCase fillRRectCase(rrect, fillPaint);
            fillPathCase.compare(reporter, fillRRectCase, TestCase::kAllSame_ComparisonExpecation);
        }

        SkPaint strokePaint;
        strokePaint.setStrokeWidth(3.f);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        TestCase strokePathCase(path, strokePaint);
        REPORTER_ASSERT(reporter, testPath.fIsRRectForStroke ==
                                  strokePathCase.baseShape().asRRect(&rrect));
        if (testPath.fIsRRectForStroke) {
            REPORTER_ASSERT(reporter, rrect == testPath.fRRect);
            TestCase strokeRRectCase(rrect, strokePaint);
            strokePathCase.compare(reporter, strokeRRectCase,
                                 TestCase::kAllSame_ComparisonExpecation);
        }
    }

    test_empty_shape(reporter);
}

#endif
