/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureRenderTargetProxy.h"

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

// Wrapped version
// This class is virtually derived from GrSurfaceProxy (via both GrTextureProxy and
// GrRenderTargetProxy) so its constructor must be explicitly called.
GrTextureRenderTargetProxy::GrTextureRenderTargetProxy(sk_sp<GrSurface> surf)
    : GrSurfaceProxy(surf, SkBackingFit::kExact)
    , GrTextureProxy(surf)
    , GrRenderTargetProxy(surf) {
    SkASSERT(surf->asTexture());
    SkASSERT(surf->asRenderTarget());
}

size_t GrTextureRenderTargetProxy::onUninstantiatedGpuMemorySize() const {
    int colorSamplesPerPixel = this->numColorSamples() + 1;

    static const bool kHasMipMaps = true;
    // TODO: add tracking of mipmap state to improve the estimate. We track whether we are created
    // with mip maps but not whether a texture read from the proxy will lazily generate mip maps.

    // TODO: do we have enough information to improve this worst case estimate?
    return GrSurface::ComputeSize(fConfig, fWidth, fHeight, colorSamplesPerPixel, kHasMipMaps,
                                  SkBackingFit::kApprox == fFit);
}

bool GrTextureRenderTargetProxy::instantiate(GrResourceProvider* resourceProvider) {
    static constexpr GrSurfaceFlags kFlags = kRenderTarget_GrSurfaceFlag;

    if (!this->instantiateImpl(resourceProvider, this->numStencilSamples(), kFlags,
                               this->isMipMapped(), this->mipColorMode())) {
        return false;
    }
    SkASSERT(fTarget->asRenderTarget());
    SkASSERT(fTarget->asTexture());

    return true;
}

sk_sp<GrSurface> GrTextureRenderTargetProxy::createSurface(
                                                    GrResourceProvider* resourceProvider) const {
    static constexpr GrSurfaceFlags kFlags = kRenderTarget_GrSurfaceFlag;

    sk_sp<GrSurface> surface = this->createSurfaceImpl(resourceProvider, this->numStencilSamples(),
                                                       kFlags, this->isMipMapped(),
                                                       this->mipColorMode());
    if (!surface) {
        return nullptr;
    }
    SkASSERT(surface->asRenderTarget());
    SkASSERT(surface->asTexture());

    return surface;
}

