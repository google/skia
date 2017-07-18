/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProxy.h"

#include "GrTexturePriv.h"

GrTextureProxy::GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit fit, SkBudgeted budgeted,
                               const void* srcData, size_t /*rowBytes*/, uint32_t flags)
        : INHERITED(srcDesc, fit, budgeted, flags)
        , fIsMipMapped(srcDesc.fIsMipMapped)
        , fMipColorMode(SkDestinationSurfaceColorMode::kLegacy) {
    SkASSERT(!srcData);  // currently handled in Make()
}

GrTextureProxy::GrTextureProxy(sk_sp<GrSurface> surf)
        : INHERITED(std::move(surf), SkBackingFit::kExact)
        , fIsMipMapped(fTarget->asTexture()->texturePriv().hasMipMaps())
        , fMipColorMode(fTarget->asTexture()->texturePriv().mipColorMode()) {
}

bool GrTextureProxy::instantiate(GrResourceProvider* resourceProvider) {
    if (!this->instantiateImpl(resourceProvider, 0, kNone_GrSurfaceFlags, fIsMipMapped,
                               fMipColorMode)) {
        return false;
    }

    SkASSERT(fTarget->asTexture());
    return true;
}

sk_sp<GrSurface> GrTextureProxy::createSurface(GrResourceProvider* resourceProvider) const {
    sk_sp<GrSurface> surface= this->createSurfaceImpl(resourceProvider, 0, kNone_GrSurfaceFlags,
                                                      fIsMipMapped, fMipColorMode);
    if (!surface) {
        return nullptr;
    }

    SkASSERT(surface->asTexture());
    return surface;
}

void GrTextureProxy::setMipColorMode(SkDestinationSurfaceColorMode colorMode) {
    SkASSERT(fTarget || fTarget->asTexture());

    if (fTarget) {
        fTarget->asTexture()->texturePriv().setMipColorMode(colorMode);
    }

    fMipColorMode = colorMode;
}

// This method parallels the highest_filter_mode functions in GrGLTexture & GrVkTexture.
GrSamplerParams::FilterMode GrTextureProxy::highestFilterMode() const {
    if (fTarget) {
        return fTarget->asTexture()->texturePriv().highestFilterMode();
    }

    if (GrPixelConfigIsSint(this->config())) {
        // We only ever want to nearest-neighbor sample signed int textures.
        return GrSamplerParams::kNone_FilterMode;
    }

    // In OpenGL, GR_GL_TEXTURE_RECTANGLE and GR_GL_TEXTURE_EXTERNAL (which have a highest filter
    // mode of bilerp) can only be created via wrapping.

    return GrSamplerParams::kMipMap_FilterMode;
}

size_t GrTextureProxy::onUninstantiatedGpuMemorySize() const {
    static const bool kHasMipMaps = true;
    // TODO: add tracking of mipmap state to improve the estimate. We track whether we are created
    // with mip maps but not whether a texture read from the proxy will lazily generate mip maps.
    return GrSurface::ComputeSize(fConfig, fWidth, fHeight, 1, kHasMipMaps,
                                  SkBackingFit::kApprox == fFit);
}
