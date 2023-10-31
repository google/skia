/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "src/core/SkRectPriv.h"
#include "tests/Test.h"

#include <climits>
#include <initializer_list>
#include <string>

static bool has_green_pixels(const SkBitmap& bm) {
    for (int j = 0; j < bm.height(); ++j) {
        for (int i = 0; i < bm.width(); ++i) {
            if (SkColorGetG(bm.getColor(i, j))) {
                return true;
            }
        }
    }

    return false;
}

static void test_stroke_width_clipping(skiatest::Reporter* reporter) {
    SkBitmap bm;
    bm.allocN32Pixels(100, 10);
    bm.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(bm);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(10);
    paint.setColor(0xff00ff00);

    // clip out the left half of our canvas
    canvas.clipRect(SkRect::MakeXYWH(51, 0, 49, 100));

    // no stroke bleed should be visible
    canvas.drawRect(SkRect::MakeWH(44, 100), paint);
    REPORTER_ASSERT(reporter, !has_green_pixels(bm));

    // right stroke edge should bleed into the visible area
    canvas.scale(2, 2);
    canvas.drawRect(SkRect::MakeWH(22, 50), paint);
    REPORTER_ASSERT(reporter, has_green_pixels(bm));
}

static void test_skbug4406(skiatest::Reporter* reporter) {
    SkBitmap bm;
    bm.allocN32Pixels(10, 10);
    bm.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(bm);
    const SkRect r = { 1.5f, 1, 3.5f, 3 };
    // draw filled green rect first
    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(0xff00ff00);
    paint.setStrokeWidth(1);
    paint.setAntiAlias(true);
    canvas.drawRect(r, paint);

    // paint black with stroke rect (that asserts in bug 4406)
    // over the filled rect, it should cover it
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(0xff000000);
    paint.setStrokeWidth(1);
    canvas.drawRect(r, paint);
    REPORTER_ASSERT(reporter, !has_green_pixels(bm));

    // do it again with thinner stroke
    paint.setStyle(SkPaint::kFill_Style);
    paint.setColor(0xff00ff00);
    paint.setStrokeWidth(1);
    paint.setAntiAlias(true);
    canvas.drawRect(r, paint);
    // paint black with stroke rect (that asserts in bug 4406)
    // over the filled rect, it doesnt cover it completelly with thinner stroke
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(0xff000000);
    paint.setStrokeWidth(0.99f);
    canvas.drawRect(r, paint);
    REPORTER_ASSERT(reporter, has_green_pixels(bm));
}

DEF_TEST(Rect, reporter) {
    test_stroke_width_clipping(reporter);
    test_skbug4406(reporter);
}

DEF_TEST(Rect_grow, reporter) {
    test_stroke_width_clipping(reporter);
    test_skbug4406(reporter);
}

DEF_TEST(Rect_path_nan, reporter) {
    SkRect r = { 0, 0, SK_ScalarNaN, 100 };
    SkPath p;
    p.addRect(r);
    // path normally just jams its bounds to be r, but it must notice that r is non-finite
    REPORTER_ASSERT(reporter, !p.isFinite());
}

DEF_TEST(Rect_largest, reporter) {
    REPORTER_ASSERT(reporter, !SkRectPriv::MakeILarge().isEmpty());
    REPORTER_ASSERT(reporter,  SkRectPriv::MakeILargestInverted().isEmpty());

    REPORTER_ASSERT(reporter, !SkRectPriv::MakeLargest().isEmpty());
    REPORTER_ASSERT(reporter, !SkRectPriv::MakeLargeS32().isEmpty());
    REPORTER_ASSERT(reporter,  SkRectPriv::MakeLargestInverted().isEmpty());
}

/*
 *  Test the setBounds always handles non-finite values correctly:
 *  - setBoundsCheck should return false, and set the rect to all zeros
 *  - setBoundsNoCheck should ensure that rect.isFinite() is false (definitely NOT all zeros)
 */
DEF_TEST(Rect_setbounds, reporter) {
    const SkPoint p0[] = { { SK_ScalarInfinity, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 } };
    const SkPoint p1[] = { { 0, SK_ScalarInfinity }, { 1, 1 }, { 2, 2 }, { 3, 3 } };
    const SkPoint p2[] = { { SK_ScalarNaN, 0 }, { 1, 1 }, { 2, 2 }, { 3, 3 } };
    const SkPoint p3[] = { { 0, SK_ScalarNaN }, { 1, 1 }, { 2, 2 }, { 3, 3 } };

    SkRect r;
    const SkRect zeror = { 0, 0, 0, 0 };
    for (const SkPoint* pts : { p0, p1, p2, p3 }) {
        for (int n = 1; n <= 4; ++n) {
            bool isfinite = r.setBoundsCheck(pts, n);
            REPORTER_ASSERT(reporter, !isfinite);
            REPORTER_ASSERT(reporter, r == zeror);

            r.setBoundsNoCheck(pts, n);
            if (r.isFinite())
                r.setBoundsNoCheck(pts, n);
            REPORTER_ASSERT(reporter, !r.isFinite());
        }
    }
}

static float make_big_value(skiatest::Reporter* reporter) {
    // need to make a big value, one that will cause rect.width() to overflow to inf.
    // however, the windows compiler wants about this if it can see the big value inlined.
    // hence, this stupid trick to try to fool their compiler.
    SkASSERT(reporter);
    return reporter ? SK_ScalarMax * 0.75f : 0;
}

DEF_TEST(Rect_whOverflow, reporter) {
    const SkScalar big = make_big_value(reporter);
    const SkRect r = { -big, -big, big, big };

    REPORTER_ASSERT(reporter, r.isFinite());
    REPORTER_ASSERT(reporter, !SkScalarIsFinite(r.width()));
    REPORTER_ASSERT(reporter, !SkScalarIsFinite(r.height()));

    // ensure we can compute center even when the width/height might overflow
    REPORTER_ASSERT(reporter, SkScalarIsFinite(r.centerX()));
    REPORTER_ASSERT(reporter, SkScalarIsFinite(r.centerY()));


    // ensure we can compute halfWidth and halfHeight even when width/height might overflow,
    // i.e. for use computing the radii filling a rectangle.
    REPORTER_ASSERT(reporter, SkScalarIsFinite(SkRectPriv::HalfWidth(r)));
    REPORTER_ASSERT(reporter, SkScalarIsFinite(SkRectPriv::HalfHeight(r)));
}

DEF_TEST(Rect_subtract, reporter) {
    struct Expectation {
        SkIRect fA;
        SkIRect fB;
        SkIRect fExpected;
        bool    fExact;
    };

    SkIRect a = SkIRect::MakeLTRB(2, 3, 12, 15);
    Expectation tests[] = {
        // B contains A == empty rect
        {a, a.makeOutset(2, 2), SkIRect::MakeEmpty(), true},
        // A contains B, producing 4x12 (left), 2x12 (right), 4x10(top), and 5x10(bottom)
        {a, {6, 6, 10, 10}, {2, 10, 12, 15}, false},
        // A is empty, B is not == empty rect
        {SkIRect::MakeEmpty(), a, SkIRect::MakeEmpty(), true},
        // A is not empty, B is empty == a
        {a, SkIRect::MakeEmpty(), a, true},
        // A and B are empty == empty
        {SkIRect::MakeEmpty(), SkIRect::MakeEmpty(), SkIRect::MakeEmpty(), true},
        // A and B do not intersect == a
        {a, {15, 17, 20, 40}, a, true},
        // B cuts off left side of A, producing 6x12 (right)
        {a, {0, 0, 6, 20}, {6, 3, 12, 15}, true},
        // B cuts off right side of A, producing 4x12 (left)
        {a, {6, 0, 20, 20}, {2, 3, 6, 15}, true},
        // B cuts off top side of A, producing 10x9 (bottom)
        {a, {0, 0, 20, 6}, {2, 6, 12, 15}, true},
        // B cuts off bottom side of A, producing 10x7 (top)
        {a, {0, 10, 20, 20}, {2, 3, 12, 10}, true},
        // B splits A horizontally, producing 10x3 (top) or 10x5 (bottom)
        {a, {0, 6, 20, 10}, {2, 10, 12, 15}, false},
        // B splits A vertically, producing 4x12 (left) or 2x12 (right)
        {a, {6, 0, 10, 20}, {2, 3, 6, 15}, false},
        // B cuts top-left of A, producing 8x12 (right) or 10x11 (bottom)
        {a, {0, 0, 4, 4}, {2, 4, 12, 15}, false},
        // B cuts top-right of A, producing 8x12 (left) or 10x8 (bottom)
        {a, {10, 0, 14, 7}, {2, 3, 10, 15}, false},
        // B cuts bottom-left of A, producing 7x12 (right) or 10x9 (top)
        {a, {0, 12, 5, 20}, {2, 3, 12, 12}, false},
        // B cuts bottom-right of A, producing 8x12 (left) or 10x9 (top)
        {a, {10, 12, 20, 20}, {2, 3, 10, 15}, false},
        // B crosses the left of A, producing 4x12 (right) or 10x3 (top) or 10x5 (bottom)
        {a, {0, 6, 8, 10}, {2, 10, 12, 15}, false},
        // B crosses the right side of A, producing 6x12 (left) or 10x3 (top) or 10x5 (bottom)
        {a, {8, 6, 20, 10}, {2, 3, 8, 15}, false},
        // B crosses the top side of A, producing 4x12 (left) or 2x12 (right) or 10x8 (bottom)
        {a, {6, 0, 10, 7}, {2, 7, 12, 15}, false},
        // B crosses the bottom side of A, producing 1x12 (left) or 4x12 (right) or 10x3 (top)
        {a, {4, 6, 8, 20}, {8, 3, 12, 15}, false}
    };

    for (const Expectation& e : tests) {
        SkIRect difference;
        bool exact = SkRectPriv::Subtract(e.fA, e.fB, &difference);
        REPORTER_ASSERT(reporter, exact == e.fExact);
        REPORTER_ASSERT(reporter, difference == e.fExpected);

        // Generate equivalent tests for the SkRect case by moving the input rects by 0.5px
        SkRect af = SkRect::Make(e.fA);
        SkRect bf = SkRect::Make(e.fB);
        SkRect ef = SkRect::Make(e.fExpected);
        af.offset(0.5f, 0.5f);
        bf.offset(0.5f, 0.5f);
        ef.offset(0.5f, 0.5f);

        SkRect df;
        exact = SkRectPriv::Subtract(af, bf, &df);
        REPORTER_ASSERT(reporter, exact == e.fExact);
        REPORTER_ASSERT(reporter, (df.isEmpty() && ef.isEmpty()) || (df == ef));
    }
}

DEF_TEST(Rect_subtract_overflow, reporter) {
    // This rectangle is sorted but whose int32 width overflows and appears negative (so
    // isEmpty() returns true).
    SkIRect reallyBig = SkIRect::MakeLTRB(-INT_MAX + 1000, 0, INT_MAX - 1000, 100);
    // However, because it's sorted, an intersection with a reasonably sized rectangle is still
    // valid so the assumption that SkIRect::Intersects() returns false when either input is
    // empty is invalid, leading to incorrect use of negative width (see crbug.com/1243206)
    SkIRect reasonable = SkIRect::MakeLTRB(-50, -5, 50, 125);

    // Ignoring overflow, "reallyBig - reasonable" should report exact = false and select either the
    // left or right portion of 'reallyBig' that excludes 'reasonable', e.g.
    // {-INT_MAX+1000, 0, -50, 100} or {150, 0, INT_MAX-1000, 100}.
    // This used to assert, but now it should be detected that 'reallyBig' overflows and is
    // technically empty, so the result should be itself and exact.
    SkIRect difference;
    bool exact = SkRectPriv::Subtract(reallyBig, reasonable, &difference);
    REPORTER_ASSERT(reporter, exact);
    REPORTER_ASSERT(reporter, difference == reallyBig);

    // Similarly, if we subtract 'reallyBig', since it's technically empty then we expect the
    // answer to remain 'reasonable'.
    exact = SkRectPriv::Subtract(reasonable, reallyBig, &difference);
    REPORTER_ASSERT(reporter, exact);
    REPORTER_ASSERT(reporter, difference == reasonable);
}

DEF_TEST(Rect_QuadContainsRect, reporter) {
    struct TestCase {
        std::string label;
        bool expect;
        SkMatrix m;
        SkIRect a;
        SkIRect b;
        float tol = 0.f;
    };

    TestCase tests[] = {
        { "Identity matrix contains success", /*expect=*/true,
          /*m=*/SkMatrix::I(), /*a=*/{0,0,15,15}, /*b=*/{2,2,10,10} },

        { "Identity matrix contains failure", /*expect=*/false,
          /*m=*/SkMatrix::I(), /*a=*/{0,0,15,15}, /*b=*/{-2,-2,10,10} },

        { "Identity mapped rect contains itself", /*expect=*/true,
          /*m=*/SkMatrix::I(), /*a=*/{0,0,10,10}, /*b=*/{ 0,0,10,10} },

        { "Scaled rect contains success", /*expect=*/true,
          /*m=*/SkMatrix::Scale(2.f, 3.4f), /*a=*/{0,0,4,4}, /*b=*/{1,1,6,6}},

        { "Scaled rect contains failure", /*expect=*/false,
          /*m=*/SkMatrix::Scale(0.25f, 0.3f), /*a=*/{0,0,8,8}, /*b=*/{0,0,5,5}},

        { "Rotate rect contains success", /*expect=*/true,
          /*m=*/SkMatrix::RotateDeg(45.f, {10.f, 10.f}), /*a=*/{0,0,20,20}, /*b=*/{3,3,17,17}},

        { "Rotate rect contains failure", /*expect=*/false,
          /*m=*/SkMatrix::RotateDeg(45.f, {10.f, 10.f}), /*a=*/{0,0,20,20}, /*b=*/{2,2,18,18}},

        { "Negative scale contains success", /*expect=*/true,
          /*m=*/SkMatrix::Scale(-1.f, 1.f), /*a=*/{0,0,10,10}, /*b=*/{-9,1,-1,9}},

        { "Empty rect contains nothing", /*expect=*/false,
          /*m=*/SkMatrix::RotateDeg(45.f, {0.f, 0.f}), /*a=*/{10,10,10,20}, /*b=*/{10,14,10,16}},

        { "MakeEmpty() contains nothing", /*expect=*/false,
          /*m=*/SkMatrix::RotateDeg(45.f, {0.f, 0.f}), /*a=*/SkIRect::MakeEmpty(), /*b=*/{0,0,1,1}},

        { "Unsorted rect contains nothing", /*expect=*/false,
          /*m=*/SkMatrix::I(), /*a=*/{10,10,0,0}, /*b=*/{2,2,8,8}},

        { "Unsorted rect is contained", /*expect=*/true,
          /*m=*/SkMatrix::I(), /*a=*/{0,0,10,10}, /*b=*/{8,8,2,2}},

        // NOTE: preTranslate(65.f, 0.f) gives enough of a different matrix that the contains()
        // passes even without the epsilon allowance.
        { "Epsilon not contained", /*expect=*/true,
          /*m=*/SkMatrix::MakeAll( 0.984808f, 0.173648f, -98.4808f,
                                  -0.173648f, 0.984808f,  17.3648f,
                                   0.000000f, 0.000000f,   1.0000f)
                         .preTranslate(65.f, 0.f),
          /*a=*/{0, 0, 134, 215}, /*b=*/{0, 0, 100, 200}, /*tol=*/0.001f},
    };

    for (const TestCase& t : tests) {
        skiatest::ReporterContext c{reporter, t.label};
        REPORTER_ASSERT(reporter, SkRectPriv::QuadContainsRect(t.m, t.a, t.b, t.tol) == t.expect);

        // Generate equivalent tests for SkRect and SkM44 by translating a by 1/2px and 'b' by
        // 1/2px in post-transform space
        SkVector bOffset = t.m.mapVector(0.5f, 0.5f);
        SkRect af = SkRect::Make(t.a).makeOffset(0.5f, 0.5f);
        SkRect bf = SkRect::Make(t.b).makeOffset(bOffset.fX, bOffset.fY);
        REPORTER_ASSERT(reporter,
                        SkRectPriv::QuadContainsRect(SkM44(t.m), af, bf, t.tol) == t.expect);

        if (t.tol != 0.f) {
            // Expect the opposite result if we do not provide any tol.
            REPORTER_ASSERT(reporter, SkRectPriv::QuadContainsRect(t.m, t.a, t.b) == !t.expect);

            bOffset = t.m.mapVector(0.5f, 0.5f);
            af = SkRect::Make(t.a).makeOffset(0.5f, 0.5f);
            bf = SkRect::Make(t.b).makeOffset(bOffset.fX, bOffset.fY);
            REPORTER_ASSERT(reporter,
                            SkRectPriv::QuadContainsRect(SkM44(t.m), af, bf) == !t.expect);
        }
    }

    // Test some more complicated scenarios with perspective that don't fit into the TestCase
    // structure as nicely.
    const SkRect a = SkRect::MakeLTRB(1.83f, -0.48f, 15.53f, 30.68f); // arbitrary

    // Perspective matrix where the mapped A has all corners' W > 0
    {
        skiatest::ReporterContext c{reporter, "Perspective, W > 0"};
        SkM44 p = SkM44::Perspective(0.01f, 10.f, SK_ScalarPI / 3.f);
        p.preTranslate(0.f, 5.f, -0.1f);
        p.preConcat(SkM44::Rotate({0.f, 1.f, 0.f}, 0.008f /* radians */));
        REPORTER_ASSERT(reporter, SkRectPriv::QuadContainsRect(p, a, {4.f,10.f,20.f,45.f}));
        REPORTER_ASSERT(reporter, !SkRectPriv::QuadContainsRect(p, a, {2.f,6.f,23.f,50.f}));
    }
    // Perspective matrix where the mapped A has some corners' W < 0
    {
        skiatest::ReporterContext c{reporter, "Perspective, some W > 0"};
        SkM44 p;
        p.setRow(3, {-.2f, -.6f, 0.f, 8.f});
        REPORTER_ASSERT(reporter, SkRectPriv::QuadContainsRect(p, a, {10.f,50.f,20.f,60.f}));
        REPORTER_ASSERT(reporter, !SkRectPriv::QuadContainsRect(p, a, {0.f,1.f,10.f,10.f}));
    }
    // Perspective matrix where the mapped A has all corners' W < 0)
    // For B, we use the previous success contains query above; a rectangle that is inside the
    // convex hull of the mapped corners of A, projecting each corner with its negative W; and a
    // rectangle that contains said convex hull.
    {
        skiatest::ReporterContext c{reporter, "Perspective, no W > 0"};
        SkM44 p;
        p.setRow(3, {-.2f, -.6f, 0.f, 8.f});
        const SkRect na = a.makeOffset(16.f, 31.f);
        REPORTER_ASSERT(reporter, !SkRectPriv::QuadContainsRect(p, na, {10.f,50.f,20.f,60.f}));
        REPORTER_ASSERT(reporter, !SkRectPriv::QuadContainsRect(p, na, {-1.1f,-1.8f,-1.f,-1.79f}));
        REPORTER_ASSERT(reporter, !SkRectPriv::QuadContainsRect(p, na, {-1.9f,-2.3f,-0.4f,-1.6f}));
    }
}

DEF_TEST(Rect_ClosestDisjointEdge, r) {
    struct TestCase {
        std::string label;
        SkIRect dst;
        SkIRect expect;
    };

    // All test cases will use this rect for the src, so dst can be conveniently relative to it.
    static constexpr SkIRect kSrc = {0,0,10,10};
    TestCase tests[] = {
        { "src left edge",                  /*dst=*/{-15, -5, -2, 15}, /*expected=*/{0, 0,  1, 10}},
        { "src left edge clipped to dst",   /*dst=*/{-15,  2, -2,  8}, /*expected=*/{0, 2,  1,  8}},
        { "src top-left corner",            /*dst=*/{-15,-15, -2, -2}, /*expected=*/{0, 0,  1,  1}},
        { "src top edge",                   /*dst=*/{ -5,-10, 15, -2}, /*expected=*/{0, 0, 10,  1}},
        { "src top edge clipped to dst",    /*dst=*/{  2,-10,  8, -2}, /*expected=*/{2, 0,  8,  1}},
        { "src top-right corner",           /*dst=*/{ 15,-15, 20, -2}, /*expected=*/{9, 0, 10,  1}},
        { "src right edge",                 /*dst=*/{ 15, -5, 20, 15}, /*expected=*/{9, 0, 10, 10}},
        { "src right edge clipped to dst",  /*dst=*/{ 15,  2, 20,  8}, /*expected=*/{9, 2, 10,  8}},
        { "src bottom-right corner",        /*dst=*/{ 15, 15, 20, 20}, /*expected=*/{9, 9, 10, 10}},
        { "src bottom edge",                /*dst=*/{ -5, 15, 15, 20}, /*expected=*/{0, 9, 10, 10}},
        { "src bottom edge clipped to dst", /*dst=*/{  2, 15,  8, 20}, /*expected=*/{2, 9,  8, 10}},
        { "src bottom-left corner",         /*dst=*/{-15, 15, -2, 20}, /*expected=*/{0, 9,  1, 10}},
        { "src intersects dst high",        /*dst=*/{  2,  2, 15, 15}, /*expected=*/{2, 2, 10, 10}},
        { "src intersects dst low",         /*dst=*/{ -5, -5,  8,  8}, /*expected=*/{0, 0,  8,  8}},
        { "src contains dst",               /*dst=*/{  2,  2,  8,  8}, /*expected=*/{2, 2,  8,  8}},
        { "src contained in dst",           /*dst=*/{ -5, -5, 15, 15}, /*expected=*/{0, 0, 10, 10}}
    };

    for (const TestCase& t : tests) {
        skiatest::ReporterContext c{r, t.label};
        SkIRect actual = SkRectPriv::ClosestDisjointEdge(kSrc, t.dst);
        REPORTER_ASSERT(r, actual == t.expect);
    }

    // Test emptiness of src and dst
    REPORTER_ASSERT(r, SkRectPriv::ClosestDisjointEdge(SkIRect::MakeEmpty(), {0,0,8,8}).isEmpty());
    REPORTER_ASSERT(r, SkRectPriv::ClosestDisjointEdge({0,0,8,8}, SkIRect::MakeEmpty()).isEmpty());
    REPORTER_ASSERT(r, SkRectPriv::ClosestDisjointEdge({10,10,-1,2}, {15,8,-2,20}).isEmpty());
}

// Before the fix, this sequence would trigger a release_assert in the Tiler
// in SkBitmapDevice.cpp
DEF_TEST(big_tiled_rect_crbug_927075, reporter) {
    // since part of the regression test allocates a huge buffer, don't bother trying on
    // 32-bit devices (e.g. chromecast) so we avoid them failing to allocated.

    if (sizeof(void*) == 8) {
        const int w = 67108863;
        const int h = 1;
        const auto info = SkImageInfo::MakeN32Premul(w, h);

        auto surf = SkSurfaces::Raster(info);
        auto canvas = surf->getCanvas();

        const SkRect r = { 257, 213, 67109120, 214 };
        SkPaint paint;
        paint.setAntiAlias(true);

        canvas->translate(-r.fLeft, -r.fTop);
        canvas->drawRect(r, paint);
    }
}
