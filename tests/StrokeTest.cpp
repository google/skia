/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"
#include "SkPath.h"
#include "SkRect.h"
#include "SkStroke.h"
#include "Test.h"

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
        REPORTER_ASSERT(reporter, fillPath.isNestedRects(nested) == isMiter);
        if (isMiter) {
            SkRect inner(r);
            inner.inset(width/2, width/2);
            REPORTER_ASSERT(reporter, equal(nested[0], outer));
            REPORTER_ASSERT(reporter, equal(nested[1], inner));
        }
    }
}

DEF_TEST(Stroke, reporter) {
    test_strokecubic(reporter);
    test_strokerect(reporter);
}
