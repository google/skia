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
    TestCase(const GEO& geo, const SkPaint& paint, skiatest::Reporter* r) : fBase(geo, paint) {
        this->init(r);
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
    void init(skiatest::Reporter* r) {
        fAppliedPE           = fBase.applyStyle(GrStyle::Apply::kPathEffectOnly);
        fAppliedPEThenStroke = fAppliedPE.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec);
        fAppliedFull         = fBase.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec);

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
        REPORTER_ASSERT(r, a == b);

        // Check that the same path is produced when style is applied by GrShape and GrStyle.
        SkPath preStyle;
        SkPath postPathEffect;
        SkPath postAllStyle;

        fBase.asPath(&preStyle);
        SkStrokeRec postPathEffectStrokeRec(SkStrokeRec::kFill_InitStyle);
        if (fBase.style().applyPathEffectToPath(&postPathEffect, &postPathEffectStrokeRec,
                                                preStyle)) {
            SkPath testPath;
            fAppliedPE.asPath(&testPath);
            REPORTER_ASSERT(r, testPath == postPathEffect);
            REPORTER_ASSERT(r,
                            postPathEffectStrokeRec.hasEqualEffect(fAppliedPE.style().strokeRec()));
        }
        SkStrokeRec::InitStyle fillOrHairline;
        if (fBase.style().applyToPath(&postAllStyle, &fillOrHairline, preStyle)) {
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
    expectations.fPEHasEffect = true;
    expectations.fStrokeApplies = true;
    stroke2RoundBevelAndFillDashCase.testExpectations(reporter, expectations);
    TestCase(geo, stroke2RoundBevelAndFillDash, reporter).compare(
        reporter, stroke2RoundBevelAndFillDashCase, TestCase::kAllSame_ComparisonExpecation);

    stroke2RoundBevelAndFillCase.compare(reporter, stroke2RoundBevelCase,
                                         TestCase::kSameUpToStroke_ComparisonExpecation);
    stroke2RoundBevelAndFillDashCase.compare(reporter, stroke2RoundBevelDashCase,
                                             TestCase::kSameUpToStroke_ComparisonExpecation);
    stroke2RoundBevelAndFillCase.compare(reporter, stroke2RoundBevelAndFillDashCase,
                                         TestCase::kSameUpToPE_ComparisonExpecation);

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

    SkPaint dashStrokeAndFillA = dashA, dashStrokeAndFillB = dashB;
    dashStrokeAndFillA.setStyle(SkPaint::kStrokeAndFill_Style);
    dashStrokeAndFillB.setStyle(SkPaint::kStrokeAndFill_Style);
    TestCase dashStrokeAndFillACase(geo, dashStrokeAndFillA, reporter);
    TestCase dashStrokeAndFillBCase(geo, dashStrokeAndFillB, reporter);
    if (paramAffectsDashAndStroke) {
        dashStrokeAndFillACase.compare(reporter, dashStrokeAndFillBCase,
                                       TestCase::kSameUpToStroke_ComparisonExpecation);
    } else {
        dashStrokeAndFillACase.compare(reporter, dashStrokeAndFillBCase,
                                       TestCase::kAllSame_ComparisonExpecation);
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
        void computeFastBounds(SkRect* dst, const SkRect& src) const override {  *dst = src; }
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

    SkPath a, b;
    peCase.baseShape().asPath(&a);
    peCase.appliedPathEffectShape().asPath(&b);
    REPORTER_ASSERT(reporter, a == b);
    peCase.appliedFullStyleShape().asPath(&b);
    REPORTER_ASSERT(reporter, a == b);
    REPORTER_ASSERT(reporter, peCase.appliedPathEffectShape().style().isSimpleHairline());
    REPORTER_ASSERT(reporter, peCase.appliedFullStyleShape().style().isSimpleHairline());
    if (isNonPath) {
        REPORTER_ASSERT(reporter, peCase.appliedPathEffectKey() == peCase.baseKey());
        REPORTER_ASSERT(reporter, peCase.appliedFullStyleKey() == peCase.baseKey());
    } else {
        REPORTER_ASSERT(reporter, peCase.appliedPathEffectKey().empty());
        REPORTER_ASSERT(reporter, peCase.appliedFullStyleKey().empty());
    }
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

    SkPaint pe;
    pe.setPathEffect(EmptyPathEffect::Make());
    TestCase geoCase(geo, pe, reporter);
    REPORTER_ASSERT(reporter, geoCase.appliedFullStyleKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoCase.appliedPathEffectKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoCase.appliedPathEffectThenStrokeKey() == emptyKey);

    SkPaint peStroke;
    peStroke.setPathEffect(EmptyPathEffect::Make());
    peStroke.setStrokeWidth(2.f);
    peStroke.setStyle(SkPaint::kStroke_Style);
    TestCase geoPEStrokeCase(geo, peStroke, reporter);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedFullStyleKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectKey() == emptyKey);
    REPORTER_ASSERT(reporter, geoPEStrokeCase.appliedPathEffectThenStrokeKey() == emptyKey);
}

void test_empty_shape(skiatest::Reporter* reporter) {
    SkPath emptyPath;
    SkPaint fill;
    TestCase fillEmptyCase(emptyPath, fill, reporter);

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
        test_stroke_param<SkRRect, SkPaint::Join>(
                          reporter, rr,
                          [](SkPaint* p, SkPaint::Join j) { p->setStrokeJoin(j);},
                          SkPaint::kMiter_Join, SkPaint::kRound_Join);
        test_stroke_cap(reporter, rr);
        test_miter_limit(reporter, rr);
        test_path_effect_makes_rrect(reporter, rr);
        test_unknown_path_effect(reporter, rr);
        test_path_effect_makes_empty_shape(reporter, rr);
        test_make_hairline_path_effect(reporter, rr, true);
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
        test_make_hairline_path_effect(reporter, path, testPath.fIsRRectForStroke);

        SkPaint fillPaint;
        TestCase fillPathCase(path, fillPaint, reporter);
        SkRRect rrect;
        REPORTER_ASSERT(reporter, testPath.fIsRRectForFill ==
                                  fillPathCase.baseShape().asRRect(&rrect));
        if (testPath.fIsRRectForFill) {
            REPORTER_ASSERT(reporter, rrect == testPath.fRRect);
            TestCase fillRRectCase(rrect, fillPaint, reporter);
            fillPathCase.compare(reporter, fillRRectCase, TestCase::kAllSame_ComparisonExpecation);
        }

        SkPaint strokePaint;
        strokePaint.setStrokeWidth(3.f);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        TestCase strokePathCase(path, strokePaint, reporter);
        REPORTER_ASSERT(reporter, testPath.fIsRRectForStroke ==
                                  strokePathCase.baseShape().asRRect(&rrect));
        if (testPath.fIsRRectForStroke) {
            REPORTER_ASSERT(reporter, rrect == testPath.fRRect);
            TestCase strokeRRectCase(rrect, strokePaint, reporter);
            strokePathCase.compare(reporter, strokeRRectCase,
                                 TestCase::kAllSame_ComparisonExpecation);
        }
    }

    // Test a volatile empty path.
    test_volatile_path(reporter, SkPath(), true);

    test_empty_shape(reporter);
}

#endif
