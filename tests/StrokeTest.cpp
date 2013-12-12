/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRect.h"
#include "SkStroke.h"

static bool equal(const SkRect& a, const SkRect& b) {
    return  SkScalarNearlyEqual(a.left(), b.left()) &&
            SkScalarNearlyEqual(a.top(), b.top()) &&
            SkScalarNearlyEqual(a.right(), b.right()) &&
            SkScalarNearlyEqual(a.bottom(), b.bottom());
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
    test_strokerect(reporter);
}
