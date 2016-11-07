/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProxy.h"

#include "GrTextureProvider.h"
#include "GrTextureRenderTargetProxy.h"

GrTextureProxy::GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit fit, SkBudgeted budgeted,
                               const void* srcData, size_t /*rowBytes*/)
    : INHERITED(srcDesc, fit, budgeted) {
    SkASSERT(!srcData);   // currently handled in Make()
}

GrTextureProxy::GrTextureProxy(sk_sp<GrTexture> tex)
    : INHERITED(std::move(tex), SkBackingFit::kExact) {
}

GrTexture* GrTextureProxy::instantiate(GrTextureProvider* texProvider) {
    GrSurface* surf = this->INHERITED::instantiate(texProvider);
    if (!surf) {
        return nullptr;
    }

    return fTarget->asTexture();
}

size_t GrTextureProxy::onGpuMemorySize() const {
    if (fTarget) {
        return fTarget->gpuMemorySize();
    }

    static const bool kHasMipMaps = true;
    // TODO: add tracking of mipmap state to improve the estimate
    return GrSurface::ComputeSize(fDesc, 1, kHasMipMaps);
}

sk_sp<GrTextureProxy> GrTextureProxy::Make(const GrCaps& caps,
                                           GrTextureProvider* texProvider,
                                           const GrSurfaceDesc& desc,
                                           SkBackingFit fit,
                                           SkBudgeted budgeted,
                                           const void* srcData,
                                           size_t rowBytes) {
    if (srcData) {
        // If we have srcData, for now, we create a wrapped GrTextureProxy
        sk_sp<GrTexture> tex(texProvider->createTexture(desc, budgeted, srcData, rowBytes));
        return GrTextureProxy::Make(std::move(tex));
    }

    if (desc.fFlags & kRenderTarget_GrSurfaceFlag) {
        return GrTextureRenderTargetProxy::Make(caps, desc, fit, budgeted);
    }

    return sk_sp<GrTextureProxy>(new GrTextureProxy(desc, fit, budgeted, nullptr, 0));
}

sk_sp<GrTextureProxy> GrTextureProxy::Make(sk_sp<GrTexture> tex) {
    if (tex->asRenderTarget()) {
        return GrTextureRenderTargetProxy::Make(std::move(tex));
    }

    // Not renderable
    return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex)));
}
