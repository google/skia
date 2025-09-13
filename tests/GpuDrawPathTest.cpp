/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "tests/Test.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "tests/CtsEnforcement.h"

#include <initializer_list>

struct GrContextOptions;

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

    const SkRect rect = SkRect::MakeWH(100, 50);
    SkPath oval1 = SkPath::Oval(rect, SkPathDirection::kCW),
           oval2 = SkPath::Oval(rect, SkPathDirection::kCCW);

    fill_and_stroke(canvas, oval1, oval2, nullptr);

    const SkScalar intervals[] = { 1, 1 };
    fill_and_stroke(canvas, oval1, oval2, SkDashPathEffect::Make(intervals, 0));
}

DEF_GANESH_TEST_FOR_GL_CONTEXT(GpuDrawPath, reporter, ctxInfo, CtsEnforcement::kNever) {
    for (auto& test_func : { &test_drawPathEmpty, &test_drawSameRectOvals }) {
        for (auto& sampleCount : {1, 4, 16}) {
            SkImageInfo info = SkImageInfo::MakeN32Premul(255, 255);
            auto surface(SkSurfaces::RenderTarget(
                    ctxInfo.directContext(), skgpu::Budgeted::kNo, info, sampleCount, nullptr));
            if (!surface) {
                continue;
            }
            test_func(reporter, surface->getCanvas());
        }
    }
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(GrDrawCollapsedPath,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_T) {
    // From https://bugs.fuchsia.dev/p/fuchsia/issues/detail?id=37330, it's possible for a convex
    // path to be accepted by AAConvexPathRenderer, then be transformed to something without a
    // computable first direction by a perspective matrix.
    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    auto dContext = ctxInfo.directContext();
    auto surface(SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info));

    SkPaint paint;
    paint.setAntiAlias(true);

    const SkPoint pts[] = { {0, 0}, {50, 0}, {0, 50} };
    SkPath path = SkPath::Polygon(pts, true);

    SkMatrix m;
    m.setAll( 0.966006875f   , -0.125156224f  , 72.0899811f,
             -0.00885376986f , -0.112347461f  , 64.7121124f,
             -8.94321693e-06f, -0.00173384184f, 0.998692870f);
    surface->getCanvas()->setMatrix(m);
    surface->getCanvas()->drawPath(path, paint);
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(PathTest_CrBug1232834,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kApiLevel_T) {
    // AAHairlinePathRenderer chops this path to quads that include infinities (and then NaNs).
    // It used to trigger asserts, now the degenerate quad segments should cause it to be rejected.
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto dContext = ctxInfo.directContext();
    auto surface(SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    SkPath path = SkPathBuilder()
                  .moveTo(9.0072E15f, 60)
                  .cubicTo(0, 3.40282e+38f, 0, 3.40282e+38f, 0, 0)
                  .detach();

    surface->getCanvas()->drawPath(path, paint);
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(StrokeCircle_Bug356182429,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(256, 256);
    auto dContext = ctxInfo.directContext();
    auto surface(SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info));

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeWidth(15.4375f);
    paint.setStrokeMiter(2.85207834E9f);

    // This draw ends up in the CircleOp, and asserted because round caps are requested,
    // but the stroke is ultimately excluded (due to the negative inner stroke radius).
    // Along the way, several other bad things happen (but they don't appear relevant to the bug):
    // - MakeArcOp asserts that sweepAngle is non-zero (and it is). After converting degrees to
    //   radians, it flushes to zero, so the sweep angle stored in the ArcParams is zero.
    // - The radius of the "circle" is tiny, but it *also* flushes to zero when we call
    //   `viewMatrix.mapRadius()`, despite the view matrix being identity. This is a result of
    //   the implementation of mapRadius (computing geometric mean of the lengths of two vectors).
    SkRect oval = SkRect::MakeLTRB(0, 0, 1.83670992E-40f, 1.21223864E-38f);
    surface->getCanvas()->drawArc(oval, 8.17909887E-41f, 2.24207754E-44f, false, paint);
    dContext->flushAndSubmit(surface.get(), GrSyncCpu::kNo);
}
