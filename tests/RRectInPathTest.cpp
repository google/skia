/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/SkPathRef.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkMalloc.h"
#include "src/base/SkFloatBits.h"
#include "src/core/SkPathPriv.h"
#include "tests/Test.h"

#include <cstdint>
#include <initializer_list>

static SkPathRRectInfo path_contains_rrect(skiatest::Reporter* reporter, const SkPath& path) {
    std::optional<SkPathRRectInfo> info = SkPathPriv::IsRRect(path);
    REPORTER_ASSERT(reporter, info.has_value());
    SkPath recreatedPath = SkPath::RRect(info->fRRect, info->fDirection, info->fStartIndex);
    REPORTER_ASSERT(reporter, path == recreatedPath);
    // Test that rotations/mirrors of the rrect path are still rrect paths and the returned
    // parameters for the transformed paths are correct.
    static const SkMatrix kMatrices[] = {
        SkMatrix::Scale( 1,  1),
        SkMatrix::Scale(-1,  1),
        SkMatrix::Scale( 1, -1),
        SkMatrix::Scale(-1, -1),
    };
    for (auto& m : kMatrices) {
        SkPath xformed = path.makeTransform(m);
        std::optional<SkPathRRectInfo> xinfo = SkPathPriv::IsRRect(xformed);
        REPORTER_ASSERT(reporter, xinfo.has_value());
        recreatedPath = SkPath::RRect(xinfo->fRRect, xinfo->fDirection, xinfo->fStartIndex);
        REPORTER_ASSERT(reporter, recreatedPath == xformed);
    }
    return *info;
}

static SkRRect inner_path_contains_rrect(skiatest::Reporter* reporter, const SkRRect& in,
                                         SkPathDirection dir, unsigned start) {
    switch (in.getType()) {
        case SkRRect::kEmpty_Type:
        case SkRRect::kRect_Type:
        case SkRRect::kOval_Type:
            return in;
        default:
            break;
    }
    SkPath path = SkPath::RRect(in, dir, start);
    SkPathRRectInfo rrect = path_contains_rrect(reporter, path);
    REPORTER_ASSERT(reporter, rrect.fDirection == dir && rrect.fStartIndex == start);
    return rrect.fRRect;
}

static void path_contains_rrect_check(skiatest::Reporter* reporter, const SkRRect& in,
                                      SkPathDirection dir, unsigned start) {
    SkRRect out = inner_path_contains_rrect(reporter, in, dir, start);
    if (in != out) {
        SkDebugf("%s", "");
    }
    REPORTER_ASSERT(reporter, in == out);
}

static void path_contains_rrect_nocheck(skiatest::Reporter* reporter, const SkRRect& in,
                                        SkPathDirection dir, unsigned start) {
    SkRRect out = inner_path_contains_rrect(reporter, in, dir, start);
    if (in == out) {
        SkDebugf("%s", "");
    }
}

static void path_contains_rrect_check(skiatest::Reporter* reporter, const SkRect& r,
        SkVector v[4], SkPathDirection dir, unsigned start) {
    SkRRect rrect;
    rrect.setRectRadii(r, v);
    path_contains_rrect_check(reporter, rrect, dir, start);
}

static const SkScalar kWidth = 100.0f;
static const SkScalar kHeight = 100.0f;

static void test_tricky_radii(skiatest::Reporter* reporter) {
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            {
                // crbug.com/458522
                SkRRect rr;
                const SkRect bounds = { 3709, 3709, 3709 + 7402, 3709 + 29825 };
                const SkScalar rad = 12814;
                const SkVector vec[] = { { rad, rad }, { 0, rad }, { rad, rad }, { 0, rad } };
                rr.setRectRadii(bounds, vec);
                path_contains_rrect_check(reporter, rr, dir, start);
            }

            {
                // crbug.com//463920
                SkRect r = SkRect::MakeLTRB(0, 0, 1009, 33554432.0);
                SkVector radii[4] = {
                    { 13.0f, 8.0f }, { 170.0f, 2.0 }, { 256.0f, 33554432.0 }, { 110.0f, 5.0f }
                };
                SkRRect rr;
                rr.setRectRadii(r, radii);
                path_contains_rrect_nocheck(reporter, rr, dir, start);
            }
        }
    }
}

static void test_empty_crbug_458524(skiatest::Reporter* reporter) {
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            SkRRect rr;
            const SkRect bounds = { 3709, 3709, 3709 + 7402, 3709 + 29825 };
            const SkScalar rad = 40;
            rr.setRectXY(bounds, rad, rad);
            path_contains_rrect_check(reporter, rr, dir, start);

            SkMatrix matrix;
            matrix.setScale(0, 1);
            (void)rr.transform(matrix);
            path_contains_rrect_check(reporter, rr, dir, start);
        }
    }
}

static void test_inset(skiatest::Reporter* reporter) {
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            SkRRect rr, rr2;
            SkRect r = { 0, 0, 100, 100 };

            rr.setRect(r);
            rr.inset(-20, -20, &rr2);
            path_contains_rrect_check(reporter, rr, dir, start);

            rr.inset(20, 20, &rr2);
            path_contains_rrect_check(reporter, rr, dir, start);

            rr.inset(r.width()/2, r.height()/2, &rr2);
            path_contains_rrect_check(reporter, rr, dir, start);

            rr.setRectXY(r, 20, 20);
            rr.inset(19, 19, &rr2);
            path_contains_rrect_check(reporter, rr, dir, start);
            rr.inset(20, 20, &rr2);
            path_contains_rrect_check(reporter, rr, dir, start);
        }
    }
}


static void test_9patch_rrect(skiatest::Reporter* reporter,
                              const SkRect& rect,
                              SkScalar l, SkScalar t, SkScalar r, SkScalar b,
                              bool checkRadii) {
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            SkRRect rr;
            rr.setNinePatch(rect, l, t, r, b);
            if (checkRadii) {
                path_contains_rrect_check(reporter, rr, dir, start);
            } else {
                path_contains_rrect_nocheck(reporter, rr, dir, start);
            }

            SkRRect rr2; // construct the same RR using the most general set function
            SkVector radii[4] = { { l, t }, { r, t }, { r, b }, { l, b } };
            rr2.setRectRadii(rect, radii);
            if (checkRadii) {
                path_contains_rrect_check(reporter, rr, dir, start);
            } else {
                path_contains_rrect_nocheck(reporter, rr, dir, start);
            }
        }
    }
}

// Test out the basic API entry points
static void test_round_rect_basic(skiatest::Reporter* reporter) {
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            //----
            SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);

            SkRRect rr1;
            rr1.setRect(rect);
            path_contains_rrect_check(reporter, rr1, dir, start);

            SkRRect rr1_2; // construct the same RR using the most general set function
            SkVector rr1_2_radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };
            rr1_2.setRectRadii(rect, rr1_2_radii);
            path_contains_rrect_check(reporter, rr1_2, dir, start);
            SkRRect rr1_3;  // construct the same RR using the nine patch set function
            rr1_3.setNinePatch(rect, 0, 0, 0, 0);
            path_contains_rrect_check(reporter, rr1_2, dir, start);

            //----
            SkPoint halfPoint = { SkScalarHalf(kWidth), SkScalarHalf(kHeight) };
            SkRRect rr2;
            rr2.setOval(rect);
            path_contains_rrect_check(reporter, rr2, dir, start);

            SkRRect rr2_2;  // construct the same RR using the most general set function
            SkVector rr2_2_radii[4] = { { halfPoint.fX, halfPoint.fY },
                                        { halfPoint.fX, halfPoint.fY },
                                        { halfPoint.fX, halfPoint.fY },
                                        { halfPoint.fX, halfPoint.fY } };
            rr2_2.setRectRadii(rect, rr2_2_radii);
            path_contains_rrect_check(reporter, rr2_2, dir, start);
            SkRRect rr2_3;  // construct the same RR using the nine patch set function
            rr2_3.setNinePatch(rect, halfPoint.fX, halfPoint.fY, halfPoint.fX, halfPoint.fY);
            path_contains_rrect_check(reporter, rr2_3, dir, start);

            //----
            SkPoint p = { 5, 5 };
            SkRRect rr3;
            rr3.setRectXY(rect, p.fX, p.fY);
            path_contains_rrect_check(reporter, rr3, dir, start);

            SkRRect rr3_2; // construct the same RR using the most general set function
            SkVector rr3_2_radii[4] = { { 5, 5 }, { 5, 5 }, { 5, 5 }, { 5, 5 } };
            rr3_2.setRectRadii(rect, rr3_2_radii);
            path_contains_rrect_check(reporter, rr3_2, dir, start);
            SkRRect rr3_3;  // construct the same RR using the nine patch set function
            rr3_3.setNinePatch(rect, 5, 5, 5, 5);
            path_contains_rrect_check(reporter, rr3_3, dir, start);

            //----
            test_9patch_rrect(reporter, rect, 10, 9, 8, 7, true);

            {
                // Test out the rrect from skbug.com/40034587
                SkRect rect2 = SkRect::MakeLTRB(0.358211994f, 0.755430222f, 0.872866154f,
                                                0.806214333f);

                test_9patch_rrect(reporter,
                                  rect2,
                                  0.926942348f, 0.642850280f, 0.529063463f, 0.587844372f,
                                  false);
            }

            //----
            SkPoint radii2[4] = { { 0, 0 }, { 0, 0 }, { 50, 50 }, { 20, 50 } };

            SkRRect rr5;
            rr5.setRectRadii(rect, radii2);
            path_contains_rrect_check(reporter, rr5, dir, start);
        }
    }
}

// Test out the cases when the RR degenerates to a rect
static void test_round_rect_rects(skiatest::Reporter* reporter) {
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            //----
            SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
            SkRRect rr1;
            rr1.setRectXY(rect, 0, 0);

            path_contains_rrect_check(reporter, rr1, dir, start);

            //----
            SkPoint radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };

            SkRRect rr2;
            rr2.setRectRadii(rect, radii);

            path_contains_rrect_check(reporter, rr2, dir, start);

            //----
            SkPoint radii2[4] = { { 0, 0 }, { 20, 20 }, { 50, 50 }, { 20, 50 } };

            SkRRect rr3;
            rr3.setRectRadii(rect, radii2);
            path_contains_rrect_check(reporter, rr3, dir, start);
        }
    }
}

// Test out the cases when the RR degenerates to an oval
static void test_round_rect_ovals(skiatest::Reporter* reporter) {
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            //----
            SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
            SkRRect rr1;
            rr1.setRectXY(rect, SkScalarHalf(kWidth), SkScalarHalf(kHeight));

            path_contains_rrect_check(reporter, rr1, dir, start);
        }
    }
}

// Test out the non-degenerate RR cases
static void test_round_rect_general(skiatest::Reporter* reporter) {
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            //----
            SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
            SkRRect rr1;
            rr1.setRectXY(rect, 20, 20);

            path_contains_rrect_check(reporter, rr1, dir, start);

            //----
            SkPoint radii[4] = { { 0, 0 }, { 20, 20 }, { 50, 50 }, { 20, 50 } };

            SkRRect rr2;
            rr2.setRectRadii(rect, radii);

            path_contains_rrect_check(reporter, rr2, dir, start);
        }
    }
}

static void test_round_rect_iffy_parameters(skiatest::Reporter* reporter) {
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            SkRect rect = SkRect::MakeLTRB(0, 0, kWidth, kHeight);
            SkPoint radii[4] = { { 50, 100 }, { 100, 50 }, { 50, 100 }, { 100, 50 } };
            SkRRect rr1;
            rr1.setRectRadii(rect, radii);
            path_contains_rrect_nocheck(reporter, rr1, dir, start);
        }
    }
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

    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            SkVector radii[4];
            for (int i = 0; i < 4; ++i) {
                set_radii(radii, i, rad);
                path_contains_rrect_check(reporter, rectx, radii, dir, start);
                path_contains_rrect_check(reporter, recty, radii, dir, start);
            }
        }
    }
}

static void test_mix(skiatest::Reporter* reporter) {
    for (auto dir : {SkPathDirection::kCW, SkPathDirection::kCCW}) {
        for (int start = 0; start < 8; ++start) {
            // Test out mixed degenerate and non-degenerate geometry with Conics
            const SkVector radii[4] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 100, 100 } };
            SkRect r = SkRect::MakeWH(100, 100);
            SkRRect rr;
            rr.setRectRadii(r, radii);
            path_contains_rrect_check(reporter, rr, dir, start);
        }
    }
}

DEF_TEST(RoundRectInPath, reporter) {
    test_tricky_radii(reporter);
    test_empty_crbug_458524(reporter);
    test_inset(reporter);
    test_round_rect_basic(reporter);
    test_round_rect_rects(reporter);
    test_round_rect_ovals(reporter);
    test_round_rect_general(reporter);
    test_round_rect_iffy_parameters(reporter);
    test_skbug_3239(reporter);
    test_mix(reporter);
}

DEF_TEST(RRect_fragile, reporter) {
    SkRect rect = {
        SkBits2Float(0x1f800000),  // 0x003F0000 was the starter value that also fails
        SkBits2Float(0x1400001C),
        SkBits2Float(0x3F000004),
        SkBits2Float(0x3F000004),
    };

    SkPoint radii[] = {
        { SkBits2Float(0x00000001), SkBits2Float(0x00000001) },
        { SkBits2Float(0x00000020), SkBits2Float(0x00000001) },
        { SkBits2Float(0x00000000), SkBits2Float(0x00000000) },
        { SkBits2Float(0x3F000004), SkBits2Float(0x3F000004) },
    };

    SkRRect rr;
    // please don't assert
    if ((false)) {    // disable until we fix this
        SkDebugf("%g 0x%08X\n", rect.fLeft, SkFloat2Bits(rect.fLeft));
        rr.setRectRadii(rect, radii);
    }
}

