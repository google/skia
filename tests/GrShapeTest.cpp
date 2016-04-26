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

namespace {
class TestCase {
public:
    TestCase(const SkRRect& rrect, const SkPaint& paint) : fBase(rrect, paint) {
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

private:
    void init() {
        fAppliedPE           = fBase.applyPathEffect();
        fAppliedPEThenStroke = fAppliedPE.applyFullStyle();
        fAppliedFull         = fBase.applyFullStyle();

        fBaseKeyIsValid                = MakeKey(&fBaseKey, fBase);
        fAppliedPEKeyIsValid           = MakeKey(&fAppliedPEKey, fAppliedPE);
        fAppliedPEThenStrokeKeyIsValid = MakeKey(&fAppliedPEThenStrokeKey, fAppliedPEThenStroke);
        fAppliedFullKeyIsValid         = MakeKey(&fAppliedFullKey, fAppliedFull);
    }

    using Key = SkTArray<uint32_t>;

    static bool MakeKey(Key* key, const GrShape& shape) {
        int size = shape.unstyledKeySize();
        if (size <= 0) {
            return false;
        }
        key->reset(size);
        shape.writeUnstyledKey(key->begin());
        return true;
    }

    GrShape fBase;
    GrShape fAppliedPE;
    GrShape fAppliedPEThenStroke;
    GrShape fAppliedFull;

    Key fBaseKey;
    Key fAppliedPEKey;
    Key fAppliedPEThenStrokeKey;
    Key fAppliedFullKey;

    bool fBaseKeyIsValid;
    bool fAppliedPEKeyIsValid;
    bool fAppliedPEThenStrokeKeyIsValid;
    bool fAppliedFullKeyIsValid;
};

void TestCase::testExpectations(skiatest::Reporter* reporter, SelfExpectations expectations) const {
    // Applying the path effect and then the stroke should always be the same as applying
    // both in one go.
    REPORTER_ASSERT(reporter, fAppliedPEThenStrokeKey == fAppliedFullKey);
    // The base's key should always be valid (unless the path is volatile)
    REPORTER_ASSERT(reporter, fBaseKeyIsValid);
    if (expectations.fPEHasEffect) {
        REPORTER_ASSERT(reporter, fBaseKey != fAppliedPEKey);
        REPORTER_ASSERT(reporter, expectations.fPEHasEffect == fAppliedPEKeyIsValid);
        REPORTER_ASSERT(reporter, fBaseKey != fAppliedFullKey);
        REPORTER_ASSERT(reporter, expectations.fPEHasEffect == fAppliedFullKeyIsValid);
        if (expectations.fStrokeApplies && expectations.fPEHasValidKey) {
            REPORTER_ASSERT(reporter, fAppliedPEKey != fAppliedFullKey);
            REPORTER_ASSERT(reporter, expectations.fPEHasEffect == fAppliedFullKeyIsValid);
        }
    } else {
        REPORTER_ASSERT(reporter, fBaseKey == fAppliedPEKey);
        if (expectations.fStrokeApplies) {
            REPORTER_ASSERT(reporter, fBaseKey != fAppliedFullKey);
        } else {
            REPORTER_ASSERT(reporter, fBaseKey == fAppliedFullKey);
        }
    }
}

void TestCase::compare(skiatest::Reporter* reporter, const TestCase& that,
                       ComparisonExpecation expectation) const {
    switch (expectation) {
        case kAllDifferent_ComparisonExpecation:
            REPORTER_ASSERT(reporter, fBaseKey != that.fBaseKey);
            REPORTER_ASSERT(reporter, fAppliedPEKey != that.fAppliedPEKey);
            REPORTER_ASSERT(reporter, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kSameUpToPE_ComparisonExpecation:
            REPORTER_ASSERT(reporter, fBaseKey == that.fBaseKey);
            REPORTER_ASSERT(reporter, fAppliedPEKey != that.fAppliedPEKey);
            REPORTER_ASSERT(reporter, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kSameUpToStroke_ComparisonExpecation:
            REPORTER_ASSERT(reporter, fBaseKey == that.fBaseKey);
            REPORTER_ASSERT(reporter, fAppliedPEKey == that.fAppliedPEKey);
            REPORTER_ASSERT(reporter, fAppliedFullKey != that.fAppliedFullKey);
            break;
        case kAllSame_ComparisonExpecation:
            REPORTER_ASSERT(reporter, fBaseKey == that.fBaseKey);
            REPORTER_ASSERT(reporter, fAppliedPEKey == that.fAppliedPEKey);
            REPORTER_ASSERT(reporter, fAppliedFullKey == that.fAppliedFullKey);
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

static void test_basic(skiatest::Reporter* reporter, const SkRRect& rrect) {
    sk_sp<SkPathEffect> dashPE = make_dash();

    TestCase::SelfExpectations expectations;
    SkPaint fill;

    TestCase fillCase(rrect, fill);
    expectations.fPEHasEffect = false;
    expectations.fPEHasValidKey = false;
    expectations.fStrokeApplies = false;
    fillCase.testExpectations(reporter, expectations);
    // Test that another GrShape instance built from the same primitive is the same.
    TestCase(rrect, fill).compare(reporter, fillCase, TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke2RoundBevel;
    stroke2RoundBevel.setStyle(SkPaint::kStroke_Style);
    stroke2RoundBevel.setStrokeCap(SkPaint::kRound_Cap);
    stroke2RoundBevel.setStrokeJoin(SkPaint::kBevel_Join);
    stroke2RoundBevel.setStrokeWidth(2.f);
    TestCase stroke2RoundBevelCase(rrect, stroke2RoundBevel);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = false;
    expectations.fStrokeApplies = true;
    stroke2RoundBevelCase.testExpectations(reporter, expectations);
    TestCase(rrect, stroke2RoundBevel).compare(reporter, stroke2RoundBevelCase,
                                               TestCase::kAllSame_ComparisonExpecation);

    SkPaint stroke2RoundBevelDash = stroke2RoundBevel;
    stroke2RoundBevelDash.setPathEffect(make_dash());
    TestCase stroke2RoundBevelDashCase(rrect, stroke2RoundBevelDash);
    expectations.fPEHasValidKey = true;
    expectations.fPEHasEffect = true;
    expectations.fStrokeApplies = true;
    stroke2RoundBevelDashCase.testExpectations(reporter, expectations);
    TestCase(rrect, stroke2RoundBevelDash).compare(reporter, stroke2RoundBevelDashCase,
                                                   TestCase::kAllSame_ComparisonExpecation);

    fillCase.compare(reporter, stroke2RoundBevelCase,
                     TestCase::kSameUpToStroke_ComparisonExpecation);
    fillCase.compare(reporter, stroke2RoundBevelDashCase,
                     TestCase::kSameUpToPE_ComparisonExpecation);
    stroke2RoundBevelCase.compare(reporter, stroke2RoundBevelDashCase,
                                  TestCase::kSameUpToPE_ComparisonExpecation);
}

template <typename T>
static void test_stroke_param(skiatest::Reporter* reporter, const SkRRect& rrect,
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

    TestCase strokeACase(rrect, strokeA);
    TestCase strokeBCase(rrect, strokeB);
    strokeACase.compare(reporter, strokeBCase, TestCase::kSameUpToStroke_ComparisonExpecation);

    // Make sure stroking params don't affect fill style.
    SkPaint fillA = strokeA, fillB = strokeB;
    fillA.setStyle(SkPaint::kFill_Style);
    fillB.setStyle(SkPaint::kFill_Style);
    TestCase fillACase(rrect, fillA);
    TestCase fillBCase(rrect, fillB);
    fillACase.compare(reporter, fillBCase, TestCase::kAllSame_ComparisonExpecation);

    // Make sure just applying the dash but not stroke gives the same key for both stroking
    // variations.
    SkPaint dashA = strokeA, dashB = strokeB;
    dashA.setPathEffect(make_dash());
    dashB.setPathEffect(make_dash());
    TestCase dashACase(rrect, dashA);
    TestCase dashBCase(rrect, dashB);
    dashACase.compare(reporter, dashBCase, TestCase::kSameUpToStroke_ComparisonExpecation);
}

static void test_miter_limit(skiatest::Reporter* reporter, const SkRRect& rrect) {
    // Miter limit should only matter when stroking with miter joins. It shouldn't affect other
    // joins or fills.
    SkPaint miterA;
    miterA.setStyle(SkPaint::kStroke_Style);
    miterA.setStrokeWidth(2.f);
    miterA.setStrokeJoin(SkPaint::kMiter_Join);
    miterA.setStrokeMiter(0.5f);
    SkPaint miterB = miterA;
    miterA.setStrokeMiter(0.6f);

    TestCase miterACase(rrect, miterA);
    TestCase miterBCase(rrect, miterB);
    miterACase.compare(reporter, miterBCase, TestCase::kSameUpToStroke_ComparisonExpecation);

    SkPaint noMiterA = miterA, noMiterB = miterB;
    noMiterA.setStrokeJoin(SkPaint::kRound_Join);
    noMiterB.setStrokeJoin(SkPaint::kRound_Join);
    TestCase noMiterACase(rrect, noMiterA);
    TestCase noMiterBCase(rrect, noMiterB);
    noMiterACase.compare(reporter, noMiterBCase, TestCase::kAllSame_ComparisonExpecation);

    SkPaint fillA = miterA, fillB = miterB;
    fillA.setStyle(SkPaint::kFill_Style);
    fillB.setStyle(SkPaint::kFill_Style);
    TestCase fillACase(rrect, fillA);
    TestCase fillBCase(rrect, fillB);
    fillACase.compare(reporter, fillBCase, TestCase::kAllSame_ComparisonExpecation);
}

static void test_dash_fill(skiatest::Reporter* reporter, const SkRRect& rrect) {
    // A dash with no stroke should have no effect
    using DashFactoryFn = sk_sp<SkPathEffect>(*)();
    for (DashFactoryFn md : {&make_dash, &make_null_dash}) {
        SkPaint dashFill;
        dashFill.setPathEffect((*md)());
        TestCase dashFillCase(rrect, dashFill);

        TestCase fillCase(rrect, SkPaint());
        dashFillCase.compare(reporter, fillCase, TestCase::kAllSame_ComparisonExpecation);
    }
}

void test_null_dash(skiatest::Reporter* reporter, const SkRRect& rrect) {
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

    TestCase fillCase(rrect, fill);
    TestCase strokeCase(rrect, stroke);
    TestCase dashCase(rrect, dash);
    TestCase nullDashCase(rrect, nullDash);

    nullDashCase.compare(reporter, fillCase, TestCase::kSameUpToStroke_ComparisonExpecation);
    nullDashCase.compare(reporter, strokeCase, TestCase::kAllSame_ComparisonExpecation);
    nullDashCase.compare(reporter, dashCase, TestCase::kSameUpToPE_ComparisonExpecation);
}

DEF_TEST(GrShape, reporter) {
    sk_sp<SkPathEffect> dashPE = make_dash();

    for (auto rr : { SkRRect::MakeRect(SkRect::MakeWH(10, 10)),
                     SkRRect::MakeRectXY(SkRect::MakeWH(10, 10), 3, 4)}) {
        test_basic(reporter, rr);
        test_dash_fill(reporter, rr);
        test_null_dash(reporter, rr);
        // Test modifying various stroke params.
        test_stroke_param<SkScalar>(
                          reporter, rr,
                          [](SkPaint* p, SkScalar w) { p->setStrokeWidth(w);},
                          SkIntToScalar(2), SkIntToScalar(4));
        test_stroke_param<SkPaint::Cap>(
                          reporter, rr,
                          [](SkPaint* p, SkPaint::Cap c) { p->setStrokeCap(c);},
                          SkPaint::kButt_Cap, SkPaint::kRound_Cap);
        test_stroke_param<SkPaint::Join>(
                          reporter, rr,
                          [](SkPaint* p, SkPaint::Join j) { p->setStrokeJoin(j);},
                          SkPaint::kMiter_Join, SkPaint::kRound_Join);
        test_miter_limit(reporter, rr);
    }
}

#endif
