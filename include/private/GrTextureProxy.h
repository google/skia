/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProxy_DEFINED
#define GrTextureProxy_DEFINED

#include "GrSurfaceProxy.h"
#include "GrTexture.h"

class GrTextureProvider;

// This class delays the acquisition of textures until they are actually required
class GrTextureProxy : public GrSurfaceProxy {
public:
    // TODO: need to refine ownership semantics of 'srcData' if we're in completely
    // deferred mode
    static sk_sp<GrTextureProxy> Make(GrTextureProvider*, const GrSurfaceDesc&,
                                      SkBackingFit, SkBudgeted,
                                      const void* srcData = nullptr, size_t rowBytes = 0);
    static sk_sp<GrTextureProxy> Make(sk_sp<GrTexture>);

    // TODO: add asRenderTargetProxy variants
    GrTextureProxy* asTextureProxy() override { return this; }
    const GrTextureProxy* asTextureProxy() const override { return this; }

    // Actually instantiate the backing texture, if necessary
    GrTexture* instantiate(GrTextureProvider*);

private:
    // Deferred version
    GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit, SkBudgeted,
                   const void* srcData, size_t srcRowBytes);
    // Wrapped version
    GrTextureProxy(sk_sp<GrTexture> tex);

    size_t onGpuMemorySize() const override;

    // For wrapped proxies the GrTexture pointer is stored in GrIORefProxy.
    // For deferred proxies that pointer will be filled n when we need to instantiate
    // the deferred resource

    typedef GrSurfaceProxy INHERITED;
};

#endif
