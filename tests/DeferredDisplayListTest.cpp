/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#include "GrBackendSurface.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetProxy.h"
#include "GrTextureProxy.h"
#include "GrTextureProxyPriv.h"
#include "GrTypes.h"
#include "GrTypesPriv.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkColorSpace.h"
#include "SkDeferredDisplayList.h"
#include "SkDeferredDisplayListRecorder.h"
#include "SkGpuDevice.h"
#include "SkImage.h"
#include "SkImageInfo.h"
#include "SkImage_Gpu.h"
#include "SkPaint.h"
#include "SkRect.h"
#include "SkRefCnt.h"
#include "SkSurface.h"
#include "SkSurfaceCharacterization.h"
#include "SkSurfaceProps.h"
#include "SkSurface_Gpu.h"
#include "Test.h"
#include "gl/GrGLCaps.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLTypes.h"

#ifdef SK_VULKAN
#include "vk/GrVkDefines.h"
#endif

#include <initializer_list>
#include <memory>
#include <utility>

// Try to create a backend format from the provided colorType and config. Return an invalid
// backend format if the combination is infeasible.
static GrBackendFormat create_backend_format(GrContext* context,
                                             SkColorType ct, SkColorSpace* cs,
                                             GrPixelConfig config) {
    const GrCaps* caps = context->contextPriv().caps();

    // TODO: what should be done if we have a colorspace that doesn't have a gammaCloseToSRGB?

    switch (context->contextPriv().getBackend()) {
    case kOpenGL_GrBackend: {
        const GrGLCaps* glCaps = static_cast<const GrGLCaps*>(caps);
        GrGLStandard standard = glCaps->standard();

        switch (ct) {
            case kUnknown_SkColorType:
                return GrBackendFormat();
            case kAlpha_8_SkColorType:
                if (kAlpha_8_as_Alpha_GrPixelConfig == config) {
                    return GrBackendFormat::MakeGL(GR_GL_ALPHA8, GR_GL_TEXTURE_2D);
                } else if (kAlpha_8_GrPixelConfig == config ||
                           kAlpha_8_as_Red_GrPixelConfig == config) {
                    return GrBackendFormat::MakeGL(GR_GL_R8, GR_GL_TEXTURE_2D);
                }
                break;
            case kRGB_565_SkColorType:
                if (kRGB_565_GrPixelConfig == config) {
                    return GrBackendFormat::MakeGL(GR_GL_RGB565, GR_GL_TEXTURE_2D);
                }
                break;
            case kARGB_4444_SkColorType:
                if (kRGBA_4444_GrPixelConfig == config) {
                    return GrBackendFormat::MakeGL(GR_GL_RGBA4, GR_GL_TEXTURE_2D);
                }
                break;
            case kRGBA_8888_SkColorType:
                if (kRGBA_8888_GrPixelConfig == config) {
                    if (!cs || (cs->gammaCloseToSRGB() && !caps->srgbSupport())) {
                        return GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_2D);
                    }
                } else if (kSRGBA_8888_GrPixelConfig == config) {
                    if (caps->srgbSupport() && cs && cs->gammaCloseToSRGB()) {
                        return GrBackendFormat::MakeGL(GR_GL_SRGB8_ALPHA8, GR_GL_TEXTURE_2D);
                    }
                }
                break;
            case kRGB_888x_SkColorType:
                if (kRGB_888_GrPixelConfig == config) {
                    return GrBackendFormat::MakeGL(GR_GL_RGB8, GR_GL_TEXTURE_2D);
                }
                break;
            case kBGRA_8888_SkColorType:
                if (kBGRA_8888_GrPixelConfig == config) {
                    if (kGL_GrGLStandard == standard) {
                        return GrBackendFormat::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_2D);
                    } else if (kGLES_GrGLStandard == standard) {
                        return GrBackendFormat::MakeGL(GR_GL_BGRA8, GR_GL_TEXTURE_2D);
                    }
                } else if (kSBGRA_8888_GrPixelConfig == config) {
                    if (caps->srgbSupport() && cs && cs->gammaCloseToSRGB()) {
                        return GrBackendFormat::MakeGL(GR_GL_SRGB8_ALPHA8, GR_GL_TEXTURE_2D);
                    }
                }
                break;
            case kRGBA_1010102_SkColorType:
                if (kRGBA_1010102_GrPixelConfig == config) {
                    return GrBackendFormat::MakeGL(GR_GL_RGB10_A2, GR_GL_TEXTURE_2D);
                }
                break;
            case kRGB_101010x_SkColorType:
                return GrBackendFormat();
            case kGray_8_SkColorType:
                if (kGray_8_as_Lum_GrPixelConfig == config) {
                    return GrBackendFormat::MakeGL(GR_GL_LUMINANCE8, GR_GL_TEXTURE_2D);
                } else if (kGray_8_GrPixelConfig == config ||
                           kGray_8_as_Red_GrPixelConfig == config) {
                    return GrBackendFormat::MakeGL(GR_GL_R8, GR_GL_TEXTURE_2D);
                }
                break;
            case kRGBA_F16_SkColorType:
                if (kRGBA_half_GrPixelConfig == config) {
                    return GrBackendFormat::MakeGL(GR_GL_RGBA16F, GR_GL_TEXTURE_2D);
                }
                break;
            case kRGBA_F32_SkColorType:
                return GrBackendFormat();
        }
    }
    break;
#ifdef SK_VULKAN
    case kVulkan_GrBackend:
        switch (ct) {
            case kUnknown_SkColorType:
                return GrBackendFormat();
            case kAlpha_8_SkColorType:
                // TODO: what about kAlpha_8_GrPixelConfig and kAlpha_8_as_Alpha_GrPixelConfig
                if (kAlpha_8_as_Red_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeVk(VK_FORMAT_R8_UNORM);
                }
                break;
            case kRGB_565_SkColorType:
                if (kRGB_565_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeVk(VK_FORMAT_R5G6B5_UNORM_PACK16);
                }
                break;
            case kARGB_4444_SkColorType:
                if (kRGBA_4444_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeVk(VK_FORMAT_B4G4R4A4_UNORM_PACK16);
                }
                break;
            case kRGBA_8888_SkColorType:
                if (kRGBA_8888_GrPixelConfig == config) {
                    if (!cs || (cs->gammaCloseToSRGB() && !caps->srgbSupport())) {
                        return GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8A8_UNORM);
                    }
                } else if (kSRGBA_8888_GrPixelConfig == config) {
                    if (caps->srgbSupport() && cs &&  cs->gammaCloseToSRGB()) {
                        return GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8A8_SRGB);
                    }
                }
                break;
            case kRGB_888x_SkColorType:
                if (kRGB_888_GrPixelConfig == config) {
                    return GrBackendFormat::MakeVk(VK_FORMAT_R8G8B8_UNORM);
                }
                break;
            case kBGRA_8888_SkColorType:
                if (kBGRA_8888_GrPixelConfig == config) {
                    return GrBackendFormat::MakeVk(VK_FORMAT_B8G8R8A8_UNORM);
                } else if (kSBGRA_8888_GrPixelConfig == config) {
                    if (caps->srgbSupport() && cs &&  cs->gammaCloseToSRGB()) {
                        return GrBackendFormat::MakeVk(VK_FORMAT_B8G8R8A8_SRGB);
                    }
                }
                break;
            case kRGBA_1010102_SkColorType:
                if (kRGBA_1010102_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeVk(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
                }
                break;
            case kRGB_101010x_SkColorType:
                return GrBackendFormat();
            case kGray_8_SkColorType:
                // TODO: what about kAlpha_8_GrPixelConfig and kGray_8_as_Lum_GrPixelConfig?
                if (kGray_8_as_Red_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeVk(VK_FORMAT_R8_UNORM);
                }
                break;
            case kRGBA_F16_SkColorType:
                if (kRGBA_half_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeVk(VK_FORMAT_R16G16B16A16_SFLOAT);
                }
                break;
            case kRGBA_F32_SkColorType:
                return GrBackendFormat();
        }
        break;
#endif
    case kMock_GrBackend:
        switch (ct) {
            case kUnknown_SkColorType:
                return GrBackendFormat();
            case kAlpha_8_SkColorType:
                if (kAlpha_8_GrPixelConfig == config ||
                    kAlpha_8_as_Alpha_GrPixelConfig == config ||
                    kAlpha_8_as_Red_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeMock(config);
                }
                break;
            case kRGB_565_SkColorType:
                if (kRGB_565_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeMock(config);
                }
                break;
            case kARGB_4444_SkColorType:
                if (kRGBA_4444_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeMock(config);
                }
                break;
            case kRGBA_8888_SkColorType:
                if (kRGBA_8888_GrPixelConfig == config) {
                    if (!cs || (cs->gammaCloseToSRGB() && !caps->srgbSupport())) {
                        return GrBackendFormat::MakeMock(config);
                    }
                } else if (kSRGBA_8888_GrPixelConfig == config) {
                    if (caps->srgbSupport() && cs && cs->gammaCloseToSRGB()) {
                        return GrBackendFormat::MakeMock(config);
                    }
                }
                break;
            case kRGB_888x_SkColorType:
                if (kRGB_888_GrPixelConfig == config) {
                    return GrBackendFormat::MakeMock(config);
                }
                break;
            case kBGRA_8888_SkColorType:
                if (kBGRA_8888_GrPixelConfig == config) {
                    return GrBackendFormat::MakeMock(config);
                } else if (kSBGRA_8888_GrPixelConfig == config) {
                    if (caps->srgbSupport() && cs &&  cs->gammaCloseToSRGB()) {
                        return GrBackendFormat::MakeMock(config);
                    }
                }
                break;
            case kRGBA_1010102_SkColorType:
                if (kRGBA_1010102_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeMock(config);
                }
                break;
            case kRGB_101010x_SkColorType:
                return GrBackendFormat();
            case kGray_8_SkColorType:
                if (kGray_8_GrPixelConfig == config ||
                    kGray_8_as_Lum_GrPixelConfig == config ||
                    kGray_8_as_Red_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeMock(config);
                }
                break;
            case kRGBA_F16_SkColorType:
                if (kRGBA_half_GrPixelConfig == config) {
                    return  GrBackendFormat::MakeMock(config);
                }
                break;
            case kRGBA_F32_SkColorType:
                return GrBackendFormat();
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

    SurfaceParameters(const GrCaps* caps)
            : fWidth(64)
            , fHeight(64)
            , fOrigin(kTopLeft_GrSurfaceOrigin)
            , fColorType(kRGBA_8888_SkColorType)
            , fConfig(caps->srgbSupport() ? kSRGBA_8888_GrPixelConfig : kRGBA_8888_GrPixelConfig)
            , fColorSpace(SkColorSpace::MakeSRGB())
            , fSampleCount(1)
            , fSurfaceProps(0x0, kUnknown_SkPixelGeometry)
            , fShouldCreateMipMaps(true) {
    }

    int sampleCount() const { return fSampleCount; }

    void setColorType(SkColorType ct) { fColorType = ct; }
    void setColorSpace(sk_sp<SkColorSpace> cs) { fColorSpace = std::move(cs); }
    void setConfig(GrPixelConfig config) { fConfig = config; }

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
            // The color type and config need to be changed together.
            // The original SRGB color space no longer makes sense for F16
            fColorType = kRGBA_F16_SkColorType;
            fConfig = kRGBA_half_GrPixelConfig;
            fColorSpace = SkColorSpace::MakeSRGBLinear();
            break;
        case 4:
            // This just needs to be a colorSpace different from that returned by MakeSRGB()
            // but still be considered SRGB. In this case we just change the gamut.
            fColorSpace = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                SkColorSpace::kAdobeRGB_Gamut);
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

    SkSurfaceCharacterization createCharacterization(GrContext* context) const {
        int maxResourceCount;
        size_t maxResourceBytes;
        context->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

        // Note that Ganesh doesn't make use of the SkImageInfo's alphaType
        SkImageInfo ii = SkImageInfo::Make(fWidth, fHeight, fColorType,
                                           kPremul_SkAlphaType, fColorSpace);

        GrBackendFormat backendFormat = create_backend_format(context, fColorType,
                                                              fColorSpace.get(), fConfig);
        if (!backendFormat.isValid()) {
            return SkSurfaceCharacterization();
        }

        SkSurfaceCharacterization c = context->threadSafeProxy()->createCharacterization(
                                                maxResourceBytes, ii, backendFormat, fSampleCount,
                                                fOrigin, fSurfaceProps, fShouldCreateMipMaps);
        return c;
    }

    // Create a DDL whose characterization captures the current settings
    std::unique_ptr<SkDeferredDisplayList> createDDL(GrContext* context) const {
        SkSurfaceCharacterization c = this->createCharacterization(context);
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
    sk_sp<SkSurface> make(GrContext* context, GrBackendTexture* backend,
                          bool nonTextureable) const {
        GrGpu* gpu = context->contextPriv().getGpu();

        GrMipMapped mipmapped = nonTextureable
                                        ? GrMipMapped::kNo
                                        : GrMipMapped(fShouldCreateMipMaps);

        *backend = gpu->createTestingOnlyBackendTexture(nullptr, fWidth, fHeight,
                                                        fConfig, true, mipmapped);
        if (!backend->isValid() || !gpu->isTestingOnlyBackendTexture(*backend)) {
            return nullptr;
        }

        sk_sp<SkSurface> surface;
        if (nonTextureable) {
            // Create a surface w/ the current parameters but make it non-textureable
            surface = SkSurface::MakeFromBackendTextureAsRenderTarget(
                                            context, *backend, fOrigin, fSampleCount, fColorType,
                                            fColorSpace, &fSurfaceProps);
        } else {
            surface = SkSurface::MakeFromBackendTexture(
                                            context, *backend, fOrigin, fSampleCount, fColorType,
                                            fColorSpace, &fSurfaceProps);
        }

        if (!surface) {
            gpu->deleteTestingOnlyBackendTexture(*backend);
            return nullptr;
        }

        return surface;
    }

    void cleanUpBackEnd(GrContext* context, const GrBackendTexture& backend) const {
        GrGpu* gpu = context->contextPriv().getGpu();

        gpu->deleteTestingOnlyBackendTexture(backend);
    }

private:
    int                 fWidth;
    int                 fHeight;
    GrSurfaceOrigin     fOrigin;
    SkColorType         fColorType;
    GrPixelConfig       fConfig;
    sk_sp<SkColorSpace> fColorSpace;
    int                 fSampleCount;
    SkSurfaceProps      fSurfaceProps;
    bool                fShouldCreateMipMaps;
};

// Test out operator== && operator!=
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DDLOperatorEqTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    for (int i = 0; i < SurfaceParameters::kNumParams; ++i) {
        SurfaceParameters params1(context->contextPriv().caps());
        params1.modify(i);

        SkSurfaceCharacterization char1 = params1.createCharacterization(context);
        if (!char1.isValid()) {
            continue;  // can happen on some platforms (ChromeOS)
        }

        for (int j = 0; j < SurfaceParameters::kNumParams; ++j) {
            SurfaceParameters params2(context->contextPriv().caps());
            params2.modify(j);

            SkSurfaceCharacterization char2 = params2.createCharacterization(context);
            if (!char2.isValid()) {
                continue;  // can happen on some platforms (ChromeOS)
            }

            if (i == j) {
                REPORTER_ASSERT(reporter, char1 == char2);
            } else {
                REPORTER_ASSERT(reporter, char1 != char2);
            }

        }
    }

    {
        SurfaceParameters params(context->contextPriv().caps());

        SkSurfaceCharacterization valid = params.createCharacterization(context);
        SkASSERT(valid.isValid());

        SkSurfaceCharacterization inval1, inval2;
        SkASSERT(!inval1.isValid() && !inval2.isValid());

        REPORTER_ASSERT(reporter, inval1 != inval2);
        REPORTER_ASSERT(reporter, valid != inval1);
        REPORTER_ASSERT(reporter, inval1 != valid);
    }
}

////////////////////////////////////////////////////////////////////////////////
// This tests SkSurfaceCharacterization/SkSurface compatibility
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DDLSurfaceCharacterizationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrGpu* gpu = context->contextPriv().getGpu();

    // Create a bitmap that we can readback into
    SkImageInfo imageInfo = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.allocPixels(imageInfo);

    std::unique_ptr<SkDeferredDisplayList> ddl;

    // First, create a DDL using the stock SkSurface parameters
    {
        SurfaceParameters params(context->contextPriv().caps());

        ddl = params.createDDL(context);
        SkAssertResult(ddl);

        // The DDL should draw into an SkSurface created with the same parameters
        GrBackendTexture backend;
        sk_sp<SkSurface> s = params.make(context, &backend, false);
        if (!s) {
            return;
        }

        REPORTER_ASSERT(reporter, s->draw(ddl.get()));
        s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);
        context->flush();
        gpu->testingOnly_flushGpuAndSync();
        s = nullptr;
        params.cleanUpBackEnd(context, backend);
    }

    // Then, alter each parameter in turn and check that the DDL & surface are incompatible
    for (int i = 0; i < SurfaceParameters::kNumParams; ++i) {
        SurfaceParameters params(context->contextPriv().caps());
        params.modify(i);

        GrBackendTexture backend;
        sk_sp<SkSurface> s = params.make(context, &backend, false);
        if (!s) {
            continue;
        }

        if (SurfaceParameters::kSampleCount == i) {
            SkSurface_Gpu* gpuSurf = static_cast<SkSurface_Gpu*>(s.get());

            int supportedSampleCount = context->contextPriv().caps()->getRenderTargetSampleCount(
                    params.sampleCount(),
                    gpuSurf->getDevice()
                            ->accessRenderTargetContext()
                            ->asRenderTargetProxy()
                            ->config());
            if (1 == supportedSampleCount) {
                // If changing the sample count won't result in a different
                // surface characterization, skip this step
                s = nullptr;
                params.cleanUpBackEnd(context, backend);
                continue;
            }
        }

        if (SurfaceParameters::kMipMipCount == i &&
            !context->contextPriv().caps()->mipMapSupport()) {
            // If changing the mipmap setting won't result in a different surface characterization,
            // skip this step
            s = nullptr;
            params.cleanUpBackEnd(context, backend);
            continue;
        }

        REPORTER_ASSERT(reporter, !s->draw(ddl.get()),
                        "DDLSurfaceCharacterizationTest failed on parameter: %d\n", i);

        context->flush();
        gpu->testingOnly_flushGpuAndSync();
        s = nullptr;
        params.cleanUpBackEnd(context, backend);
    }

    // Next test the compatibility of resource cache parameters
    {
        const SurfaceParameters params(context->contextPriv().caps());
        GrBackendTexture backend;

        sk_sp<SkSurface> s = params.make(context, &backend, false);

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

        context->flush();
        gpu->testingOnly_flushGpuAndSync();
        s = nullptr;
        params.cleanUpBackEnd(context, backend);
    }

    // Test that the textureability of the DDL characterization can block a DDL draw
    {
        GrBackendTexture backend;
        const SurfaceParameters params(context->contextPriv().caps());
        sk_sp<SkSurface> s = params.make(context, &backend, true);
        if (s) {
            REPORTER_ASSERT(reporter, !s->draw(ddl.get()));

            context->flush();
            gpu->testingOnly_flushGpuAndSync();
            s = nullptr;
            params.cleanUpBackEnd(context, backend);
        }
    }

    // Make sure non-GPU-backed surfaces fail characterization
    {
        SkImageInfo ii = SkImageInfo::MakeN32(64, 64, kOpaque_SkAlphaType);

        sk_sp<SkSurface> rasterSurface = SkSurface::MakeRaster(ii);
        SkSurfaceCharacterization c;
        REPORTER_ASSERT(reporter, !rasterSurface->characterize(&c));
    }

    // Exercise the createResized method
    {
        SurfaceParameters params(context->contextPriv().caps());
        GrBackendTexture backend;

        sk_sp<SkSurface> s = params.make(context, &backend, false);
        if (!s) {
            return;
        }

        SkSurfaceCharacterization char0;
        SkAssertResult(s->characterize(&char0));

        // Too small
        SkSurfaceCharacterization char1 = char0.createResized(-1, -1);
        REPORTER_ASSERT(reporter, !char1.isValid());

        // Too large
        SkSurfaceCharacterization char2 = char0.createResized(1000000, 32);
        REPORTER_ASSERT(reporter, !char2.isValid());

        // Just right
        SkSurfaceCharacterization char3 = char0.createResized(32, 32);
        REPORTER_ASSERT(reporter, char3.isValid());
        REPORTER_ASSERT(reporter, 32 == char3.width());
        REPORTER_ASSERT(reporter, 32 == char3.height());

        s = nullptr;
        params.cleanUpBackEnd(context, backend);
    }
}

////////////////////////////////////////////////////////////////////////////////
// This tests the SkSurface::MakeRenderTarget variant that takes an SkSurfaceCharacterization.
// In particular, the SkSurface and the SkSurfaceCharacterization should always be compatible.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DDLMakeRenderTargetTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    for (int i = 0; i < SurfaceParameters::kNumParams; ++i) {
        SurfaceParameters params(context->contextPriv().caps());
        params.modify(i);

        SkSurfaceCharacterization c = params.createCharacterization(context);
        GrBackendTexture backend;

        if (!c.isValid()) {
            sk_sp<SkSurface> tmp = params.make(context, &backend, false);

            // If we couldn't characterize the surface we shouldn't be able to create it either
            REPORTER_ASSERT(reporter, !tmp);
            if (tmp) {
                tmp = nullptr;
                params.cleanUpBackEnd(context, backend);
            }
            continue;
        }

        sk_sp<SkSurface> s = params.make(context, &backend, false);
        if (!s) {
            REPORTER_ASSERT(reporter, !c.isValid());
            continue;
        }

        REPORTER_ASSERT(reporter, c.isValid());

        s = SkSurface::MakeRenderTarget(context, c, SkBudgeted::kYes);
        REPORTER_ASSERT(reporter, s);

        SkSurface_Gpu* g = static_cast<SkSurface_Gpu*>(s.get());
        REPORTER_ASSERT(reporter, g->isCompatible(c));

        s = nullptr;
        params.cleanUpBackEnd(context, backend);
    }
}

////////////////////////////////////////////////////////////////////////////////
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
    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
            nullptr, kSize, kSize, kRGBA_8888_GrPixelConfig, false, GrMipMapped::kNo);
    if (!backendTex.isValid()) {
        return;
    }

    SurfaceParameters params(context->contextPriv().caps());
    GrBackendTexture backend;

    sk_sp<SkSurface> s = params.make(context, &backend, false);
    if (!s) {
        gpu->deleteTestingOnlyBackendTexture(backendTex);
        return;
    }

    SkSurfaceCharacterization c;
    SkAssertResult(s->characterize(&c));

    std::unique_ptr<SkDeferredDisplayListRecorder> recorder(new SkDeferredDisplayListRecorder(c));

    SkCanvas* canvas = recorder->getCanvas();
    if (!canvas) {
        s = nullptr;
        params.cleanUpBackEnd(context, backend);
        gpu->deleteTestingOnlyBackendTexture(backendTex);
        return;
    }

    GrContext* deferredContext = canvas->getGrContext();
    if (!deferredContext) {
        s = nullptr;
        params.cleanUpBackEnd(context, backend);
        gpu->deleteTestingOnlyBackendTexture(backendTex);
        return;
    }

    // Wrapped Backend Textures are not supported in DDL
    sk_sp<SkImage> image =
            SkImage::MakeFromAdoptedTexture(deferredContext, backendTex, kTopLeft_GrSurfaceOrigin,
                                            kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    REPORTER_ASSERT(reporter, !image);

    TextureReleaseChecker releaseChecker;
    image = SkImage::MakeFromTexture(deferredContext, backendTex, kTopLeft_GrSurfaceOrigin,
                                     kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr,
                                     TextureReleaseChecker::Release, &releaseChecker);
    REPORTER_ASSERT(reporter, !image);

    gpu->deleteTestingOnlyBackendTexture(backendTex);

    s = nullptr;
    params.cleanUpBackEnd(context, backend);
}

static void dummy_fulfill_proc(void*, GrBackendTexture*) { SkASSERT(0); }
static void dummy_release_proc(void*) { SkASSERT(0); }
static void dummy_done_proc(void*) { }

////////////////////////////////////////////////////////////////////////////////
// Test out the behavior of an invalid DDLRecorder
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DDLInvalidRecorder, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    {
        SkImageInfo ii = SkImageInfo::MakeN32Premul(32, 32);
        sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii);

        SkSurfaceCharacterization characterization;
        SkAssertResult(s->characterize(&characterization));

        // never calling getCanvas means the backing surface is never allocated
        SkDeferredDisplayListRecorder recorder(characterization);
    }

    {
        SkSurfaceCharacterization invalid;

        SkDeferredDisplayListRecorder recorder(invalid);

        const SkSurfaceCharacterization c = recorder.characterization();
        REPORTER_ASSERT(reporter, !c.isValid());
        REPORTER_ASSERT(reporter, !recorder.getCanvas());
        REPORTER_ASSERT(reporter, !recorder.detach());

        GrBackendFormat format = create_backend_format(context, kRGBA_8888_SkColorType,
                                                       nullptr, kRGBA_8888_GrPixelConfig);
        sk_sp<SkImage> image = recorder.makePromiseTexture(format, 32, 32, GrMipMapped::kNo,
                                                           kTopLeft_GrSurfaceOrigin,
                                                           kRGBA_8888_SkColorType,
                                                           kPremul_SkAlphaType, nullptr,
                                                           dummy_fulfill_proc,
                                                           dummy_release_proc,
                                                           dummy_done_proc,
                                                           nullptr);
        REPORTER_ASSERT(reporter, !image);
    }

}

////////////////////////////////////////////////////////////////////////////////
// Ensure that flushing while DDL recording doesn't cause a crash
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(DDLFlushWhileRecording, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    SkImageInfo ii = SkImageInfo::MakeN32Premul(32, 32);
    sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii);

    SkSurfaceCharacterization characterization;
    SkAssertResult(s->characterize(&characterization));

    SkDeferredDisplayListRecorder recorder(characterization);
    SkCanvas* canvas = recorder.getCanvas();

    canvas->flush();
    canvas->getGrContext()->flush();
}

////////////////////////////////////////////////////////////////////////////////
// Check that the texture-specific flags (i.e., for external & rectangle textures) work
// for promise images. As such, this is a GL-only test.
DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(DDLTextureFlagsTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    SkImageInfo ii = SkImageInfo::MakeN32Premul(32, 32);
    sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, ii);

    SkSurfaceCharacterization characterization;
    SkAssertResult(s->characterize(&characterization));

    SkDeferredDisplayListRecorder recorder(characterization);

    for (GrGLenum target : { GR_GL_TEXTURE_EXTERNAL, GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_2D } ) {
        for (auto mipMapped : { GrMipMapped::kNo, GrMipMapped::kYes }) {
            GrBackendFormat format = GrBackendFormat::MakeGL(GR_GL_RGBA8, target);

            sk_sp<SkImage> image = recorder.makePromiseTexture(format, 32, 32, mipMapped,
                                                               kTopLeft_GrSurfaceOrigin,
                                                               kRGBA_8888_SkColorType,
                                                               kPremul_SkAlphaType, nullptr,
                                                               dummy_fulfill_proc,
                                                               dummy_release_proc,
                                                               dummy_done_proc,
                                                               nullptr);
            if (GR_GL_TEXTURE_2D != target && mipMapped == GrMipMapped::kYes) {
                REPORTER_ASSERT(reporter, !image);
                continue;
            }
            REPORTER_ASSERT(reporter, image);

            GrTextureProxy* backingProxy = ((SkImage_Gpu*) image.get())->peekProxy();

            REPORTER_ASSERT(reporter, backingProxy->mipMapped() == mipMapped);
            if (GR_GL_TEXTURE_2D == target) {
                REPORTER_ASSERT(reporter, !backingProxy->texPriv().isClampOnly());
            } else {
                REPORTER_ASSERT(reporter, backingProxy->texPriv().isClampOnly());
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

// Exhaustively test colorType and pixelConfig compatibility.
DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(DDLCompatibilityTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    for (int ct = 0; ct <= kLastEnum_SkColorType; ++ct) {
        SkColorType colorType = static_cast<SkColorType>(ct);

        for (int config = 0; config < kGrPixelConfigCnt; ++config) {
            GrPixelConfig pixelConfig = static_cast<GrPixelConfig>(config);

            SurfaceParameters params(context->contextPriv().caps());
            params.setColorType(colorType);
            params.setConfig(pixelConfig);

            params.setColorSpace(nullptr);
            if (kSRGBA_8888_GrPixelConfig == pixelConfig ||
                kSBGRA_8888_GrPixelConfig == pixelConfig) {
                params.setColorSpace(SkColorSpace::MakeSRGB());
            }

            SkSurfaceCharacterization c = params.createCharacterization(context);
            GrBackendTexture backend;

            if (!c.isValid()) {
                // TODO: this would be cool to enable but there is, currently, too much crossover
                // allowed internally (e.g., kAlpha_8_SkColorType/kGray_8_as_Red_GrPixelConfig
                // is permitted on GL).
#if 0
                sk_sp<SkSurface> tmp = params.make(context, &backend, false);

                // If we couldn't characterize the surface we shouldn't be able to create it either
                REPORTER_ASSERT(reporter, !tmp);
                if (tmp) {
                    tmp = nullptr;
                    params.cleanUpBackEnd(context, backend);
                }
#endif
                continue;
            }

            sk_sp<SkSurface> s = params.make(context, &backend, false);
            REPORTER_ASSERT(reporter, s);
            if (!s) {
                s = nullptr;
                params.cleanUpBackEnd(context, backend);
                continue;
            }

            SkSurface_Gpu* gpuSurface = static_cast<SkSurface_Gpu*>(s.get());
            REPORTER_ASSERT(reporter, gpuSurface->isCompatible(c));

            s = nullptr;
            params.cleanUpBackEnd(context, backend);

            s = SkSurface::MakeRenderTarget(context, c, SkBudgeted::kYes);
            REPORTER_ASSERT(reporter, s);
            if (!s) {
                continue;
            }

            gpuSurface = static_cast<SkSurface_Gpu*>(s.get());
            REPORTER_ASSERT(reporter, gpuSurface->isCompatible(c));
        }
    }

}
