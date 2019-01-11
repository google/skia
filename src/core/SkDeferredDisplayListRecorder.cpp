/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDeferredDisplayListRecorder.h"
#include "SkMessageBus.h"
#include "SkDeferredDisplayList.h"
#include "SkSurface.h"
#include "SkSurfaceCharacterization.h"

#if !SK_SUPPORT_GPU
SkDeferredDisplayListRecorder::SkDeferredDisplayListRecorder(const SkSurfaceCharacterization&) {}

SkDeferredDisplayListRecorder::~SkDeferredDisplayListRecorder() {}

bool SkDeferredDisplayListRecorder::init() { return false; }

SkCanvas* SkDeferredDisplayListRecorder::getCanvas() { return nullptr; }

std::unique_ptr<SkDeferredDisplayList> SkDeferredDisplayListRecorder::detach() { return nullptr; }

sk_sp<SkImage> SkDeferredDisplayListRecorder::makePromiseTexture(
        const GrBackendFormat& backendFormat,
        int width,
        int height,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        SkColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        PromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureDoneProc textureDoneProc,
        PromiseImageTextureContext textureContext) {
    return nullptr;
}

sk_sp<SkImage> SkDeferredDisplayListRecorder::makePromiseTexture(
        const GrBackendFormat& backendFormat,
        int width,
        int height,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        SkColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        LegacyPromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureDoneProc textureDoneProc,
        PromiseImageTextureContext textureContext) {
    return nullptr;
}

sk_sp<SkImage> SkDeferredDisplayListRecorder::makeYUVAPromiseTexture(
        SkYUVColorSpace yuvColorSpace,
        const GrBackendFormat yuvaFormats[],
        const SkISize yuvaSizes[],
        const SkYUVAIndex yuvaIndices[4],
        int imageWidth,
        int imageHeight,
        GrSurfaceOrigin imageOrigin,
        sk_sp<SkColorSpace> imageColorSpace,
        PromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureDoneProc textureDoneProc,
        PromiseImageTextureContext textureContexts[]) {
    return nullptr;
}

sk_sp<SkImage> SkDeferredDisplayListRecorder::makeYUVAPromiseTexture(
        SkYUVColorSpace yuvColorSpace,
        const GrBackendFormat yuvaFormats[],
        const SkISize yuvaSizes[],
        const SkYUVAIndex yuvaIndices[4],
        int imageWidth,
        int imageHeight,
        GrSurfaceOrigin imageOrigin,
        sk_sp<SkColorSpace> imageColorSpace,
        LegacyPromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureDoneProc textureDoneProc,
        PromiseImageTextureContext textureContexts[]) {
    return nullptr;
}

#else

#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "SkGr.h"
#include "SkImage_Gpu.h"
#include "SkImage_GpuYUVA.h"
#include "SkMakeUnique.h"
#include "SkPromiseImageTexture.h"
#include "SkSurface_Gpu.h"
#include "SkYUVASizeInfo.h"

SkDeferredDisplayListRecorder::SkDeferredDisplayListRecorder(const SkSurfaceCharacterization& c)
        : fCharacterization(c) {
    if (fCharacterization.isValid()) {
        fContext = GrContextPriv::MakeDDL(fCharacterization.refContextInfo());
    }
}

SkDeferredDisplayListRecorder::~SkDeferredDisplayListRecorder() {
    if (fContext) {
        auto proxyProvider = fContext->contextPriv().proxyProvider();

        // This allows the uniquely keyed proxies to keep their keys but removes their back
        // pointer to the about-to-be-deleted proxy provider. The proxies will use their
        // unique key to reattach to cached versions of themselves or to appropriately tag new
        // resources (if a cached version was not found). This system operates independent of
        // the replaying context's proxy provider (i.e., these uniquely keyed proxies will not
        // appear in the replaying proxy providers uniquely keyed proxy map). This should be fine
        // since no one else should be trying to reconnect to the orphaned proxies and orphaned
        // proxies from different DDLs that share the same key should simply reconnect to the
        // same cached resource.
        proxyProvider->orphanAllUniqueKeys();
    }
}


bool SkDeferredDisplayListRecorder::init() {
    SkASSERT(fContext);
    SkASSERT(!fLazyProxyData);
    SkASSERT(!fSurface);

    if (!fCharacterization.isValid()) {
        return false;
    }

    fLazyProxyData = sk_sp<SkDeferredDisplayList::LazyProxyData>(
                                                    new SkDeferredDisplayList::LazyProxyData);

    auto proxyProvider = fContext->contextPriv().proxyProvider();

    bool usesGLFBO0 = fCharacterization.usesGLFBO0();
    if (usesGLFBO0) {
        if (GrBackendApi::kOpenGL != fContext->contextPriv().getBackend() ||
            fCharacterization.isTextureable()) {
            return false;
        }
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = fCharacterization.width();
    desc.fHeight = fCharacterization.height();
    desc.fConfig = fCharacterization.config();
    desc.fSampleCnt = fCharacterization.stencilCount();

    sk_sp<SkDeferredDisplayList::LazyProxyData> lazyProxyData = fLazyProxyData;

    // What we're doing here is we're creating a lazy proxy to back the SkSurface. The lazy
    // proxy, when instantiated, will use the GrRenderTarget that backs the SkSurface that the
    // DDL is being replayed into.

    GrInternalSurfaceFlags surfaceFlags = GrInternalSurfaceFlags::kNone;
    if (fContext->contextPriv().caps()->usesMixedSamples() && desc.fSampleCnt > 1 && !usesGLFBO0) {
        // In GL, FBO 0 never supports mixed samples
        surfaceFlags |= GrInternalSurfaceFlags::kMixedSampled;
    }
    if (usesGLFBO0) {
        surfaceFlags |= GrInternalSurfaceFlags::kGLRTFBOIDIs0;
    }
    static constexpr GrProxyProvider::TextureInfo kTextureInfo{GrMipMapped::kNo,
                                                               GrTextureType::k2D};
    const GrProxyProvider::TextureInfo* optionalTextureInfo = nullptr;
    if (fCharacterization.isTextureable()) {
        optionalTextureInfo = &kTextureInfo;
    }

    const GrBackendFormat format = fContext->contextPriv().caps()->getBackendFormatFromColorType(
            fCharacterization.colorType());

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [lazyProxyData](GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    return sk_sp<GrSurface>();
                }

                // The proxy backing the destination surface had better have been instantiated
                // prior to the proxy backing the DLL's surface. Steal its GrRenderTarget.
                SkASSERT(lazyProxyData->fReplayDest->peekSurface());
                return sk_ref_sp<GrSurface>(lazyProxyData->fReplayDest->peekSurface());
            },
            format,
            desc,
            fCharacterization.origin(),
            surfaceFlags,
            optionalTextureInfo,
            SkBackingFit::kExact,
            SkBudgeted::kYes);

    sk_sp<GrSurfaceContext> c = fContext->contextPriv().makeWrappedSurfaceContext(
                                                                 std::move(proxy),
                                                                 fCharacterization.refColorSpace(),
                                                                 &fCharacterization.surfaceProps());
    fSurface = SkSurface_Gpu::MakeWrappedRenderTarget(fContext.get(),
                                                      sk_ref_sp(c->asRenderTargetContext()));
    return SkToBool(fSurface.get());
}

SkCanvas* SkDeferredDisplayListRecorder::getCanvas() {
    if (!fContext) {
        return nullptr;
    }

    if (!fSurface && !this->init()) {
        return nullptr;
    }

    return fSurface->getCanvas();
}

std::unique_ptr<SkDeferredDisplayList> SkDeferredDisplayListRecorder::detach() {
    if (!fContext) {
        return nullptr;
    }

    if (fSurface) {
        SkCanvas* canvas = fSurface->getCanvas();

        canvas->restoreToCount(0);
    }

    auto ddl = std::unique_ptr<SkDeferredDisplayList>(
                           new SkDeferredDisplayList(fCharacterization, std::move(fLazyProxyData)));

    fContext->contextPriv().moveOpListsToDDL(ddl.get());

    // We want a new lazy proxy target for each recorded DDL so force the (lazy proxy-backed)
    // SkSurface to be regenerated for each DDL.
    fSurface = nullptr;
    return ddl;
}

sk_sp<SkImage> SkDeferredDisplayListRecorder::makePromiseTexture(
        const GrBackendFormat& backendFormat,
        int width,
        int height,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        SkColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        PromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureDoneProc textureDoneProc,
        PromiseImageTextureContext textureContext) {
    if (!fContext) {
        return nullptr;
    }

    return SkImage_Gpu::MakePromiseTexture(fContext.get(),
                                           backendFormat,
                                           width,
                                           height,
                                           mipMapped,
                                           origin,
                                           colorType,
                                           alphaType,
                                           std::move(colorSpace),
                                           textureFulfillProc,
                                           textureReleaseProc,
                                           textureDoneProc,
                                           textureContext);
}

// Converts from the old legacy APIs based on GrBackendTexture to the new implementation based on
// PromiseImageTexture.
static void wrap_legacy(
        SkDeferredDisplayListRecorder::LegacyPromiseImageTextureFulfillProc textureFulfillProc,
        SkDeferredDisplayListRecorder::PromiseImageTextureReleaseProc textureReleaseProc,
        SkDeferredDisplayListRecorder::PromiseImageTextureDoneProc textureDoneProc,
        const SkDeferredDisplayListRecorder::PromiseImageTextureContext textureContexts[],
        int numTextures,
        SkDeferredDisplayListRecorder::PromiseImageTextureFulfillProc* wrappedFulfillProc,
        SkDeferredDisplayListRecorder::PromiseImageTextureReleaseProc* wrappedReleaseProc,
        SkDeferredDisplayListRecorder::PromiseImageTextureDoneProc* wrappedDoneProc,
        SkDeferredDisplayListRecorder::PromiseImageTextureContext wrappedTextureContext[]) {
    struct WrapperContext {
        SkDeferredDisplayListRecorder::LegacyPromiseImageTextureFulfillProc fLegacyFulfill;
        SkDeferredDisplayListRecorder::PromiseImageTextureReleaseProc fLegacyRelease;
        SkDeferredDisplayListRecorder::PromiseImageTextureDoneProc fDone;
        SkDeferredDisplayListRecorder::PromiseImageTextureContext fOriginalContext;
    };
    *wrappedFulfillProc = [](SkDeferredDisplayListRecorder::PromiseImageTextureContext context) {
        auto* wc = static_cast<WrapperContext*>(context);
        GrBackendTexture backendTexture;
        wc->fLegacyFulfill(wc->fOriginalContext, &backendTexture);
        return SkPromiseImageTexture::Make(backendTexture);
    };
    *wrappedReleaseProc = [](SkDeferredDisplayListRecorder::PromiseImageTextureContext context) {
        auto* wc = static_cast<WrapperContext*>(context);
        wc->fLegacyRelease(wc->fOriginalContext);
    };
    *wrappedDoneProc = [](SkDeferredDisplayListRecorder::PromiseImageTextureContext context) {
        const auto* wc = static_cast<WrapperContext*>(context);
        wc->fDone(wc->fOriginalContext);
        delete wc;
    };
    for (int i = 0; i < numTextures; ++i) {
        wrappedTextureContext[i] = new WrapperContext{textureFulfillProc, textureReleaseProc,
                                                      textureDoneProc, textureContexts[i]};
    }
}

sk_sp<SkImage> SkDeferredDisplayListRecorder::makePromiseTexture(
        const GrBackendFormat& backendFormat,
        int width,
        int height,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        SkColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        LegacyPromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureDoneProc textureDoneProc,
        PromiseImageTextureContext textureContext) {
    if (!fContext) {
        return nullptr;
    }

    SkDeferredDisplayListRecorder::PromiseImageTextureFulfillProc wrappedFulfillProc;
    SkDeferredDisplayListRecorder::PromiseImageTextureReleaseProc wrappedReleaseProc;
    SkDeferredDisplayListRecorder::PromiseImageTextureDoneProc wrappedDoneProc;
    SkDeferredDisplayListRecorder::PromiseImageTextureContext wrappedTextureContext;
    if (!textureDoneProc) {
        return nullptr;
    }
    if (!textureFulfillProc || !textureReleaseProc) {
        textureDoneProc(textureContext);
        return nullptr;
    }

    wrap_legacy(textureFulfillProc,
                textureReleaseProc,
                textureDoneProc,
                &textureContext,
                1,
                &wrappedFulfillProc,
                &wrappedReleaseProc,
                &wrappedDoneProc,
                &wrappedTextureContext);
    return SkImage_Gpu::MakePromiseTexture(fContext.get(),
                                           backendFormat,
                                           width,
                                           height,
                                           mipMapped,
                                           origin,
                                           colorType,
                                           alphaType,
                                           std::move(colorSpace),
                                           wrappedFulfillProc,
                                           wrappedReleaseProc,
                                           wrappedDoneProc,
                                           wrappedTextureContext);
}

sk_sp<SkImage> SkDeferredDisplayListRecorder::makeYUVAPromiseTexture(
        SkYUVColorSpace yuvColorSpace,
        const GrBackendFormat yuvaFormats[],
        const SkISize yuvaSizes[],
        const SkYUVAIndex yuvaIndices[4],
        int imageWidth,
        int imageHeight,
        GrSurfaceOrigin imageOrigin,
        sk_sp<SkColorSpace> imageColorSpace,
        PromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureDoneProc textureDoneProc,
        PromiseImageTextureContext textureContexts[]) {
    if (!fContext) {
        return nullptr;
    }

    return SkImage_GpuYUVA::MakePromiseYUVATexture(fContext.get(),
                                                   yuvColorSpace,
                                                   yuvaFormats,
                                                   yuvaSizes,
                                                   yuvaIndices,
                                                   imageWidth,
                                                   imageHeight,
                                                   imageOrigin,
                                                   std::move(imageColorSpace),
                                                   textureFulfillProc,
                                                   textureReleaseProc,
                                                   textureDoneProc,
                                                   textureContexts);
}

sk_sp<SkImage> SkDeferredDisplayListRecorder::makeYUVAPromiseTexture(
        SkYUVColorSpace yuvColorSpace,
        const GrBackendFormat yuvaFormats[],
        const SkISize yuvaSizes[],
        const SkYUVAIndex yuvaIndices[4],
        int imageWidth,
        int imageHeight,
        GrSurfaceOrigin imageOrigin,
        sk_sp<SkColorSpace> imageColorSpace,
        LegacyPromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureDoneProc textureDoneProc,
        PromiseImageTextureContext textureContexts[]) {
    if (!fContext) {
        return nullptr;
    }

    int numTextures;
    bool valid = SkYUVAIndex::AreValidIndices(yuvaIndices, &numTextures);

    SkDeferredDisplayListRecorder::PromiseImageTextureFulfillProc wrappedFulfillProc;
    SkDeferredDisplayListRecorder::PromiseImageTextureReleaseProc wrappedReleaseProc;
    SkDeferredDisplayListRecorder::PromiseImageTextureDoneProc wrappedDoneProc;
    SkDeferredDisplayListRecorder::PromiseImageTextureContext wrappedTextureContexts[4];
    if (!textureDoneProc) {
        return nullptr;
    }
    if (!valid || !textureFulfillProc || !textureReleaseProc) {
        for (int i = 0; i < numTextures; ++i) {
            textureDoneProc(textureContexts[i]);
        }
        return nullptr;
    }

    wrap_legacy(textureFulfillProc,
                textureReleaseProc,
                textureDoneProc,
                textureContexts,
                numTextures,
                &wrappedFulfillProc,
                &wrappedReleaseProc,
                &wrappedDoneProc,
                wrappedTextureContexts);

    return SkImage_GpuYUVA::MakePromiseYUVATexture(fContext.get(),
                                                   yuvColorSpace,
                                                   yuvaFormats,
                                                   yuvaSizes,
                                                   yuvaIndices,
                                                   imageWidth,
                                                   imageHeight,
                                                   imageOrigin,
                                                   std::move(imageColorSpace),
                                                   wrappedFulfillProc,
                                                   wrappedReleaseProc,
                                                   wrappedDoneProc,
                                                   wrappedTextureContexts);
}
#endif
