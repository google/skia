/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureRenderTargetProxy_DEFINED
#define GrTextureRenderTargetProxy_DEFINED

#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrTextureProxy.h"

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
                               int sampleCnt, GrSurfaceOrigin, GrMipMapped,
                               const GrSwizzle& textureSwizzle, const GrSwizzle& outputSwizzle,
                               SkBackingFit, SkBudgeted, GrProtected, GrInternalSurfaceFlags);

    // Lazy-callback version
    GrTextureRenderTargetProxy(LazyInstantiateCallback&&, LazyInstantiationType,
                               const GrBackendFormat&, const GrSurfaceDesc& desc, int sampleCnt,
                               GrSurfaceOrigin, GrMipMapped, const GrSwizzle& textureSwizzle,
                               const GrSwizzle& outputSwizzle, SkBackingFit, SkBudgeted,
                               GrProtected, GrInternalSurfaceFlags);

    // Wrapped version
    GrTextureRenderTargetProxy(sk_sp<GrSurface>, GrSurfaceOrigin, const GrSwizzle& textureSwizzle,
                               const GrSwizzle& outputSwizzle);

    bool instantiate(GrResourceProvider*) override;
    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

    size_t onUninstantiatedGpuMemorySize() const override;

    SkDEBUGCODE(void onValidateSurface(const GrSurface*) override;)
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
