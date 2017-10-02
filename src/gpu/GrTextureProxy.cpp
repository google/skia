/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProxy.h"

#include "GrContext.h"
#include "GrResourceCache.h"

#include "GrTexturePriv.h"

GrTextureProxy::GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit fit, SkBudgeted budgeted,
                               const void* srcData, size_t /*rowBytes*/, uint32_t flags)
        : INHERITED(srcDesc, fit, budgeted, flags)
        , fIsMipMapped(false)
        , fMipColorMode(SkDestinationSurfaceColorMode::kLegacy)
        , fCache(nullptr) {
    SkASSERT(!srcData);  // currently handled in Make()
}

GrTextureProxy::GrTextureProxy(sk_sp<GrSurface> surf, GrSurfaceOrigin origin)
        : INHERITED(std::move(surf), origin, SkBackingFit::kExact)
        , fIsMipMapped(fTarget->asTexture()->texturePriv().hasMipMaps())
        , fMipColorMode(fTarget->asTexture()->texturePriv().mipColorMode())
        , fCache(nullptr) {
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
                               kNone_GrSurfaceFlags, fIsMipMapped, fMipColorMode,
                               fUniqueKey.isValid() ? &fUniqueKey : nullptr)) {
        return false;
    }

    SkASSERT(fTarget->asTexture());
    return true;
}

sk_sp<GrSurface> GrTextureProxy::createSurface(GrResourceProvider* resourceProvider) const {
    sk_sp<GrSurface> surface= this->createSurfaceImpl(resourceProvider, 0,
                                                      /* needsStencil = */ false,
                                                      kNone_GrSurfaceFlags,
                                                      fIsMipMapped, fMipColorMode);
    if (!surface) {
        return nullptr;
    }

    SkASSERT(surface->asTexture());
    return surface;
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
    static const bool kHasMipMaps = true;
    // TODO: add tracking of mipmap state to improve the estimate. We track whether we are created
    // with mip maps but not whether a texture read from the proxy will lazily generate mip maps.
    return GrSurface::ComputeSize(fConfig, fWidth, fHeight, 1, kHasMipMaps,
                                  SkBackingFit::kApprox == fFit);
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

