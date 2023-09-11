/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrPaint.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrStyle.h"
#include "src/gpu/ganesh/GrUserStencilSettings.h"
#include "src/gpu/ganesh/PathRenderer.h"
#include "src/gpu/ganesh/SurfaceDrawContext.h"
#include "src/gpu/ganesh/effects/GrPorterDuffXferProcessor.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"
#include "src/gpu/ganesh/ops/SoftwarePathRenderer.h"
#include "src/gpu/ganesh/ops/TriangulatingPathRenderer.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"

#include <functional>
#include <memory>
#include <utility>

struct GrContextOptions;

static SkPath create_concave_path() {
    SkPath path;
    path.moveTo(100, 0);
    path.lineTo(200, 200);
    path.lineTo(100, 150);
    path.lineTo(0, 200);
    path.close();
    return path;
}

static void draw_path(GrRecordingContext* rContext,
                      skgpu::ganesh::SurfaceDrawContext* sdc,
                      const SkPath& path,
                      skgpu::ganesh::PathRenderer* pr,
                      GrAAType aaType,
                      const GrStyle& style,
                      float scaleX = 1.f) {
    GrPaint paint;
    paint.setXPFactory(GrPorterDuffXPFactory::Get(SkBlendMode::kSrc));

    SkIRect clipConservativeBounds = SkIRect::MakeWH(sdc->width(),
                                                     sdc->height());
    GrStyledShape shape(path, style);
    if (shape.style().applies()) {
        shape = shape.applyStyle(GrStyle::Apply::kPathEffectAndStrokeRec, 1.0f);
    }
    SkMatrix matrix = SkMatrix::I();
    matrix.setScaleX(scaleX);
    skgpu::ganesh::PathRenderer::DrawPathArgs args{rContext,
                                                   std::move(paint),
                                                   &GrUserStencilSettings::kUnused,
                                                   sdc,
                                                   nullptr,
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

static void test_path(
        skiatest::Reporter* reporter,
        std::function<SkPath(void)> createPath,
        std::function<skgpu::ganesh::PathRenderer*(GrRecordingContext*)> makePathRenderer,
        int expected,
        bool checkListeners,
        GrAAType aaType = GrAAType::kNone,
        GrStyle style = GrStyle(SkStrokeRec::kFill_InitStyle)) {
    sk_sp<GrDirectContext> dContext = GrDirectContext::MakeMock(nullptr);
    // The cache needs to be big enough that nothing gets flushed, or our expectations can be wrong
    dContext->setResourceCacheLimit(8000000);
    GrResourceCache* cache = dContext->priv().getResourceCache();

    auto sdc = skgpu::ganesh::SurfaceDrawContext::Make(dContext.get(),
                                                       GrColorType::kRGBA_8888,
                                                       nullptr,
                                                       SkBackingFit::kApprox,
                                                       {800, 800},
                                                       SkSurfaceProps(),
                                                       /*label=*/{},
                                                       /* sampleCnt= */ 1,
                                                       skgpu::Mipmapped::kNo,
                                                       GrProtected::kNo,
                                                       kTopLeft_GrSurfaceOrigin);
    if (!sdc) {
        return;
    }

    sk_sp<skgpu::ganesh::PathRenderer> pathRenderer(makePathRenderer(dContext.get()));
    SkPath path = createPath();

    // Initially, cache only has the render target context
    REPORTER_ASSERT(reporter, cache_non_scratch_resources_equals(cache, 0));

    // Draw the path, check that new resource count matches expectations
    draw_path(dContext.get(), sdc.get(), path, pathRenderer.get(), aaType, style);
    dContext->flushAndSubmit();
    REPORTER_ASSERT(reporter, cache_non_scratch_resources_equals(cache, expected));

    // Nothing should be purgeable yet
    cache->purgeAsNeeded();
    REPORTER_ASSERT(reporter, cache_non_scratch_resources_equals(cache, expected));

    // Reset the path to change the GenID, which should invalidate one resource in the cache.
    // Some path renderers may leave other unique-keyed resources in the cache, though.
    path.reset();
    cache->purgeAsNeeded();
    REPORTER_ASSERT(reporter, cache_non_scratch_resources_equals(cache, expected - 1));

    if (!checkListeners) {
        return;
    }

    // Test that purging the cache of masks also removes listeners from the path.
    path = createPath();
    REPORTER_ASSERT(reporter, SkPathPriv::GenIDChangeListenersCount(path) == 0);
    for (int i = 0; i < 20; ++i) {
        float scaleX = 1 + ((float)i + 1)/20.f;
        draw_path(dContext.get(), sdc.get(), path, pathRenderer.get(), aaType, style, scaleX);
    }
    dContext->flushAndSubmit();
    REPORTER_ASSERT(reporter, SkPathPriv::GenIDChangeListenersCount(path) == 20);
    cache->purgeUnlockedResources(GrPurgeResourceOptions::kAllResources);
    // The listeners don't actually purge until we try to add another one.
    draw_path(dContext.get(), sdc.get(), path, pathRenderer.get(), aaType, style);
    REPORTER_ASSERT(reporter, SkPathPriv::GenIDChangeListenersCount(path) == 1);
}

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
// Test that deleting the original path invalidates the VBs cached by the tessellating path renderer
DEF_GANESH_TEST(TriangulatingPathRendererCacheTest,
                reporter,
                /* options */,
                CtsEnforcement::kNever) {
    auto createPR = [](GrRecordingContext*) {
        return new skgpu::ganesh::TriangulatingPathRenderer();
    };

    // Triangulating path renderer creates a single vertex buffer for non-AA paths. No other
    // resources should be created.
    const int kExpectedResources = 1;

    test_path(reporter, create_concave_path, createPR, kExpectedResources, false);

    // Test with a style that alters the path geometry. This needs to attach the invalidation logic
    // to the original path, not the modified path produced by the style.
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    GrStyle style(paint);
    test_path(reporter, create_concave_path, createPR, kExpectedResources, false, GrAAType::kNone,
              style);
}
#endif

// Test that deleting the original path invalidates the textures cached by the SW path renderer
DEF_GANESH_TEST(SoftwarePathRendererCacheTest,
                reporter,
                /* options */,
                CtsEnforcement::kApiLevel_T) {
    auto createPR = [](GrRecordingContext* rContext) {
        return new skgpu::ganesh::SoftwarePathRenderer(rContext->priv().proxyProvider(), true);
    };

    // Software path renderer creates a mask texture and renders with a non-AA rect, but the flush
    // only contains a single quad so FillRectOp doesn't need to use the shared index buffer.
    const int kExpectedResources = 1;

    test_path(reporter, create_concave_path, createPR, kExpectedResources, true,
              GrAAType::kCoverage);

    // Test with a style that alters the path geometry. This needs to attach the invalidation logic
    // to the original path, not the modified path produced by the style.
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(1);
    GrStyle style(paint);
    test_path(reporter, create_concave_path, createPR, kExpectedResources, true,
              GrAAType::kCoverage, style);
}
