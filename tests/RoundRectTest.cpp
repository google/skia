/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkRRect.h"

static const SkScalar kWidth = 100.0f;
static const SkScalar kHeight = 100.0f;

static void test_inset(skiatest::Reporter* reporter) {
    SkRRect rr, rr2;
    SkRect r = { 0, 0, 100, 100 };

    rr.setRect(r);
    rr.inset(-20, -20, &rr2);
    REPORTER_ASSERT(reporter, rr2.isRect());

    rr.inset(20, 20, &rr2);
    REPORTER_ASSERT(reporter, rr2.isRect());

    rr.inset(r.width()/2, r.height()/2, &rr2);
    REPORTER_ASSERT(reporter, rr2.isEmpty());

    rr.setRectXY(r, 20, 20);
    rr.inset(19, 19, &rr2);
    REPORTER_ASSERT(reporter, rr2.isSimple());
    rr.inset(20, 20, &rr2);
    REPORTER_ASSERT(reporter, rr2.isRect());
}

// Test out the basic API entry points
static void test_round_rect_basic(skiatest::Reporter* reporter) {
    // Test out initialization methods
    SkPoint zeroPt = { 0, 0 };
    SkRRect empty;

    empty.setEmpty();

    REPORTER_ASSERT(reporter, SkRRect::kEmpty_Type == empty.type());
    REPORTER_ASSERT(reporter, empty.rect().isEmpty());

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, zeroPt == empty.radii((SkRRect::Corner) i));
    }

    //----
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);

    SkRRect rr1;
    rr1.setRect(rect);

    REPORTER_ASSERT(reporter, SkRRect::kRect_Type == rr1.type());
    REPORTER_ASSERT(reporter, rr1.rect() == rect);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, zeroPt == rr1.radii((SkRRect::Corner) i));
    }

    //----
    SkPoint halfPoint = { SkScalarHalf(kWidth), SkScalarHalf(kHeight) };
    SkRRect rr2;
    rr2.setOval(rect);

    REPORTER_ASSERT(reporter, SkRRect::kOval_Type == rr2.type());
    REPORTER_ASSERT(reporter, rr2.rect() == rect);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter,
                        rr2.radii((SkRRect::Corner) i).equalsWithinTolerance(halfPoint));
    }

    //----
    SkPoint p = { 5, 5 };
    SkRRect rr3;
    rr3.setRectXY(rect, p.fX, p.fY);

    REPORTER_ASSERT(reporter, SkRRect::kSimple_Type == rr3.type());
    REPORTER_ASSERT(reporter, rr3.rect() == rect);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, p == rr3.radii((SkRRect::Corner) i));
    }

    //----
    SkPoint radii[4] = { { 5, 5 }, { 5, 5 }, { 5, 5 }, { 5, 5 } };

    SkRRect rr4;
    rr4.setRectRadii(rect, radii);

    REPORTER_ASSERT(reporter, SkRRect::kSimple_Type == rr4.type());
    REPORTER_ASSERT(reporter, rr4.rect() == rect);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, radii[i] == rr4.radii((SkRRect::Corner) i));
    }

    //----
    SkPoint radii2[4] = { { 0, 0 }, { 0, 0 }, { 50, 50 }, { 20, 50 } };

    SkRRect rr5;
    rr5.setRectRadii(rect, radii2);

    REPORTER_ASSERT(reporter, SkRRect::kComplex_Type == rr5.type());
    REPORTER_ASSERT(reporter, rr5.rect() == rect);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, radii2[i] == rr5.radii((SkRRect::Corner) i));
    }

    // Test out == & !=
    REPORTER_ASSERT(reporter, empty != rr3);
    REPORTER_ASSERT(reporter, rr3 == rr4);
    REPORTER_ASSERT(reporter, rr4 != rr5);
}

// Test out the cases when the RR degenerates to a rect
static void test_round_rect_rects(skiatest::Reporter* reporter) {
    SkRect r;
    static const SkPoint pts[] = {
        // Upper Left
        { -SK_Scalar1, -SK_Scalar1 },               // out
        { SK_Scalar1, SK_Scalar1 },                 // in
        // Upper Right
        { SkIntToScalar(101), -SK_Scalar1},         // out
        { SkIntToScalar(99), SK_Scalar1 },          // in
        // Lower Right
        { SkIntToScalar(101), SkIntToScalar(101) }, // out
        { SkIntToScalar(99), SkIntToScalar(99) },   // in
        // Lower Left
        { -SK_Scalar1, SkIntToScalar(101) },        // out
        { SK_Scalar1, SkIntToScalar(99) },          // in
        // Middle
        { SkIntToScalar(50), SkIntToScalar(50) }    // in
    };
    static const bool isIn[] = { false, true, false, true, false, true, false, true, true };

    SkASSERT(SK_ARRAY_COUNT(pts) == SK_ARRAY_COUNT(isIn));

    //----
    SkRRect empty;

    empty.setEmpty();

    REPORTER_ASSERT(reporter, SkRRect::kEmpty_Type == empty.type());
    r = empty.rect();
    REPORTER_ASSERT(reporter, 0 == r.fLeft && 0 == r.fTop && 0 == r.fRight && 0 == r.fBottom);

    //----
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
    SkRRect rr1;
    rr1.setRectXY(rect, 0, 0);

    REPORTER_ASSERT(reporter, SkRRect::kRect_Type == rr1.type());
    r = rr1.rect();
    REPORTER_ASSERT(reporter, rect == r);
    for (size_t i = 0; i < SK_ARRAY_COUNT(pts); ++i) {
        REPORTER_ASSERT(reporter, isIn[i] == rr1.contains(pts[i].fX, pts[i].fY));
    }

    //----
    SkPoint radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };

    SkRRect rr2;
    rr2.setRectRadii(rect, radii);

    REPORTER_ASSERT(reporter, SkRRect::kRect_Type == rr2.type());
    r = rr2.rect();
    REPORTER_ASSERT(reporter, rect == r);
    for (size_t i = 0; i < SK_ARRAY_COUNT(pts); ++i) {
        REPORTER_ASSERT(reporter, isIn[i] == rr2.contains(pts[i].fX, pts[i].fY));
    }

    //----
    SkPoint radii2[4] = { { 0, 0 }, { 20, 20 }, { 50, 50 }, { 20, 50 } };

    SkRRect rr3;
    rr3.setRectRadii(rect, radii2);
    REPORTER_ASSERT(reporter, SkRRect::kComplex_Type == rr3.type());
}

// Test out the cases when the RR degenerates to an oval
static void test_round_rect_ovals(skiatest::Reporter* reporter) {
    static const SkScalar kEps = 0.1f;
    static const SkScalar kWidthTol = SkScalarHalf(kWidth) * (SK_Scalar1 - SK_ScalarRoot2Over2);
    static const SkScalar kHeightTol = SkScalarHalf(kHeight) * (SK_Scalar1 - SK_ScalarRoot2Over2);
    static const SkPoint pts[] = {
        // Upper Left
        { kWidthTol - kEps, kHeightTol - kEps },       // out
        { kWidthTol + kEps, kHeightTol + kEps },       // in
        // Upper Right
        { kWidth + kEps - kWidthTol, kHeightTol - kEps },     // out
        { kWidth - kEps - kWidthTol, kHeightTol + kEps },      // in
        // Lower Right
        { kWidth + kEps - kWidthTol, kHeight + kEps - kHeightTol },   // out
        { kWidth - kEps - kWidthTol, kHeight - kEps - kHeightTol },   // in
        // Lower Left
        { kWidthTol - kEps, kHeight + kEps - kHeightTol },     //out
        { kWidthTol + kEps, kHeight - kEps - kHeightTol },     // in
        // Middle
        { SkIntToScalar(50), SkIntToScalar(50) } // in
    };
    static const bool isIn[] = { false, true, false, true, false, true, false, true, true };

    SkASSERT(SK_ARRAY_COUNT(pts) == SK_ARRAY_COUNT(isIn));

    //----
    SkRect oval;
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
    SkRRect rr1;
    rr1.setRectXY(rect, SkScalarHalf(kWidth), SkScalarHalf(kHeight));

    REPORTER_ASSERT(reporter, SkRRect::kOval_Type == rr1.type());
    oval = rr1.rect();
    REPORTER_ASSERT(reporter, oval == rect);
    for (size_t i = 0; i < SK_ARRAY_COUNT(pts); ++i) {
        REPORTER_ASSERT(reporter, isIn[i] == rr1.contains(pts[i].fX, pts[i].fY));
    }
}

// Test out the non-degenerate RR cases
static void test_round_rect_general(skiatest::Reporter* reporter) {
    static const SkScalar kEps = 0.1f;
    static const SkScalar kDist20 = 20 * (SK_Scalar1 - SK_ScalarRoot2Over2);
    static const SkPoint pts[] = {
        // Upper Left
        { kDist20 - kEps, kDist20 - kEps },       // out
        { kDist20 + kEps, kDist20 + kEps },       // in
        // Upper Right
        { kWidth + kEps - kDist20, kDist20 - kEps },     // out
        { kWidth - kEps - kDist20, kDist20 + kEps },      // in
        // Lower Right
        { kWidth + kEps - kDist20, kHeight + kEps - kDist20 },   // out
        { kWidth - kEps - kDist20, kHeight - kEps - kDist20 },   // in
        // Lower Left
        { kDist20 - kEps, kHeight + kEps - kDist20 },     //out
        { kDist20 + kEps, kHeight - kEps - kDist20 },     // in
        // Middle
        { SkIntToScalar(50), SkIntToScalar(50) } // in
    };
    static const bool isIn[] = { false, true, false, true, false, true, false, true, true };

    SkASSERT(SK_ARRAY_COUNT(pts) == SK_ARRAY_COUNT(isIn));

    //----
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
    SkRRect rr1;
    rr1.setRectXY(rect, 20, 20);

    REPORTER_ASSERT(reporter, SkRRect::kSimple_Type == rr1.type());
    for (size_t i = 0; i < SK_ARRAY_COUNT(pts); ++i) {
        REPORTER_ASSERT(reporter, isIn[i] == rr1.contains(pts[i].fX, pts[i].fY));
    }

    //----
    static const SkScalar kDist50 = 50*(SK_Scalar1 - SK_ScalarRoot2Over2);
    static const SkPoint pts2[] = {
        // Upper Left
        { -SK_Scalar1, -SK_Scalar1 },           // out
        { SK_Scalar1, SK_Scalar1 },             // in
        // Upper Right
        { kWidth + kEps - kDist20, kDist20 - kEps },     // out
        { kWidth - kEps - kDist20, kDist20 + kEps },     // in
        // Lower Right
        { kWidth + kEps - kDist50, kHeight + kEps - kDist50 },   // out
        { kWidth - kEps - kDist50, kHeight - kEps - kDist50 },   // in
        // Lower Left
        { kDist20 - kEps, kHeight + kEps - kDist50 },     // out
        { kDist20 + kEps, kHeight - kEps - kDist50 },     // in
        // Middle
        { SkIntToScalar(50), SkIntToScalar(50) }  // in
    };

    SkASSERT(SK_ARRAY_COUNT(pts2) == SK_ARRAY_COUNT(isIn));

    SkPoint radii[4] = { { 0, 0 }, { 20, 20 }, { 50, 50 }, { 20, 50 } };

    SkRRect rr2;
    rr2.setRectRadii(rect, radii);

    REPORTER_ASSERT(reporter, SkRRect::kComplex_Type == rr2.type());
    for (size_t i = 0; i < SK_ARRAY_COUNT(pts); ++i) {
        REPORTER_ASSERT(reporter, isIn[i] == rr2.contains(pts2[i].fX, pts2[i].fY));
    }
}

// Test out questionable-parameter handling
static void test_round_rect_iffy_parameters(skiatest::Reporter* reporter) {

    // When the radii exceed the base rect they are proportionally scaled down
    // to fit
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
    SkPoint radii[4] = { { 50, 100 }, { 100, 50 }, { 50, 100 }, { 100, 50 } };

    SkRRect rr1;
    rr1.setRectRadii(rect, radii);

    REPORTER_ASSERT(reporter, SkRRect::kComplex_Type == rr1.type());

    const SkPoint& p = rr1.radii(SkRRect::kUpperLeft_Corner);

    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(p.fX, 33.33333f));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(p.fY, 66.66666f));

    // Negative radii should be capped at zero
    SkRRect rr2;
    rr2.setRectXY(rect, -10, -20);

    REPORTER_ASSERT(reporter, SkRRect::kRect_Type == rr2.type());

    const SkPoint& p2 = rr2.radii(SkRRect::kUpperLeft_Corner);

    REPORTER_ASSERT(reporter, 0.0f == p2.fX);
    REPORTER_ASSERT(reporter, 0.0f == p2.fY);
}

static void TestRoundRect(skiatest::Reporter* reporter) {
    test_round_rect_basic(reporter);
    test_round_rect_rects(reporter);
    test_round_rect_ovals(reporter);
    test_round_rect_general(reporter);
    test_round_rect_iffy_parameters(reporter);
    test_inset(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("RoundRect", TestRoundRectClass, TestRoundRect)
