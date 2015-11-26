/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrPath.h"
#include "GrStrokeInfo.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkDashPathEffect.h"
#include "SkRRect.h"
#include "SkRect.h"
#include "SkSurface.h"
#include "Test.h"

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
                            SkPathEffect* effect) {
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
    SkAutoTUnref<SkPathEffect> dashEffect(SkDashPathEffect::Create(intervals, 2, 0));
    fill_and_stroke(canvas, oval1, oval2, dashEffect);
}

DEF_GPUTEST(GpuDrawPath, reporter, factory) {
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContextFactory::GLContextType glType = static_cast<GrContextFactory::GLContextType>(type);

        GrContext* grContext = factory->get(glType);
        if (nullptr == grContext) {
            continue;
        }
        static const int sampleCounts[] = { 0, 4, 16 };

        for (size_t i = 0; i < SK_ARRAY_COUNT(sampleCounts); ++i) {
            SkImageInfo info = SkImageInfo::MakeN32Premul(255, 255);
            
            SkAutoTUnref<SkSurface> surface(
                SkSurface::NewRenderTarget(grContext, SkSurface::kNo_Budgeted, info,
                                           sampleCounts[i], nullptr));
            if (!surface) {
                continue;
            }
            test_drawPathEmpty(reporter, surface->getCanvas());
        }
    }
}

DEF_GPUTEST(GpuDrawPathSameRectOvals, reporter, factory) {
    GrContext* grContext = factory->get(GrContextFactory::kNVPR_GLContextType);
    if (!grContext) {
        return;
    }

    SkAutoTUnref<SkSurface> surface(
        SkSurface::NewRenderTarget(grContext, SkSurface::kNo_Budgeted,
                                   SkImageInfo::MakeN32Premul(255, 255), 4));
    test_drawSameRectOvals(reporter, surface->getCanvas());
}

DEF_TEST(GrPathKeys, reporter) {
    // Keys should not ignore conic weights.
    SkPath path1, path2;
    path1.setIsVolatile(true);
    path2.setIsVolatile(true);
    SkPoint p0 = SkPoint::Make(100, 0);
    SkPoint p1 = SkPoint::Make(100, 100);

    path1.conicTo(p0, p1, .5f);
    path2.conicTo(p0, p1, .7f);

    bool isVolatile;
    GrUniqueKey key1, key2;
    GrStrokeInfo stroke(SkStrokeRec::kFill_InitStyle);
    GrPath::ComputeKey(path1, stroke, &key1, &isVolatile);
    GrPath::ComputeKey(path2, stroke, &key2, &isVolatile);
    REPORTER_ASSERT(reporter, key1 != key2);
}

#endif
