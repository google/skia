/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/GrTextureProxyPriv.h"

#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrDeferredProxyUploader.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrSurfaceProxyPriv.h"
#include "src/gpu/ganesh/GrTexture.h"

#include <utility>

class GrOpFlushState;

// Deferred version - no data
GrTextureProxy::GrTextureProxy(const GrBackendFormat& format,
                               SkISize dimensions,
                               skgpu::Mipmapped mipmapped,
                               GrMipmapStatus mipmapStatus,
                               SkBackingFit fit,
                               skgpu::Budgeted budgeted,
                               GrProtected isProtected,
                               GrInternalSurfaceFlags surfaceFlags,
                               UseAllocator useAllocator,
                               GrDDLProvider creatingProvider,
                               std::string_view label)
        : INHERITED(
                  format, dimensions, fit, budgeted, isProtected, surfaceFlags, useAllocator, label)
        , fMipmapped(mipmapped)
        , fMipmapStatus(mipmapStatus) SkDEBUGCODE(, fInitialMipmapStatus(fMipmapStatus))
        , fCreatingProvider(creatingProvider)
        , fProxyProvider(nullptr)
        , fDeferredUploader(nullptr) {
    SkASSERT(!(fSurfaceFlags & GrInternalSurfaceFlags::kFramebufferOnly));
    if (this->textureType() == GrTextureType::kExternal) {
        fSurfaceFlags |= GrInternalSurfaceFlags::kReadOnly;
    }
}

// Lazy-callback version
GrTextureProxy::GrTextureProxy(LazyInstantiateCallback&& callback,
                               const GrBackendFormat& format,
                               SkISize dimensions,
                               skgpu::Mipmapped mipmapped,
                               GrMipmapStatus mipmapStatus,
                               SkBackingFit fit,
                               skgpu::Budgeted budgeted,
                               GrProtected isProtected,
                               GrInternalSurfaceFlags surfaceFlags,
                               UseAllocator useAllocator,
                               GrDDLProvider creatingProvider,
                               std::string_view label)
        : INHERITED(std::move(callback),
                    format,
                    dimensions,
                    fit,
                    budgeted,
                    isProtected,
                    surfaceFlags,
                    useAllocator,
                    label)
        , fMipmapped(mipmapped)
        , fMipmapStatus(mipmapStatus) SkDEBUGCODE(, fInitialMipmapStatus(fMipmapStatus))
        , fCreatingProvider(creatingProvider)
        , fProxyProvider(nullptr)
        , fDeferredUploader(nullptr) {
    SkASSERT(!(fSurfaceFlags & GrInternalSurfaceFlags::kFramebufferOnly));
    if (this->textureType() == GrTextureType::kExternal) {
        fSurfaceFlags |= GrInternalSurfaceFlags::kReadOnly;
    }
}

// Wrapped version
GrTextureProxy::GrTextureProxy(sk_sp<GrSurface> surf,
                               UseAllocator useAllocator,
                               GrDDLProvider creatingProvider)
        : INHERITED(std::move(surf), SkBackingFit::kExact, useAllocator)
        , fMipmapped(fTarget->asTexture()->mipmapped())
        , fMipmapStatus(fTarget->asTexture()->mipmapStatus())
        SkDEBUGCODE(, fInitialMipmapStatus(fMipmapStatus))
        , fCreatingProvider(creatingProvider)
        , fProxyProvider(nullptr)
        , fDeferredUploader(nullptr) {
    if (fTarget->getUniqueKey().isValid()) {
        fProxyProvider = fTarget->asTexture()->getContext()->priv().proxyProvider();
        fProxyProvider->adoptUniqueKeyFromSurface(this, fTarget.get());
    }
    if (this->textureType() == GrTextureType::kExternal) {
        fSurfaceFlags |= GrInternalSurfaceFlags::kReadOnly;
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
    if (!this->instantiateImpl(resourceProvider, 1, GrRenderable::kNo, fMipmapped,
                               fUniqueKey.isValid() ? &fUniqueKey : nullptr)) {
        return false;
    }

    SkASSERT(!this->peekRenderTarget());
    SkASSERT(this->peekTexture());
    return true;
}

sk_sp<GrSurface> GrTextureProxy::createSurface(GrResourceProvider* resourceProvider) const {
    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, 1, GrRenderable::kNo,
                                                       fMipmapped);
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

skgpu::Mipmapped GrTextureProxy::mipmapped() const {
    if (this->isInstantiated()) {
        return this->peekTexture()->mipmapped();
    }
    return fMipmapped;
}

GrTextureType GrTextureProxy::textureType() const { return this->backendFormat().textureType(); }

size_t GrTextureProxy::onUninstantiatedGpuMemorySize() const {
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  /*colorSamplesPerPixel=*/1, this->proxyMipmapped(),
                                  !this->priv().isExact());
}

bool GrTextureProxy::ProxiesAreCompatibleAsDynamicState(const GrSurfaceProxy* first,
                                                        const GrSurfaceProxy* second) {
    // In order to be compatible, the proxies should also have the same texture type. This is
    // checked explicitly since the GrBackendFormat == operator does not compare texture type
    return first->backendFormat().textureType() == second->backendFormat().textureType() &&
           first->backendFormat() == second->backendFormat();
}

void GrTextureProxy::setUniqueKey(GrProxyProvider* proxyProvider, const skgpu::UniqueKey& key) {
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

GrSurfaceProxy::LazySurfaceDesc GrTextureProxy::callbackDesc() const {
    SkISize dims;
    SkBackingFit fit;
    if (this->isFullyLazy()) {
        fit = SkBackingFit::kApprox;
        dims = {-1, -1};
    } else {
        fit = this->isFunctionallyExact() ? SkBackingFit::kExact : SkBackingFit::kApprox;
        dims = this->dimensions();
    }
    return {
            dims,
            fit,
            GrRenderable::kNo,
            fMipmapped,
            1,
            this->backendFormat(),
            this->textureType(),
            this->isProtected(),
            this->isBudgeted(),
            this->getLabel(),
    };
}

#ifdef SK_DEBUG
void GrTextureProxy::onValidateSurface(const GrSurface* surface) {
    SkASSERT(!surface->asRenderTarget());

    // Anything that is checked here should be duplicated in GrTextureRenderTargetProxy's version
    SkASSERT(surface->asTexture());
    // It is possible to fulfill a non-mipmapped proxy with a mipmapped texture.
    SkASSERT(skgpu::Mipmapped::kNo == this->proxyMipmapped() ||
             skgpu::Mipmapped::kYes == surface->asTexture()->mipmapped());

    SkASSERT(surface->asTexture()->textureType() == this->textureType());

    GrInternalSurfaceFlags proxyFlags = fSurfaceFlags;
    GrInternalSurfaceFlags surfaceFlags = surface->flags();
    SkASSERT(((int)proxyFlags & kGrInternalTextureFlagsMask) ==
             ((int)surfaceFlags & kGrInternalTextureFlagsMask));
}

#endif

