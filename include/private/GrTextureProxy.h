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
    static sk_sp<GrTextureProxy> Make(const GrSurfaceDesc&, SkBackingFit, SkBudgeted,
                                      const void* srcData = nullptr, size_t rowBytes = 0);
    static sk_sp<GrTextureProxy> Make(sk_sp<GrTexture>);

    // TODO: add asRenderTargetProxy variants
    GrTextureProxy* asTextureProxy() override { return this; }
    const GrTextureProxy* asTextureProxy() const override { return this; }

    // Actually instantiate the backing texture, if necessary
    GrTexture* instantiate(GrTextureProvider* texProvider);

private:
    GrTextureProxy(const GrSurfaceDesc& desc, SkBackingFit fit, SkBudgeted budgeted,
                   const void* /*srcData*/, size_t /*rowBytes*/)
        : INHERITED(desc, fit, budgeted) {
        // TODO: Handle 'srcData' here
    }

    // Wrapped version
    GrTextureProxy(sk_sp<GrTexture> tex);

    // For wrapped textures we store it here.
    // For deferred proxies we will fill this in when we need to instantiate the deferred resource
    sk_sp<GrTexture> fTexture;

    typedef GrSurfaceProxy INHERITED;
};

#endif
