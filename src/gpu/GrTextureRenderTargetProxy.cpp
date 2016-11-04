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
                                                       SkBudgeted budgeted)
    : GrSurfaceProxy(desc, fit, budgeted)
    , GrTextureProxy(desc, fit, budgeted, nullptr, 0) // 4 now textures w/ data are always wrapped
    , GrRenderTargetProxy(caps, desc, fit, budgeted) {
}

// Wrapped version
// This class is virtually derived from GrSurfaceProxy (via both GrTextureProxy and 
// GrRenderTargetProxy) so its constructor must be explicitly called.
GrTextureRenderTargetProxy::GrTextureRenderTargetProxy(sk_sp<GrRenderTarget> rt)
    : GrSurfaceProxy(rt, SkBackingFit::kExact)
    , GrTextureProxy(sk_ref_sp(rt->asTexture()))
    , GrRenderTargetProxy(rt) {
    SkASSERT(rt->asTexture());
}

size_t GrTextureRenderTargetProxy::onGpuMemorySize() const {
    if (fTarget) {
        return fTarget->gpuMemorySize();
    }

    // TODO: do we have enough information to improve this worst case estimate?
    return GrSurface::ComputeSize(fDesc, fDesc.fSampleCnt+1, true);
}

sk_sp<GrTextureRenderTargetProxy> GrTextureRenderTargetProxy::Make(const GrCaps& caps,
                                                                   const GrSurfaceDesc& desc,
                                                                   SkBackingFit fit,
                                                                   SkBudgeted budgeted) {
    SkASSERT(desc.fFlags & kRenderTarget_GrSurfaceFlag);

    return sk_sp<GrTextureRenderTargetProxy>(new GrTextureRenderTargetProxy(caps, desc,
                                                                            fit, budgeted));
}

sk_sp<GrTextureRenderTargetProxy> GrTextureRenderTargetProxy::Make(sk_sp<GrTexture> tex) {
    SkASSERT(tex->asRenderTarget());

    return sk_sp<GrTextureRenderTargetProxy>(new GrTextureRenderTargetProxy(
                                                            sk_ref_sp(tex->asRenderTarget())));
}

sk_sp<GrTextureRenderTargetProxy> GrTextureRenderTargetProxy::Make(sk_sp<GrRenderTarget> rt) {
    SkASSERT(rt->asTexture());

    return sk_sp<GrTextureRenderTargetProxy>(new GrTextureRenderTargetProxy(std::move(rt)));
}



