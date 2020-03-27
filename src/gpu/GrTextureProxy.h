/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureProxy_DEFINED
#define GrTextureProxy_DEFINED

#include "src/gpu/GrSamplerState.h"
#include "src/gpu/GrSurfaceProxy.h"

class GrCaps;
class GrDeferredProxyUploader;
class GrProxyProvider;
class GrResourceProvider;
class GrTextureProxyPriv;

// This class delays the acquisition of textures until they are actually required
class GrTextureProxy : virtual public GrSurfaceProxy {
public:
    GrTextureProxy* asTextureProxy() override { return this; }
    const GrTextureProxy* asTextureProxy() const override { return this; }

    // Actually instantiate the backing texture, if necessary
    bool instantiate(GrResourceProvider*) override;

    // If we are instantiated and have a target, return the mip state of that target. Otherwise
    // returns the proxy's mip state from creation time. This is useful for lazy proxies which may
    // claim to not need mips at creation time, but the instantiation happens to give us a mipped
    // target. In that case we should use that for our benefit to avoid possible copies/mip
    // generation later.
    GrMipMapped mipMapped() const;

    bool mipMapsAreDirty() const {
        SkASSERT((GrMipMapped::kNo == fMipMapped) ==
                 (GrMipMapsStatus::kNotAllocated == fMipMapsStatus));
        return GrMipMapped::kYes == fMipMapped && GrMipMapsStatus::kValid != fMipMapsStatus;
    }
    void markMipMapsDirty() {
        SkASSERT(GrMipMapped::kYes == fMipMapped);
        fMipMapsStatus = GrMipMapsStatus::kDirty;
    }
    void markMipMapsClean() {
        SkASSERT(GrMipMapped::kYes == fMipMapped);
        fMipMapsStatus = GrMipMapsStatus::kValid;
    }

    // Returns the GrMipMapped value of the proxy from creation time regardless of whether it has
    // been instantiated or not.
    GrMipMapped proxyMipMapped() const { return fMipMapped; }

    GrTextureType textureType() const { return this->backendFormat().textureType(); }

    /** If true then the texture does not support MIP maps and only supports clamp wrap mode. */
    bool hasRestrictedSampling() const {
        return GrTextureTypeHasRestrictedSampling(this->textureType());
    }

    // Returns the highest allowed filter mode for a given texture type
    static GrSamplerState::Filter HighestFilterMode(const GrTextureType textureType);

    // Returns true if the passed in proxies can be used as dynamic state together when flushing
    // draws to the gpu. This accepts GrSurfaceProxy since the information needed is defined on
    // that type, but this function exists in GrTextureProxy because it's only relevant when the
    // proxies are being used as textures.
    static bool ProxiesAreCompatibleAsDynamicState(const GrSurfaceProxy* first,
                                                   const GrSurfaceProxy* second);

    /**
     * Return the texture proxy's unique key. It will be invalid if the proxy doesn't have one.
     */
    const GrUniqueKey& getUniqueKey() const {
#ifdef SK_DEBUG
        if (this->isInstantiated() && fUniqueKey.isValid() && fSyncTargetKey) {
            GrSurface* surface = this->peekSurface();
            SkASSERT(surface);

            SkASSERT(surface->getUniqueKey().isValid());
            // It is possible for a non-keyed proxy to have a uniquely keyed resource assigned to
            // it. This just means that a future user of the resource will be filling it with unique
            // data. However, if the proxy has a unique key its attached resource should also
            // have that key.
            SkASSERT(fUniqueKey == surface->getUniqueKey());
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
    friend class GrSurfaceProxy;   // for ctors
    friend class GrProxyProvider;  // for ctors
    friend class GrTextureProxyPriv;
    friend class GrSurfaceProxyPriv;  // ability to change key sync state after lazy instantiation.

    // Deferred version - no data.
    GrTextureProxy(const GrBackendFormat&,
                   SkISize,
                   GrMipMapped,
                   GrMipMapsStatus,
                   SkBackingFit,
                   SkBudgeted,
                   GrProtected,
                   GrInternalSurfaceFlags,
                   UseAllocator);

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
    GrTextureProxy(LazyInstantiateCallback&&,
                   const GrBackendFormat&,
                   SkISize,
                   GrMipMapped,
                   GrMipMapsStatus,
                   SkBackingFit,
                   SkBudgeted,
                   GrProtected,
                   GrInternalSurfaceFlags,
                   UseAllocator);

    // Wrapped version
    GrTextureProxy(sk_sp<GrSurface>, UseAllocator);

    ~GrTextureProxy() override;

    sk_sp<GrSurface> createSurface(GrResourceProvider*) const override;

    void setTargetKeySync(bool sync) { fSyncTargetKey = sync; }

private:
    // WARNING: Be careful when adding or removing fields here. ASAN is likely to trigger warnings
    // when instantiating GrTextureRenderTargetProxy. The std::function in GrSurfaceProxy makes
    // each class in the diamond require 16 byte alignment. Clang appears to layout the fields for
    // each class to achieve the necessary alignment. However, ASAN checks the alignment of 'this'
    // in the constructors, and always looks for the full 16 byte alignment, even if the fields in
    // that particular class don't require it. Changing the size of this object can move the start
    // address of other types, leading to this problem.

    GrMipMapped      fMipMapped;

    // This tracks the mipmap status at the proxy level and is thus somewhat distinct from the
    // backing GrTexture's mipmap status. In particular, this status is used to determine when
    // mipmap levels need to be explicitly regenerated during the execution of a DAG of opsTasks.
    GrMipMapsStatus  fMipMapsStatus;
    // TEMPORARY: We are in the process of moving GrMipMapsStatus from the texture to the proxy.
    // We track the fInitialMipMapsStatus here so we can assert that the proxy did indeed expect
    // the correct mipmap status immediately after instantiation.
    //
    // NOTE: fMipMapsStatus may no longer be equal to fInitialMipMapsStatus by the time the texture
    // is instantiated, since it tracks mipmaps in the time frame in which the DAG is being built.
    SkDEBUGCODE(const GrMipMapsStatus fInitialMipMapsStatus;)

    bool             fSyncTargetKey = true;  // Should target's unique key be sync'ed with ours.

    GrUniqueKey      fUniqueKey;
    GrProxyProvider* fProxyProvider; // only set when fUniqueKey is valid

    // Only used for proxies whose contents are being prepared on a worker thread. This object
    // stores the texture data, allowing the proxy to remain uninstantiated until flush. At that
    // point, the proxy is instantiated, and this data is used to perform an ASAP upload.
    std::unique_ptr<GrDeferredProxyUploader> fDeferredUploader;

    size_t onUninstantiatedGpuMemorySize(const GrCaps&) const override;

    // Methods made available via GrTextureProxy::CacheAccess
    void setUniqueKey(GrProxyProvider*, const GrUniqueKey&);
    void clearUniqueKey();

    SkDEBUGCODE(void onValidateSurface(const GrSurface*) override;)

    typedef GrSurfaceProxy INHERITED;
};

#endif
