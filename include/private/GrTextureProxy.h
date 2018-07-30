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
class GrDeferredProxyUploader;
class GrProxyProvider;
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

    GrSamplerState::Filter highestFilterMode() const;

    // If we are instantiated and have a target, return the mip state of that target. Otherwise
    // returns the proxy's mip state from creation time. This is useful for lazy proxies which may
    // claim to not need mips at creation time, but the instantiation happens to give us a mipped
    // target. In that case we should use that for our benefit to avoid possible copies/mip
    // generation later.
    GrMipMapped mipMapped() const;

    /**
     * Return the texture proxy's unique key. It will be invalid if the proxy doesn't have one.
     */
    const GrUniqueKey& getUniqueKey() const {
#ifdef SK_DEBUG
        if (fTarget && fUniqueKey.isValid()) {
            SkASSERT(fTarget->getUniqueKey().isValid());
            // It is possible for a non-keyed proxy to have a uniquely keyed resource assigned to
            // it. This just means that a future user of the resource will be filling it with unique
            // data. However, if the proxy has a unique key its attached resource should also
            // have that key.
            SkASSERT(fUniqueKey == fTarget->getUniqueKey());
        }
#endif

        return fUniqueKey;
    }

    /**
     * Internal-only helper class used for manipulations of the resource by the cache.
     */
    class CacheAccess;
    inline CacheAccess cacheAccess();
    inline const CacheAccess cacheAccess() const;

    // Provides access to special purpose functions.
    GrTextureProxyPriv texPriv();
    const GrTextureProxyPriv texPriv() const;

protected:
    // DDL TODO: rm the GrSurfaceProxy friending
    friend class GrSurfaceProxy; // for ctors
    friend class GrProxyProvider; // for ctors
    friend class GrTextureProxyPriv;

    // Deferred version - when constructed with data the origin is always kTopLeft.
    GrTextureProxy(const GrSurfaceDesc& srcDesc, GrMipMapped, SkBackingFit, SkBudgeted,
                   const void* srcData, size_t srcRowBytes, GrInternalSurfaceFlags);

    // Deferred version - no data.
    GrTextureProxy(const GrSurfaceDesc& srcDesc, GrSurfaceOrigin, GrMipMapped, SkBackingFit,
                   SkBudgeted, GrInternalSurfaceFlags);

    // Lazy-callback version
    // There are two main use cases for lazily-instantiated proxies:
    //   basic knowledge - width, height, config, origin are known
    //   minimal knowledge - only config is known.
    //
    // The basic knowledge version is used for DDL where we know the type of proxy we are going to
    // use, but we don't have access to the GPU yet to instantiate it.
    //
    // The minimal knowledge version is used for CCPR where we are generating an atlas but we do not
    // know the final size until flush time.
    GrTextureProxy(LazyInstantiateCallback&&, LazyInstantiationType, const GrSurfaceDesc& desc,
                   GrSurfaceOrigin, GrMipMapped, SkBackingFit, SkBudgeted,
                   GrInternalSurfaceFlags);

    // Wrapped version
    GrTextureProxy(sk_sp<GrSurface>, GrSurfaceOrigin);

    ~GrTextureProxy() override;

    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

    void setIsGLTextureRectangleOrExternal() {
        fSurfaceFlags |= GrInternalSurfaceFlags::kIsGLTextureRectangleOrExternal;
    }
    bool isGLTextureRectangleOrExternal() const {
        return fSurfaceFlags & GrInternalSurfaceFlags::kIsGLTextureRectangleOrExternal;
    }

private:
    GrMipMapped      fMipMapped;

    GrUniqueKey      fUniqueKey;
    GrProxyProvider* fProxyProvider; // only set when fUniqueKey is valid

    // Only used for proxies whose contents are being prepared on a worker thread. This object
    // stores the texture data, allowing the proxy to remain uninstantiated until flush. At that
    // point, the proxy is instantiated, and this data is used to perform an ASAP upload.
    std::unique_ptr<GrDeferredProxyUploader> fDeferredUploader;

    size_t onUninstantiatedGpuMemorySize() const override;

    // Methods made available via GrTextureProxy::CacheAccess
    void setUniqueKey(GrProxyProvider*, const GrUniqueKey&);
    void clearUniqueKey();

    SkDEBUGCODE(void onValidateSurface(const GrSurface*) override;)

    // For wrapped proxies the GrTexture pointer is stored in GrIORefProxy.
    // For deferred proxies that pointer will be filled in when we need to instantiate
    // the deferred resource

    typedef GrSurfaceProxy INHERITED;
};

#endif
