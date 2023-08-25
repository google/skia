/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/chromium/GrDeferredDisplayList.h"
#include "include/private/chromium/GrDeferredDisplayListRecorder.h"
#include "include/private/chromium/GrPromiseImageTexture.h"
#include "include/private/chromium/GrSurfaceCharacterization.h"
#include "include/private/chromium/SkImageChromium.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDeferredDisplayListPriv.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/gpu/BackendSurfaceFactory.h"
#include "tools/gpu/ManagedBackendTexture.h"
#include "tools/gpu/ProxyUtils.h"

#include <cstddef>
#include <initializer_list>
#include <memory>
#include <utility>

class SkImage;
struct GrContextOptions;

#ifdef SK_GL
#include "include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "include/gpu/gl/GrGLTypes.h"
#include "src/gpu/ganesh/gl/GrGLDefines.h"
#endif

#ifdef SK_VULKAN
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/chromium/GrVkSecondaryCBDrawContext.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "tools/gpu/vk/VkTestHelper.h"
#include <vulkan/vulkan_core.h>
#endif

class SurfaceParameters {
public:
    static const int kNumParams      = 13;
    static const int kFBO0Count      = 9;
    static const int kVkSCBCount     = 12;

    SurfaceParameters(GrRecordingContext* rContext)
            : fBackend(rContext->backend())
            , fCanBeProtected(false)
            , fWidth(64)
            , fHeight(64)
            , fOrigin(kTopLeft_GrSurfaceOrigin)
            , fColorType(kRGBA_8888_SkColorType)
            , fColorSpace(SkColorSpace::MakeSRGB())
            , fSampleCount(1)
            , fSurfaceProps(0x0, kUnknown_SkPixelGeometry)
            , fShouldCreateMipMaps(true)
            , fUsesGLFBO0(false)
            , fIsTextureable(true)
            , fIsProtected(GrProtected::kNo)
            , fVkRTSupportsInputAttachment(false)
            , fForVulkanSecondaryCommandBuffer(false) {
#ifdef SK_VULKAN
        if (rContext->backend() == GrBackendApi::kVulkan) {
            auto vkCaps = static_cast<const GrVkCaps*>(rContext->priv().caps());
            fCanBeProtected = vkCaps->supportsProtectedContent();
            if (fCanBeProtected) {
                fIsProtected = GrProtected::kYes;
            }
        }
#endif
        if (!rContext->priv().caps()->mipmapSupport()) {
            fShouldCreateMipMaps = false;
        }
    }

    int sampleCount() const { return fSampleCount; }

    void setColorType(SkColorType ct) { fColorType = ct; }
    SkColorType colorType() const { return fColorType; }
    void setColorSpace(sk_sp<SkColorSpace> cs) { fColorSpace = std::move(cs); }
    void disableTextureability() {
        fIsTextureable = false;
        fShouldCreateMipMaps = false;
    }
    void setShouldCreateMipMaps(bool shouldCreateMipMaps) {
        fShouldCreateMipMaps = shouldCreateMipMaps;
    }
    void setVkRTInputAttachmentSupport(bool inputSupport) {
        fVkRTSupportsInputAttachment = inputSupport;
    }
    void setForVulkanSecondaryCommandBuffer(bool forVkSCB) {
        fForVulkanSecondaryCommandBuffer = forVkSCB;
    }

    // Modify the SurfaceParameters in just one way. Returns false if the requested modification had
    // no effect.
    bool modify(int i) {
        bool changed = false;
        auto set = [&changed](auto& var, auto value) {
            if (var != value) {
                changed = true;
            }
            var = value;
        };
        switch (i) {
        case 0:
            set(fWidth, 63);
            break;
        case 1:
            set(fHeight, 63);
            break;
        case 2:
            set(fOrigin, kBottomLeft_GrSurfaceOrigin);
            break;
        case 3:
            set(fColorType, kRGBA_F16_SkColorType);
            break;
        case 4:
            // This just needs to be a colorSpace different from that returned by MakeSRGB().
            // In this case we just change the gamut.
            set(fColorSpace, SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                   SkNamedGamut::kAdobeRGB));
            break;
        case 5:
            set(fSampleCount, 4);
            break;
        case 6:
            set(fSurfaceProps, SkSurfaceProps(0x0, kRGB_H_SkPixelGeometry));
            break;
        case 7:
            set(fSurfaceProps, SkSurfaceProps(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
                                              kUnknown_SkPixelGeometry));
            break;
        case 8:
            set(fShouldCreateMipMaps, false);
            break;
        case 9:
            if (GrBackendApi::kOpenGL == fBackend) {
                set(fUsesGLFBO0, true);
                set(fShouldCreateMipMaps, false);  // needs to changed in tandem w/ textureability
                set(fIsTextureable, false);
            }
            break;
        case 10:
            set(fShouldCreateMipMaps, false);  // needs to changed in tandem w/ textureability
            set(fIsTextureable, false);
            break;
        case 11:
            if (fCanBeProtected) {
                set(fIsProtected, GrProtected(!static_cast<bool>(fIsProtected)));
            }
            break;
        case 12:
            if (GrBackendApi::kVulkan == fBackend) {
                set(fForVulkanSecondaryCommandBuffer, true);
                set(fUsesGLFBO0, false);
                set(fShouldCreateMipMaps, false);  // needs to changed in tandem w/ textureability
                set(fIsTextureable, false);
                set(fVkRTSupportsInputAttachment, false);
            }
            break;
        }
        return changed;
    }

    GrSurfaceCharacterization createCharacterization(GrDirectContext* dContext) const {
        size_t maxResourceBytes = dContext->getResourceCacheLimit();

        if (!dContext->colorTypeSupportedAsSurface(fColorType)) {
            return GrSurfaceCharacterization();
        }

        // Note that Ganesh doesn't make use of the SkImageInfo's alphaType
        SkImageInfo ii = SkImageInfo::Make(fWidth, fHeight, fColorType,
                                           kPremul_SkAlphaType, fColorSpace);

        GrBackendFormat backendFormat = dContext->defaultBackendFormat(fColorType,
                                                                       GrRenderable::kYes);
        if (!backendFormat.isValid()) {
            return GrSurfaceCharacterization();
        }

        GrSurfaceCharacterization c = dContext->threadSafeProxy()->createCharacterization(
                                                maxResourceBytes, ii, backendFormat, fSampleCount,
                                                fOrigin, fSurfaceProps, fShouldCreateMipMaps,
                                                fUsesGLFBO0, fIsTextureable, fIsProtected,
                                                fVkRTSupportsInputAttachment,
                                                fForVulkanSecondaryCommandBuffer);
        return c;
    }

    // Create a DDL whose characterization captures the current settings
    sk_sp<GrDeferredDisplayList> createDDL(GrDirectContext* dContext) const {
        GrSurfaceCharacterization c = this->createCharacterization(dContext);
        SkAssertResult(c.isValid());

        GrDeferredDisplayListRecorder r(c);
        SkCanvas* canvas = r.getCanvas();
        if (!canvas) {
            return nullptr;
        }

        canvas->drawRect(SkRect::MakeXYWH(10, 10, 10, 10), SkPaint());
        return r.detach();
    }

    // Create the surface with the current set of parameters
    sk_sp<SkSurface> make(GrDirectContext* dContext) const {
        const GrSurfaceCharacterization c = this->createCharacterization(dContext);

#ifdef SK_GL
        if (fUsesGLFBO0) {
            if (GrBackendApi::kOpenGL != dContext->backend()) {
                return nullptr;
            }

            GrGLFramebufferInfo fboInfo;
            fboInfo.fFBOID = 0;
            fboInfo.fFormat = GR_GL_RGBA8;
            fboInfo.fProtected = skgpu::Protected::kNo;
            static constexpr int kStencilBits = 8;
            GrBackendRenderTarget backendRT =
                    GrBackendRenderTargets::MakeGL(fWidth, fHeight, 1, kStencilBits, fboInfo);

            if (!backendRT.isValid()) {
                return nullptr;
            }

            sk_sp<SkSurface> result = SkSurfaces::WrapBackendRenderTarget(
                    dContext, backendRT, fOrigin, fColorType, fColorSpace, &fSurfaceProps);
            SkASSERT(result->isCompatible(c));
            return result;
        }
#endif

        // We can't make SkSurfaces for vulkan secondary command buffers.
        if (fForVulkanSecondaryCommandBuffer) {
            return nullptr;
        }

        sk_sp<SkSurface> surface;
        if (fIsTextureable) {
            surface = sk_gpu_test::MakeBackendTextureSurface(dContext,
                                                             {fWidth, fHeight},
                                                             fOrigin,
                                                             fSampleCount,
                                                             fColorType,
                                                             fColorSpace,
                                                             GrMipmapped(fShouldCreateMipMaps),
                                                             fIsProtected,
                                                             &fSurfaceProps);
        } else {
            // Create a surface w/ the current parameters but make it non-textureable
            SkASSERT(!fShouldCreateMipMaps);
            surface = sk_gpu_test::MakeBackendRenderTargetSurface(dContext,
                                                                  {fWidth, fHeight},
                                                                  fOrigin,
                                                                  fSampleCount,
                                                                  fColorType,
                                                                  fColorSpace,
                                                                  fIsProtected,
                                                                  &fSurfaceProps);
        }

        if (!surface) {
            SkASSERT(!c.isValid());
            return nullptr;
        }

        GrBackendTexture texture = SkSurfaces::GetBackendTexture(
                surface.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
        if (texture.isValid()) {
            SkASSERT(c.isCompatible(texture));
        }

        SkASSERT(c.isValid());
        SkASSERT(surface->isCompatible(c));
        return surface;
    }

#ifdef SK_VULKAN
    sk_sp<GrVkSecondaryCBDrawContext> makeVkSCB(GrDirectContext* dContext) {
        const GrSurfaceCharacterization c = this->createCharacterization(dContext);
        SkImageInfo imageInfo = SkImageInfo::Make({fWidth, fHeight},
                                                  {fColorType, kPremul_SkAlphaType, fColorSpace});
        GrVkDrawableInfo vkInfo;
        // putting in a bunch of placeholder values here
        vkInfo.fSecondaryCommandBuffer = (VkCommandBuffer)1;
        vkInfo.fColorAttachmentIndex = 0;
        vkInfo.fCompatibleRenderPass = (VkRenderPass)1;
        vkInfo.fFormat = VK_FORMAT_R8G8B8A8_UNORM;
        vkInfo.fDrawBounds = nullptr;

        return GrVkSecondaryCBDrawContext::Make(dContext, imageInfo, vkInfo, &fSurfaceProps);
    }
#endif

private:
    GrBackendApi        fBackend;
    bool                fCanBeProtected;

    int                 fWidth;
    int                 fHeight;
    GrSurfaceOrigin     fOrigin;
    SkColorType         fColorType;
    sk_sp<SkColorSpace> fColorSpace;
    int                 fSampleCount;
    SkSurfaceProps      fSurfaceProps;
    bool                fShouldCreateMipMaps;
    bool                fUsesGLFBO0;
    bool                fIsTextureable;
    GrProtected         fIsProtected;
    bool                fVkRTSupportsInputAttachment;
    bool                fForVulkanSecondaryCommandBuffer;
};

// Test out operator== && operator!=
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DDLOperatorEqTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto context = ctxInfo.directContext();

    for (int i = -1; i < SurfaceParameters::kNumParams; ++i) {
        SurfaceParameters params1(context);
        bool didModify1 = i >= 0 && params1.modify(i);

        GrSurfaceCharacterization char1 = params1.createCharacterization(context);
        if (!char1.isValid()) {
            continue;  // can happen on some platforms (ChromeOS)
        }

        for (int j = -1; j < SurfaceParameters::kNumParams; ++j) {
            SurfaceParameters params2(context);
            bool didModify2 = j >= 0 && params2.modify(j);

            GrSurfaceCharacterization char2 = params2.createCharacterization(context);
            if (!char2.isValid()) {
                continue;  // can happen on some platforms (ChromeOS)
            }

            if (i == j || (!didModify1 && !didModify2)) {
                REPORTER_ASSERT(reporter, char1 == char2);
            } else {
                REPORTER_ASSERT(reporter, char1 != char2);
            }
        }
    }

    {
        SurfaceParameters params(context);

        GrSurfaceCharacterization valid = params.createCharacterization(context);
        SkASSERT(valid.isValid());

        GrSurfaceCharacterization inval1, inval2;
        SkASSERT(!inval1.isValid() && !inval2.isValid());

        REPORTER_ASSERT(reporter, inval1 != inval2);
        REPORTER_ASSERT(reporter, valid != inval1);
        REPORTER_ASSERT(reporter, inval1 != valid);
    }
}

////////////////////////////////////////////////////////////////////////////////
// This tests GrSurfaceCharacterization/SkSurface compatibility
void DDLSurfaceCharacterizationTestImpl(GrDirectContext* dContext, skiatest::Reporter* reporter) {
    // Create a bitmap that we can readback into
    SkImageInfo imageInfo = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.allocPixels(imageInfo);

    sk_sp<GrDeferredDisplayList> ddl;

    // First, create a DDL using the stock SkSurface parameters
    {
        SurfaceParameters params(dContext);
        if (dContext->backend() == GrBackendApi::kVulkan) {
            params.setVkRTInputAttachmentSupport(true);
        }
        ddl = params.createDDL(dContext);
        SkAssertResult(ddl);

        // The DDL should draw into an SkSurface created with the same parameters
        sk_sp<SkSurface> s = params.make(dContext);
        if (!s) {
            return;
        }

        REPORTER_ASSERT(reporter, skgpu::ganesh::DrawDDL(s, ddl));
        s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);

        dContext->flush();
    }

    // Then, alter each parameter in turn and check that the DDL & surface are incompatible
    for (int i = 0; i < SurfaceParameters::kNumParams; ++i) {
        SurfaceParameters params(dContext);
        if (!params.modify(i)) {
            continue;
        }

        sk_sp<SkSurface> s = params.make(dContext);
        if (!s) {
            continue;
        }

        REPORTER_ASSERT(reporter, !skgpu::ganesh::DrawDDL(s, ddl),
                        "DDLSurfaceCharacterizationTest failed on parameter: %d\n", i);
        dContext->flush();
    }

    // Next test the compatibility of resource cache parameters
    {
        const SurfaceParameters params(dContext);

        sk_sp<SkSurface> s = params.make(dContext);

        size_t maxResourceBytes = dContext->getResourceCacheLimit();

        dContext->setResourceCacheLimit(maxResourceBytes/2);
        REPORTER_ASSERT(reporter, !skgpu::ganesh::DrawDDL(s, ddl));

        // DDL TODO: once proxies/ops can be de-instantiated we can re-enable these tests.
        // For now, DDLs are drawn once.
#if 0
        // resource limits >= those at characterization time are accepted
        context->setResourceCacheLimits(2*maxResourceCount, maxResourceBytes);
        REPORTER_ASSERT(reporter, skgpu::ganesh::DrawDDL(s, ddl));
        s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);

        context->setResourceCacheLimits(maxResourceCount, 2*maxResourceBytes);
        REPORTER_ASSERT(reporter, skgpu::ganesh::DrawDDL(s, ddl));
        s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);

        context->setResourceCacheLimits(maxResourceCount, maxResourceBytes);
        REPORTER_ASSERT(reporter, skgpu::ganesh::DrawDDL(s, ddl));
        s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);
#endif

        dContext->flush();
    }

    // Test that the textureability of the DDL characterization can block a DDL draw
    {
        SurfaceParameters params(dContext);
        params.disableTextureability();

        sk_sp<SkSurface> s = params.make(dContext);
        if (s) {
            // bc the DDL was made w/ textureability
            REPORTER_ASSERT(reporter, !skgpu::ganesh::DrawDDL(s, ddl));

            dContext->flush();
        }
    }

    // Make sure non-GPU-backed surfaces fail characterization
    {
        SkImageInfo ii = SkImageInfo::MakeN32(64, 64, kOpaque_SkAlphaType);

        sk_sp<SkSurface> rasterSurface = SkSurfaces::Raster(ii);
        GrSurfaceCharacterization c;
        REPORTER_ASSERT(reporter, !rasterSurface->characterize(&c));
    }

    // Exercise the createResized method
    {
        SurfaceParameters params(dContext);

        sk_sp<SkSurface> s = params.make(dContext);
        if (!s) {
            return;
        }

        GrSurfaceCharacterization char0;
        SkAssertResult(s->characterize(&char0));

        // Too small
        GrSurfaceCharacterization char1 = char0.createResized(-1, -1);
        REPORTER_ASSERT(reporter, !char1.isValid());

        // Too large
        GrSurfaceCharacterization char2 = char0.createResized(1000000, 32);
        REPORTER_ASSERT(reporter, !char2.isValid());

        // Just right
        GrSurfaceCharacterization char3 = char0.createResized(32, 32);
        REPORTER_ASSERT(reporter, char3.isValid());
        REPORTER_ASSERT(reporter, 32 == char3.width());
        REPORTER_ASSERT(reporter, 32 == char3.height());
    }

    // Exercise the createColorSpace method
    {
        SurfaceParameters params(dContext);

        sk_sp<SkSurface> s = params.make(dContext);
        if (!s) {
            return;
        }

        GrSurfaceCharacterization char0;
        SkAssertResult(s->characterize(&char0));

        // The default params create an sRGB color space
        REPORTER_ASSERT(reporter, char0.colorSpace()->isSRGB());
        REPORTER_ASSERT(reporter, !char0.colorSpace()->gammaIsLinear());

        {
            sk_sp<SkColorSpace> newCS = SkColorSpace::MakeSRGBLinear();

            GrSurfaceCharacterization char1 = char0.createColorSpace(std::move(newCS));
            REPORTER_ASSERT(reporter, char1.isValid());
            REPORTER_ASSERT(reporter, !char1.colorSpace()->isSRGB());
            REPORTER_ASSERT(reporter, char1.colorSpace()->gammaIsLinear());
        }

        {
            GrSurfaceCharacterization char2 = char0.createColorSpace(nullptr);
            REPORTER_ASSERT(reporter, char2.isValid());
            REPORTER_ASSERT(reporter, !char2.colorSpace());
        }

        {
            sk_sp<SkColorSpace> newCS = SkColorSpace::MakeSRGBLinear();

            GrSurfaceCharacterization invalid;
            REPORTER_ASSERT(reporter, !invalid.isValid());
            GrSurfaceCharacterization stillInvalid = invalid.createColorSpace(std::move(newCS));
            REPORTER_ASSERT(reporter, !stillInvalid.isValid());
        }
    }

    // Exercise the createBackendFormat method
    {
        SurfaceParameters params(dContext);

        sk_sp<SkSurface> s = params.make(dContext);
        if (!s) {
            return;
        }

        GrSurfaceCharacterization char0;
        SkAssertResult(s->characterize(&char0));

        // The default params create a renderable RGBA8 surface
        auto originalBackendFormat = dContext->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                                    GrRenderable::kYes);
        REPORTER_ASSERT(reporter, originalBackendFormat.isValid());
        REPORTER_ASSERT(reporter, char0.backendFormat() == originalBackendFormat);

        auto newBackendFormat = dContext->defaultBackendFormat(kRGB_565_SkColorType,
                                                               GrRenderable::kYes);

        if (newBackendFormat.isValid()) {
            GrSurfaceCharacterization char1 = char0.createBackendFormat(kRGB_565_SkColorType,
                                                                        newBackendFormat);
            REPORTER_ASSERT(reporter, char1.isValid());
            REPORTER_ASSERT(reporter, char1.backendFormat() == newBackendFormat);

            GrSurfaceCharacterization invalid;
            REPORTER_ASSERT(reporter, !invalid.isValid());
            auto stillInvalid = invalid.createBackendFormat(kRGB_565_SkColorType,
                                                            newBackendFormat);
            REPORTER_ASSERT(reporter, !stillInvalid.isValid());
        }
    }

    // Exercise the createFBO0 method
    if (dContext->backend() == GrBackendApi::kOpenGL) {
        SurfaceParameters params(dContext);
        // If the original characterization is textureable then we will fail trying to make an
        // FBO0 characterization
        params.disableTextureability();

        sk_sp<SkSurface> s = params.make(dContext);
        if (!s) {
            return;
        }

        GrSurfaceCharacterization char0;
        SkAssertResult(s->characterize(&char0));

        // The default params create a non-FBO0 surface
        REPORTER_ASSERT(reporter, !char0.usesGLFBO0());

        {
            GrSurfaceCharacterization char1 = char0.createFBO0(true);
            REPORTER_ASSERT(reporter, char1.isValid());
            REPORTER_ASSERT(reporter, char1.usesGLFBO0());
        }

        {
            GrSurfaceCharacterization invalid;
            REPORTER_ASSERT(reporter, !invalid.isValid());
            GrSurfaceCharacterization stillInvalid = invalid.createFBO0(true);
            REPORTER_ASSERT(reporter, !stillInvalid.isValid());
        }
    }
}

#ifdef SK_GL

// Test out the surface compatibility checks regarding FBO0-ness. This test constructs
// two parallel arrays of characterizations and surfaces in the order:
//    FBO0 w/ MSAA, FBO0 w/o MSAA, not-FBO0 w/ MSAA, not-FBO0 w/o MSAA
// and then tries all sixteen combinations to check the expected compatibility.
// Note: this is a GL-only test
DEF_GANESH_TEST_FOR_GL_CONTEXT(CharacterizationFBO0nessTest,
                               reporter,
                               ctxInfo,
                               CtsEnforcement::kApiLevel_T) {
    auto context = ctxInfo.directContext();
    const GrCaps* caps = context->priv().caps();
    sk_sp<GrContextThreadSafeProxy> proxy = context->threadSafeProxy();
    const size_t resourceCacheLimit = context->getResourceCacheLimit();

    GrBackendFormat format = GrBackendFormats::MakeGL(GR_GL_RGBA8, GR_GL_TEXTURE_2D);

    int availableSamples = caps->getRenderTargetSampleCount(4, format);
    if (availableSamples <= 1) {
        // This context doesn't support MSAA for RGBA8
        return;
    }

    SkImageInfo ii = SkImageInfo::Make({ 128, 128 }, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    static constexpr int kStencilBits = 8;
    static constexpr bool kNotMipMapped = false;
    static constexpr bool kNotTextureable = false;
    const SkSurfaceProps surfaceProps(0x0, kRGB_H_SkPixelGeometry);

    // Rows are characterizations and columns are surfaces
    static const bool kExpectedCompatibility[4][4] = {
                    //  FBO0 & MSAA, FBO0 & not-MSAA, not-FBO0 & MSAA, not-FBO0 & not-MSAA
/* FBO0 & MSAA     */ { true,        false,           false,           false },
/* FBO0 & not-MSAA */ { false,       true,            false,           true  },
/* not-FBO0 & MSAA */ { false,       false,           true,            false },
/* not-FBO0 & not- */ { false,       false,           false,           true  }
    };

    GrSurfaceCharacterization characterizations[4];
    sk_sp<SkSurface> surfaces[4];

    int index = 0;
    for (bool isFBO0 : { true, false }) {
        for (int numSamples : { availableSamples, 1 }) {
            characterizations[index] = proxy->createCharacterization(resourceCacheLimit,
                                                                     ii, format, numSamples,
                                                                     kTopLeft_GrSurfaceOrigin,
                                                                     surfaceProps, kNotMipMapped,
                                                                     isFBO0, kNotTextureable);
            SkASSERT(characterizations[index].sampleCount() == numSamples);
            SkASSERT(characterizations[index].usesGLFBO0() == isFBO0);

            GrGLFramebufferInfo fboInfo{ isFBO0 ? 0 : (GrGLuint) 1, GR_GL_RGBA8 };
            GrBackendRenderTarget backendRT =
                    GrBackendRenderTargets::MakeGL(128, 128, numSamples, kStencilBits, fboInfo);
            SkAssertResult(backendRT.isValid());

            surfaces[index] = SkSurfaces::WrapBackendRenderTarget(context,
                                                                  backendRT,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  kRGBA_8888_SkColorType,
                                                                  nullptr,
                                                                  &surfaceProps);
            ++index;
        }
    }

    for (int c = 0; c < 4; ++c) {
        for (int s = 0; s < 4; ++s) {
            REPORTER_ASSERT(reporter,
                            kExpectedCompatibility[c][s] ==
                                                 surfaces[s]->isCompatible(characterizations[c]));
        }
    }
}
#endif

#ifdef SK_VULKAN
DEF_GANESH_TEST_FOR_VULKAN_CONTEXT(CharacterizationVkSCBnessTest,
                                   reporter,
                                   ctxInfo,
                                   CtsEnforcement::kApiLevel_T) {
    auto dContext = ctxInfo.directContext();

    SurfaceParameters params(dContext);
    params.modify(SurfaceParameters::kVkSCBCount);
    GrSurfaceCharacterization characterization = params.createCharacterization(dContext);
    REPORTER_ASSERT(reporter, characterization.isValid());

    sk_sp<GrDeferredDisplayList> ddl = params.createDDL(dContext);
    REPORTER_ASSERT(reporter, ddl.get());

    sk_sp<GrVkSecondaryCBDrawContext> scbDrawContext = params.makeVkSCB(dContext);
    REPORTER_ASSERT(reporter, scbDrawContext->isCompatible(characterization));

    scbDrawContext->releaseResources();
}
#endif

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DDLSurfaceCharacterizationTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto context = ctxInfo.directContext();

    DDLSurfaceCharacterizationTestImpl(context, reporter);
}

#if defined(SK_VULKAN)
DEF_GANESH_TEST(VkProtectedContext_DDLSurfaceCharacterizationTest,
                reporter,
                ctxInfo,
                CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    DDLSurfaceCharacterizationTestImpl(protectedTestHelper->directContext(), reporter);
}
#endif

// Test that a DDL created w/o textureability can be replayed into both a textureable and
// non-textureable destination. Note that DDLSurfaceCharacterizationTest tests that a
// textureable DDL cannot be played into a non-textureable destination but can be replayed
// into a textureable destination.
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DDLNonTextureabilityTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto context = ctxInfo.directContext();

    // Create a bitmap that we can readback into
    SkImageInfo imageInfo = SkImageInfo::Make(64, 64, kRGBA_8888_SkColorType,
                                              kPremul_SkAlphaType);
    SkBitmap bitmap;
    bitmap.allocPixels(imageInfo);

    for (bool textureability : { true, false }) {
        sk_sp<GrDeferredDisplayList> ddl;

        // First, create a DDL w/o textureability (and thus no mipmaps). TODO: once we have
        // reusable DDLs, move this outside of the loop.
        {
            SurfaceParameters params(context);
            params.disableTextureability();
            if (context->backend() == GrBackendApi::kVulkan) {
                params.setVkRTInputAttachmentSupport(true);
            }

            ddl = params.createDDL(context);
            SkAssertResult(ddl);
        }

        // Then verify it can draw into either flavor of destination
        SurfaceParameters params(context);
        if (!textureability) {
            params.disableTextureability();
        }
        if (context->backend() == GrBackendApi::kVulkan) {
            params.setVkRTInputAttachmentSupport(true);
        }

        sk_sp<SkSurface> s = params.make(context);
        if (!s) {
            continue;
        }

        REPORTER_ASSERT(reporter, skgpu::ganesh::DrawDDL(s, ddl));
        s->readPixels(imageInfo, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);

        context->flush();
    }
}

static void test_make_render_target(skiatest::Reporter* reporter,
                                    GrDirectContext* dContext,
                                    const SurfaceParameters& params) {
    {
        const GrSurfaceCharacterization c = params.createCharacterization(dContext);

        if (!c.isValid()) {
            sk_sp<SkSurface> tmp = params.make(dContext);
            // If we couldn't characterize the surface we shouldn't be able to create it either
            REPORTER_ASSERT(reporter, !tmp);
            return;
        }
    }

    const GrSurfaceCharacterization c = params.createCharacterization(dContext);
    {
        sk_sp<SkSurface> s = params.make(dContext);
        REPORTER_ASSERT(reporter, s);
        if (!s) {
            REPORTER_ASSERT(reporter, !c.isValid());
            return;
        }

        REPORTER_ASSERT(reporter, c.isValid());
        GrBackendTexture backend =
                SkSurfaces::GetBackendTexture(s.get(), SkSurfaces::BackendHandleAccess::kFlushRead);
        if (backend.isValid()) {
            REPORTER_ASSERT(reporter, c.isCompatible(backend));
        }
        REPORTER_ASSERT(reporter, s->isCompatible(c));
        // Note that we're leaving 'backend' live here
    }

    // Make an SkSurface from scratch
    {
        sk_sp<SkSurface> s = SkSurfaces::RenderTarget(dContext, c, skgpu::Budgeted::kYes);
        REPORTER_ASSERT(reporter, s);
        REPORTER_ASSERT(reporter, s->isCompatible(c));
    }
}

////////////////////////////////////////////////////////////////////////////////
// This tests the SkSurfaces::RenderTarget variants that take a GrSurfaceCharacterization.
// In particular, the SkSurface, backendTexture and GrSurfaceCharacterization
// should always be compatible.
void DDLMakeRenderTargetTestImpl(GrDirectContext* dContext, skiatest::Reporter* reporter) {
    for (int i = -1; i < SurfaceParameters::kNumParams; ++i) {
        if (i == SurfaceParameters::kFBO0Count || i == SurfaceParameters::kVkSCBCount) {
            // MakeRenderTarget doesn't support FBO0 or vulkan secondary command buffers
            continue;
        }

        SurfaceParameters params(dContext);
        if (i >= 0 && !params.modify(i)) {
            continue;
        }

        test_make_render_target(reporter, dContext, params);
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DDLMakeRenderTargetTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto context = ctxInfo.directContext();

    DDLMakeRenderTargetTestImpl(context, reporter);
}

#if defined(SK_VULKAN)

DEF_GANESH_TEST(VkProtectedContext_DDLMakeRenderTargetTest,
                reporter,
                ctxInfo,
                CtsEnforcement::kNever) {
    auto protectedTestHelper = std::make_unique<VkTestHelper>(true);
    if (!protectedTestHelper->init()) {
        return;
    }
    REPORTER_ASSERT(reporter, protectedTestHelper->directContext() != nullptr);

    DDLMakeRenderTargetTestImpl(protectedTestHelper->directContext(), reporter);
}
#endif

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
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DDLWrapBackendTest,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeWithoutData(dContext,
                                                                    kSize,
                                                                    kSize,
                                                                    kRGBA_8888_SkColorType,
                                                                    GrMipmapped::kNo,
                                                                    GrRenderable::kNo,
                                                                    GrProtected::kNo);
    if (!mbet) {
        return;
    }

    SurfaceParameters params(dContext);

    sk_sp<SkSurface> s = params.make(dContext);
    if (!s) {
        return;
    }

    GrSurfaceCharacterization c;
    SkAssertResult(s->characterize(&c));

    GrDeferredDisplayListRecorder recorder(c);

    SkCanvas* canvas = recorder.getCanvas();
    SkASSERT(canvas);

    auto rContext = canvas->recordingContext();
    if (!rContext) {
        return;
    }

    // Wrapped Backend Textures are not supported in DDL
    TextureReleaseChecker releaseChecker;
    sk_sp<SkImage> image = SkImages::BorrowTextureFrom(
            rContext,
            mbet->texture(),
            kTopLeft_GrSurfaceOrigin,
            kRGBA_8888_SkColorType,
            kPremul_SkAlphaType,
            nullptr,
            sk_gpu_test::ManagedBackendTexture::ReleaseProc,
            mbet->releaseContext(TextureReleaseChecker::Release, &releaseChecker));
    REPORTER_ASSERT(reporter, !image);
}

////////////////////////////////////////////////////////////////////////////////
// Test out the behavior of an invalid DDLRecorder
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DDLInvalidRecorder,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();

    {
        SkImageInfo ii = SkImageInfo::MakeN32Premul(32, 32);
        sk_sp<SkSurface> s = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, ii);

        GrSurfaceCharacterization characterization;
        SkAssertResult(s->characterize(&characterization));

        // never calling getCanvas means the backing surface is never allocated
        GrDeferredDisplayListRecorder recorder(characterization);
    }

    {
        GrSurfaceCharacterization invalid;

        GrDeferredDisplayListRecorder recorder(invalid);

        const GrSurfaceCharacterization c = recorder.characterization();
        REPORTER_ASSERT(reporter, !c.isValid());
        REPORTER_ASSERT(reporter, !recorder.getCanvas());
        REPORTER_ASSERT(reporter, !recorder.detach());
    }
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DDLCreateCharacterizationFailures,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto dContext = ctxInfo.directContext();
    size_t maxResourceBytes = dContext->getResourceCacheLimit();
    auto proxy = dContext->threadSafeProxy().get();

    auto check_create_fails =
            [proxy, reporter, maxResourceBytes](const GrBackendFormat& backendFormat,
                                                int width, int height,
                                                SkColorType ct, bool willUseGLFBO0,
                                                bool isTextureable,
                                                GrProtected prot,
                                                bool vkRTSupportsInputAttachment,
                                                bool forVulkanSecondaryCommandBuffer) {
        const SkSurfaceProps surfaceProps(0x0, kRGB_H_SkPixelGeometry);

        SkImageInfo ii = SkImageInfo::Make(width, height, ct,
                                           kPremul_SkAlphaType, nullptr);

        GrSurfaceCharacterization c = proxy->createCharacterization(
                                                maxResourceBytes, ii, backendFormat, 1,
                                                kBottomLeft_GrSurfaceOrigin, surfaceProps, false,
                                                willUseGLFBO0, isTextureable, prot,
                                                vkRTSupportsInputAttachment,
                                                forVulkanSecondaryCommandBuffer);
        REPORTER_ASSERT(reporter, !c.isValid());
    };

    GrBackendFormat goodBackendFormat = dContext->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                                       GrRenderable::kYes);
    SkASSERT(goodBackendFormat.isValid());

    GrBackendFormat badBackendFormat;
    SkASSERT(!badBackendFormat.isValid());

    SkColorType kGoodCT = kRGBA_8888_SkColorType;
    SkColorType kBadCT = kUnknown_SkColorType;

    static const bool kIsTextureable = true;
    static const bool kIsNotTextureable = false;

    static const bool kGoodUseFBO0 = false;
    static const bool kBadUseFBO0 = true;

    static const bool kGoodVkInputAttachment = false;
    static const bool kBadVkInputAttachment = true;

    static const bool kGoodForVkSCB = false;
    static const bool kBadForVkSCB = true;

    int goodWidth = 64;
    int goodHeight = 64;
    int badWidths[] = { 0, 1048576 };
    int badHeights[] = { 0, 1048576 };


    // In each of the check_create_fails calls there is one bad parameter that should cause the
    // creation of the characterization to fail.
    check_create_fails(goodBackendFormat, goodWidth, badHeights[0], kGoodCT, kGoodUseFBO0,
                       kIsTextureable, GrProtected::kNo, kGoodVkInputAttachment, kGoodForVkSCB);
    check_create_fails(goodBackendFormat, goodWidth, badHeights[1], kGoodCT, kGoodUseFBO0,
                       kIsTextureable, GrProtected::kNo, kGoodVkInputAttachment, kGoodForVkSCB);
    check_create_fails(goodBackendFormat, badWidths[0], goodHeight, kGoodCT, kGoodUseFBO0,
                       kIsTextureable, GrProtected::kNo, kGoodVkInputAttachment, kGoodForVkSCB);
    check_create_fails(goodBackendFormat, badWidths[1], goodHeight, kGoodCT, kGoodUseFBO0,
                       kIsTextureable, GrProtected::kNo, kGoodVkInputAttachment, kGoodForVkSCB);
    check_create_fails(badBackendFormat, goodWidth, goodHeight, kGoodCT, kGoodUseFBO0,
                       kIsTextureable, GrProtected::kNo, kGoodVkInputAttachment, kGoodForVkSCB);
    check_create_fails(goodBackendFormat, goodWidth, goodHeight, kBadCT, kGoodUseFBO0,
                       kIsTextureable, GrProtected::kNo, kGoodVkInputAttachment, kGoodForVkSCB);
    // This fails because we always try to make a characterization that is textureable and we can't
    // have UseFBO0 be true and textureable.
    check_create_fails(goodBackendFormat, goodWidth, goodHeight, kGoodCT, kBadUseFBO0,
                       kIsTextureable, GrProtected::kNo, kGoodVkInputAttachment, kGoodForVkSCB);
    if (dContext->backend() == GrBackendApi::kVulkan) {
        // The bad parameter in this case is the GrProtected::kYes since none of our test contexts
        // are made protected we can't have a protected surface.
        check_create_fails(goodBackendFormat, goodWidth, goodHeight, kGoodCT, kGoodUseFBO0,
                           kIsTextureable, GrProtected::kYes, kGoodVkInputAttachment,
                           kGoodForVkSCB);
        // The following fails because forVulkanSecondaryCommandBuffer is true and
        // isTextureable is true. This is not a legal combination.
        check_create_fails(goodBackendFormat, goodWidth, goodHeight, kGoodCT, kGoodUseFBO0,
                           kIsTextureable, GrProtected::kNo, kGoodVkInputAttachment, kBadForVkSCB);
        // The following fails because forVulkanSecondaryCommandBuffer is true and
        // vkRTSupportsInputAttachment is true. This is not a legal combination.
        check_create_fails(goodBackendFormat, goodWidth, goodHeight, kGoodCT, kGoodUseFBO0,
                           kIsNotTextureable, GrProtected::kNo, kBadVkInputAttachment,
                           kBadForVkSCB);
        // The following fails because forVulkanSecondaryCommandBuffer is true and
        // willUseGLFBO0 is true. This is not a legal combination.
        check_create_fails(goodBackendFormat, goodWidth, goodHeight, kGoodCT, kBadUseFBO0,
                           kIsNotTextureable, GrProtected::kNo, kGoodVkInputAttachment,
                           kBadForVkSCB);
    } else {
        // The following set vulkan only flags on non vulkan backends.
        check_create_fails(goodBackendFormat, goodWidth, goodHeight, kGoodCT, kGoodUseFBO0,
                           kIsTextureable, GrProtected::kNo, kBadVkInputAttachment, kGoodForVkSCB);
        check_create_fails(goodBackendFormat, goodWidth, goodHeight, kGoodCT, kGoodUseFBO0,
                           kIsNotTextureable, GrProtected::kNo, kGoodVkInputAttachment,
                           kBadForVkSCB);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Test that flushing a DDL via SkSurface::flush works

struct FulfillInfo {
    sk_sp<GrPromiseImageTexture> fTex;
    bool fFulfilled = false;
    bool fReleased  = false;
};

static sk_sp<GrPromiseImageTexture> tracking_fulfill_proc(void* context) {
    FulfillInfo* info = (FulfillInfo*) context;
    info->fFulfilled = true;
    return info->fTex;
}

static void tracking_release_proc(void* context) {
    FulfillInfo* info = (FulfillInfo*) context;
    info->fReleased = true;
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DDLSkSurfaceFlush,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    auto context = ctxInfo.directContext();

    SkImageInfo ii = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    sk_sp<SkSurface> s = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kNo, ii);

    GrSurfaceCharacterization characterization;
    SkAssertResult(s->characterize(&characterization));

    auto mbet = sk_gpu_test::ManagedBackendTexture::MakeFromInfo(context, ii);
    if (!mbet) {
        ERRORF(reporter, "Could not make texture.");
        return;
    }

    FulfillInfo fulfillInfo;
    fulfillInfo.fTex = GrPromiseImageTexture::Make(mbet->texture());

    sk_sp<GrDeferredDisplayList> ddl;

    {
        GrDeferredDisplayListRecorder recorder(characterization);

        GrBackendFormat format = context->defaultBackendFormat(kRGBA_8888_SkColorType,
                                                               GrRenderable::kNo);
        SkASSERT(format.isValid());

        SkCanvas* canvas = recorder.getCanvas();

        sk_sp<SkImage> promiseImage =
                SkImages::PromiseTextureFrom(canvas->recordingContext()->threadSafeProxy(),
                                             format,
                                             SkISize::Make(32, 32),
                                             GrMipmapped::kNo,
                                             kTopLeft_GrSurfaceOrigin,
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType,
                                             nullptr,
                                             tracking_fulfill_proc,
                                             tracking_release_proc,
                                             &fulfillInfo);

        canvas->clear(SK_ColorRED);
        canvas->drawImage(promiseImage, 0, 0);
        ddl = recorder.detach();
    }

    context->flushAndSubmit();

    REPORTER_ASSERT(reporter, skgpu::ganesh::DrawDDL(s, ddl));

    GrFlushInfo flushInfo;
    context->flush(s, SkSurfaces::BackendSurfaceAccess::kPresent, flushInfo);
    context->submit();

    REPORTER_ASSERT(reporter, fulfillInfo.fFulfilled);

    // In order to receive the done callback with the low-level APIs we need to re-flush
    context->flush(s);
    context->submit(true);

    REPORTER_ASSERT(reporter, fulfillInfo.fReleased);

    REPORTER_ASSERT(reporter, fulfillInfo.fTex->unique());
    fulfillInfo.fTex.reset();
}

////////////////////////////////////////////////////////////////////////////////
// Ensure that reusing a single DDLRecorder to create multiple DDLs works cleanly
DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(DDLMultipleDDLs, reporter, ctxInfo, CtsEnforcement::kNever) {
    auto context = ctxInfo.directContext();

    SkImageInfo ii = SkImageInfo::MakeN32Premul(32, 32);
    sk_sp<SkSurface> s = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kNo, ii);

    SkBitmap bitmap;
    bitmap.allocPixels(ii);

    GrSurfaceCharacterization characterization;
    SkAssertResult(s->characterize(&characterization));

    GrDeferredDisplayListRecorder recorder(characterization);

    SkCanvas* canvas1 = recorder.getCanvas();

    canvas1->clear(SK_ColorRED);

    canvas1->save();
    canvas1->clipRect(SkRect::MakeXYWH(8, 8, 16, 16));

    sk_sp<GrDeferredDisplayList> ddl1 = recorder.detach();

    SkCanvas* canvas2 = recorder.getCanvas();

    SkPaint p;
    p.setColor(SK_ColorGREEN);
    canvas2->drawRect(SkRect::MakeWH(32, 32), p);

    sk_sp<GrDeferredDisplayList> ddl2 = recorder.detach();

    REPORTER_ASSERT(reporter, ddl1->priv().lazyProxyData());
    REPORTER_ASSERT(reporter, ddl2->priv().lazyProxyData());

    // The lazy proxy data being different ensures that the SkSurface, SkCanvas and backing-
    // lazy proxy are all different between the two DDLs
    REPORTER_ASSERT(reporter, ddl1->priv().lazyProxyData() != ddl2->priv().lazyProxyData());

    skgpu::ganesh::DrawDDL(s, ddl1);
    skgpu::ganesh::DrawDDL(s, ddl2);

    // Make sure the clipRect from DDL1 didn't percolate into DDL2
    s->readPixels(ii, bitmap.getPixels(), bitmap.rowBytes(), 0, 0);
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 32; ++x) {
            REPORTER_ASSERT(reporter, bitmap.getColor(x, y) == SK_ColorGREEN);
            if (bitmap.getColor(x, y) != SK_ColorGREEN) {
                return; // we only really need to report the error once
            }
        }
    }
}

#ifdef SK_GL

static sk_sp<GrPromiseImageTexture> noop_fulfill_proc(void*) {
    SkASSERT(0);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Check that the texture-specific flags (i.e., for external & rectangle textures) work
// for promise images. As such, this is a GL-only test.
DEF_GANESH_TEST_FOR_GL_CONTEXT(DDLTextureFlagsTest, reporter, ctxInfo, CtsEnforcement::kNever) {
    auto context = ctxInfo.directContext();

    SkImageInfo ii = SkImageInfo::MakeN32Premul(32, 32);
    sk_sp<SkSurface> s = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kNo, ii);

    GrSurfaceCharacterization characterization;
    SkAssertResult(s->characterize(&characterization));

    GrDeferredDisplayListRecorder recorder(characterization);

    for (GrGLenum target : { GR_GL_TEXTURE_EXTERNAL, GR_GL_TEXTURE_RECTANGLE, GR_GL_TEXTURE_2D } ) {
        for (auto mipmapped : { GrMipmapped::kNo, GrMipmapped::kYes }) {
            GrBackendFormat format = GrBackendFormats::MakeGL(GR_GL_RGBA8, target);

            sk_sp<SkImage> image = SkImages::PromiseTextureFrom(
                    recorder.getCanvas()->recordingContext()->threadSafeProxy(),
                    format,
                    SkISize::Make(32, 32),
                    mipmapped,
                    kTopLeft_GrSurfaceOrigin,
                    kRGBA_8888_SkColorType,
                    kPremul_SkAlphaType,
                    /*color space*/ nullptr,
                    noop_fulfill_proc,
                    /*release proc*/ nullptr,
                    /*context*/ nullptr);
            if (GR_GL_TEXTURE_2D != target && mipmapped == GrMipmapped::kYes) {
                REPORTER_ASSERT(reporter, !image);
                continue;
            }
            if (!context->priv().caps()->isFormatTexturable(format, format.textureType())) {
                REPORTER_ASSERT(reporter, !image);
                continue;
            }
            REPORTER_ASSERT(reporter, image);

            GrTextureProxy* backingProxy = sk_gpu_test::GetTextureImageProxy(image.get(), context);

            REPORTER_ASSERT(reporter, backingProxy->mipmapped() == mipmapped);
            if (GR_GL_TEXTURE_2D == target) {
                REPORTER_ASSERT(reporter, !backingProxy->hasRestrictedSampling());
            } else {
                REPORTER_ASSERT(reporter, backingProxy->hasRestrictedSampling());
            }
        }
    }
}
#endif  // SK_GL

////////////////////////////////////////////////////////////////////////////////
// Test colorType and pixelConfig compatibility.
DEF_GANESH_TEST_FOR_GL_CONTEXT(DDLCompatibilityTest, reporter, ctxInfo, CtsEnforcement::kNever) {
    auto context = ctxInfo.directContext();

    for (int ct = 0; ct <= kLastEnum_SkColorType; ++ct) {
        SkColorType colorType = static_cast<SkColorType>(ct);

        SurfaceParameters params(context);
        params.setColorType(colorType);
        params.setColorSpace(nullptr);

        test_make_render_target(reporter, context, params);
    }
}
