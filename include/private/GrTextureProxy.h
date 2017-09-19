/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProxy_DEFINED
#define GrTextureProxy_DEFINED

#include "GrSamplerState.h"
#include "GrSurfaceProxy.h"

class GrCaps;
class GrResourceProvider;
class GrTextureOpList;
class GrTextureProxyPriv;

// This class delays the acquisition of textures until they are actually required
class GrTextureProxy : virtual public GrSurfaceProxy {
public:
    GrTextureProxy* asTextureProxy() override { return this; }
    const GrTextureProxy* asTextureProxy() const override { return this; }

    // Actually instantiate the backing texture, if necessary
    bool instantiate(GrResourceProvider*) override;

    void setMipColorMode(SkDestinationSurfaceColorMode colorMode);

    GrSamplerState::Filter highestFilterMode() const;

    GrSLType imageStorageType() const {
        if (GrPixelConfigIsSint(this->config())) {
            return kIImageStorage2D_GrSLType;
        } else {
            return kImageStorage2D_GrSLType;
        }
    }

    bool isMipMapped() const { return fIsMipMapped; }

    /**
     * Returns the texture proxy's unique key. It will be invalid if the proxy doesn't have one.
     */
    const GrUniqueKey& getUniqueKey() const {
#ifdef SK_DEBUG
        if (fTarget && (fUniqueKey.isValid() || fTarget->getUniqueKey().isValid())) {
            SkASSERT(fUniqueKey.isValid() && fTarget->getUniqueKey().isValid());
            SkASSERT(fUniqueKey == fTarget->getUniqueKey());
        }
#endif

        return fUniqueKey;
    }

    // Provides access to functions that shouldn't be widely used.
    GrTextureProxyPriv texPriv();
    const GrTextureProxyPriv texPriv() const;

protected:
    friend class GrSurfaceProxy; // for ctors

    // Deferred version
    GrTextureProxy(const GrSurfaceDesc& srcDesc, SkBackingFit, SkBudgeted,
                   const void* srcData, size_t srcRowBytes, uint32_t flags);
    // Wrapped version
    GrTextureProxy(sk_sp<GrSurface>, GrSurfaceOrigin);

    ~GrTextureProxy() override;

    SkDestinationSurfaceColorMode mipColorMode() const { return fMipColorMode;  }

    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

private:
    bool fIsMipMapped;
    SkDestinationSurfaceColorMode fMipColorMode;

    GrUniqueKey fUniqueKey;
    GrResourceProvider* fProvider; // only set when fUniqueKey is valid

    size_t onUninstantiatedGpuMemorySize() const override;

    friend class GrTextureProxyPriv;

    // Method made available via GrTextureProxyPriv
    void setUniqueKey(GrResourceProvider*, const GrUniqueKey&);

    // For wrapped proxies the GrTexture pointer is stored in GrIORefProxy.
    // For deferred proxies that pointer will be filled in when we need to instantiate
    // the deferred resource

    typedef GrSurfaceProxy INHERITED;
};

#endif
