/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTest.h"
#include "batches/GrAADistanceFieldPathRenderer.h"
#include "SkPath.h"

// This test case including path coords and matrix taken from crbug.com/627443.
// Because of inaccuracies in large floating point values this causes the
// the path renderer to attempt to add a path DF to its atlas that is larger
// than the plot size which used to crash rather than fail gracefully.
static void test_far_from_origin(GrDrawContext* drawContext, GrPathRenderer* pr,
                                 GrResourceProvider* rp) {
    SkPath path;
    path.lineTo(49.0255089839f, 0.473541f);
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
    paint.setXPFactory(GrPorterDuffXPFactory::Make(SkXfermode::kSrc_Mode));

    GrNoClip noClip;
    GrPathRenderer::DrawPathArgs args;
    args.fPaint = &paint;
    args.fUserStencilSettings = &GrUserStencilSettings::kUnused;
    args.fDrawContext = drawContext;
    args.fClip = &noClip;
    args.fResourceProvider = rp;
    args.fViewMatrix = &matrix;
    args.fShape = &shape;
    args.fAntiAlias = true;
    args.fGammaCorrect = false;
    args.fColor = 0x0;
    pr->drawPath(args);
}

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(AADistanceFieldPathRenderer, reporter, ctxInfo) {
    // The DF PR only works with contexts that support derivatives
    if (!ctxInfo.grContext()->caps()->shaderCaps()->shaderDerivativeSupport()) {
        return;
    }
    sk_sp<GrDrawContext> drawContext(ctxInfo.grContext()->newDrawContext(SkBackingFit::kApprox,
                                                                         800, 800,
                                                                         kSkia8888_GrPixelConfig,
                                                                         0,
                                                                         kTopLeft_GrSurfaceOrigin));
    if (!drawContext) {
        return;
    }

    GrAADistanceFieldPathRenderer dfpr;
    GrTestTarget tt;
    ctxInfo.grContext()->getTestTarget(&tt, drawContext);
    GrResourceProvider* rp = tt.resourceProvider();

    test_far_from_origin(drawContext.get(), &dfpr, rp);
    ctxInfo.grContext()->flush();
}
#endif
