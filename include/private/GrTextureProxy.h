/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProxy_DEFINED
#define GrTextureProxy_DEFINED

#include "GrSamplerParams.h"
#include "GrSurfaceProxy.h"

class GrCaps;
class GrResourceProvider;
class GrTextureOpList;

// This class delays the acquisition of textures until they are actually required
class GrTextureProxy : virtual public GrSurfaceProxy {
public:
    GrTextureProxy* asTextureProxy() override { return this; }
    const GrTextureProxy* asTextureProxy() const override { return this; }

    // Actually instantiate the backing texture, if necessary
    bool instantiate(GrResourceProvider*) override;

    void setMipColorMode(SkDestinationSurfaceColorMode colorMode);

    GrSamplerParams::FilterMode highestFilterMode() const;

    GrSLType imageStorageType() const {
        if (GrPixelConfigIsSint(this->config())) {
            return kIImageStorage2D_GrSLType;
        } else {
            return kImageStorage2D_GrSLType;
        }
    }

    bool isMipMapped() const { return fIsMipMapped; }

protected:
    friend class GrSurfaceProxy; // for ctors

    // Deferred version
    GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit, SkBudgeted,
                   const void* srcData, size_t srcRowBytes, uint32_t flags);
    // Wrapped version
    GrTextureProxy(sk_sp<GrSurface>);

    SkDestinationSurfaceColorMode mipColorMode() const { return fMipColorMode;  }

    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

private:
    bool fIsMipMapped;
    SkDestinationSurfaceColorMode fMipColorMode;

    size_t onUninstantiatedGpuMemorySize() const override;

    // For wrapped proxies the GrTexture pointer is stored in GrIORefProxy.
    // For deferred proxies that pointer will be filled in when we need to instantiate
    // the deferred resource

    typedef GrSurfaceProxy INHERITED;
};

#endif
