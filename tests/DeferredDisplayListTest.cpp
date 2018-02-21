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

#include "gl/GrGLDefines.h"
#ifdef SK_VULKAN
#include "vk/GrVkDefines.h"
#endif

static GrBackendFormat create_backend_format(GrContext* context, SkColorType colorType) {
    const GrCaps* caps = context->caps();

    switch (context->contextPriv().getBackend()) {
    case kOpenGL_GrBackend:
        if (kRGBA_8888_SkColorType == colorType) {
            GrGLenum format = caps->srgbSupport() ? GR_GL_SRGB8_ALPHA8 : GR_GL_RGBA8;
            return GrBackendFormat::MakeGL(format, GR_GL_TEXTURE_2D);
        } else if (kRGBA_F16_SkColorType == colorType) {
            return GrBackendFormat::MakeGL(GR_GL_RGBA16F, GR_GL_TEXTURE_2D);
        }
        break;
#ifdef SK_VULKAN
    case kVulkan_GrBackend:
        if (kRGBA_8888_SkColorType == colorType) {
            VkFormat format =  caps->srgbSupport() ? VK_FORMAT_R8G8B8A8_SRGB
                                                   : VK_FORMAT_R8G8B8A8_UNORM;
            return GrBackendFormat::MakeVK(format);
        } else if (kRGBA_F16_SkColorType == colorType) {
            return GrBackendFormat::MakeVK(VK_FORMAT_R16G16B16A16_SFLOAT);
        }
        break;
#endif
    case kMock_GrBackend:
        if (kRGBA_8888_SkColorType == colorType) {
            GrPixelConfig config = caps->srgbSupport() ? kSRGBA_8888_GrPixelConfig
                                                       : kRGBA_8888_GrPixelConfig;
            return GrBackendFormat::MakeMock(config);
        } else if (kRGBA_F16_SkColorType == colorType) {
            return GrBackendFormat::MakeMock(kRGBA_half_GrPixelConfig);
        }
        break;
    default:
        return GrBackendFormat(); // return an invalid format
    }

    return GrBackendFormat(); // return an invalid format
}


class SurfaceParameters {
public:
    static const int kNumParams = 9;
    static const int kSampleCount = 5;
    static const int kMipMipCount = 8;

    SurfaceParameters()
            : fWidth(64)
            , fHeight(64)
            , fOrigin(kTopLeft_GrSurfaceOrigin)
            , fColorType(kRGBA_8888_SkColorType)
            , fColorSpace(SkColorSpace::MakeSRGB())
            , fSampleCount(1)
            , fSurfaceProps(0x0, kUnknown_SkPixelGeometry)
            , fShouldCreateMipMaps(true) {
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
        case 8:
            fShouldCreateMipMaps = false;
            break;
        }
    }

    // Create a DDL whose characterization captures the current settings
    std::unique_ptr<SkDeferredDisplayList> createDDL(GrContext* context) const {
        sk_sp<SkSurface> s = this->make(context);
        if (!s) {
            return nullptr;
        }

        int maxResourceCount;
        size_t maxResourceBytes;
        context->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

        // Note that Ganesh doesn't make use of the SkImageInfo's alphaType
        SkImageInfo ii = SkImageInfo::Make(fWidth, fHeight, fColorType,
                                           kPremul_SkAlphaType, fColorSpace);

        GrBackendFormat backendFormat = create_backend_format(context, fColorType);

        SkSurfaceCharacterization c = context->threadSafeProxy()->createCharacterization(
                                                maxResourceBytes, ii, backendFormat, fSampleCount,
                                                fOrigin, fSurfaceProps, fShouldCreateMipMaps);
        SkAssertResult(c.isValid());

        SkDeferredDisplayListRecorder r(c);
        SkCanvas* canvas = r.getCanvas();
        if (!canvas) {
            return nullptr;
        }

        canvas->drawRect(SkRect::MakeXYWH(10, 10, 10, 10), SkPaint());
        return r.detach();
    }

    // Create the surface with the current set of parameters
    sk_sp<SkSurface> make(GrContext* context) const {
        // Note that Ganesh doesn't make use of the SkImageInfo's alphaType
        SkImageInfo ii = SkImageInfo::Make(fWidth, fHeight, fColorType,
                                           kPremul_SkAlphaType, fColorSpace);

        return SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, ii, fSampleCount,
                                           fOrigin, &fSurfaceProps, fShouldCreateMipMaps);
    }

    // Create a surface w/ the current parameters but make it non-textureable
    sk_sp<SkSurface> makeNonTextureable(GrContext* context, GrBackendTexture* backend) const {
        GrGpu* gpu = context->contextPriv().getGpu();

        GrPixelConfig config = SkImageInfo2GrPixelConfig(fColorType, nullptr, *context->caps());
        SkASSERT(kUnknown_GrPixelConfig != config);

        *backend = gpu->createTestingOnlyBackendTexture(nullptr, fWidth, fHeight,
                                                        config, true, GrMipMapped::kNo);

        if (!backend->isValid() || !gpu->isTestingOnlyBackendTexture(*backend)) {
            return nullptr;
        }

        sk_sp<SkSurface> surface = SkSurface::MakeFromBackendTextureAsRenderTarget(
            context, *backend, fOrigin, fSampleCount, fColorType, nullptr, nullptr);

        if (!surface) {
            gpu->deleteTestingOnlyBackendTexture(backend);
            return nullptr;
        }

        return surface;
    }

    void cleanUpBackEnd(GrContext* context, GrBackendTexture* backend) const {
        GrGpu* gpu = context->contextPriv().getGpu();

        gpu->deleteTestingOnlyBackendTexture(backend);
    }

private:
    int                 fWidth;
    int                 fHeight;
    GrSurfaceOrigin     fOrigin;
    SkColorType         fColorType;
    sk_sp<SkColorSpace> fColorSpace;
    int                 fSampleCount;
    SkSurfaceProps      fSurfaceProps;
    bool                fShouldCreateMipMaps;
};

// This tests SkSurfaceCharacterization/SkSurface compatibility
DEF_GPUTEST_FOR_ALL_CONTEXTS(DDLSurfaceCharacterizationTest, reporter, ctxInfo) {
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

        ddl = params.createDDL(context);
        SkAssertResult(ddl);

        // The DDL should draw into an SkSurface created with the same parameters
        sk_sp<SkSurface> s = params.make(context);
        if (!s) {
            return;
        }

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

            int supportedSampleCount = context->caps()->getRenderTargetSampleCount(
                params.sampleCount(),
                gpuSurf->getDevice()->accessRenderTargetContext()->asRenderTargetProxy()->config());
            if (1 == supportedSampleCount) {
                // If changing the sample count won't result in a different
                // surface characterization, skip this step
                continue;
            }
        }

        if (SurfaceParameters::kMipMipCount == i && !context->caps()->mipMapSupport()) {
            continue;
        }

        REPORTER_ASSERT(reporter, !s->draw(ddl.get()),
                        "DDLSurfaceCharacterizationTest failed on parameter: %d\n", i);
    }

    // Next test the compatibility of resource cache parameters
    {
        const SurfaceParameters params;
        sk_sp<SkSurface> s = params.make(context);

        int maxResourceCount;
        size_t maxResourceBytes;
        context->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

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

    // Test that the textureability of the DDL characterization can block a DDL draw
    {
        GrBackendTexture backend;
        const SurfaceParameters params;
        sk_sp<SkSurface> s = params.makeNonTextureable(context, &backend);
        if (s) {
            REPORTER_ASSERT(reporter, !s->draw(ddl.get()));

            s = nullptr;
            params.cleanUpBackEnd(context, &backend);
        }
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
