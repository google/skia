/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProxy.h"
#include "GrTextureProxyPriv.h"

#include "GrContext.h"
#include "GrDeferredProxyUploader.h"
#include "GrResourceCache.h"
#include "GrTexturePriv.h"

// Deferred version
GrTextureProxy::GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit fit, SkBudgeted budgeted,
                               const void* srcData, size_t /*rowBytes*/, uint32_t flags)
        : INHERITED(srcDesc, fit, budgeted, flags)
        , fMipMapped(GrMipMapped::kNo)
        , fMipColorMode(SkDestinationSurfaceColorMode::kLegacy)
        , fCache(nullptr)
        , fDeferredUploader(nullptr) {
    SkASSERT(!srcData);  // currently handled in Make()
}

// Lazy-callback version
GrTextureProxy::GrTextureProxy(LazyInstantiateCallback&& callback, GrPixelConfig config)
        : INHERITED(std::move(callback), config)
        , fMipMapped(GrMipMapped::kNo)
        , fMipColorMode(SkDestinationSurfaceColorMode::kLegacy)
        , fCache(nullptr)
        , fDeferredUploader(nullptr) {
}

// Wrapped version
GrTextureProxy::GrTextureProxy(sk_sp<GrSurface> surf, GrSurfaceOrigin origin)
        : INHERITED(std::move(surf), origin, SkBackingFit::kExact)
        , fMipMapped(fTarget->asTexture()->texturePriv().mipMapped())
        , fMipColorMode(fTarget->asTexture()->texturePriv().mipColorMode())
        , fCache(nullptr)
        , fDeferredUploader(nullptr) {
    if (fTarget->getUniqueKey().isValid()) {
        fCache = fTarget->asTexture()->getContext()->getResourceCache();
        fCache->adoptUniqueKeyFromSurface(this, fTarget);
    }
}

GrTextureProxy::~GrTextureProxy() {
    // Due to the order of cleanup the GrSurface this proxy may have wrapped may have gone away
    // at this point. Zero out the pointer so the cache invalidation code doesn't try to use it.
    fTarget = nullptr;
    if (fUniqueKey.isValid()) {
        fCache->processInvalidProxyUniqueKey(fUniqueKey, this, false);
    } else {
        SkASSERT(!fCache);
    }
}

bool GrTextureProxy::instantiate(GrResourceProvider* resourceProvider) {
    if (!this->instantiateImpl(resourceProvider, 0, /* needsStencil = */ false,
                               kNone_GrSurfaceFlags, fMipMapped, fMipColorMode,
                               fUniqueKey.isValid() ? &fUniqueKey : nullptr)) {
        return false;
    }

    SkASSERT(!fTarget->asRenderTarget());
    SkASSERT(fTarget->asTexture());
    return true;
}

sk_sp<GrSurface> GrTextureProxy::createSurface(GrResourceProvider* resourceProvider) const {
    sk_sp<GrSurface> surface= this->createSurfaceImpl(resourceProvider, 0,
                                                      /* needsStencil = */ false,
                                                      kNone_GrSurfaceFlags,
                                                      fMipMapped, fMipColorMode);
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
    SkASSERT(fTextureProxy->fDeferredUploader);

    // Instantiate might have failed
    if (fTextureProxy->fTarget) {
        fTextureProxy->fDeferredUploader->scheduleUpload(flushState, fTextureProxy);
    }
}

void GrTextureProxyPriv::resetDeferredUploader() {
    SkASSERT(fTextureProxy->fDeferredUploader);
    fTextureProxy->fDeferredUploader.reset();
}

// This method parallels the highest_filter_mode functions in GrGLTexture & GrVkTexture.
GrSamplerState::Filter GrTextureProxy::highestFilterMode() const {
    if (fTarget) {
        return fTarget->asTexture()->texturePriv().highestFilterMode();
    }

    if (GrPixelConfigIsSint(this->config())) {
        // We only ever want to nearest-neighbor sample signed int textures.
        return GrSamplerState::Filter::kNearest;
    }

    // In OpenGL, GR_GL_TEXTURE_RECTANGLE and GR_GL_TEXTURE_EXTERNAL (which have a highest filter
    // mode of bilerp) can only be created via wrapping.

    return GrSamplerState::Filter::kMipMap;
}

size_t GrTextureProxy::onUninstantiatedGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(), 1,
                                  this->mipMapped(), !this->priv().isExact());
}

void GrTextureProxy::setUniqueKey(GrResourceCache* cache, const GrUniqueKey& key) {
    SkASSERT(key.isValid());
    SkASSERT(!fUniqueKey.isValid()); // proxies can only ever get one uniqueKey

    if (fTarget && !fTarget->getUniqueKey().isValid()) {
        fTarget->resourcePriv().setUniqueKey(key);
        SkASSERT(fTarget->getUniqueKey() == key);
    }

    fUniqueKey = key;
    fCache = cache;
}

void GrTextureProxy::clearUniqueKey() {
    fUniqueKey.reset();
    fCache = nullptr;
}

#ifdef SK_DEBUG
void GrTextureProxy::validateLazyTexture(const GrTexture* texture) {
    SkASSERT(!texture->asRenderTarget());
    SkASSERT(GrMipMapped::kNo == this->mipMapped());
}
#endif

