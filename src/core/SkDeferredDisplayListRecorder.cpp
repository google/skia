/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/private/SkDeferredDisplayList.h"
#include "src/core/SkMessageBus.h"

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
        PromiseImageTextureContext textureContext,
        PromiseImageApiVersion) {
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
        PromiseImageTextureContext textureContexts[],
        PromiseImageApiVersion) {
    return nullptr;
}

#else

#include "include/core/SkPromiseImageTexture.h"
#include "include/core/SkYUVASizeInfo.h"
#include "include/gpu/GrTexture.h"
#include "src/core/SkMakeUnique.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/SkGr.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkImage_GpuYUVA.h"
#include "src/image/SkSurface_Gpu.h"

SkDeferredDisplayListRecorder::SkDeferredDisplayListRecorder(const SkSurfaceCharacterization& c)
        : fCharacterization(c) {
    if (fCharacterization.isValid()) {
        fContext = GrContextPriv::MakeDDL(fCharacterization.refContextInfo());
    }
}

SkDeferredDisplayListRecorder::~SkDeferredDisplayListRecorder() {
    if (fContext) {
        auto proxyProvider = fContext->priv().proxyProvider();

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

    auto proxyProvider = fContext->priv().proxyProvider();
    const GrCaps* caps = fContext->priv().caps();

    bool usesGLFBO0 = fCharacterization.usesGLFBO0();
    if (usesGLFBO0) {
        if (GrBackendApi::kOpenGL != fContext->backend() ||
            fCharacterization.isTextureable()) {
            return false;
        }
    }

    if (fCharacterization.vulkanSecondaryCBCompatible()) {
        // Because of the restrictive API allowed for a GrVkSecondaryCBDrawContext, we know ahead
        // of time that we don't be able to support certain parameter combinations. Specifially we
        // fail on usesGLFBO0 since we can't mix GL and Vulkan. We can't have a texturable object.
        // And finally the GrVkSecondaryCBDrawContext always assumes a top left origin.
        if (usesGLFBO0 ||
            fCharacterization.isTextureable() ||
            fCharacterization.origin() == kBottomLeft_GrSurfaceOrigin) {
            return false;
        }
    }

    GrColorType grColorType = SkColorTypeToGrColorType(fCharacterization.colorType());

    GrPixelConfig config = caps->getConfigFromBackendFormat(fCharacterization.backendFormat(),
                                                            grColorType);
    if (config == kUnknown_GrPixelConfig) {
        return false;
    }

    GrSurfaceDesc desc;
    desc.fWidth = fCharacterization.width();
    desc.fHeight = fCharacterization.height();
    desc.fConfig = config;

    sk_sp<SkDeferredDisplayList::LazyProxyData> lazyProxyData = fLazyProxyData;

    // What we're doing here is we're creating a lazy proxy to back the SkSurface. The lazy
    // proxy, when instantiated, will use the GrRenderTarget that backs the SkSurface that the
    // DDL is being replayed into.

    GrInternalSurfaceFlags surfaceFlags = GrInternalSurfaceFlags::kNone;
    if (usesGLFBO0) {
        surfaceFlags |= GrInternalSurfaceFlags::kGLRTFBOIDIs0;
    }
    // FIXME: Why do we use GrMipMapped::kNo instead of SkSurfaceCharacterization::fIsMipMapped?
    static constexpr GrProxyProvider::TextureInfo kTextureInfo{GrMipMapped::kNo,
                                                               GrTextureType::k2D};
    const GrProxyProvider::TextureInfo* optionalTextureInfo = nullptr;
    if (fCharacterization.isTextureable()) {
        optionalTextureInfo = &kTextureInfo;
    }

    sk_sp<GrRenderTargetProxy> proxy = proxyProvider->createLazyRenderTargetProxy(
            [lazyProxyData](GrResourceProvider* resourceProvider) {
                // The proxy backing the destination surface had better have been instantiated
                // prior to the proxy backing the DLL's surface. Steal its GrRenderTarget.
                SkASSERT(lazyProxyData->fReplayDest->peekSurface());
                auto surface = sk_ref_sp<GrSurface>(lazyProxyData->fReplayDest->peekSurface());
                return GrSurfaceProxy::LazyInstantiationResult(std::move(surface));
            },
            fCharacterization.backendFormat(),
            desc,
            fCharacterization.sampleCount(),
            fCharacterization.origin(),
            surfaceFlags,
            optionalTextureInfo,
            GrMipMapsStatus::kNotAllocated,
            SkBackingFit::kExact,
            SkBudgeted::kYes,
            fCharacterization.isProtected(),
            fCharacterization.vulkanSecondaryCBCompatible());

    if (!proxy) {
        return false;
    }

    auto c = fContext->priv().makeWrappedSurfaceContext(std::move(proxy),
                                                        grColorType,
                                                        kPremul_SkAlphaType,
                                                        fCharacterization.refColorSpace(),
                                                        &fCharacterization.surfaceProps());
    SkASSERT(c->asRenderTargetContext());
    std::unique_ptr<GrRenderTargetContext> rtc(c.release()->asRenderTargetContext());
    fSurface = SkSurface_Gpu::MakeWrappedRenderTarget(fContext.get(), std::move(rtc));
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

    fContext->priv().moveRenderTasksToDDL(ddl.get());

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
        PromiseImageTextureContext textureContext,
        PromiseImageApiVersion version) {
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
                                           textureContext,
                                           version);
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
        PromiseImageTextureContext textureContexts[],
        PromiseImageApiVersion version) {
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
                                                   textureContexts,
                                                   version);
}

#endif
