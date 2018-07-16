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
#include "GrTextureProxyPriv.h"

#include "SkCanvas.h"
#include "SkDeferredDisplayListRecorder.h"
#include "SkGpuDevice.h"
#include "SkImage_Gpu.h"
#include "SkSurface.h"
#include "SkSurface_Gpu.h"
#include "SkSurfaceCharacterization.h"
#include "SkSurfaceProps.h"
#include "SkUtils.h"
#include "Test.h"

#include "gl/GrGLDefines.h"
#ifdef SK_VULKAN
#include "vk/GrVkDefines.h"
#endif

static GrBackendFormat create_backend_format(GrContext* context, SkColorType colorType,
                                             SkColorSpace* cs) {
    const GrCaps* caps = context->caps();

    switch (context->contextPriv().getBackend()) {
    case kOpenGL_GrBackend:
        if (kRGBA_8888_SkColorType == colorType) {
            GrGLenum format = caps->srgbSupport() && cs && cs->gammaCloseToSRGB()
                                                                    ? GR_GL_SRGB8_ALPHA8
                                                                    : GR_GL_RGBA8;
            return GrBackendFormat::MakeGL(format, GR_GL_TEXTURE_2D);
        } else if (kRGBA_F16_SkColorType == colorType) {
            return GrBackendFormat::MakeGL(GR_GL_RGBA16F, GR_GL_TEXTURE_2D);
        }
        break;
#ifdef SK_VULKAN
    case kVulkan_GrBackend:
        if (kRGBA_8888_SkColorType == colorType) {
            VkFormat format =  caps->srgbSupport() && cs && cs->gammaCloseToSRGB()
                                                   ? VK_FORMAT_R8G8B8A8_SRGB
                                                   : VK_FORMAT_R8G8B8A8_UNORM;
            return GrBackendFormat::MakeVk(format);
        } else if (kRGBA_F16_SkColorType == colorType) {
            return GrBackendFormat::MakeVk(VK_FORMAT_R16G16B16A16_SFLOAT);
        }
        break;
#endif
    case kMock_GrBackend:
        if (kRGBA_8888_SkColorType == colorType) {
            GrPixelConfig config = caps->srgbSupport() && cs && cs->gammaCloseToSRGB()
                                                       ? kSRGBA_8888_GrPixelConfig
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
        int maxResourceCount;
        size_t maxResourceBytes;
        context->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

        // Note that Ganesh doesn't make use of the SkImageInfo's alphaType
        SkImageInfo ii = SkImageInfo::Make(fWidth, fHeight, fColorType,
                                           kPremul_SkAlphaType, fColorSpace);

        GrBackendFormat backendFormat = create_backend_format(context, fColorType,
                                                              fColorSpace.get());

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
        SurfaceParameters params;

        sk_sp<SkSurface> s = params.make(context);
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
    GrBackendTexture backendTex = gpu->createTestingOnlyBackendTexture(
            nullptr, kSize, kSize, kRGBA_8888_GrPixelConfig, false, GrMipMapped::kNo);
    if (!backendTex.isValid()) {
        return;
    }

    SurfaceParameters params;

    sk_sp<SkSurface> s = params.make(context);
    if (!s) {
        gpu->deleteTestingOnlyBackendTexture(backendTex);
        return;
    }

    SkSurfaceCharacterization c;
    SkAssertResult(s->characterize(&c));

    std::unique_ptr<SkDeferredDisplayListRecorder> recorder(new SkDeferredDisplayListRecorder(c));

    SkCanvas* canvas = recorder->getCanvas();
    if (!canvas) {
        gpu->deleteTestingOnlyBackendTexture(backendTex);
        return;
    }

    GrContext* deferredContext = canvas->getGrContext();
    if (!deferredContext) {
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
}

static void dummy_fulfill_proc(void*, GrBackendTexture*) { SkASSERT(0); }
static void dummy_release_proc(void*) { SkASSERT(0); }
static void dummy_done_proc(void*) { }

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

        GrBackendFormat format = create_backend_format(context, kRGBA_8888_SkColorType, nullptr);
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
        GrBackendFormat format = GrBackendFormat::MakeGL(GR_GL_RGBA8, target);

        sk_sp<SkImage> image = recorder.makePromiseTexture(format, 32, 32, GrMipMapped::kNo,
                                                           kTopLeft_GrSurfaceOrigin,
                                                           kRGBA_8888_SkColorType,
                                                           kPremul_SkAlphaType, nullptr,
                                                           dummy_fulfill_proc,
                                                           dummy_release_proc,
                                                           dummy_done_proc,
                                                           nullptr);
        REPORTER_ASSERT(reporter, image);

        GrTextureProxy* backingProxy = ((SkImage_Gpu*) image.get())->peekProxy();

        if (GR_GL_TEXTURE_2D == target) {
            REPORTER_ASSERT(reporter, !backingProxy->texPriv().doesNotSupportMipMaps());
            REPORTER_ASSERT(reporter, !backingProxy->texPriv().isClampOnly());
        } else {
            REPORTER_ASSERT(reporter, backingProxy->texPriv().doesNotSupportMipMaps());
            REPORTER_ASSERT(reporter, backingProxy->texPriv().isClampOnly());
        }
    }

}

static const int kTileSize = 64;

static std::unique_ptr<SkDeferredDisplayList> make_ddl(GrContext* context, int size,
                                                       std::function<void(SkCanvas*)> drawFn) {
    int maxResourceCount;
    size_t maxResourceBytes;
    context->getResourceCacheLimits(&maxResourceCount, &maxResourceBytes);

    // Note that Ganesh doesn't make use of the SkImageInfo's alphaType
    SkImageInfo ii = SkImageInfo::Make(size, size, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType, nullptr);

    GrBackendFormat backendFormat = create_backend_format(context, kRGBA_8888_SkColorType, nullptr);

    SkSurfaceProps props(0x0, kUnknown_SkPixelGeometry);

    // DDL TODO: try out specifying the isMipMapped flag here
    SkSurfaceCharacterization c = context->threadSafeProxy()->createCharacterization(
                                                    maxResourceBytes, ii, backendFormat, 1,
                                                    kTopLeft_GrSurfaceOrigin, props, false);
    SkAssertResult(c.isValid());

    SkDeferredDisplayListRecorder r(c);
    SkCanvas* canvas = r.getCanvas();
    if (!canvas) {
        return nullptr;
    }

    drawFn(canvas);
    return r.detach();
}

static std::unique_ptr<SkDeferredDisplayList> make_color_ddl(GrContext* context, SkColor color) {
    return make_ddl(context, kTileSize,
                    [&] (SkCanvas* canvas) {
                        SkPaint p;
                        p.setColor(color);

                        canvas->drawRect(SkRect::MakeWH(kTileSize, kTileSize), p);
                    });
}

// Create an SkSurface compatible w/ a DDL created in make_ddl
sk_sp<SkSurface> make_surface(GrContext* context, int size) {
    SkImageInfo ii = SkImageInfo::Make(size, size, kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType, nullptr);

    SkSurfaceProps props(0x0, kUnknown_SkPixelGeometry);

    return SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, ii, 1,
                                       kTopLeft_GrSurfaceOrigin, &props, false);
}

SkBitmap read_back(SkSurface* src, int size) {
    SkImageInfo ii = SkImageInfo::Make(size, size, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap result;
    result.allocPixels(ii);
    SkAssertResult(src->readPixels(result, 0, 0));
    return result;
}

void save_bm(const SkBitmap& bm, const char* name) {
    static int index = 0;

    char filename[256];
    _snprintf(filename, 256, "C:\\src\\skia.2\\local-win\\%s-%d.png", name, index++);
    filename[255] = '\0';

    SkFILEWStream file(filename);
    SkASSERT(file.isValid());

    SkEncodeImage(&file, bm, SkEncodedImageFormat::kPNG, 100);
}

// Test that chaining together DDLs via SkImages works.
// It creates the following DAG of DDL/SkImage draws
//
//           Red   Green  Blue  Cyan
//             \    \       /    /
//              \    \     /    /
//               \    \   /    /
//                \    \ /    /
//                  Rect clip
//
// and then draws them all at once in R,G,B,C,clip order in the pattern
//
//              Red  | Green
//             --------------
//              Blue | Cyan
//
DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(DDLSnapImageTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    std::unique_ptr<SkDeferredDisplayList> redDDL   = make_color_ddl(context, SK_ColorRED);
    std::unique_ptr<SkDeferredDisplayList> greenDDL = make_color_ddl(context, SK_ColorGREEN);
    std::unique_ptr<SkDeferredDisplayList> blueDDL  = make_color_ddl(context, SK_ColorBLUE);
    std::unique_ptr<SkDeferredDisplayList> cyanDDL  = make_color_ddl(context, SK_ColorCYAN);

    sk_sp<SkImage> redImg   = redDDL->futureImage();
    sk_sp<SkImage> greenImg = greenDDL->futureImage();
    sk_sp<SkImage> blueImg  = blueDDL->futureImage();
    sk_sp<SkImage> cyanImg  = cyanDDL->futureImage();

    std::unique_ptr<SkDeferredDisplayList> clipDDL = make_ddl(context, 2*kTileSize,
                    [&] (SkCanvas* canvas) {
                        canvas->clear(SK_ColorBLACK);
                        SkRect r = SkRect::MakeXYWH(kTileSize/2, kTileSize/2, kTileSize, kTileSize);
                        canvas->clipRect(r, false);
                        canvas->drawImage(redImg,   0,         0);
                        canvas->drawImage(greenImg, kTileSize, 0);
//                        canvas->drawImage(blueImg,  0,         kTileSize);
//                        canvas->drawImage(cyanImg,  kTileSize, kTileSize);
                    });

    sk_sp<SkSurface> redDest   = make_surface(context, kTileSize);
    sk_sp<SkSurface> greenDest = make_surface(context, kTileSize);
//    sk_sp<SkSurface> blueDest  = make_surface(context, kTileSize);
//    sk_sp<SkSurface> cyanDest  = make_surface(context, kTileSize);
    sk_sp<SkSurface> clipDest  = make_surface(context, 2*kTileSize);

    SkAssertResult(redDest->draw(redDDL.get()));
    SkAssertResult(greenDest->draw(greenDDL.get()));
//    SkAssertResult(blueDest->draw(blueDDL.get()));
//    SkAssertResult(cyanDest->draw(cyanDDL.get()));
    SkAssertResult(clipDest->draw(clipDDL.get()));
    context->flush();

    SkBitmap red = read_back(redDest.get(), kTileSize);
    save_bm(red, "red");

    SkBitmap green = read_back(greenDest.get(), kTileSize);
    save_bm(green, "green");

    SkBitmap actual = read_back(clipDest.get(), 2*kTileSize);

    SkImageInfo ii = SkImageInfo::Make(2*kTileSize, 2*kTileSize, kRGBA_8888_SkColorType, 
                                       kPremul_SkAlphaType);

    SkIRect r = SkIRect::MakeWH(kTileSize/2, kTileSize/2);

    SkBitmap expected;
    expected.allocPixels(ii);
    expected.eraseColor(SK_ColorBLACK);
    expected.erase(SK_ColorRED,     r.makeOffset(kTileSize/2, kTileSize/2));
    expected.erase(SK_ColorGREEN,   r.makeOffset(kTileSize,   kTileSize/2));
//    expected.erase(SK_ColorBLUE,    r.makeOffset(kTileSize/2, kTileSize));
//    expected.erase(SK_ColorMAGENTA, r.makeOffset(kTileSize,   kTileSize));

    bool abort = false;
    for (int y = 0; y < 2*kTileSize && !abort; ++y) {
        for (int x = 0; x < 2*kTileSize; ++x) {
            uint32_t expectedColor = *expected.getAddr32(x, y);
            uint32_t actualColor = *actual.getAddr32(x, y);

            if (expectedColor != actualColor) {
                ERRORF(reporter, "[%d, %d] expected 0x%08x actual 0x%08x\n", x, y,
                        expectedColor, actualColor);
                abort = true;
                break;
            }
        }
    }

    save_bm(actual, "actual");
    save_bm(expected, "expected");
}

#endif
