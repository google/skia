/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkDeferredDisplayListRecorder.h"

#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "src/core/SkMessageBus.h"

#if !SK_SUPPORT_GPU
SkDeferredDisplayListRecorder::SkDeferredDisplayListRecorder(const SkSurfaceCharacterization&) {}

SkDeferredDisplayListRecorder::~SkDeferredDisplayListRecorder() {}

bool SkDeferredDisplayListRecorder::init() { return false; }

SkCanvas* SkDeferredDisplayListRecorder::getCanvas() { return nullptr; }

sk_sp<SkDeferredDisplayList> SkDeferredDisplayListRecorder::detach() { return nullptr; }

sk_sp<SkImage> SkDeferredDisplayListRecorder::makePromiseTexture(
        const GrBackendFormat& backendFormat,
        int width,
        int height,
        GrMipmapped mipMapped,
        GrSurfaceOrigin origin,
        SkColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        PromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureContext textureContext) {
    return nullptr;
}

sk_sp<SkImage> SkDeferredDisplayListRecorder::makeYUVAPromiseTexture(
        const GrYUVABackendTextureInfo& yuvaBackendTextureInfo,
        sk_sp<SkColorSpace> imageColorSpace,
        PromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureContext textureContexts[]) {
    return nullptr;
}

#else

#include "include/core/SkPromiseImageTexture.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrYUVABackendTextures.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/SkGr.h"
#include "src/image/SkImage_Gpu.h"
#include "src/image/SkImage_GpuYUVA.h"
#include "src/image/SkSurface_Gpu.h"

SkDeferredDisplayListRecorder::SkDeferredDisplayListRecorder(const SkSurfaceCharacterization& c)
        : fCharacterization(c) {
    if (fCharacterization.isValid()) {
        fContext = GrRecordingContextPriv::MakeDDL(fCharacterization.refContextInfo());
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
    SkASSERT(!fTargetProxy);
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

    bool vkRTSupportsInputAttachment = fCharacterization.vkRTSupportsInputAttachment();
    if (vkRTSupportsInputAttachment && GrBackendApi::kVulkan != fContext->backend()) {
        return false;
    }

    if (fCharacterization.vulkanSecondaryCBCompatible()) {
        // Because of the restrictive API allowed for a GrVkSecondaryCBDrawContext, we know ahead
        // of time that we don't be able to support certain parameter combinations. Specifically we
        // fail on usesGLFBO0 since we can't mix GL and Vulkan. We can't have a texturable object.
        // We can't use it as in input attachment since we don't control the render pass this will
        // be played into and thus can't force it to have an input attachment and the correct
        // dependencies. And finally the GrVkSecondaryCBDrawContext always assumes a top left
        // origin.
        if (usesGLFBO0 ||
            vkRTSupportsInputAttachment ||
            fCharacterization.isTextureable() ||
            fCharacterization.origin() == kBottomLeft_GrSurfaceOrigin) {
            return false;
        }
    }

    GrColorType grColorType = SkColorTypeToGrColorType(fCharacterization.colorType());

    // What we're doing here is we're creating a lazy proxy to back the SkSurface. The lazy
    // proxy, when instantiated, will use the GrRenderTarget that backs the SkSurface that the
    // DDL is being replayed into.

    GrInternalSurfaceFlags surfaceFlags = GrInternalSurfaceFlags::kNone;
    if (usesGLFBO0) {
        surfaceFlags |= GrInternalSurfaceFlags::kGLRTFBOIDIs0;
    } else if (fCharacterization.sampleCount() > 1 && !caps->msaaResolvesAutomatically() &&
               fCharacterization.isTextureable()) {
        surfaceFlags |= GrInternalSurfaceFlags::kRequiresManualMSAAResolve;
    }

    if (vkRTSupportsInputAttachment) {
        surfaceFlags |= GrInternalSurfaceFlags::kVkRTSupportsInputAttachment;
    }

    // FIXME: Why do we use GrMipmapped::kNo instead of SkSurfaceCharacterization::fIsMipMapped?
    static constexpr GrProxyProvider::TextureInfo kTextureInfo{GrMipmapped::kNo,
                                                               GrTextureType::k2D};
    const GrProxyProvider::TextureInfo* optionalTextureInfo = nullptr;
    if (fCharacterization.isTextureable()) {
        optionalTextureInfo = &kTextureInfo;
    }

    fTargetProxy = proxyProvider->createLazyRenderTargetProxy(
            [lazyProxyData = fLazyProxyData](GrResourceProvider* resourceProvider,
                                             const GrSurfaceProxy::LazySurfaceDesc&) {
                // The proxy backing the destination surface had better have been instantiated
                // prior to this one (i.e., the proxy backing the DDL's surface).
                // Fulfill this lazy proxy with the destination surface's GrRenderTarget.
                SkASSERT(lazyProxyData->fReplayDest->peekSurface());
                auto surface = sk_ref_sp<GrSurface>(lazyProxyData->fReplayDest->peekSurface());
                return GrSurfaceProxy::LazyCallbackResult(std::move(surface));
            },
            fCharacterization.backendFormat(),
            fCharacterization.dimensions(),
            fCharacterization.sampleCount(),
            surfaceFlags,
            optionalTextureInfo,
            GrMipmapStatus::kNotAllocated,
            SkBackingFit::kExact,
            SkBudgeted::kYes,
            fCharacterization.isProtected(),
            fCharacterization.vulkanSecondaryCBCompatible(),
            GrSurfaceProxy::UseAllocator::kYes);

    if (!fTargetProxy) {
        return false;
    }
    fTargetProxy->priv().setIsDDLTarget();

    auto device = fContext->priv().createDevice(grColorType,
                                                fTargetProxy,
                                                fCharacterization.refColorSpace(),
                                                fCharacterization.origin(),
                                                fCharacterization.surfaceProps(),
                                                skgpu::BaseDevice::InitContents::kUninit);
    if (!device) {
        return false;
    }

    fSurface = sk_make_sp<SkSurface_Gpu>(std::move(device));
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

sk_sp<SkDeferredDisplayList> SkDeferredDisplayListRecorder::detach() {
    if (!fContext || !fTargetProxy) {
        return nullptr;
    }

    if (fSurface) {
        SkCanvas* canvas = fSurface->getCanvas();

        canvas->restoreToCount(0);
    }

    auto ddl = sk_sp<SkDeferredDisplayList>(new SkDeferredDisplayList(fCharacterization,
                                                                      std::move(fTargetProxy),
                                                                      std::move(fLazyProxyData)));

    fContext->priv().moveRenderTasksToDDL(ddl.get());

    // We want a new lazy proxy target for each recorded DDL so force the (lazy proxy-backed)
    // SkSurface to be regenerated for each DDL.
    fSurface = nullptr;
    return ddl;
}

#ifndef SK_MAKE_PROMISE_TEXTURE_DISABLE_LEGACY_API
sk_sp<SkImage> SkDeferredDisplayListRecorder::makePromiseTexture(
        const GrBackendFormat& backendFormat,
        int width,
        int height,
        GrMipmapped mipMapped,
        GrSurfaceOrigin origin,
        SkColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        PromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureContext textureContext) {
    if (!fContext) {
        return nullptr;
    }
    return SkImage::MakePromiseTexture(fContext->threadSafeProxy(),
                                       backendFormat,
                                       {width, height},
                                       mipMapped,
                                       origin,
                                       colorType,
                                       alphaType,
                                       std::move(colorSpace),
                                       textureFulfillProc,
                                       textureReleaseProc,
                                       textureContext);
}

sk_sp<SkImage> SkDeferredDisplayListRecorder::makeYUVAPromiseTexture(
        const GrYUVABackendTextureInfo& backendTextureInfo,
        sk_sp<SkColorSpace> imageColorSpace,
        PromiseImageTextureFulfillProc textureFulfillProc,
        PromiseImageTextureReleaseProc textureReleaseProc,
        PromiseImageTextureContext textureContexts[]) {
    if (!fContext) {
        return nullptr;
    }
    return SkImage::MakePromiseYUVATexture(fContext->threadSafeProxy(),
                                           backendTextureInfo,
                                           std::move(imageColorSpace),
                                           textureFulfillProc,
                                           textureReleaseProc,
                                           textureContexts);
}
#endif // !SK_MAKE_PROMISE_TEXTURE_DISABLE_LEGACY_API

#endif
