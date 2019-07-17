/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrTextureRenderTargetProxy.h"

#include "include/gpu/GrRenderTarget.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureProxyPriv.h"

// Deferred version
// This class is virtually derived from GrSurfaceProxy (via both GrTextureProxy and
// GrRenderTargetProxy) so its constructor must be explicitly called.
GrTextureRenderTargetProxy::GrTextureRenderTargetProxy(const GrCaps& caps,
                                                       const GrBackendFormat& format,
                                                       const GrSurfaceDesc& desc,
                                                       GrSurfaceOrigin origin,
                                                       GrMipMapped mipMapped,
                                                       const GrSwizzle& texSwizzle,
                                                       const GrSwizzle& outSwizzle,
                                                       SkBackingFit fit,
                                                       SkBudgeted budgeted,
                                                       GrInternalSurfaceFlags surfaceFlags)
        : GrSurfaceProxy(format, desc, GrRenderable::kYes, origin, texSwizzle, fit, budgeted,
                         surfaceFlags)
        // for now textures w/ data are always wrapped
        , GrRenderTargetProxy(caps, format, desc, origin, texSwizzle, outSwizzle, fit, budgeted,
                              surfaceFlags)
        , GrTextureProxy(format, desc, origin, mipMapped, texSwizzle, fit, budgeted, surfaceFlags) {
}

// Lazy-callback version
GrTextureRenderTargetProxy::GrTextureRenderTargetProxy(LazyInstantiateCallback&& callback,
                                                       LazyInstantiationType lazyType,
                                                       const GrBackendFormat& format,
                                                       const GrSurfaceDesc& desc,
                                                       GrSurfaceOrigin origin,
                                                       GrMipMapped mipMapped,
                                                       const GrSwizzle& texSwizzle,
                                                       const GrSwizzle& outSwizzle,
                                                       SkBackingFit fit,
                                                       SkBudgeted budgeted,
                                                       GrInternalSurfaceFlags surfaceFlags)
        : GrSurfaceProxy(std::move(callback), lazyType, format, desc, GrRenderable::kYes, origin,
                         texSwizzle, fit, budgeted, surfaceFlags)
        // Since we have virtual inheritance, we initialize GrSurfaceProxy directly. Send null
        // callbacks to the texture and RT proxies simply to route to the appropriate constructors.
        , GrRenderTargetProxy(LazyInstantiateCallback(), lazyType, format, desc, origin, texSwizzle,
                              outSwizzle, fit, budgeted, surfaceFlags, WrapsVkSecondaryCB::kNo)
        , GrTextureProxy(LazyInstantiateCallback(), lazyType, format, desc, origin, mipMapped,
                         texSwizzle, fit, budgeted, surfaceFlags) {}

// Wrapped version
// This class is virtually derived from GrSurfaceProxy (via both GrTextureProxy and
// GrRenderTargetProxy) so its constructor must be explicitly called.
GrTextureRenderTargetProxy::GrTextureRenderTargetProxy(sk_sp<GrSurface> surf,
                                                       GrSurfaceOrigin origin,
                                                       const GrSwizzle& texSwizzle,
                                                       const GrSwizzle& outSwizzle)
        : GrSurfaceProxy(surf, origin, texSwizzle, SkBackingFit::kExact)
        , GrRenderTargetProxy(surf, origin, texSwizzle, outSwizzle)
        , GrTextureProxy(surf, origin, texSwizzle) {
    SkASSERT(surf->asTexture());
    SkASSERT(surf->asRenderTarget());
}

size_t GrTextureRenderTargetProxy::onUninstantiatedGpuMemorySize() const {
    int colorSamplesPerPixel = this->numSamples();
    if (colorSamplesPerPixel > 1) {
        // Add one to account for the resolve buffer.
        ++colorSamplesPerPixel;
    }

    // TODO: do we have enough information to improve this worst case estimate?
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                  colorSamplesPerPixel, this->proxyMipMapped(),
                                  !this->priv().isExact());
}

bool GrTextureRenderTargetProxy::instantiate(GrResourceProvider* resourceProvider) {
    if (LazyState::kNot != this->lazyInstantiationState()) {
        return false;
    }

    const GrUniqueKey& key = this->getUniqueKey();

    if (!this->instantiateImpl(resourceProvider, this->numSamples(), this->numStencilSamples(),
                               GrRenderable::kYes, this->mipMapped(),
                               key.isValid() ? &key : nullptr)) {
        return false;
    }
    if (key.isValid()) {
        SkASSERT(key == this->getUniqueKey());
    }

    SkASSERT(this->peekRenderTarget());
    SkASSERT(this->peekTexture());

    return true;
}

sk_sp<GrSurface> GrTextureRenderTargetProxy::createSurface(
                                                    GrResourceProvider* resourceProvider) const {
    sk_sp<GrSurface> surface =
            this->createSurfaceImpl(resourceProvider, this->numSamples(), this->numStencilSamples(),
                                    GrRenderable::kYes, this->mipMapped());
    if (!surface) {
        return nullptr;
    }
    SkASSERT(surface->asRenderTarget());
    SkASSERT(surface->asTexture());

    return surface;
}

#ifdef SK_DEBUG
void GrTextureRenderTargetProxy::onValidateSurface(const GrSurface* surface) {
    // Anything checked here should also be checking the GrTextureProxy version
    SkASSERT(surface->asTexture());
    SkASSERT(GrMipMapped::kNo == this->proxyMipMapped() ||
             GrMipMapped::kYes == surface->asTexture()->texturePriv().mipMapped());

    // Anything checked here should also be checking the GrRenderTargetProxy version
    SkASSERT(surface->asRenderTarget());
    SkASSERT(surface->asRenderTarget()->numSamples() == this->numSamples());

    SkASSERT(surface->asTexture()->texturePriv().textureType() == this->textureType());

    GrInternalSurfaceFlags proxyFlags = fSurfaceFlags;
    GrInternalSurfaceFlags surfaceFlags = surface->surfacePriv().flags();

    // Only non-RT textures can be read only.
    SkASSERT(!(proxyFlags & GrInternalSurfaceFlags::kReadOnly));
    SkASSERT(!(surfaceFlags & GrInternalSurfaceFlags::kReadOnly));

    SkASSERT((proxyFlags & GrInternalSurfaceFlags::kRenderTargetMask) ==
             (surfaceFlags & GrInternalSurfaceFlags::kRenderTargetMask));
    SkASSERT((proxyFlags & GrInternalSurfaceFlags::kTextureMask) ==
             (surfaceFlags & GrInternalSurfaceFlags::kTextureMask));
}
#endif

