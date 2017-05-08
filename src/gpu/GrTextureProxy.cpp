/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProxy.h"

#include "GrResourceProvider.h"
#include "GrTexturePriv.h"

GrTextureProxy::GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit fit, SkBudgeted budgeted,
                               const void* srcData, size_t /*rowBytes*/, uint32_t flags)
    : INHERITED(srcDesc, fit, budgeted, flags) {
    SkASSERT(!srcData);   // currently handled in Make()
}

GrTextureProxy::GrTextureProxy(sk_sp<GrSurface> surf)
    : INHERITED(std::move(surf), SkBackingFit::kExact) {
}

GrTexture* GrTextureProxy::instantiate(GrResourceProvider* resourceProvider) {
    GrSurface* surf = this->INHERITED::instantiate(resourceProvider);
    if (!surf) {
        return nullptr;
    }

    return fTarget->asTexture();
}

void GrTextureProxy::setMipColorMode(SkDestinationSurfaceColorMode colorMode) {
    SkASSERT(fTarget || fTarget->asTexture());

    if (fTarget) {
        fTarget->asTexture()->texturePriv().setMipColorMode(colorMode);
    }

    fMipColorMode = colorMode;
}

size_t GrTextureProxy::onGpuMemorySize() const {
    if (fTarget) {
        return fTarget->gpuMemorySize();
    }

    static const bool kHasMipMaps = true;
    // TODO: add tracking of mipmap state to improve the estimate
    return GrSurface::ComputeSize(fDesc, 1, kHasMipMaps, SkBackingFit::kApprox == fFit);
}
