/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrBackendSurface.h"
#include "GrGpu.h"
#include "SkCanvas.h"
#include "SkDeferredDisplayListRecorder.h"
#include "SkGpuDevice.h"
#include "SkSurface.h"
#include "SkSurface_Gpu.h"
#include "SkSurfaceCharacterization.h"
#include "SkSurfaceProps.h"
#include "Test.h"

class SurfaceParameters {
public:
    static const int kNumParams = 8;
    static const int kSampleCount = 5;

    SurfaceParameters()
            : fWidth(64)
            , fHeight(64)
            , fOrigin(kTopLeft_GrSurfaceOrigin)
            , fColorType(kRGBA_8888_SkColorType)
            , fColorSpace(SkColorSpace::MakeSRGB())
            , fSampleCount(0)
            , fSurfaceProps(0x0, kUnknown_SkPixelGeometry) {
    }

    int sampleCount() const { return fSampleCount; }

    // Modify the SurfaceParameters in just one way
    void modify(int i) {
        switch (i) {
        case 0:
            fWidth = 63;
            break;
        case 1:
            fHeight = 63;
            break;
        case 2:
            fOrigin = kBottomLeft_GrSurfaceOrigin;
            break;
        case 3:
            fColorType = kRGBA_F16_SkColorType;
            break;
        case 4:
            fColorSpace = SkColorSpace::MakeSRGBLinear();
            break;
        case kSampleCount:
            fSampleCount = 4;
            break;
        case 6:
            fSurfaceProps = SkSurfaceProps(0x0, kRGB_H_SkPixelGeometry);
            break;
        case 7:
            fSurfaceProps = SkSurfaceProps(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
                                           kUnknown_SkPixelGeometry);
            break;
        }
    }

    // Create the surface with the current set of parameters
    sk_sp<SkSurface> make(GrContext* context) const {
        // Note that Ganesh doesn't make use of the SkImageInfo's alphaType
        SkImageInfo ii = SkImageInfo::Make(fWidth, fHeight, fColorType,
                                           kPremul_SkAlphaType, fColorSpace);

        return SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, ii, fSampleCount,
                                           fOrigin, &fSurfaceProps);
    }

private:
    int                 fWidth;
    int                 fHeight;
    GrSurfaceOrigin     fOrigin;
    SkColorType         fColorType;
    sk_sp<SkColorSpace> fColorSpace;
    int                 fSampleCount;
    SkSurfaceProps      fSurfaceProps;
};

// This tests SkSurfaceCharacterization/SkSurface compatibility
DEF_GPUTEST_FOR_ALL_CONTEXTS(SkSurfaceCharacterization, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    // Create a bitmap that we can readback into
    SkImageInfo imageInfo = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.allocPixels(imageInfo);

    std::unique_ptr<SkDeferredDisplayList> ddl;

    // First, create a DDL using the stock SkSurface parameters
    {
        SurfaceParameters params;

        sk_sp<SkSurface> s = params.make(context);
        if (!s) {
            return;
        }

        SkSurfaceCharacterization c;
        SkAssertResult(s->characterize(&c));

        SkDeferredDisplayListRecorder r(c);
        SkCanvas* canvas = r.getCanvas();
        if (!canvas) {
            return;
        }

        canvas->drawRect(SkRect::MakeXYWH(10, 10, 10, 10), SkPaint());
        ddl = r.detach();

        REPORTER_ASSERT(reporter, s->draw(ddl.get()));
        s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);
    }

    // Then, alter each parameter in turn and check that the DDL & surface are incompatible
    for (int i = 0; i < SurfaceParameters::kNumParams; ++i) {
        SurfaceParameters params;
        params.modify(i);

        sk_sp<SkSurface> s = params.make(context);
        if (!s) {
            continue;
        }

        if (SurfaceParameters::kSampleCount == i) {
            SkSurface_Gpu* gpuSurf = static_cast<SkSurface_Gpu*>(s.get());

            int supportedSampleCount = context->caps()->getSampleCount(
                params.sampleCount(),
                gpuSurf->getDevice()->accessRenderTargetContext()->asRenderTargetProxy()->config());
            if (0 == supportedSampleCount) {
                // If changing the sample count won't result in a different
                // surface characterization, skip this step
                continue;
            }
        }

        REPORTER_ASSERT(reporter, !s->draw(ddl.get()));
    }

    // Next test the compatibility of resource cache parameters
    {
        const SurfaceParameters params;
        sk_sp<SkSurface> s = params.make(context);

        int maxResourceCount;
        size_t maxResourceBytes;
        context->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

        context->setResourceCacheLimits(maxResourceCount/2, maxResourceBytes);
        REPORTER_ASSERT(reporter, !s->draw(ddl.get()));

        context->setResourceCacheLimits(maxResourceCount, maxResourceBytes/2);
        REPORTER_ASSERT(reporter, !s->draw(ddl.get()));

        // DDL TODO: once proxies/ops can be de-instantiated we can re-enable these tests.
        // For now, DDLs are drawn once.
#if 0
        // resource limits >= those at characterization time are accepted
        context->setResourceCacheLimits(2*maxResourceCount, maxResourceBytes);
        REPORTER_ASSERT(reporter, s->draw(ddl.get()));
        s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);

        context->setResourceCacheLimits(maxResourceCount, 2*maxResourceBytes);
        REPORTER_ASSERT(reporter, s->draw(ddl.get()));
        s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);

        context->setResourceCacheLimits(maxResourceCount, maxResourceBytes);
        REPORTER_ASSERT(reporter, s->draw(ddl.get()));
        s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);
#endif
    }

    // Make sure non-GPU-backed surfaces fail characterization
    {
        SkImageInfo ii = SkImageInfo::MakeN32(64, 64, kOpaque_SkAlphaType);

        sk_sp<SkSurface> rasterSurface = SkSurface::MakeRaster(ii);
        SkSurfaceCharacterization c;
        REPORTER_ASSERT(reporter, !rasterSurface->characterize(&c));
    }
}

static constexpr int kSize = 8;

struct TextureReleaseChecker {
    TextureReleaseChecker() : fReleaseCount(0) {}
    int fReleaseCount;
    static void Release(void* self) {
        static_cast<TextureReleaseChecker*>(self)->fReleaseCount++;
    }
};

enum class DDLStage { kMakeImage, kDrawImage, kDetach, kDrawDDL };

// This tests the ability to create and use wrapped textures in a DDL world
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DDLWrapBackendTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGpu* gpu = context->contextPriv().getGpu();
    for (auto lastStage : { DDLStage::kMakeImage, DDLStage::kDrawImage,
                            DDLStage::kDetach, DDLStage::kDrawDDL } ) {
        for (auto earlyImageReset : { false , true } ) {
            GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
                    nullptr, kSize, kSize, kRGBA_8888_GrPixelConfig, false, GrMipMapped::kNo);
            if (!backendTex.isValid()) {
                continue;
            }

            SurfaceParameters params;

            sk_sp<SkSurface> s = params.make(context);
            if (!s) {
                gpu->deleteTestingOnlyBackendTexture(&backendTex);
                continue;
            }

            SkSurfaceCharacterization c;
            SkAssertResult(s->characterize(&c));

            std::unique_ptr<SkDeferredDisplayListRecorder> recorder(
                    new SkDeferredDisplayListRecorder(c));

            SkCanvas* canvas = recorder->getCanvas();
            if (!canvas) {
                gpu->deleteTestingOnlyBackendTexture(&backendTex);
                continue;
            }

            GrContext* deferredContext = canvas->getGrContext();
            if (!deferredContext) {
                gpu->deleteTestingOnlyBackendTexture(&backendTex);
                continue;
            }

            sk_sp<SkImage> image = SkImage::MakeFromAdoptedTexture(deferredContext, backendTex,
                                                                   kTopLeft_GrSurfaceOrigin,
                                                                   kRGBA_8888_SkColorType,
                                                                   kPremul_SkAlphaType, nullptr);
            // Adopted Textures are not supported in DDL
            REPORTER_ASSERT(reporter, !image);

            TextureReleaseChecker releaseChecker;
            image = SkImage::MakeFromTexture(deferredContext, backendTex,
                                             kTopLeft_GrSurfaceOrigin,
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType, nullptr,
                                             TextureReleaseChecker::Release, &releaseChecker);

            REPORTER_ASSERT(reporter, image);
            if (!image) {
                gpu->deleteTestingOnlyBackendTexture(&backendTex);
                continue;
            }

            if (DDLStage::kMakeImage == lastStage) {
                REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
                image.reset();
                REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);
                recorder.reset();
                REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);
                gpu->deleteTestingOnlyBackendTexture(&backendTex);
                continue;
            }

            canvas->drawImage(image.get(), 0, 0);

            if (earlyImageReset) {
                REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
                image.reset();
                // Ref should still be held by DDL recorder since we did the draw
                REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
            }

            if (DDLStage::kDrawImage == lastStage) {
                REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
                recorder.reset();
                if (earlyImageReset) {
                    REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);
                } else {
                    REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
                    image.reset();
                    REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);
                }
                gpu->deleteTestingOnlyBackendTexture(&backendTex);
                continue;
            }

            std::unique_ptr<SkDeferredDisplayList> ddl = recorder->detach();
            if (DDLStage::kDetach == lastStage) {
                REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
                recorder.reset();
#ifndef SK_RASTER_RECORDER_IMPLEMENTATION
                REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
#endif
                ddl.reset();
                if (earlyImageReset) {
                    REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);
                } else {
                    REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
                    image.reset();
                    REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);
                }
                gpu->deleteTestingOnlyBackendTexture(&backendTex);
                continue;
            }

            REPORTER_ASSERT(reporter, s->draw(ddl.get()));

            REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
            recorder.reset();
#ifndef SK_RASTER_RECORDER_IMPLEMENTATION
            REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
#endif
            ddl.reset();
#ifndef SK_RASTER_RECORDER_IMPLEMENTATION
            REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
#endif

            // Force all draws to flush and sync by calling a read pixels
            SkImageInfo imageInfo = SkImageInfo::Make(kSize, kSize, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);
            SkBitmap bitmap;
            bitmap.allocPixels(imageInfo);
            s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);

            if (earlyImageReset) {
                REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);
            } else {
                REPORTER_ASSERT(reporter, 0 == releaseChecker.fReleaseCount);
                image.reset();
                REPORTER_ASSERT(reporter, 1 == releaseChecker.fReleaseCount);
            }

            gpu->deleteTestingOnlyBackendTexture(&backendTex);
        }
    }
}


#endif
