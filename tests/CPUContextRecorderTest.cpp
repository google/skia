/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/RasterContext.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkContext.h"
#include "include/core/SkContextOptions.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "src/capture/SkCapture.h"
#include "src/capture/SkCaptureCanvas.h"
#include "src/capture/SkCaptureManager.h"
#include "src/core/SkResourceCache.h"
#include "tests/Test.h"

#include <memory>

DEF_TEST(SkContext_CaptureDisabledByDefault, reporter) {
    SkContextOptions opts;
    std::unique_ptr<SkContext> ctx = SkContexts::MakeRaster(opts);
    REPORTER_ASSERT(reporter, ctx != nullptr);

    ctx->startCapture();
    sk_sp<SkCapture> capture = ctx->endCapture();
    REPORTER_ASSERT(reporter, capture == nullptr);
}

DEF_TEST(SkCaptureCanvas_LazilyReattachAfterSnapPicture, reporter) {
    sk_sp<SkCaptureManager> manager = sk_make_sp<SkCaptureManager>();
    SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
    SkCanvas* baseCanvas = surface->getCanvas();

    SkCanvas* canvas = manager->makeCaptureCanvas(baseCanvas);
    REPORTER_ASSERT(reporter, canvas != nullptr);

    manager->toggleCapture(true);

    // Draw something to trigger pollCapturingStatus() and attach a recording canvas.
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawRect(SkRect::MakeWH(50, 50), paint);

    // Snapping the picture should return the recorded draw and reset fCapturing to false
    // without immediately re-attaching a new recording canvas.
    sk_sp<SkPicture> pic1 = manager->snapPicture(surface.get());
    REPORTER_ASSERT(reporter, pic1 != nullptr);
    REPORTER_ASSERT(reporter, pic1->approximateOpCount() > 0);

    // A subsequent snapPicture() with no intervening draws should return nullptr rather than an
    // empty SkPicture.
    sk_sp<SkPicture> pic2 = manager->snapPicture(surface.get());
    REPORTER_ASSERT(reporter, pic2 == nullptr);

    // Ending capture without any new draws should not add an empty picture asset via
    // captureUninsertedDrawTasks().
    manager->toggleCapture(false);
    sk_sp<SkCapture> capture = manager->getLastCapture();
    REPORTER_ASSERT(reporter, capture != nullptr);
    REPORTER_ASSERT(reporter, capture->getMetadata().numAssets == 0);

    // Start a new capture and verify that a draw after snapPicture() lazily re-attaches a
    // recording canvas.
    manager->toggleCapture(true);
    canvas->drawRect(SkRect::MakeWH(25, 25), paint);
    sk_sp<SkPicture> pic3 = manager->snapPicture(surface.get());
    REPORTER_ASSERT(reporter, pic3 != nullptr);

    // Draw again after snapPicture() -> should re-attach and be captured on toggleCapture(false).
    paint.setColor(SK_ColorBLUE);
    canvas->drawRect(SkRect::MakeXYWH(25, 25, 50, 50), paint);

    manager->toggleCapture(false);
    capture = manager->getLastCapture();
    REPORTER_ASSERT(reporter, capture != nullptr);
    REPORTER_ASSERT(reporter, capture->getMetadata().numAssets == 1);
    REPORTER_ASSERT(reporter, capture->getAsset(0) != nullptr);
    REPORTER_ASSERT(reporter, capture->getAsset(0)->approximateOpCount() > 0);

    manager->deregisterCaptureCanvas(canvas);
}

// TODO(alexisdavidc) Re-enable once the new SkContext / CPU Context & Recorder API is implemented.
#if 0
DEF_TEST(CPUSurface_UsesCPUContextAndRecorderToDraw_DrawsPixels, reporter) {
    skcpu::Context::Options opts;
    auto ctx = skcpu::Context::Make(opts);
    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeRecorder();
    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = recorder->makeBitmapSurface(imageInfo, imageInfo.minRowBytes(), {});
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);
    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);
    REPORTER_ASSERT(reporter, surface->getCanvas()->baseRecorder() == recorder.get());
}

DEF_TEST(CPUSurface_UsesTODORecorder_DrawsPixels, reporter) {
    SkImageInfo imageInfo =
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surface = skcpu::Recorder::TODO()->makeBitmapSurface(imageInfo, imageInfo.minRowBytes(), {});
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, 3.1f));
    surface->getCanvas()->clear(SK_ColorGREEN);
    surface->getCanvas()->drawRRect(SkRRect::MakeRectXY(SkRect::MakeWH(50, 50), 10, 15), paint);
    SkPixmap pmap;
    REPORTER_ASSERT(reporter, surface->peekPixels(&pmap));
    REPORTER_ASSERT(reporter, pmap.getColor(25, 25) == SK_ColorRED);
    REPORTER_ASSERT(reporter, surface->getCanvas()->baseRecorder() == skcpu::Recorder::TODO());
}

DEF_TEST(ImageMakeColorSpace_UsesCPURecorderToMakeImage_Success, reporter) {
    auto ctx = skcpu::Context::Make();
    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeRecorder();
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto img = SkImages::RasterFromBitmap(bm);
    SkASSERT(img);
    REPORTER_ASSERT(reporter, img->isValid(recorder.get()));
    auto newImg = img->makeColorSpace(recorder.get(), SkColorSpace::MakeSRGBLinear(), {});
    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 100);
    REPORTER_ASSERT(reporter, !newImg->isTextureBacked());
}

DEF_TEST(ImageMakeScaled_UsesCPURecorderToMakeImage_Success, reporter) {
    auto ctx = skcpu::Context::Make();
    std::unique_ptr<skcpu::Recorder> recorder = ctx->makeRecorder();
    SkBitmap bm;
    bm.setInfo(SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    bm.allocPixels();
    auto img = SkImages::RasterFromBitmap(bm);
    SkASSERT(img);
    auto newImg =
            img->makeScaled(recorder.get(),
                            SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            {SkCubicResampler::Mitchell()});
    REPORTER_ASSERT(reporter, newImg);
    REPORTER_ASSERT(reporter, newImg->width() == 70);
    REPORTER_ASSERT(reporter, !newImg->isTextureBacked());
    auto legacyAPI =
            img->makeScaled(SkImageInfo::Make(70, 70, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
                            {SkCubicResampler::Mitchell()});
    REPORTER_ASSERT(reporter, legacyAPI);
    REPORTER_ASSERT(reporter, legacyAPI->width() == 70);
    REPORTER_ASSERT(reporter, !legacyAPI->isTextureBacked());
}

#endif  // 0
