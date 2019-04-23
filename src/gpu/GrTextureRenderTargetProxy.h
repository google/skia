/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureRenderTargetProxy_DEFINED
#define GrTextureRenderTargetProxy_DEFINED

#include "include/private/GrRenderTargetProxy.h"
#include "include/private/GrTextureProxy.h"

#ifdef SK_BUILD_FOR_WIN
// Windows gives warnings about inheriting asTextureProxy/asRenderTargetProxy via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

// This class delays the acquisition of RenderTargets that are also textures until
// they are actually required
// Beware: the uniqueID of the TextureRenderTargetProxy will usually be different than
// the uniqueID of the RenderTarget/Texture it represents!
class GrTextureRenderTargetProxy : public GrRenderTargetProxy, public GrTextureProxy {
private:
    // DDL TODO: rm the GrSurfaceProxy friending
    friend class GrSurfaceProxy; // for ctors
    friend class GrProxyProvider; // for ctors

    // Deferred version
    GrTextureRenderTargetProxy(const GrCaps&, const GrBackendFormat&, const GrSurfaceDesc&,
                               GrSurfaceOrigin, GrMipMapped, SkBackingFit, SkBudgeted,
                               GrInternalSurfaceFlags);

    // Lazy-callback version
    GrTextureRenderTargetProxy(LazyInstantiateCallback&&, LazyInstantiationType,
                               const GrBackendFormat&, const GrSurfaceDesc& desc, GrSurfaceOrigin,
                               GrMipMapped, SkBackingFit, SkBudgeted, GrInternalSurfaceFlags);

    // Wrapped version
    GrTextureRenderTargetProxy(sk_sp<GrSurface>, GrSurfaceOrigin);

    bool instantiate(GrResourceProvider*, bool dontForceNoPendingIO = false) override;
    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

    size_t onUninstantiatedGpuMemorySize() const override;

    SkDEBUGCODE(void onValidateSurface(const GrSurface*) override;)
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
