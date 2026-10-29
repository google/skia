/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkContext.h"
#include "include/core/SkContextOptions.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRecorder.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkContextPriv.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "tests/Test.h"
#include "tools/graphite/GraphiteTestContext.h"
#include "tools/graphite/TestOptions.h"

#include <memory>

// TODO(alexisdavidc) Re-enable once the new SkContext / CPU Context & Recorder API is implemented.
#if 0
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(
        CPUSurface_UsesGraphiteContextAndRasterRecorderToDraw_DrawsPixels,
        reporter,
        context,
        CtsEnforcement::kApiLevel_202604) {
    std::unique_ptr<skcpu::Recorder> recorder = context->makeCPURecorder();

    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = recorder->makeBitmapSurface(imageInfo, imageInfo.minRowBytes(), {});

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);

    // If this were a graphite surface, we'd have to do async read-back, but we make a raster
    // surface which is synchronous.
    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);
}
#endif  // 0

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ImageMakeColorSpace_GraphiteImageWithRecorder_Success,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202604) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();

    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto rImg = SkImages::RasterFromBitmap(bm);
    SkASSERT(rImg);

    auto img = SkImages::TextureFromImage(recorder.get(), rImg);
    SkASSERT(img);
    SkASSERT(img->isTextureBacked());

    auto newImg = img->makeColorSpace(recorder.get(), SkColorSpace::MakeSRGBLinear(), {});

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, newImg->colorSpace() == SkColorSpace::MakeSRGBLinear().get());
    REPORTER_ASSERT(reporter, newImg->isTextureBacked());
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ImageMakeScaled_GraphiteImageWithRecorder_Success,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202604) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();

    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto rImg = SkImages::RasterFromBitmap(bm);
    SkASSERT(rImg);

    auto img = SkImages::TextureFromImage(recorder.get(), rImg);
    SkASSERT(img);
    SkASSERT(img->isTextureBacked());

    REPORTER_ASSERT(reporter, img->isValid(recorder.get()));

    auto newImg =
            img->makeScaled(recorder.get(),
                            SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            {SkCubicResampler::Mitchell()});

    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 70);
    REPORTER_ASSERT(reporter, newImg->isTextureBacked());

    // Unlike Ganesh and Graphite, makeScaled doesn't work for Graphite images w/o a recorder
    auto legacyAPI =
            img->makeScaled(SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            {SkCubicResampler::Mitchell()});

    REPORTER_ASSERT(reporter, !legacyAPI);
}

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(CanvasBaseRecorder_GraphiteBasedCanvas_IsOriginalRecorder,
                                   reporter,
                                   context,
                                   CtsEnforcement::kApiLevel_202604) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder();

    auto ii = SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = SkSurfaces::RenderTarget(recorder.get(), ii);
    SkASSERT(surface);

    auto canvas = surface->getCanvas();
    SkASSERT(canvas);

    auto baseRecorder = canvas->baseRecorder();

    REPORTER_ASSERT(reporter, baseRecorder);
    REPORTER_ASSERT(reporter, baseRecorder->type() == SkRecorder::Type::kGraphite);
    REPORTER_ASSERT(reporter, baseRecorder == recorder.get());
}

DEF_CONDITIONAL_GRAPHITE_TEST_FOR_ALL_CONTEXTS(
        SkContext_MakeRecorder_GraphiteBackendSelectionAndOptions,
        reporter,
        context,
        testContext,
        true,
        CtsEnforcement::kApiLevel_202604) {
    if (!testContext) {
        return;
    }
    std::unique_ptr<skgpu::graphite::Context> graphiteContext =
            testContext->makeContext(skiatest::graphite::TestOptions{});
    if (!graphiteContext) {
        return;
    }

    SkContextOptions opts;

    std::unique_ptr<SkContext> ctx = SkContextCtorAccessor::MakeContext(
            opts, skgpu::graphite::ContextPriv::AsGPUContext(std::move(graphiteContext)));
    REPORTER_ASSERT(reporter, ctx != nullptr);

    // Passing nullptr explicitly returns default Graphite recorder.
    std::unique_ptr<SkRecorder> nullRecorder =
            ctx->makeRecorder(SkContext::BackendSelection::kAutomatic);
    REPORTER_ASSERT(reporter, nullRecorder != nullptr);
    REPORTER_ASSERT(reporter, nullRecorder->type() == SkRecorder::Type::kGraphite);

    // Explicit kAutomatic with options
    std::unique_ptr<SkRecorder> autoRecorder =
            ctx->makeRecorder(SkContext::BackendSelection::kAutomatic);
    REPORTER_ASSERT(reporter, autoRecorder != nullptr);
    REPORTER_ASSERT(reporter, autoRecorder->type() == SkRecorder::Type::kGraphite);

    // Explicit kGPU with options
    std::unique_ptr<SkRecorder> gpuRecorder = ctx->makeRecorder(SkContext::BackendSelection::kGPU);
    REPORTER_ASSERT(reporter, gpuRecorder != nullptr);
    REPORTER_ASSERT(reporter, gpuRecorder->type() == SkRecorder::Type::kGraphite);

    // Explicit kCPU with options falls back to CPU recorder
    std::unique_ptr<SkRecorder> cpuRecorder = ctx->makeRecorder(SkContext::BackendSelection::kCPU);
    REPORTER_ASSERT(reporter, cpuRecorder != nullptr);
    REPORTER_ASSERT(reporter, cpuRecorder->type() == SkRecorder::Type::kCPU);
}
