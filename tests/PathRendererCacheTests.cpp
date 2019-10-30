/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkPath.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrSoftwarePathRenderer.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/geometry/GrShape.h"
#include "src/gpu/ops/GrTessellatingPathRenderer.h"

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
                      GrAAType aaType,
                      const GrStyle& style) {
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

static bool cache_non_scratch_resources_equals(GrResourceCache* cache, int expected) {
#if GR_CACHE_STATS
    GrResourceCache::Stats stats;
    cache->getStats(&stats);
    return (stats.fTotal - stats.fScratch) == expected;
#else
    return true;
#endif
}

static void test_path(skiatest::Reporter* reporter,
                      std::function<SkPath(void)> createPath,
                      std::function<GrPathRenderer*(GrContext*)> createPathRenderer,
                      int expected,
                      GrAAType aaType = GrAAType::kNone,
                      GrStyle style = GrStyle(SkStrokeRec::kFill_InitStyle)) {
    sk_sp<GrContext> ctx = GrContext::MakeMock(nullptr);
    // The cache needs to be big enough that nothing gets flushed, or our expectations can be wrong
    ctx->setResourceCacheLimit(8000000);
    GrResourceCache* cache = ctx->priv().getResourceCache();

    auto rtc = ctx->priv().makeDeferredRenderTargetContext(
            SkBackingFit::kApprox, 800, 800, GrColorType::kRGBA_8888, nullptr, 1, GrMipMapped::kNo,
            kTopLeft_GrSurfaceOrigin);
    if (!rtc) {
        return;
    }

    sk_sp<GrPathRenderer> pathRenderer(createPathRenderer(ctx.get()));
    SkPath path = createPath();

    // Initially, cache only has the render target context
    REPORTER_ASSERT(reporter, cache_non_scratch_resources_equals(cache, 0));

    // Draw the path, check that new resource count matches expectations
    draw_path(ctx.get(), rtc.get(), path, pathRenderer.get(), aaType, style);
    ctx->flush();
    REPORTER_ASSERT(reporter, cache_non_scratch_resources_equals(cache, expected));

    // Nothing should be purgeable yet
    cache->purgeAsNeeded();
    REPORTER_ASSERT(reporter, cache_non_scratch_resources_equals(cache, expected));

    // Reset the path to change the GenID, which should invalidate one resource in the cache.
    // Some path renderers may leave other unique-keyed resources in the cache, though.
    path.reset();
    cache->purgeAsNeeded();
    REPORTER_ASSERT(reporter, cache_non_scratch_resources_equals(cache, expected - 1));
}

// Test that deleting the original path invalidates the VBs cached by the tessellating path renderer
DEF_GPUTEST(TessellatingPathRendererCacheTest, reporter, /* options */) {
    auto createPR = [](GrContext*) {
        return new GrTessellatingPathRenderer();
    };

    // Tessellating path renderer creates a single vertex buffer for non-AA paths. No other
    // resources should be created.
    const int kExpectedResources = 1;

    test_path(reporter, create_concave_path, createPR, kExpectedResources);

    // Test with a style that alters the path geometry. This needs to attach the invalidation logic
    // to the original path, not the modified path produced by the style.
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    GrStyle style(paint);
    test_path(
            reporter, create_concave_path, createPR, kExpectedResources, GrAAType::kNone, style);
}

// Test that deleting the original path invalidates the textures cached by the SW path renderer
DEF_GPUTEST(SoftwarePathRendererCacheTest, reporter, /* options */) {
    auto createPR = [](GrContext* ctx) {
        return new GrSoftwarePathRenderer(ctx->priv().proxyProvider(), true);
    };

    // Software path renderer creates a mask texture and renders with a non-AA rect, but the flush
    // only contains a single quad so GrFillRectOp doesn't need to use the shared index buffer.
    const int kExpectedResources = 1;

    test_path(reporter, create_concave_path, createPR, kExpectedResources, GrAAType::kCoverage);

    // Test with a style that alters the path geometry. This needs to attach the invalidation logic
    // to the original path, not the modified path produced by the style.
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    GrStyle style(paint);
    test_path(reporter, create_concave_path, createPR, kExpectedResources, GrAAType::kCoverage,
              style);
}
