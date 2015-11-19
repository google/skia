/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrix.h"
#include "SkPath.h"
#include "SkPathRef.h"
#include "SkPathOps.h"
#include "SkRRect.h"
#include "Test.h"

static SkRRect path_contains_rrect(skiatest::Reporter* reporter, const SkPath& path) {
    SkRRect out;
    REPORTER_ASSERT(reporter, path.isRRect(&out));
    SkPath path2, xorBoth;
    path2.addRRect(out);
    if (path == path2) {
        return out;
    }
    Op(path, path2, SkPathOp::kXOR_SkPathOp, &xorBoth);
    REPORTER_ASSERT(reporter, xorBoth.isEmpty());
    return out;
}

static SkRRect inner_path_contains_rrect(skiatest::Reporter* reporter, const SkRRect& in) {
    switch (in.getType()) {
        case SkRRect::kEmpty_Type:
        case SkRRect::kRect_Type:
        case SkRRect::kOval_Type:
            return in;
        default:
            break;
    }
    SkPath path;
    path.addRRect(in);
    return path_contains_rrect(reporter, path);
}

static void path_contains_rrect_check(skiatest::Reporter* reporter, const SkRRect& in) {
    SkRRect out = inner_path_contains_rrect(reporter, in);
    if (in != out) {
        SkDebugf("");
    }
    REPORTER_ASSERT(reporter, in == out);
}

static void path_contains_rrect_nocheck(skiatest::Reporter* reporter, const SkRRect& in) {
    SkRRect out = inner_path_contains_rrect(reporter, in);
    if (in == out) {
        SkDebugf("");
    }
}

static void path_contains_rrect_check(skiatest::Reporter* reporter, const SkRect& r,
        SkVector v[4]) {
    SkRRect rrect;
    rrect.setRectRadii(r, v);
    path_contains_rrect_check(reporter, rrect);
}

class ForceIsRRect_Private {
public:
    ForceIsRRect_Private(SkPath* path) {
        path->fPathRef->setIsRRect(true);
    }
};

static void force_path_contains_rrect(skiatest::Reporter* reporter, SkPath& path) {
    ForceIsRRect_Private force_rrect(&path);
    path_contains_rrect(reporter, path);
}

static void test_undetected_paths(skiatest::Reporter* reporter) {
    SkPath path;
    path.moveTo(0, 62.5f);
    path.lineTo(0, 3.5f);
    path.conicTo(0, 0, 3.5f, 0, 0.70710677f);
    path.lineTo(196.5f, 0);
    path.conicTo(200, 0, 200, 3.5f, 0.70710677f);
    path.lineTo(200, 62.5f);
    path.conicTo(200, 66, 196.5f, 66, 0.70710677f);
    path.lineTo(3.5f, 66);
    path.conicTo(0, 66, 0, 62.5, 0.70710677f);
    path.close();
    force_path_contains_rrect(reporter, path);

    path.reset();
    path.moveTo(0, 81.5f);
    path.lineTo(0, 3.5f);
    path.conicTo(0, 0, 3.5f, 0, 0.70710677f);
    path.lineTo(149.5, 0);
    path.conicTo(153, 0, 153, 3.5f, 0.70710677f);
    path.lineTo(153, 81.5f);
    path.conicTo(153, 85, 149.5f, 85, 0.70710677f);
    path.lineTo(3.5f, 85);
    path.conicTo(0, 85, 0, 81.5f, 0.70710677f);
    path.close();
    force_path_contains_rrect(reporter, path);

    path.reset();
    path.moveTo(14, 1189);
    path.lineTo(14, 21);
    path.conicTo(14, 14, 21, 14, 0.70710677f);
    path.lineTo(1363, 14);
    path.conicTo(1370, 14, 1370, 21, 0.70710677f);
    path.lineTo(1370, 1189);
    path.conicTo(1370, 1196, 1363, 1196, 0.70710677f);
    path.lineTo(21, 1196);
    path.conicTo(14, 1196, 14, 1189, 0.70710677f);
    path.close();
    force_path_contains_rrect(reporter, path);

    path.reset();
    path.moveTo(14, 1743);
    path.lineTo(14, 21);
    path.conicTo(14, 14, 21, 14, 0.70710677f);
    path.lineTo(1363, 14);
    path.conicTo(1370, 14, 1370, 21, 0.70710677f);
    path.lineTo(1370, 1743);
    path.conicTo(1370, 1750, 1363, 1750, 0.70710677f);
    path.lineTo(21, 1750);
    path.conicTo(14, 1750, 14, 1743, 0.70710677f);
    path.close();
    force_path_contains_rrect(reporter, path);
}

static const SkScalar kWidth = 100.0f;
static const SkScalar kHeight = 100.0f;

static void test_tricky_radii(skiatest::Reporter* reporter) {
    {
        // crbug.com/458522
        SkRRect rr;
        const SkRect bounds = { 3709, 3709, 3709 + 7402, 3709 + 29825 };
        const SkScalar rad = 12814;
        const SkVector vec[] = { { rad, rad }, { 0, rad }, { rad, rad }, { 0, rad } };
        rr.setRectRadii(bounds, vec);
        path_contains_rrect_check(reporter, rr);
    }

    {
        // crbug.com//463920
        SkRect r = SkRect::MakeLTRB(0, 0, 1009, 33554432.0);
        SkVector radii[4] = {
            { 13.0f, 8.0f }, { 170.0f, 2.0 }, { 256.0f, 33554432.0 }, { 110.0f, 5.0f }
        };
        SkRRect rr;
        rr.setRectRadii(r, radii);
        path_contains_rrect_nocheck(reporter, rr);
    }
}

static void test_empty_crbug_458524(skiatest::Reporter* reporter) {
    SkRRect rr;
    const SkRect bounds = { 3709, 3709, 3709 + 7402, 3709 + 29825 };
    const SkScalar rad = 40;
    rr.setRectXY(bounds, rad, rad);
    path_contains_rrect_check(reporter, rr);

    SkRRect other;
    SkMatrix matrix;
    matrix.setScale(0, 1);
    rr.transform(matrix, &other);
    path_contains_rrect_check(reporter, rr);
}

static void test_inset(skiatest::Reporter* reporter) {
    SkRRect rr, rr2;
    SkRect r = { 0, 0, 100, 100 };

    rr.setRect(r);
    rr.inset(-20, -20, &rr2);
    path_contains_rrect_check(reporter, rr);

    rr.inset(20, 20, &rr2);
    path_contains_rrect_check(reporter, rr);

    rr.inset(r.width()/2, r.height()/2, &rr2);
    path_contains_rrect_check(reporter, rr);

    rr.setRectXY(r, 20, 20);
    rr.inset(19, 19, &rr2);
    path_contains_rrect_check(reporter, rr);
    rr.inset(20, 20, &rr2);
    path_contains_rrect_check(reporter, rr);
}


static void test_9patch_rrect(skiatest::Reporter* reporter,
                              const SkRect& rect,
                              SkScalar l, SkScalar t, SkScalar r, SkScalar b,
                              bool checkRadii) {
    SkRRect rr;
    rr.setNinePatch(rect, l, t, r, b);
    if (checkRadii) {
        path_contains_rrect_check(reporter, rr);
    } else {
        path_contains_rrect_nocheck(reporter, rr);
    }

    SkRRect rr2; // construct the same RR using the most general set function
    SkVector radii[4] = { { l, t }, { r, t }, { r, b }, { l, b } };
    rr2.setRectRadii(rect, radii);
    if (checkRadii) {
        path_contains_rrect_check(reporter, rr);
    } else {
        path_contains_rrect_nocheck(reporter, rr);
    }
}

// Test out the basic API entry points
static void test_round_rect_basic(skiatest::Reporter* reporter) {

    //----
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);

    SkRRect rr1;
    rr1.setRect(rect);
    path_contains_rrect_check(reporter, rr1);

    SkRRect rr1_2; // construct the same RR using the most general set function
    SkVector rr1_2_radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };
    rr1_2.setRectRadii(rect, rr1_2_radii);
    path_contains_rrect_check(reporter, rr1_2);
    SkRRect rr1_3;  // construct the same RR using the nine patch set function
    rr1_3.setNinePatch(rect, 0, 0, 0, 0);
    path_contains_rrect_check(reporter, rr1_2);

    //----
    SkPoint halfPoint = { SkScalarHalf(kWidth), SkScalarHalf(kHeight) };
    SkRRect rr2;
    rr2.setOval(rect);
    path_contains_rrect_check(reporter, rr2);

    SkRRect rr2_2;  // construct the same RR using the most general set function
    SkVector rr2_2_radii[4] = { { halfPoint.fX, halfPoint.fY }, { halfPoint.fX, halfPoint.fY },
                                { halfPoint.fX, halfPoint.fY }, { halfPoint.fX, halfPoint.fY } };
    rr2_2.setRectRadii(rect, rr2_2_radii);
    path_contains_rrect_check(reporter, rr2_2);
    SkRRect rr2_3;  // construct the same RR using the nine patch set function
    rr2_3.setNinePatch(rect, halfPoint.fX, halfPoint.fY, halfPoint.fX, halfPoint.fY);
    path_contains_rrect_check(reporter, rr2_3);

    //----
    SkPoint p = { 5, 5 };
    SkRRect rr3;
    rr3.setRectXY(rect, p.fX, p.fY);
    path_contains_rrect_check(reporter, rr3);

    SkRRect rr3_2; // construct the same RR using the most general set function
    SkVector rr3_2_radii[4] = { { 5, 5 }, { 5, 5 }, { 5, 5 }, { 5, 5 } };
    rr3_2.setRectRadii(rect, rr3_2_radii);
    path_contains_rrect_check(reporter, rr3_2);
    SkRRect rr3_3;  // construct the same RR using the nine patch set function
    rr3_3.setNinePatch(rect, 5, 5, 5, 5);
    path_contains_rrect_check(reporter, rr3_3);

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
    path_contains_rrect_check(reporter, rr5);
}

// Test out the cases when the RR degenerates to a rect
static void test_round_rect_rects(skiatest::Reporter* reporter) {

    //----
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
    SkRRect rr1;
    rr1.setRectXY(rect, 0, 0);

    path_contains_rrect_check(reporter, rr1);

    //----
    SkPoint radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };

    SkRRect rr2;
    rr2.setRectRadii(rect, radii);

    path_contains_rrect_check(reporter, rr2);

    //----
    SkPoint radii2[4] = { { 0, 0 }, { 20, 20 }, { 50, 50 }, { 20, 50 } };

    SkRRect rr3;
    rr3.setRectRadii(rect, radii2);
    path_contains_rrect_check(reporter, rr3);
}

// Test out the cases when the RR degenerates to an oval
static void test_round_rect_ovals(skiatest::Reporter* reporter) {
    //----
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
    SkRRect rr1;
    rr1.setRectXY(rect, SkScalarHalf(kWidth), SkScalarHalf(kHeight));

    path_contains_rrect_check(reporter, rr1);
}

// Test out the non-degenerate RR cases
static void test_round_rect_general(skiatest::Reporter* reporter) {
    //----
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
    SkRRect rr1;
    rr1.setRectXY(rect, 20, 20);

    path_contains_rrect_check(reporter, rr1);

    //----
    SkPoint radii[4] = { { 0, 0 }, { 20, 20 }, { 50, 50 }, { 20, 50 } };

    SkRRect rr2;
    rr2.setRectRadii(rect, radii);

    path_contains_rrect_check(reporter, rr2);
}

static void test_round_rect_iffy_parameters(skiatest::Reporter* reporter) {
    SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
    SkPoint radii[4] = { { 50, 100 }, { 100, 50 }, { 50, 100 }, { 100, 50 } };
    SkRRect rr1;
    rr1.setRectRadii(rect, radii);
    path_contains_rrect_nocheck(reporter, rr1);
}

static void set_radii(SkVector radii[4], int index, float rad) {
    sk_bzero(radii, sizeof(SkVector) * 4);
    radii[index].set(rad, rad);
}

static void test_skbug_3239(skiatest::Reporter* reporter) {
    const float min = SkBits2Float(0xcb7f16c8); /* -16717512.000000 */
    const float max = SkBits2Float(0x4b7f1c1d); /*  16718877.000000 */
    const float big = SkBits2Float(0x4b7f1bd7); /*  16718807.000000 */

    const float rad = 33436320;

    const SkRect rectx = SkRect::MakeLTRB(min, min, max, big);
    const SkRect recty = SkRect::MakeLTRB(min, min, big, max);

    SkVector radii[4];
    for (int i = 0; i < 4; ++i) {
        set_radii(radii, i, rad);
        path_contains_rrect_check(reporter, rectx, radii);
        path_contains_rrect_check(reporter, recty, radii);
    }
}

static void test_mix(skiatest::Reporter* reporter) {
    // Test out mixed degenerate and non-degenerate geometry with Conics
    const SkVector radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 100, 100 } };
    SkRect r = SkRect::MakeWH(100, 100);
    SkRRect rr;
    rr.setRectRadii(r, radii);
    path_contains_rrect_check(reporter, rr);
}

DEF_TEST(RoundRectInPath, reporter) {
    test_tricky_radii(reporter);
    test_empty_crbug_458524(reporter);
    test_inset(reporter);
    test_round_rect_basic(reporter);
    test_round_rect_rects(reporter);
    test_round_rect_ovals(reporter);
    test_round_rect_general(reporter);
    test_undetected_paths(reporter);
    test_round_rect_iffy_parameters(reporter);
    test_skbug_3239(reporter);
    test_mix(reporter);
}
