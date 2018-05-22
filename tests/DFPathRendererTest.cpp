/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkPath.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "ops/GrSmallPathRenderer.h"

#if 0
// This test case including path coords and matrix taken from crbug.com/627443.
// Because of inaccuracies in large floating point values this causes the
// the path renderer to attempt to add a path DF to its atlas that is larger
// than the plot size which used to crash rather than fail gracefully.
static void test_far_from_origin(GrContext* ctx, GrRenderTargetContext* renderTargetContext,
                                 GrPathRenderer* pr) {
    SkPath path;
    path.lineTo(49.0255089839f, 0.473541f);
    // This extra line wasn't in the original bug but was added to fake out GrShape's special
    // handling of single line segments.
    path.rLineTo(0.015f, 0.015f);
    static constexpr SkScalar mvals[] = {14.0348252854f, 2.13026182736f,
                                         13.6122547187f, 118.309922702f,
                                         1912337682.09f, 2105391889.87f};
    SkMatrix matrix;
    matrix.setAffine(mvals);
    SkMatrix inverse;
    SkAssertResult(matrix.invert(&inverse));
    path.transform(inverse);

    SkStrokeRec rec(SkStrokeRec::kFill_InitStyle);
    rec.setStrokeStyle(1.f);
    rec.setStrokeParams(SkPaint::kRound_Cap, SkPaint::kRound_Join, 1.f);
    GrStyle style(rec, nullptr);

    GrShape shape(path, style);
    shape = shape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, 1.f);

    GrPaint paint;
    paint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));

    GrNoClip noClip;
    GrPathRenderer::DrawPathArgs args{ctx,
                                      std::move(paint),
                                      &GrUserStencilSettings::kUnused,
                                      renderTargetContext,
                                      &noClip,
                                      &matrix,
                                      &shape,
                                      GrAAType::kCoverage,
                                      false};
    pr->drawPath(args);
}

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(SmallPathRenderer, reporter, ctxInfo) {
    GrContext* ctx = ctxInfo.grContext();
    // The DF PR only works with contexts that support derivatives
    if (!ctx->caps()->shaderCaps()->shaderDerivativeSupport()) {
        return;
    }
    sk_sp<GrRenderTargetContext> rtc(ctx->makeRenderTargetContext(SkBackingFit::kApprox,
                                                                  800, 800,
                                                                  kRGBA_8888_GrPixelConfig,
                                                                  nullptr,
                                                                  0,
                                                                  kTopLeft_GrSurfaceOrigin));
    if (!rtc) {
        return;
    }

    GrSmallPathRenderer spr;

    ctx->flush();
    test_far_from_origin(ctx, rtc.get(), &spr);
    ctx->flush();
}
#endif
#endif
