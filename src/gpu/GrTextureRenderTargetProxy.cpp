/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureRenderTargetProxy.h"

#include "GrCaps.h"
#include "GrTexture.h"
#include "GrRenderTarget.h"
#include "GrSurfaceProxyPriv.h"

// Deferred version
// This class is virtually derived from GrSurfaceProxy (via both GrTextureProxy and
// GrRenderTargetProxy) so its constructor must be explicitly called.
GrTextureRenderTargetProxy::GrTextureRenderTargetProxy(const GrCaps& caps,
                                                       const GrSurfaceDesc& desc,
                                                       SkBackingFit fit,
                                                       SkBudgeted budgeted,
                                                       uint32_t flags)
        : GrSurfaceProxy(desc, fit, budgeted, flags)
        // for now textures w/ data are always wrapped
        , GrTextureProxy(desc, fit, budgeted, nullptr, 0, flags)
        , GrRenderTargetProxy(caps, desc, fit, budgeted, flags) {
}

// Lazy-callback version
GrTextureRenderTargetProxy::GrTextureRenderTargetProxy(LazyInstantiateCallback&& callback,
                                                       const GrSurfaceDesc& desc,
                                                       GrMipMapped mipMapped,
                                                       SkBackingFit fit,
                                                       SkBudgeted budgeted,
                                                       uint32_t flags)
        : GrSurfaceProxy(std::move(callback), desc, fit, budgeted, flags)
        // Since we have virtual inheritance, we initialize GrSurfaceProxy directly. Send null
        // callbacks to the texture and RT proxies simply to route to the appropriate constructors.
        , GrTextureProxy(LazyInstantiateCallback(), desc, mipMapped, fit, budgeted, flags)
        , GrRenderTargetProxy(LazyInstantiateCallback(), desc, fit, budgeted, flags) {
}

// Wrapped version
// This class is virtually derived from GrSurfaceProxy (via both GrTextureProxy and
// GrRenderTargetProxy) so its constructor must be explicitly called.
GrTextureRenderTargetProxy::GrTextureRenderTargetProxy(sk_sp<GrSurface> surf,
                                                       GrSurfaceOrigin origin)
        : GrSurfaceProxy(surf, origin, SkBackingFit::kExact)
        , GrTextureProxy(surf, origin)
        , GrRenderTargetProxy(surf, origin) {
    SkASSERT(surf->asTexture());
    SkASSERT(surf->asRenderTarget());
}

size_t GrTextureRenderTargetProxy::onUninstantiatedGpuMemorySize() const {
    int colorSamplesPerPixel = this->numColorSamples() + 1;

    // TODO: do we have enough information to improve this worst case estimate?
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  colorSamplesPerPixel, this->mipMapped(), !this->priv().isExact());
}

bool GrTextureRenderTargetProxy::instantiate(GrResourceProvider* resourceProvider) {
    static constexpr GrSurfaceFlags kFlags = kRenderTarget_GrSurfaceFlag;

    const GrUniqueKey& key = this->getUniqueKey();

    if (!this->instantiateImpl(resourceProvider, this->numStencilSamples(), this->needsStencil(),
                               kFlags, this->mipMapped(), this->mipColorMode(),
                               key.isValid() ? &key : nullptr)) {
        return false;
    }
    if (key.isValid()) {
        SkASSERT(key == this->getUniqueKey());
    }

    SkASSERT(fTarget->asRenderTarget());
    SkASSERT(fTarget->asTexture());

    return true;
}

sk_sp<GrSurface> GrTextureRenderTargetProxy::createSurface(
                                                    GrResourceProvider* resourceProvider) const {
    static constexpr GrSurfaceFlags kFlags = kRenderTarget_GrSurfaceFlag;

    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, this->numStencilSamples(),
                                                       this->needsStencil(), kFlags,
                                                       this->mipMapped(), this->mipColorMode());
    if (!surface) {
        return nullptr;
    }
    SkASSERT(surface->asRenderTarget());
    SkASSERT(surface->asTexture());

    return surface;
}

#ifdef SK_DEBUG
void GrTextureRenderTargetProxy::validateLazyTexture(const GrTexture* texture) {
    SkASSERT(texture->asRenderTarget());
    SkASSERT(texture->asRenderTarget()->numStencilSamples() == this->numStencilSamples());
    SkASSERT(GrMipMapped::kNo == this->mipMapped());
}
#endif

