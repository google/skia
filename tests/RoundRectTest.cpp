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

// Move a small box from the start position by (stepX, stepY) 'numSteps' times
// testing for containment in 'rr' at each step.
static void test_direction(skiatest::Reporter* reporter, const SkRRect &rr,
                           SkScalar initX, int stepX, SkScalar initY, int stepY,
                           int numSteps, const bool* contains) {
    SkScalar x = initX, y = initY;
    for (int i = 0; i < numSteps; ++i) {
        SkRect test = SkRect::MakeXYWH(x, y,
                                       stepX ? SkIntToScalar(stepX) : SK_Scalar1,
                                       stepY ? SkIntToScalar(stepY) : SK_Scalar1);
        test.sort();

        REPORTER_ASSERT(reporter, contains[i] == rr.contains(test));

        x += stepX;
        y += stepY;
    }
}

// Exercise the RR's contains rect method
static void test_round_rect_contains_rect(skiatest::Reporter* reporter) {

    static const int kNumRRects = 4;
    static const SkVector gRadii[kNumRRects][4] = {
        { {  0,  0 }, {  0,  0 }, {  0,  0 }, {  0,  0 } },  // rect
        { { 20, 20 }, { 20, 20 }, { 20, 20 }, { 20, 20 } },  // circle
        { { 10, 10 }, { 10, 10 }, { 10, 10 }, { 10, 10 } },  // simple
        { {  0,  0 }, { 20, 20 }, { 10, 10 }, { 30, 30 } }   // complex
    };

    SkRRect rrects[kNumRRects];
    for (int i = 0; i < kNumRRects; ++i) {
        rrects[i].setRectRadii(SkRect::MakeWH(40, 40), gRadii[i]);
    }

    // First test easy outs - boxes that are obviously out on
    // each corner and edge
    static const SkRect easyOuts[] = {
        { -5, -5,  5,  5 }, // NW
        { 15, -5, 20,  5 }, // N
        { 35, -5, 45,  5 }, // NE
        { 35, 15, 45, 20 }, // E
        { 35, 45, 35, 45 }, // SE
        { 15, 35, 20, 45 }, // S
        { -5, 35,  5, 45 }, // SW
        { -5, 15,  5, 20 }  // W
    };

    for (int i = 0; i < kNumRRects; ++i) {
        for (size_t j = 0; j < SK_ARRAY_COUNT(easyOuts); ++j) {
            REPORTER_ASSERT(reporter, !rrects[i].contains(easyOuts[j]));
        }
    }

    // Now test non-trivial containment. For each compass
    // point walk a 1x1 rect in from the edge  of the bounding
    // rect
    static const int kNumSteps = 15;
    bool answers[kNumRRects][8][kNumSteps] = {
        // all the test rects are inside the degenerate rrect
        {
            // rect
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        },
        // for the circle we expect 6 blocks to be out on the
        // corners (then the rest in) and only the first block
        // out on the vertical and horizontal axes (then
        // the rest in)
        {
            // circle
            { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        },
        // for the simple round rect we expect 3 out on
        // the corners (then the rest in) and no blocks out
        // on the vertical and horizontal axes
        {
            // simple RR
            { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
        },
        // for the complex case the answer is different for each direction
        {
            // complex RR
            // all in for NW (rect) corner (same as rect case)
            { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            // only first block out for N (same as circle case)
            { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            // first 6 blocks out for NE (same as circle case)
            { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            // only first block out for E (same as circle case)
            { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            // first 3 blocks out for SE (same as simple case)
            { 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            // first two blocks out for S
            { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
            // first 9 blocks out for SW
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
            // first two blocks out for W (same as S)
            { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
         }
    };

    for (int i = 0; i < kNumRRects; ++i) {
        test_direction(reporter, rrects[i],     0,  1,     0,  1, kNumSteps, answers[i][0]); // NW
        test_direction(reporter, rrects[i], 19.5f,  0,     0,  1, kNumSteps, answers[i][1]); // N
        test_direction(reporter, rrects[i],    40, -1,     0,  1, kNumSteps, answers[i][2]); // NE
        test_direction(reporter, rrects[i],    40, -1, 19.5f,  0, kNumSteps, answers[i][3]); // E
        test_direction(reporter, rrects[i],    40, -1,    40, -1, kNumSteps, answers[i][4]); // SE
        test_direction(reporter, rrects[i], 19.5f,  0,    40, -1, kNumSteps, answers[i][5]); // S
        test_direction(reporter, rrects[i],     0,  1,    40, -1, kNumSteps, answers[i][6]); // SW
        test_direction(reporter, rrects[i],     0,  1, 19.5f,  0, kNumSteps, answers[i][7]); // W
    }
}

static void TestRoundRect(skiatest::Reporter* reporter) {
    test_round_rect_basic(reporter);
    test_round_rect_rects(reporter);
    test_round_rect_ovals(reporter);
    test_round_rect_general(reporter);
    test_round_rect_iffy_parameters(reporter);
    test_inset(reporter);
    test_round_rect_contains_rect(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("RoundRect", TestRoundRectClass, TestRoundRect)
