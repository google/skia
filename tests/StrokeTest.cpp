/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkStrokeRec.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkStroke.h"
#include "tests/Test.h"

static bool equal(const SkRect& a, const SkRect& b) {
    return  SkScalarNearlyEqual(a.left(), b.left()) &&
            SkScalarNearlyEqual(a.top(), b.top()) &&
            SkScalarNearlyEqual(a.right(), b.right()) &&
            SkScalarNearlyEqual(a.bottom(), b.bottom());
}

static void test_strokecubic(skiatest::Reporter* reporter) {
    uint32_t hexCubicVals[] = {
        0x424c1086, 0x44bcf0cb,  // fX=51.0161362 fY=1511.52478
        0x424c107c, 0x44bcf0cb,  // fX=51.0160980 fY=1511.52478
        0x424c10c2, 0x44bcf0cb,  // fX=51.0163651 fY=1511.52478
        0x424c1119, 0x44bcf0ca,  // fX=51.0166969 fY=1511.52466
    };
    SkPoint cubicVals[] = {
        {51.0161362f, 1511.52478f },
        {51.0160980f, 1511.52478f },
        {51.0163651f, 1511.52478f },
        {51.0166969f, 1511.52466f },
    };
    SkPaint paint;

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(0.394537568f);
    SkPath path, fillPath;
    path.moveTo(cubicVals[0]);
    path.cubicTo(cubicVals[1], cubicVals[2], cubicVals[3]);
    paint.getFillPath(path, &fillPath);
    path.reset();
    path.moveTo(SkBits2Float(hexCubicVals[0]), SkBits2Float(hexCubicVals[1]));
    path.cubicTo(SkBits2Float(hexCubicVals[2]), SkBits2Float(hexCubicVals[3]),
            SkBits2Float(hexCubicVals[4]), SkBits2Float(hexCubicVals[5]),
            SkBits2Float(hexCubicVals[6]), SkBits2Float(hexCubicVals[7]));
    paint.getFillPath(path, &fillPath);
}

static void test_strokerect(skiatest::Reporter* reporter) {
    const SkScalar width = SkIntToScalar(10);
    SkPaint paint;

    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(width);

    SkRect r = { 0, 0, SkIntToScalar(200), SkIntToScalar(100) };

    SkRect outer(r);
    outer.outset(width/2, width/2);

    static const SkPaint::Join joins[] = {
        SkPaint::kMiter_Join, SkPaint::kRound_Join, SkPaint::kBevel_Join
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(joins); ++i) {
        paint.setStrokeJoin(joins[i]);

        SkPath path, fillPath;
        path.addRect(r);
        paint.getFillPath(path, &fillPath);

        REPORTER_ASSERT(reporter, equal(outer, fillPath.getBounds()));

        bool isMiter = SkPaint::kMiter_Join == joins[i];
        SkRect nested[2];
        REPORTER_ASSERT(reporter, SkPathPriv::IsNestedFillRects(fillPath, nested) == isMiter);
        if (isMiter) {
            SkRect inner(r);
            inner.inset(width/2, width/2);
            REPORTER_ASSERT(reporter, equal(nested[0], outer));
            REPORTER_ASSERT(reporter, equal(nested[1], inner));
        }
    }
}

static void test_strokerec_equality(skiatest::Reporter* reporter) {
    {
        SkStrokeRec s1(SkStrokeRec::kFill_InitStyle);
        SkStrokeRec s2(SkStrokeRec::kFill_InitStyle);
        REPORTER_ASSERT(reporter, s1.hasEqualEffect(s2));

        // Test that style mismatch is detected.
        s2.setHairlineStyle();
        REPORTER_ASSERT(reporter, !s1.hasEqualEffect(s2));

        s1.setHairlineStyle();
        REPORTER_ASSERT(reporter, s1.hasEqualEffect(s2));

        // ResScale is not part of equality.
        s1.setResScale(2.1f);
        s2.setResScale(1.2f);
        REPORTER_ASSERT(reporter, s1.hasEqualEffect(s2));
        s1.setFillStyle();
        s2.setFillStyle();
        REPORTER_ASSERT(reporter, s1.hasEqualEffect(s2));
        s1.setStrokeStyle(1.0f, false);
        s2.setStrokeStyle(1.0f, false);
        s1.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kRound_Join, 2.9f);
        s2.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kRound_Join, 2.9f);
        REPORTER_ASSERT(reporter, s1.hasEqualEffect(s2));
    }

    // Stroke parameters on fill or hairline style are not part of equality.
    {
        SkStrokeRec s1(SkStrokeRec::kFill_InitStyle);
        SkStrokeRec s2(SkStrokeRec::kFill_InitStyle);
        for (int i = 0; i < 2; ++i) {
            s1.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kRound_Join, 2.9f);
            s2.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kRound_Join, 2.1f);
            REPORTER_ASSERT(reporter, s1.hasEqualEffect(s2));
            s2.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kBevel_Join, 2.9f);
            REPORTER_ASSERT(reporter, s1.hasEqualEffect(s2));
            s2.setStrokeParams(SkPaint::kRound_Cap, SkPaint::kRound_Join, 2.9f);
            REPORTER_ASSERT(reporter, s1.hasEqualEffect(s2));
            s1.setHairlineStyle();
            s2.setHairlineStyle();
        }
    }

    // Stroke parameters on stroke style are part of equality.
    {
        SkStrokeRec s1(SkStrokeRec::kFill_InitStyle);
        SkStrokeRec s2(SkStrokeRec::kFill_InitStyle);
        s1.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kRound_Join, 2.9f);
        s2.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kRound_Join, 2.9f);
        s1.setStrokeStyle(1.0f, false);

        s2.setStrokeStyle(1.0f, true);
        REPORTER_ASSERT(reporter, !s1.hasEqualEffect(s2));

        s2.setStrokeStyle(2.1f, false);
        REPORTER_ASSERT(reporter, !s1.hasEqualEffect(s2));

        s2.setStrokeStyle(1.0f, false);
        REPORTER_ASSERT(reporter, s1.hasEqualEffect(s2));

        s2.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kRound_Join, 2.1f);
        REPORTER_ASSERT(reporter, !s1.hasEqualEffect(s2));
        s2.setStrokeParams(SkPaint::kButt_Cap, SkPaint::kBevel_Join, 2.9f);
        REPORTER_ASSERT(reporter, !s1.hasEqualEffect(s2));
        s2.setStrokeParams(SkPaint::kRound_Cap, SkPaint::kRound_Join, 2.9f);
        REPORTER_ASSERT(reporter, !s1.hasEqualEffect(s2));

        // Sets fill.
        s1.setStrokeStyle(0.0f, true);
        s2.setStrokeStyle(0.0f, true);
        REPORTER_ASSERT(reporter, s1.hasEqualEffect(s2));
    }
}

// From skbug.com/6491. The large stroke width can cause numerical instabilities.
static void test_big_stroke(skiatest::Reporter* reporter) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setStrokeWidth(1.49679073e+10f);

    SkPath path;
    path.setFillType(SkPathFillType::kWinding);
    path.moveTo(SkBits2Float(0x46380000), SkBits2Float(0xc6380000));  // 11776, -11776
    path.lineTo(SkBits2Float(0x46a00000), SkBits2Float(0xc6a00000));  // 20480, -20480
    path.lineTo(SkBits2Float(0x468c0000), SkBits2Float(0xc68c0000));  // 17920, -17920
    path.lineTo(SkBits2Float(0x46100000), SkBits2Float(0xc6100000));  // 9216, -9216
    path.lineTo(SkBits2Float(0x46380000), SkBits2Float(0xc6380000));  // 11776, -11776
    path.close();

    SkPath strokeAndFillPath;
    paint.getFillPath(path, &strokeAndFillPath);
}

DEF_TEST(Stroke, reporter) {
    test_strokecubic(reporter);
    test_strokerect(reporter);
    test_strokerec_equality(reporter);
    test_big_stroke(reporter);
}
