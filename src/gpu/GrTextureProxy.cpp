/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrTextureProxyPriv.h"

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDeferredProxyUploader.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTexturePriv.h"

// Deferred version - no data
GrTextureProxy::GrTextureProxy(const GrBackendFormat& format,
                               const GrSurfaceDesc& srcDesc,
                               GrSurfaceOrigin origin,
                               GrMipMapped mipMapped,
                               GrMipMapsStatus mipMapsStatus,
                               const GrSwizzle& textureSwizzle,
                               SkBackingFit fit,
                               SkBudgeted budgeted,
                               GrProtected isProtected,
                               GrInternalSurfaceFlags surfaceFlags,
                               UseAllocator useAllocator)
        : INHERITED(format, srcDesc, GrRenderable::kNo, origin, textureSwizzle, fit, budgeted,
                    isProtected, surfaceFlags, useAllocator)
        , fMipMapped(mipMapped)
        , fMipMapsStatus(mipMapsStatus) SkDEBUGCODE(, fInitialMipMapsStatus(fMipMapsStatus))
        , fProxyProvider(nullptr)
        , fDeferredUploader(nullptr) {}

// Lazy-callback version
GrTextureProxy::GrTextureProxy(LazyInstantiateCallback&& callback,
                               const GrBackendFormat& format,
                               const GrSurfaceDesc& desc,
                               GrSurfaceOrigin origin,
                               GrMipMapped mipMapped,
                               GrMipMapsStatus mipMapsStatus,
                               const GrSwizzle& texSwizzle,
                               SkBackingFit fit,
                               SkBudgeted budgeted,
                               GrProtected isProtected,
                               GrInternalSurfaceFlags surfaceFlags,
                               UseAllocator useAllocator)
        : INHERITED(std::move(callback), format, desc, GrRenderable::kNo, origin, texSwizzle, fit,
                    budgeted, isProtected, surfaceFlags, useAllocator)
        , fMipMapped(mipMapped)
        , fMipMapsStatus(mipMapsStatus) SkDEBUGCODE(, fInitialMipMapsStatus(fMipMapsStatus))
        , fProxyProvider(nullptr)
        , fDeferredUploader(nullptr) {}

// Wrapped version
GrTextureProxy::GrTextureProxy(sk_sp<GrSurface> surf,
                               GrSurfaceOrigin origin,
                               const GrSwizzle& textureSwizzle,
                               UseAllocator useAllocator)
        : INHERITED(std::move(surf), origin, textureSwizzle, SkBackingFit::kExact, useAllocator)
        , fMipMapped(fTarget->asTexture()->texturePriv().mipMapped())
        , fMipMapsStatus(fTarget->asTexture()->texturePriv().mipMapsStatus())
                  SkDEBUGCODE(, fInitialMipMapsStatus(fMipMapsStatus))
        , fProxyProvider(nullptr)
        , fDeferredUploader(nullptr) {
    if (fTarget->getUniqueKey().isValid()) {
        fProxyProvider = fTarget->asTexture()->getContext()->priv().proxyProvider();
        fProxyProvider->adoptUniqueKeyFromSurface(this, fTarget.get());
    }
}

GrTextureProxy::~GrTextureProxy() {
    // Due to the order of cleanup the GrSurface this proxy may have wrapped may have gone away
    // at this point. Zero out the pointer so the cache invalidation code doesn't try to use it.
    fTarget = nullptr;

    // In DDL-mode, uniquely keyed proxies keep their key even after their originating
    // proxy provider has gone away. In that case there is noone to send the invalid key
    // message to (Note: in this case we don't want to remove its cached resource).
    if (fUniqueKey.isValid() && fProxyProvider) {
        fProxyProvider->processInvalidUniqueKey(fUniqueKey, this,
                                                GrProxyProvider::InvalidateGPUResource::kNo);
    } else {
        SkASSERT(!fProxyProvider);
    }
}

bool GrTextureProxy::instantiate(GrResourceProvider* resourceProvider) {
    if (this->isLazy()) {
        return false;
    }
    if (!this->instantiateImpl(resourceProvider, 1, /* needsStencil = */ false, GrRenderable::kNo,
                               fMipMapped, fUniqueKey.isValid() ? &fUniqueKey : nullptr)) {
        return false;
    }

    SkASSERT(!this->peekRenderTarget());
    SkASSERT(this->peekTexture());
    return true;
}

sk_sp<GrSurface> GrTextureProxy::createSurface(GrResourceProvider* resourceProvider) const {
    sk_sp<GrSurface> surface =
            this->createSurfaceImpl(resourceProvider, 1,
                                    /* needsStencil = */ false, GrRenderable::kNo, fMipMapped);
    if (!surface) {
        return nullptr;
    }

    SkASSERT(!surface->asRenderTarget());
    SkASSERT(surface->asTexture());
    return surface;
}

void GrTextureProxyPriv::setDeferredUploader(std::unique_ptr<GrDeferredProxyUploader> uploader) {
    SkASSERT(!fTextureProxy->fDeferredUploader);
    fTextureProxy->fDeferredUploader = std::move(uploader);
}

void GrTextureProxyPriv::scheduleUpload(GrOpFlushState* flushState) {
    // The texture proxy's contents may already have been uploaded or instantiation may have failed
    if (fTextureProxy->fDeferredUploader && fTextureProxy->isInstantiated()) {
        fTextureProxy->fDeferredUploader->scheduleUpload(flushState, fTextureProxy);
    }
}

void GrTextureProxyPriv::resetDeferredUploader() {
    SkASSERT(fTextureProxy->fDeferredUploader);
    fTextureProxy->fDeferredUploader.reset();
}

GrSamplerState::Filter GrTextureProxy::highestFilterMode() const {
    return this->hasRestrictedSampling() ? GrSamplerState::Filter::kBilerp
                                         : GrSamplerState::Filter::kMipMap;
}

GrMipMapped GrTextureProxy::mipMapped() const {
    if (this->isInstantiated()) {
        return this->peekTexture()->texturePriv().mipMapped();
    }
    return fMipMapped;
}

size_t GrTextureProxy::onUninstantiatedGpuMemorySize(const GrCaps& caps) const {
    return GrSurface::ComputeSize(caps, this->backendFormat(),  this->width(), this->height(),
                                  1, this->proxyMipMapped(), !this->priv().isExact());
}

bool GrTextureProxy::ProxiesAreCompatibleAsDynamicState(const GrTextureProxy* first,
                                                        const GrTextureProxy* second) {
    return first->config() == second->config() &&
           first->textureType() == second->textureType() &&
           first->backendFormat() == second->backendFormat();
}

void GrTextureProxy::setUniqueKey(GrProxyProvider* proxyProvider, const GrUniqueKey& key) {
    SkASSERT(key.isValid());
    SkASSERT(!fUniqueKey.isValid()); // proxies can only ever get one uniqueKey

    if (fTarget && fSyncTargetKey) {
        if (!fTarget->getUniqueKey().isValid()) {
            fTarget->resourcePriv().setUniqueKey(key);
        }
        SkASSERT(fTarget->getUniqueKey() == key);
    }

    fUniqueKey = key;
    fProxyProvider = proxyProvider;
}

void GrTextureProxy::clearUniqueKey() {
    fUniqueKey.reset();
    fProxyProvider = nullptr;
}

#ifdef SK_DEBUG
void GrTextureProxy::onValidateSurface(const GrSurface* surface) {
    SkASSERT(!surface->asRenderTarget());

    // Anything that is checked here should be duplicated in GrTextureRenderTargetProxy's version
    SkASSERT(surface->asTexture());
    // It is possible to fulfill a non-mipmapped proxy with a mipmapped texture.
    SkASSERT(GrMipMapped::kNo == this->proxyMipMapped() ||
             GrMipMapped::kYes == surface->asTexture()->texturePriv().mipMapped());

    SkASSERT(surface->asTexture()->texturePriv().textureType() == this->textureType());

    GrInternalSurfaceFlags proxyFlags = fSurfaceFlags;
    GrInternalSurfaceFlags surfaceFlags = surface->surfacePriv().flags();
    SkASSERT(((int)proxyFlags & kGrInternalTextureFlagsMask) ==
             ((int)surfaceFlags & kGrInternalTextureFlagsMask));
}

#endif

