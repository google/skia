/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureRenderTargetProxy_DEFINED
#define GrTextureRenderTargetProxy_DEFINED

#include "GrRenderTargetProxy.h"
#include "GrTextureProxy.h"

#ifdef SK_BUILD_FOR_WIN
// Windows gives warnings about inheriting asTextureProxy/asRenderTargetProxy via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

// This class delays the acquisition of RenderTargets that are also textures until
// they are actually required
// Beware: the uniqueID of the TextureRenderTargetProxy will usually be different than
// the uniqueID of the RenderTarget/Texture it represents!
class GrTextureRenderTargetProxy : public GrTextureProxy, public GrRenderTargetProxy {
private:
    friend class GrSurfaceProxy; // for ctors

    // Deferred version
    GrTextureRenderTargetProxy(const GrCaps&, const GrSurfaceDesc&,
                               SkBackingFit, SkBudgeted, uint32_t flags);

    // Wrapped version
    GrTextureRenderTargetProxy(sk_sp<GrSurface>);

    size_t onGpuMemorySize() const override;
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
