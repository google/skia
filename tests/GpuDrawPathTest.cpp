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
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/geometry/GrStyledShape.h"
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
    oval1.addOval(rect, SkPathDirection::kCW);
    oval2.addOval(rect, SkPathDirection::kCCW);

    fill_and_stroke(canvas, oval1, oval2, nullptr);

    const SkScalar intervals[] = { 1, 1 };
    fill_and_stroke(canvas, oval1, oval2, SkDashPathEffect::Make(intervals, 2, 0));
}

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GpuDrawPath, reporter, ctxInfo) {
    for (auto& test_func : { &test_drawPathEmpty, &test_drawSameRectOvals }) {
        for (auto& sampleCount : {1, 4, 16}) {
            SkImageInfo info = SkImageInfo::MakeN32Premul(255, 255);
            auto surface(
                SkSurface::MakeRenderTarget(ctxInfo.directContext(), SkBudgeted::kNo, info,
                                            sampleCount, nullptr));
            if (!surface) {
                continue;
            }
            test_func(reporter, surface->getCanvas());
        }
    }
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(GrDrawCollapsedPath, reporter, ctxInfo) {
    // From https://bugs.fuchsia.dev/p/fuchsia/issues/detail?id=37330, it's possible for a convex
    // path to be accepted by AAConvexPathRenderer, then be transformed to something without a
    // computable first direction by a perspective matrix.
    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    auto surface(SkSurface::MakeRenderTarget(ctxInfo.directContext(), SkBudgeted::kNo, info));

    SkPaint paint;
    paint.setAntiAlias(true);

    SkPath path;
    path.moveTo(0, 0);
    path.lineTo(50, 0);
    path.lineTo(0, 50);
    path.close();

    SkMatrix m;
    m.setAll( 0.966006875f   , -0.125156224f  , 72.0899811f,
             -0.00885376986f , -0.112347461f  , 64.7121124f,
             -8.94321693e-06f, -0.00173384184f, 0.998692870f);
    surface->getCanvas()->setMatrix(m);
    surface->getCanvas()->drawPath(path, paint);
    surface->flushAndSubmit();
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(PathTest_CrBug1232834, reporter, ctxInfo) {
    // GrAAHairlinePathRenderer chops this path to quads that include infinities (and then NaNs).
    // It used to trigger asserts, now the degenerate quad segments should cause it to be rejected.
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto surface(SkSurface::MakeRenderTarget(ctxInfo.directContext(), SkBudgeted::kNo, info));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    SkPath path;
    path.moveTo(9.0072E15f, 60);
    path.cubicTo(0, 3.40282e+38f, 0, 3.40282e+38f, 0, 0);

    surface->getCanvas()->drawPath(path, paint);
    surface->flushAndSubmit();
}
