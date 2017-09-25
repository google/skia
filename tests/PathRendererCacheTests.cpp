/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#include "SkPath.h"

#if SK_SUPPORT_GPU
#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrResourceCache.h"
#include "effects/GrPorterDuffXferProcessor.h"
#include "ops/GrTessellatingPathRenderer.h"

static SkPath create_concave_path() {
    SkPath path;
    path.moveTo(100, 0);
    path.lineTo(200, 200);
    path.lineTo(100, 150);
    path.lineTo(0, 200);
    path.close();
    return path;
}

static void draw_path(GrContext* ctx,
                      GrRenderTargetContext* renderTargetContext,
                      const SkPath& path,
                      GrPathRenderer* pr,
                      GrAAType aaType = GrAAType::kNone,
                      GrStyle style = GrStyle(SkStrokeRec::kFill_InitStyle)) {
    GrPaint paint;
    paint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));

    GrNoClip noClip;
    SkIRect clipConservativeBounds = SkIRect::MakeWH(renderTargetContext->width(),
                                                     renderTargetContext->height());
    GrShape shape(path, style);
    if (shape.style().applies()) {
        shape = shape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, 1.0f);
    }
    SkMatrix matrix = SkMatrix::I();
    GrPathRenderer::DrawPathArgs args{ctx,
                                      std::move(paint),
                                      &GrUserStencilSettings::kUnused,
                                      renderTargetContext,
                                      &noClip,
                                      &clipConservativeBounds,
                                      &matrix,
                                      &shape,
                                      aaType,
                                      false};
    pr->drawPath(args);
}

static void test_path(skiatest::Reporter* reporter,
                      std::function<SkPath(void)> createPath,
                      GrPathRenderer* pathRenderer,
                      int expectedResources = 1,
                      GrAAType aaType = GrAAType::kNone,
                      GrStyle style = GrStyle(SkStrokeRec::kFill_InitStyle)) {
    sk_sp<GrContext> ctx = GrContext::MakeMock(nullptr);
    ctx->setResourceCacheLimits(100, 10000);
    GrResourceCache* cache = ctx->getResourceCache();

    sk_sp<GrRenderTargetContext> rtc(ctx->makeDeferredRenderTargetContext(
            SkBackingFit::kApprox, 800, 800, kRGBA_8888_GrPixelConfig, nullptr, 0,
            kTopLeft_GrSurfaceOrigin));
    if (!rtc) {
        return;
    }

    SkPath path = createPath();

    // Initially, cache only has the render target context
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());

    // Draw the path, check that new resource count matches expectations
    draw_path(ctx.get(), rtc.get(), path, pathRenderer);
    ctx->flush();
    REPORTER_ASSERT(reporter, (1 + expectedResources) == cache->getResourceCount());

    // Nothing should be purgeable yet
    cache->purgeAsNeeded();
    REPORTER_ASSERT(reporter, (1 + expectedResources) == cache->getResourceCount());

    // Reset the path to change the GenID, which should invalidate any resources in the cache
    path.reset();
    cache->purgeAsNeeded();
    REPORTER_ASSERT(reporter, 1 == cache->getResourceCount());
}

// Test that deleting the original path invalidates the VBs cached by the tessellating path renderer
DEF_GPUTEST(TessellatingPathRendererCacheTest, reporter, factory) {
    GrTessellatingPathRenderer tess;

    // Tessellating path renderer stores vertex buffers in the cache (for non-AA paths)
    test_path(reporter, create_concave_path, &tess);

    // Test with a shape that applies. This needs to attach the invalidation logic to the original
    // path, not the modified path produced by the style.
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    GrStyle style(paint);
    test_path(reporter, create_concave_path, &tess, 1, GrAAType::kNone, style);
}

#endif
