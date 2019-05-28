/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrPath.h"
#include "src/gpu/geometry/GrShape.h"
#include "tests/Test.h"

#include <initializer_list>

static void test_drawPathEmpty(skiatest::Reporter*, SkCanvas* canvas) {
    // Filling an empty path should not crash.
    SkPaint paint;
    SkRect emptyRect = SkRect::MakeEmpty();
    canvas->drawRect(emptyRect, paint);
    canvas->drawPath(SkPath(), paint);
    canvas->drawOval(emptyRect, paint);
    canvas->drawRect(emptyRect, paint);
    canvas->drawRRect(SkRRect::MakeRect(emptyRect), paint);

    // Stroking an empty path should not crash.
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(SK_ColorGRAY);
    paint.setStrokeWidth(SkIntToScalar(20));
    paint.setStrokeJoin(SkPaint::kRound_Join);
    canvas->drawRect(emptyRect, paint);
    canvas->drawPath(SkPath(), paint);
    canvas->drawOval(emptyRect, paint);
    canvas->drawRect(emptyRect, paint);
    canvas->drawRRect(SkRRect::MakeRect(emptyRect), paint);
}

static void fill_and_stroke(SkCanvas* canvas, const SkPath& p1, const SkPath& p2,
                            sk_sp<SkPathEffect> effect) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setPathEffect(effect);

    canvas->drawPath(p1, paint);
    canvas->drawPath(p2, paint);

    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(p1, paint);
    canvas->drawPath(p2, paint);
}

static void test_drawSameRectOvals(skiatest::Reporter*, SkCanvas* canvas) {
    // Drawing ovals with similar bounds but different points order should not crash.

    SkPath oval1, oval2;
    const SkRect rect = SkRect::MakeWH(100, 50);
    oval1.addOval(rect, SkPath::kCW_Direction);
    oval2.addOval(rect, SkPath::kCCW_Direction);

    fill_and_stroke(canvas, oval1, oval2, nullptr);

    const SkScalar intervals[] = { 1, 1 };
    fill_and_stroke(canvas, oval1, oval2, SkDashPathEffect::Make(intervals, 2, 0));
}

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GpuDrawPath, reporter, ctxInfo) {
    for (auto& test_func : { &test_drawPathEmpty, &test_drawSameRectOvals }) {
        for (auto& sampleCount : {1, 4, 16}) {
            SkImageInfo info = SkImageInfo::MakeN32Premul(255, 255);
            auto surface(
                SkSurface::MakeRenderTarget(ctxInfo.grContext(), SkBudgeted::kNo, info,
                                            sampleCount, nullptr));
            if (!surface) {
                continue;
            }
            test_func(reporter, surface->getCanvas());
        }
    }
}

DEF_GPUTEST(GrPathKeys, reporter, /* options */) {
    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setStrokeWidth(10.f);
    GrStyle styles[] = {
        GrStyle::SimpleFill(),
        GrStyle::SimpleHairline(),
        GrStyle(strokePaint)
    };

    for (const GrStyle& style : styles) {
        // Keys should not ignore conic weights.
        SkPath path1, path2;
        SkPoint p0 = SkPoint::Make(100, 0);
        SkPoint p1 = SkPoint::Make(100, 100);

        path1.conicTo(p0, p1, .5f);
        path2.conicTo(p0, p1, .7f);

        GrUniqueKey key1, key2;
        // We expect these small paths to be keyed based on their data.
        bool isVolatile;
        GrPath::ComputeKey(GrShape(path1, GrStyle::SimpleFill()), &key1, &isVolatile);
        REPORTER_ASSERT(reporter, !isVolatile);
        REPORTER_ASSERT(reporter, key1.isValid());
        GrPath::ComputeKey(GrShape(path2, GrStyle::SimpleFill()), &key2, &isVolatile);
        REPORTER_ASSERT(reporter, !isVolatile);
        REPORTER_ASSERT(reporter, key1.isValid());
        REPORTER_ASSERT(reporter, key1 != key2);
        {
            GrUniqueKey tempKey;
            path1.setIsVolatile(true);
            GrPath::ComputeKey(GrShape(path1, style), &key1, &isVolatile);
            REPORTER_ASSERT(reporter, isVolatile);
            REPORTER_ASSERT(reporter, !tempKey.isValid());
        }

        // Ensure that recreating the GrShape doesn't change the key.
        {
            GrUniqueKey tempKey;
            GrPath::ComputeKey(GrShape(path2, GrStyle::SimpleFill()), &tempKey, &isVolatile);
            REPORTER_ASSERT(reporter, key2 == tempKey);
        }

        // Try a large path that is too big to be keyed off its data.
        SkPath path3;
        SkPath path4;
        for (int i = 0; i < 1000; ++i) {
            SkScalar s = SkIntToScalar(i);
            path3.conicTo(s, 3.f * s / 4, s + 1.f, s, 0.5f + s / 2000.f);
            path4.conicTo(s, 3.f * s / 4, s + 1.f, s, 0.3f + s / 2000.f);
        }

        GrUniqueKey key3, key4;
        // These aren't marked volatile and so should have keys
        GrPath::ComputeKey(GrShape(path3, style), &key3, &isVolatile);
        REPORTER_ASSERT(reporter, !isVolatile);
        REPORTER_ASSERT(reporter, key3.isValid());
        GrPath::ComputeKey(GrShape(path4, style), &key4, &isVolatile);
        REPORTER_ASSERT(reporter, !isVolatile);
        REPORTER_ASSERT(reporter, key4.isValid());
        REPORTER_ASSERT(reporter, key3 != key4);

        {
            GrUniqueKey tempKey;
            path3.setIsVolatile(true);
            GrPath::ComputeKey(GrShape(path3, style), &key1, &isVolatile);
            REPORTER_ASSERT(reporter, isVolatile);
            REPORTER_ASSERT(reporter, !tempKey.isValid());
        }
    }
}
