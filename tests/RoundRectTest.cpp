/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrix.h"
#include "SkPointPriv.h"
#include "SkRRect.h"
#include "Test.h"

static void test_tricky_radii(skiatest::Reporter* reporter) {
    {
        // crbug.com/458522
        SkRRect rr;
        const SkRect bounds = { 3709, 3709, 3709 + 7402, 3709 + 29825 };
        const SkScalar rad = 12814;
        const SkVector vec[] = { { rad, rad }, { 0, rad }, { rad, rad }, { 0, rad } };
        rr.setRectRadii(bounds, vec);
    }

    {
        // crbug.com//463920
        SkRect r = SkRect::MakeLTRB(0, 0, 1009, 33554432.0);
        SkVector radii[4] = {
            { 13.0f, 8.0f }, { 170.0f, 2.0 }, { 256.0f, 33554432.0 }, { 110.0f, 5.0f }
        };
        SkRRect rr;
        rr.setRectRadii(r, radii);

        REPORTER_ASSERT(reporter, (double) rr.radii(SkRRect::kUpperRight_Corner).fY +
                                  (double) rr.radii(SkRRect::kLowerRight_Corner).fY <=
                                  rr.height());
    }
}

static void test_empty_crbug_458524(skiatest::Reporter* reporter) {
    SkRRect rr;
    const SkRect bounds = { 3709, 3709, 3709 + 7402, 3709 + 29825 };
    const SkScalar rad = 40;
    rr.setRectXY(bounds, rad, rad);

    SkRRect other;
    SkMatrix matrix;
    matrix.setScale(0, 1);
    rr.transform(matrix, &other);
    REPORTER_ASSERT(reporter, SkRRect::kEmpty_Type == other.getType());
}

// Test that all the SkRRect entry points correctly handle un-sorted and
// zero-sized input rects
static void test_empty(skiatest::Reporter* reporter) {
    static const SkRect oooRects[] = {  // out of order
        { 100, 0, 0, 100 },  // ooo horizontal
        { 0, 100, 100, 0 },  // ooo vertical
        { 100, 100, 0, 0 },  // ooo both
    };

    static const SkRect emptyRects[] = {
        { 100, 100, 100, 200 }, // empty horizontal
        { 100, 100, 200, 100 }, // empty vertical
        { 100, 100, 100, 100 }, // empty both
        { 0, 0, 0, 0 }          // setEmpty-empty
    };

    static const SkVector radii[4] = { { 0, 1 }, { 2, 3 }, { 4, 5 }, { 6, 7 } };

    SkRRect r;

    for (size_t i = 0; i < SK_ARRAY_COUNT(oooRects); ++i) {
        r.setRect(oooRects[i]);
        REPORTER_ASSERT(reporter, !r.isEmpty());
        REPORTER_ASSERT(reporter, r.rect() == oooRects[i].makeSorted());

        r.setOval(oooRects[i]);
        REPORTER_ASSERT(reporter, !r.isEmpty());
        REPORTER_ASSERT(reporter, r.rect() == oooRects[i].makeSorted());

        r.setRectXY(oooRects[i], 1, 2);
        REPORTER_ASSERT(reporter, !r.isEmpty());
        REPORTER_ASSERT(reporter, r.rect() == oooRects[i].makeSorted());

        r.setNinePatch(oooRects[i], 0, 1, 2, 3);
        REPORTER_ASSERT(reporter, !r.isEmpty());
        REPORTER_ASSERT(reporter, r.rect() == oooRects[i].makeSorted());

        r.setRectRadii(oooRects[i], radii);
        REPORTER_ASSERT(reporter, !r.isEmpty());
        REPORTER_ASSERT(reporter, r.rect() == oooRects[i].makeSorted());
    }

    for (size_t i = 0; i < SK_ARRAY_COUNT(emptyRects); ++i) {
        r.setRect(emptyRects[i]);
        REPORTER_ASSERT(reporter, r.isEmpty());
        REPORTER_ASSERT(reporter, r.rect() == emptyRects[i]);

        r.setOval(emptyRects[i]);
        REPORTER_ASSERT(reporter, r.isEmpty());
        REPORTER_ASSERT(reporter, r.rect() == emptyRects[i]);

        r.setRectXY(emptyRects[i], 1, 2);
        REPORTER_ASSERT(reporter, r.isEmpty());
        REPORTER_ASSERT(reporter, r.rect() == emptyRects[i]);

        r.setNinePatch(emptyRects[i], 0, 1, 2, 3);
        REPORTER_ASSERT(reporter, r.isEmpty());
        REPORTER_ASSERT(reporter, r.rect() == emptyRects[i]);

        r.setRectRadii(emptyRects[i], radii);
        REPORTER_ASSERT(reporter, r.isEmpty());
        REPORTER_ASSERT(reporter, r.rect() == emptyRects[i]);
    }

    r.setRect({SK_ScalarNaN, 10, 10, 20});
    REPORTER_ASSERT(reporter, r == SkRRect::MakeEmpty());
    r.setRect({0, 10, 10, SK_ScalarInfinity});
    REPORTER_ASSERT(reporter, r == SkRRect::MakeEmpty());
}

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


static void test_9patch_rrect(skiatest::Reporter* reporter,
                              const SkRect& rect,
                              SkScalar l, SkScalar t, SkScalar r, SkScalar b,
                              bool checkRadii) {
    SkRRect rr;
    rr.setNinePatch(rect, l, t, r, b);

    REPORTER_ASSERT(reporter, SkRRect::kNinePatch_Type == rr.type());
    REPORTER_ASSERT(reporter, rr.rect() == rect);

    if (checkRadii) {
        // This test doesn't hold if the radii will be rescaled by SkRRect
        SkRect ninePatchRadii = { l, t, r, b };
        SkPoint rquad[4];
        ninePatchRadii.toQuad(rquad);
        for (int i = 0; i < 4; ++i) {
            REPORTER_ASSERT(reporter, rquad[i] == rr.radii((SkRRect::Corner) i));
        }
    }
    SkRRect rr2; // construct the same RR using the most general set function
    SkVector radii[4] = { { l, t }, { r, t }, { r, b }, { l, b } };
    rr2.setRectRadii(rect, radii);
    REPORTER_ASSERT(reporter, rr2 == rr && rr2.getType() == rr.getType());
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
    SkRRect rr1_2; // construct the same RR using the most general set function
    SkVector rr1_2_radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };
    rr1_2.setRectRadii(rect, rr1_2_radii);
    REPORTER_ASSERT(reporter, rr1_2 == rr1 && rr1_2.getType() == rr1.getType());
    SkRRect rr1_3;  // construct the same RR using the nine patch set function
    rr1_3.setNinePatch(rect, 0, 0, 0, 0);
    REPORTER_ASSERT(reporter, rr1_3 == rr1 && rr1_3.getType() == rr1.getType());

    //----
    SkPoint halfPoint = { SkScalarHalf(kWidth), SkScalarHalf(kHeight) };
    SkRRect rr2;
    rr2.setOval(rect);

    REPORTER_ASSERT(reporter, SkRRect::kOval_Type == rr2.type());
    REPORTER_ASSERT(reporter, rr2.rect() == rect);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter,
                        SkPointPriv::EqualsWithinTolerance(rr2.radii((SkRRect::Corner) i),
                        halfPoint));
    }
    SkRRect rr2_2;  // construct the same RR using the most general set function
    SkVector rr2_2_radii[4] = { { halfPoint.fX, halfPoint.fY }, { halfPoint.fX, halfPoint.fY },
                                { halfPoint.fX, halfPoint.fY }, { halfPoint.fX, halfPoint.fY } };
    rr2_2.setRectRadii(rect, rr2_2_radii);
    REPORTER_ASSERT(reporter, rr2_2 == rr2 && rr2_2.getType() == rr2.getType());
    SkRRect rr2_3;  // construct the same RR using the nine patch set function
    rr2_3.setNinePatch(rect, halfPoint.fX, halfPoint.fY, halfPoint.fX, halfPoint.fY);
    REPORTER_ASSERT(reporter, rr2_3 == rr2 && rr2_3.getType() == rr2.getType());

    //----
    SkPoint p = { 5, 5 };
    SkRRect rr3;
    rr3.setRectXY(rect, p.fX, p.fY);

    REPORTER_ASSERT(reporter, SkRRect::kSimple_Type == rr3.type());
    REPORTER_ASSERT(reporter, rr3.rect() == rect);

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, p == rr3.radii((SkRRect::Corner) i));
    }
    SkRRect rr3_2; // construct the same RR using the most general set function
    SkVector rr3_2_radii[4] = { { 5, 5 }, { 5, 5 }, { 5, 5 }, { 5, 5 } };
    rr3_2.setRectRadii(rect, rr3_2_radii);
    REPORTER_ASSERT(reporter, rr3_2 == rr3 && rr3_2.getType() == rr3.getType());
    SkRRect rr3_3;  // construct the same RR using the nine patch set function
    rr3_3.setNinePatch(rect, 5, 5, 5, 5);
    REPORTER_ASSERT(reporter, rr3_3 == rr3 && rr3_3.getType() == rr3.getType());

    //----
    test_9patch_rrect(reporter, rect, 10, 9, 8, 7, true);

    {
        // Test out the rrect from skia:3466
        SkRect rect2 = SkRect::MakeLTRB(0.358211994f, 0.755430222f, 0.872866154f, 0.806214333f);

        test_9patch_rrect(reporter,
                          rect2,
                          0.926942348f, 0.642850280f, 0.529063463f, 0.587844372f,
                          false);
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
    REPORTER_ASSERT(reporter, rr3 != rr5);
}

// Test out the cases when the RR degenerates to a rect
static void test_round_rect_rects(skiatest::Reporter* reporter) {
    SkRect r;

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

    //----
    SkPoint radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };

    SkRRect rr2;
    rr2.setRectRadii(rect, radii);

    REPORTER_ASSERT(reporter, SkRRect::kRect_Type == rr2.type());
    r = rr2.rect();
    REPORTER_ASSERT(reporter, rect == r);

    //----
    SkPoint radii2[4] = { { 0, 0 }, { 20, 20 }, { 50, 50 }, { 20, 50 } };

    SkRRect rr3;
    rr3.setRectRadii(rect, radii2);
    REPORTER_ASSERT(reporter, SkRRect::kComplex_Type == rr3.type());
}

// Test out the cases when the RR degenerates to an oval
static void test_round_rect_ovals(skiatest::Reporter* reporter) {
    //----
    SkRect oval;
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
    SkRRect rr1;
    rr1.setRectXY(rect, SkScalarHalf(kWidth), SkScalarHalf(kHeight));

    REPORTER_ASSERT(reporter, SkRRect::kOval_Type == rr1.type());
    oval = rr1.rect();
    REPORTER_ASSERT(reporter, oval == rect);
}

// Test out the non-degenerate RR cases
static void test_round_rect_general(skiatest::Reporter* reporter) {
    //----
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
    SkRRect rr1;
    rr1.setRectXY(rect, 20, 20);

    REPORTER_ASSERT(reporter, SkRRect::kSimple_Type == rr1.type());

    //----
    SkPoint radii[4] = { { 0, 0 }, { 20, 20 }, { 50, 50 }, { 20, 50 } };

    SkRRect rr2;
    rr2.setRectRadii(rect, radii);

    REPORTER_ASSERT(reporter, SkRRect::kComplex_Type == rr2.type());
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

// Called for a matrix that should cause SkRRect::transform to fail.
static void assert_transform_failure(skiatest::Reporter* reporter, const SkRRect& orig,
                                     const SkMatrix& matrix) {
    // The test depends on the fact that the original is not empty.
    SkASSERT(!orig.isEmpty());
    SkRRect dst;
    dst.setEmpty();

    const SkRRect copyOfDst = dst;
    const SkRRect copyOfOrig = orig;
    bool success = orig.transform(matrix, &dst);
    // This transform should fail.
    REPORTER_ASSERT(reporter, !success);
    // Since the transform failed, dst should be unchanged.
    REPORTER_ASSERT(reporter, copyOfDst == dst);
    // original should not be modified.
    REPORTER_ASSERT(reporter, copyOfOrig == orig);
    REPORTER_ASSERT(reporter, orig != dst);
}

#define GET_RADII                                                       \
    const SkVector& origUL = orig.radii(SkRRect::kUpperLeft_Corner);    \
    const SkVector& origUR = orig.radii(SkRRect::kUpperRight_Corner);   \
    const SkVector& origLR = orig.radii(SkRRect::kLowerRight_Corner);   \
    const SkVector& origLL = orig.radii(SkRRect::kLowerLeft_Corner);    \
    const SkVector& dstUL = dst.radii(SkRRect::kUpperLeft_Corner);      \
    const SkVector& dstUR = dst.radii(SkRRect::kUpperRight_Corner);     \
    const SkVector& dstLR = dst.radii(SkRRect::kLowerRight_Corner);     \
    const SkVector& dstLL = dst.radii(SkRRect::kLowerLeft_Corner)

// Called to test various transforms on a single SkRRect.
static void test_transform_helper(skiatest::Reporter* reporter, const SkRRect& orig) {
    SkRRect dst;
    dst.setEmpty();

    // The identity matrix will duplicate the rrect.
    bool success = orig.transform(SkMatrix::I(), &dst);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, orig == dst);

    // Skew and Perspective make transform fail.
    SkMatrix matrix;
    matrix.reset();
    matrix.setSkewX(SkIntToScalar(2));
    assert_transform_failure(reporter, orig, matrix);

    matrix.reset();
    matrix.setSkewY(SkIntToScalar(3));
    assert_transform_failure(reporter, orig, matrix);

    matrix.reset();
    matrix.setPerspX(4);
    assert_transform_failure(reporter, orig, matrix);

    matrix.reset();
    matrix.setPerspY(5);
    assert_transform_failure(reporter, orig, matrix);

    // Rotation fails.
    matrix.reset();
    matrix.setRotate(SkIntToScalar(37));
    assert_transform_failure(reporter, orig, matrix);

    // Translate will keep the rect moved, but otherwise the same.
    matrix.reset();
    SkScalar translateX = SkIntToScalar(32);
    SkScalar translateY = SkIntToScalar(15);
    matrix.setTranslateX(translateX);
    matrix.setTranslateY(translateY);
    dst.setEmpty();
    success = orig.transform(matrix, &dst);
    REPORTER_ASSERT(reporter, success);
    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter,
                orig.radii((SkRRect::Corner) i) == dst.radii((SkRRect::Corner) i));
    }
    REPORTER_ASSERT(reporter, orig.rect().width() == dst.rect().width());
    REPORTER_ASSERT(reporter, orig.rect().height() == dst.rect().height());
    REPORTER_ASSERT(reporter, dst.rect().left() == orig.rect().left() + translateX);
    REPORTER_ASSERT(reporter, dst.rect().top() == orig.rect().top() + translateY);

    // Keeping the translation, but adding skew will make transform fail.
    matrix.setSkewY(SkIntToScalar(7));
    assert_transform_failure(reporter, orig, matrix);

    // Scaling in -x will flip the round rect horizontally.
    matrix.reset();
    matrix.setScaleX(SkIntToScalar(-1));
    dst.setEmpty();
    success = orig.transform(matrix, &dst);
    REPORTER_ASSERT(reporter, success);
    {
        GET_RADII;
        // Radii have swapped in x.
        REPORTER_ASSERT(reporter, origUL == dstUR);
        REPORTER_ASSERT(reporter, origUR == dstUL);
        REPORTER_ASSERT(reporter, origLR == dstLL);
        REPORTER_ASSERT(reporter, origLL == dstLR);
    }
    // Width and height remain the same.
    REPORTER_ASSERT(reporter, orig.rect().width() == dst.rect().width());
    REPORTER_ASSERT(reporter, orig.rect().height() == dst.rect().height());
    // Right and left have swapped (sort of)
    REPORTER_ASSERT(reporter, orig.rect().right() == -dst.rect().left());
    // Top has stayed the same.
    REPORTER_ASSERT(reporter, orig.rect().top() == dst.rect().top());

    // Keeping the scale, but adding a persp will make transform fail.
    matrix.setPerspX(7);
    assert_transform_failure(reporter, orig, matrix);

    // Scaling in -y will flip the round rect vertically.
    matrix.reset();
    matrix.setScaleY(SkIntToScalar(-1));
    dst.setEmpty();
    success = orig.transform(matrix, &dst);
    REPORTER_ASSERT(reporter, success);
    {
        GET_RADII;
        // Radii have swapped in y.
        REPORTER_ASSERT(reporter, origUL == dstLL);
        REPORTER_ASSERT(reporter, origUR == dstLR);
        REPORTER_ASSERT(reporter, origLR == dstUR);
        REPORTER_ASSERT(reporter, origLL == dstUL);
    }
    // Width and height remain the same.
    REPORTER_ASSERT(reporter, orig.rect().width() == dst.rect().width());
    REPORTER_ASSERT(reporter, orig.rect().height() == dst.rect().height());
    // Top and bottom have swapped (sort of)
    REPORTER_ASSERT(reporter, orig.rect().top() == -dst.rect().bottom());
    // Left has stayed the same.
    REPORTER_ASSERT(reporter, orig.rect().left() == dst.rect().left());

    // Scaling in -x and -y will swap in both directions.
    matrix.reset();
    matrix.setScaleY(SkIntToScalar(-1));
    matrix.setScaleX(SkIntToScalar(-1));
    dst.setEmpty();
    success = orig.transform(matrix, &dst);
    REPORTER_ASSERT(reporter, success);
    {
        GET_RADII;
        REPORTER_ASSERT(reporter, origUL == dstLR);
        REPORTER_ASSERT(reporter, origUR == dstLL);
        REPORTER_ASSERT(reporter, origLR == dstUL);
        REPORTER_ASSERT(reporter, origLL == dstUR);
    }
    // Width and height remain the same.
    REPORTER_ASSERT(reporter, orig.rect().width() == dst.rect().width());
    REPORTER_ASSERT(reporter, orig.rect().height() == dst.rect().height());
    REPORTER_ASSERT(reporter, orig.rect().top() == -dst.rect().bottom());
    REPORTER_ASSERT(reporter, orig.rect().right() == -dst.rect().left());

    // Scale in both directions.
    SkScalar xScale = SkIntToScalar(3);
    SkScalar yScale = 3.2f;
    matrix.reset();
    matrix.setScaleX(xScale);
    matrix.setScaleY(yScale);
    dst.setEmpty();
    success = orig.transform(matrix, &dst);
    REPORTER_ASSERT(reporter, success);
    // Radii are scaled.
    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.radii((SkRRect::Corner) i).fX,
                                    orig.radii((SkRRect::Corner) i).fX * xScale));
        REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.radii((SkRRect::Corner) i).fY,
                                    orig.radii((SkRRect::Corner) i).fY * yScale));
    }
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.rect().width(),
                                                  orig.rect().width() * xScale));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.rect().height(),
                                                  orig.rect().height() * yScale));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.rect().left(),
                                                  orig.rect().left() * xScale));
    REPORTER_ASSERT(reporter, SkScalarNearlyEqual(dst.rect().top(),
                                                  orig.rect().top() * yScale));


    //  a-----b            d-----a
    //  |     |     ->     |     |
    //  |     |  Rotate 90 |     |
    //  d-----c            c-----b
    matrix.reset();
    matrix.setRotate(SkIntToScalar(90));
    dst.setEmpty();
    success = orig.transform(matrix, &dst);
    REPORTER_ASSERT(reporter, success);
    {
        GET_RADII;
        // Radii have cycled clockwise and swapped their x and y axis.
        REPORTER_ASSERT(reporter, dstUL.x() == origLL.y());
        REPORTER_ASSERT(reporter, dstUL.y() == origLL.x());
        REPORTER_ASSERT(reporter, dstUR.x() == origUL.y());
        REPORTER_ASSERT(reporter, dstUR.y() == origUL.x());
        REPORTER_ASSERT(reporter, dstLR.x() == origUR.y());
        REPORTER_ASSERT(reporter, dstLR.y() == origUR.x());
        REPORTER_ASSERT(reporter, dstLL.x() == origLR.y());
        REPORTER_ASSERT(reporter, dstLL.y() == origLR.x());
    }
    // Width and height would get swapped.
    REPORTER_ASSERT(reporter, orig.rect().width() == dst.rect().height());
    REPORTER_ASSERT(reporter, orig.rect().height() == dst.rect().width());

    //  a-----b        b-----a           c-----b
    //  |     |   ->   |     |    ->     |     |
    //  |     | Flip X |     | Rotate 90 |     |
    //  d-----c        c-----d           d-----a
    matrix.reset();
    matrix.setRotate(SkIntToScalar(90));
    matrix.postScale(SkIntToScalar(-1), SkIntToScalar(1));
    dst.setEmpty();
    success = orig.transform(matrix, &dst);
    REPORTER_ASSERT(reporter, success);
    {
        GET_RADII;
        REPORTER_ASSERT(reporter, dstUL.x() == origLR.y());
        REPORTER_ASSERT(reporter, dstUL.y() == origLR.x());
        REPORTER_ASSERT(reporter, dstUR.x() == origUR.y());
        REPORTER_ASSERT(reporter, dstUR.y() == origUR.x());
        REPORTER_ASSERT(reporter, dstLR.x() == origUL.y());
        REPORTER_ASSERT(reporter, dstLR.y() == origUL.x());
        REPORTER_ASSERT(reporter, dstLL.x() == origLL.y());
        REPORTER_ASSERT(reporter, dstLL.y() == origLL.x());
    }
    // Width and height would get swapped.
    REPORTER_ASSERT(reporter, orig.rect().width() == dst.rect().height());
    REPORTER_ASSERT(reporter, orig.rect().height() == dst.rect().width());

    //  a-----b           d-----a        c-----b
    //  |     |    ->     |     |   ->   |     |
    //  |     | Rotate 90 |     | Flip Y |     |
    //  d-----c           c-----b        d-----a
    //
    // This is the same as Flip X and Rotate 90.
    matrix.reset();
    matrix.setScale(SkIntToScalar(1), SkIntToScalar(-1));
    matrix.postRotate(SkIntToScalar(90));
    SkRRect dst2;
    dst2.setEmpty();
    success = orig.transform(matrix, &dst2);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, dst == dst2);

    //  a-----b            b-----c        c-----b
    //  |     |     ->     |     |   ->   |     |
    //  |     | Rotate 270 |     | Flip X |     |
    //  d-----c            a-----d        d-----a
    matrix.reset();
    matrix.setScale(SkIntToScalar(-1), SkIntToScalar(1));
    matrix.postRotate(SkIntToScalar(270));
    dst2.setEmpty();
    success = orig.transform(matrix, &dst2);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, dst == dst2);

    //  a-----b        d-----c            c-----b
    //  |     |   ->   |     |     ->     |     |
    //  |     | Flip Y |     | Rotate 270 |     |
    //  d-----c        a-----b            d-----a
    matrix.reset();
    matrix.setRotate(SkIntToScalar(270));
    matrix.postScale(SkIntToScalar(1), SkIntToScalar(-1));
    dst2.setEmpty();
    success = orig.transform(matrix, &dst2);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, dst == dst2);

    //  a-----b           d-----a        a-----d
    //  |     |    ->     |     |   ->   |     |
    //  |     | Rotate 90 |     | Flip X |     |
    //  d-----c           c-----b        b-----c
    matrix.reset();
    matrix.setScale(SkIntToScalar(-1), SkIntToScalar(1));
    matrix.postRotate(SkIntToScalar(90));
    dst.setEmpty();
    success = orig.transform(matrix, &dst);
    REPORTER_ASSERT(reporter, success);
    {
        GET_RADII;
        REPORTER_ASSERT(reporter, dstUL.x() == origUL.y());
        REPORTER_ASSERT(reporter, dstUL.y() == origUL.x());
        REPORTER_ASSERT(reporter, dstUR.x() == origLL.y());
        REPORTER_ASSERT(reporter, dstUR.y() == origLL.x());
        REPORTER_ASSERT(reporter, dstLR.x() == origLR.y());
        REPORTER_ASSERT(reporter, dstLR.y() == origLR.x());
        REPORTER_ASSERT(reporter, dstLL.x() == origUR.y());
        REPORTER_ASSERT(reporter, dstLL.y() == origUR.x());
    }
    // Width and height would get swapped.
    REPORTER_ASSERT(reporter, orig.rect().width() == dst.rect().height());
    REPORTER_ASSERT(reporter, orig.rect().height() == dst.rect().width());

    //  a-----b        d-----c           a-----d
    //  |     |   ->   |     |    ->     |     |
    //  |     | Flip Y |     | Rotate 90 |     |
    //  d-----c        a-----b           b-----c
    // This is the same as rotate 90 and flip x.
    matrix.reset();
    matrix.setRotate(SkIntToScalar(90));
    matrix.postScale(SkIntToScalar(1), SkIntToScalar(-1));
    dst2.setEmpty();
    success = orig.transform(matrix, &dst2);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, dst == dst2);

    //  a-----b        b-----a            a-----d
    //  |     |   ->   |     |     ->     |     |
    //  |     | Flip X |     | Rotate 270 |     |
    //  d-----c        c-----d            b-----c
    matrix.reset();
    matrix.setRotate(SkIntToScalar(270));
    matrix.postScale(SkIntToScalar(-1), SkIntToScalar(1));
    dst2.setEmpty();
    success = orig.transform(matrix, &dst2);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, dst == dst2);

    //  a-----b            b-----c        a-----d
    //  |     |     ->     |     |   ->   |     |
    //  |     | Rotate 270 |     | Flip Y |     |
    //  d-----c            a-----d        b-----c
    matrix.reset();
    matrix.setScale(SkIntToScalar(1), SkIntToScalar(-1));
    matrix.postRotate(SkIntToScalar(270));
    dst2.setEmpty();
    success = orig.transform(matrix, &dst2);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, dst == dst2);


    //  a-----b        b-----a        c-----d            b-----c
    //  |     |   ->   |     |   ->   |     |    ->      |     |
    //  |     | Flip X |     | Flip Y |     | Rotate 90  |     |
    //  d-----c        c-----d        b-----a            a-----d
    //
    // This is the same as rotation by 270.
    matrix.reset();
    matrix.setRotate(SkIntToScalar(90));
    matrix.postScale(SkIntToScalar(-1), SkIntToScalar(-1));
    dst.setEmpty();
    success = orig.transform(matrix, &dst);
    REPORTER_ASSERT(reporter, success);
    {
        GET_RADII;
        // Radii have cycled clockwise and swapped their x and y axis.
        REPORTER_ASSERT(reporter, dstUL.x() == origUR.y());
        REPORTER_ASSERT(reporter, dstUL.y() == origUR.x());
        REPORTER_ASSERT(reporter, dstUR.x() == origLR.y());
        REPORTER_ASSERT(reporter, dstUR.y() == origLR.x());
        REPORTER_ASSERT(reporter, dstLR.x() == origLL.y());
        REPORTER_ASSERT(reporter, dstLR.y() == origLL.x());
        REPORTER_ASSERT(reporter, dstLL.x() == origUL.y());
        REPORTER_ASSERT(reporter, dstLL.y() == origUL.x());
    }
    // Width and height would get swapped.
    REPORTER_ASSERT(reporter, orig.rect().width() == dst.rect().height());
    REPORTER_ASSERT(reporter, orig.rect().height() == dst.rect().width());

    //  a-----b             b-----c
    //  |     |     ->      |     |
    //  |     | Rotate 270  |     |
    //  d-----c             a-----d
    //
    dst2.setEmpty();
    matrix.reset();
    matrix.setRotate(SkIntToScalar(270));
    success = orig.transform(matrix, &dst2);
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter, dst == dst2);

    //  a-----b        b-----a        c-----d             d-----a
    //  |     |   ->   |     |   ->   |     |     ->      |     |
    //  |     | Flip X |     | Flip Y |     | Rotate 270  |     |
    //  d-----c        c-----d        b-----a             c-----b
    //
    // This is the same as rotation by 90 degrees.
    matrix.reset();
    matrix.setRotate(SkIntToScalar(270));
    matrix.postScale(SkIntToScalar(-1), SkIntToScalar(-1));
    dst.setEmpty();
    success = orig.transform(matrix, &dst);
    REPORTER_ASSERT(reporter, success);

    matrix.reset();
    matrix.setRotate(SkIntToScalar(90));
    dst2.setEmpty();
    success = orig.transform(matrix, &dst2);
    REPORTER_ASSERT(reporter, dst == dst2);

}

static void test_round_rect_transform(skiatest::Reporter* reporter) {
    SkRRect rrect;
    {
        SkRect r = { 0, 0, kWidth, kHeight };
        rrect.setRectXY(r, SkIntToScalar(4), SkIntToScalar(7));
        test_transform_helper(reporter, rrect);
    }
    {
        SkRect r = { SkIntToScalar(5), SkIntToScalar(15),
                     SkIntToScalar(27), SkIntToScalar(34) };
        SkVector radii[4] = { { 0, SkIntToScalar(1) },
                              { SkIntToScalar(2), SkIntToScalar(3) },
                              { SkIntToScalar(4), SkIntToScalar(5) },
                              { SkIntToScalar(6), SkIntToScalar(7) } };
        rrect.setRectRadii(r, radii);
        test_transform_helper(reporter, rrect);
    }
}

// Test out the case where an oval already off in space is translated/scaled
// further off into space - yielding numerical issues when the rect & radii
// are transformed separatly
// BUG=skia:2696
static void test_issue_2696(skiatest::Reporter* reporter) {
    SkRRect rrect;
    SkRect r = { 28443.8594f, 53.1428604f, 28446.7148f, 56.0000038f };
    rrect.setOval(r);

    SkMatrix xform;
    xform.setAll(2.44f,  0.0f, 485411.7f,
                 0.0f,  2.44f,   -438.7f,
                 0.0f,   0.0f,      1.0f);
    SkRRect dst;

    bool success = rrect.transform(xform, &dst);
    REPORTER_ASSERT(reporter, success);

    SkScalar halfWidth = SkScalarHalf(dst.width());
    SkScalar halfHeight = SkScalarHalf(dst.height());

    for (int i = 0; i < 4; ++i) {
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(dst.radii((SkRRect::Corner)i).fX, halfWidth));
        REPORTER_ASSERT(reporter,
                        SkScalarNearlyEqual(dst.radii((SkRRect::Corner)i).fY, halfHeight));
    }
}

void test_read_rrect(skiatest::Reporter* reporter, const SkRRect& rrect, bool shouldEqualSrc) {
    // It would be cleaner to call rrect.writeToMemory into a buffer. However, writeToMemory asserts
    // that the rrect is valid and our caller may have fiddled with the internals of rrect to make
    // it invalid.
    const void* buffer = reinterpret_cast<const void*>(&rrect);
    SkRRect deserialized;
    size_t size = deserialized.readFromMemory(buffer, sizeof(SkRRect));
    REPORTER_ASSERT(reporter, size == SkRRect::kSizeInMemory);
    REPORTER_ASSERT(reporter, deserialized.isValid());
    if (shouldEqualSrc) {
       REPORTER_ASSERT(reporter, rrect == deserialized);
    }
}

static void test_read(skiatest::Reporter* reporter) {
    static const SkRect kRect = {10.f, 10.f, 20.f, 20.f};
    static const SkRect kNaNRect = {10.f, 10.f, 20.f, SK_ScalarNaN};
    static const SkRect kInfRect = {10.f, 10.f, SK_ScalarInfinity, 20.f};
    SkRRect rrect;

    test_read_rrect(reporter, SkRRect::MakeEmpty(), true);
    test_read_rrect(reporter, SkRRect::MakeRect(kRect), true);
    // These get coerced to empty.
    test_read_rrect(reporter, SkRRect::MakeRect(kInfRect), true);
    test_read_rrect(reporter, SkRRect::MakeRect(kNaNRect), true);

    rrect.setRect(kRect);
    SkRect* innerRect = reinterpret_cast<SkRect*>(&rrect);
    SkASSERT(*innerRect == kRect);
    *innerRect = kInfRect;
    test_read_rrect(reporter, rrect, false);
    *innerRect = kNaNRect;
    test_read_rrect(reporter, rrect, false);

    test_read_rrect(reporter, SkRRect::MakeOval(kRect), true);
    test_read_rrect(reporter, SkRRect::MakeOval(kInfRect), true);
    test_read_rrect(reporter, SkRRect::MakeOval(kNaNRect), true);
    rrect.setOval(kRect);
    *innerRect = kInfRect;
    test_read_rrect(reporter, rrect, false);
    *innerRect = kNaNRect;
    test_read_rrect(reporter, rrect, false);

    test_read_rrect(reporter, SkRRect::MakeRectXY(kRect, 5.f, 5.f), true);
    // rrect should scale down the radii to make this legal
    test_read_rrect(reporter, SkRRect::MakeRectXY(kRect, 5.f, 400.f), true);

    static const SkVector kRadii[4] = {{0.5f, 1.f}, {1.5f, 2.f}, {2.5f, 3.f}, {3.5f, 4.f}};
    rrect.setRectRadii(kRect, kRadii);
    test_read_rrect(reporter, rrect, true);
    SkScalar* innerRadius = reinterpret_cast<SkScalar*>(&rrect) + 6;
    SkASSERT(*innerRadius == 1.5f);
    *innerRadius = 400.f;
    test_read_rrect(reporter, rrect, false);
    *innerRadius = SK_ScalarInfinity;
    test_read_rrect(reporter, rrect, false);
    *innerRadius = SK_ScalarNaN;
    test_read_rrect(reporter, rrect, false);
    *innerRadius = -10.f;
    test_read_rrect(reporter, rrect, false);
}

DEF_TEST(RoundRect, reporter) {
    test_round_rect_basic(reporter);
    test_round_rect_rects(reporter);
    test_round_rect_ovals(reporter);
    test_round_rect_general(reporter);
    test_round_rect_iffy_parameters(reporter);
    test_inset(reporter);
    test_round_rect_contains_rect(reporter);
    test_round_rect_transform(reporter);
    test_issue_2696(reporter);
    test_tricky_radii(reporter);
    test_empty_crbug_458524(reporter);
    test_empty(reporter);
    test_read(reporter);
}
