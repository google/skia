/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCPURecorder.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkSurface.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Surface.h"
#include "tests/Test.h"

#include <memory>

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(
        CPUSurface_UsesGraphiteContextAndRasterRecorderToDraw_DrawsPixels,
        reporter,
        context,
        CtsEnforcement::kNextRelease) {
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

DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(ImageMakeColorSpace_GraphiteImageWithRecorder_Success,
                                   reporter,
                                   context,
                                   CtsEnforcement::kNextRelease) {
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
                                   CtsEnforcement::kNextRelease) {
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
                                   CtsEnforcement::kNextRelease) {
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
