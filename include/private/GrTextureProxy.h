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

class GrCaps;
class GrResourceProvider;
class GrTextureOpList;

// This class delays the acquisition of textures until they are actually required
class GrTextureProxy : virtual public GrSurfaceProxy {
public:
    GrTextureProxy* asTextureProxy() override { return this; }
    const GrTextureProxy* asTextureProxy() const override { return this; }

    // Actually instantiate the backing texture, if necessary
    GrTexture* instantiate(GrResourceProvider*);

    void setMipColorMode(SkDestinationSurfaceColorMode colorMode);

protected:
    friend class GrSurfaceProxy; // for ctors

    // Deferred version
    GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit, SkBudgeted,
                   const void* srcData, size_t srcRowBytes, uint32_t flags);
    // Wrapped version
    GrTextureProxy(sk_sp<GrSurface>);

private:
    size_t onGpuMemorySize() const override;

    // For wrapped proxies the GrTexture pointer is stored in GrIORefProxy.
    // For deferred proxies that pointer will be filled in when we need to instantiate
    // the deferred resource

    typedef GrSurfaceProxy INHERITED;
};

#endif
