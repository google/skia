/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProxyProvider_DEFINED
#define GrProxyProvider_DEFINED

#include "GrResourceKey.h"
#include "GrTextureProxy.h"
#include "GrTypes.h"
#include "SkRefCnt.h"
#include "SkTDynamicHash.h"

class GrCaps;
class GrResourceProvider;
class GrSingleOwner;
class GrBackendRenderTarget;

/*
 * A factory for creating GrSurfaceProxy-derived objects.
 */
class GrProxyProvider {
public:
    GrProxyProvider(GrResourceProvider*, GrResourceCache*, sk_sp<const GrCaps>, GrSingleOwner*);

    ~GrProxyProvider();

    /*
     * Assigns a unique key to a proxy. The proxy will be findable via this key using
     * findProxyByUniqueKey(). It is an error if an existing proxy already has a key.
     */
    void assignUniqueKeyToProxy(const GrUniqueKey&, GrTextureProxy*);

    /*
     * Sets the unique key of the provided proxy to the unique key of the surface. The surface must
     * have a valid unique key.
     */
    void adoptUniqueKeyFromSurface(GrTextureProxy* proxy, const GrSurface*);

    /*
     * Removes a unique key from a proxy. If the proxy has already been instantiated, it will
     * also remove the unique key from the target GrSurface.
     */
    void removeUniqueKeyFromProxy(const GrUniqueKey&, GrTextureProxy*);

    /*
     * Finds a proxy by unique key.
     */
    sk_sp<GrTextureProxy> findProxyByUniqueKey(const GrUniqueKey&, GrSurfaceOrigin);

    /*
     * Finds a proxy by unique key or creates a new one that wraps a resource matching the unique
     * key.
     */
    sk_sp<GrTextureProxy> findOrCreateProxyByUniqueKey(const GrUniqueKey&, GrSurfaceOrigin);

    /*
     * Create a texture proxy that is backed by an instantiated GrSurface. This is almost entirely
     * used by Skia's testing code.
     * DDL TODO: remove the remaining Skia-internal use of this method and make it truly
     * testing-only.
     */
    sk_sp<GrTextureProxy> createInstantiatedProxy(const GrSurfaceDesc&, SkBackingFit, SkBudgeted,
                                                  uint32_t flags = 0);

    /*
     * Create an un-mipmapped texture proxy with data.
     * DDL TODO: need to refine ownership semantics of 'srcData' if we're in completely
     * deferred mode
     */
    sk_sp<GrTextureProxy> createTextureProxy(const GrSurfaceDesc&, SkBudgeted,
                                             const void* srcData, size_t rowBytes);

    /*
     * Create a mipmapped texture proxy with data.
     *
     * @param desc          Description of the texture properties.
     * @param budgeted      Does the texture count against the resource cache budget?
     * @param texels        A contiguous array of mipmap levels
     * @param mipLevelCount The amount of elements in the texels array
     */
    sk_sp<GrTextureProxy> createMipMapProxy(const GrSurfaceDesc&, SkBudgeted,
                                            const GrMipLevel texels[], int mipLevelCount,
                                            SkDestinationSurfaceColorMode mipColorMode =
                                                            SkDestinationSurfaceColorMode::kLegacy);

    /*
     * Create a mipmapped texture proxy without any data.
     *
     * Like the call above but there are no texels to upload. A texture proxy is returned that
     * simply has space allocated for the mips. We will allocated the full amount of mip levels
     * based on the width and height in the GrSurfaceDesc.
     */
    sk_sp<GrTextureProxy> createMipMapProxy(const GrSurfaceDesc&, SkBudgeted);

    /*
     * Create a GrSurfaceProxy without any data.
     */
    sk_sp<GrTextureProxy> createProxy(const GrSurfaceDesc&, SkBackingFit, SkBudgeted,
                                      uint32_t flags = 0);

    /*
     * Create a texture proxy that wraps a (non-renderable) backend texture.
     */
    sk_sp<GrTextureProxy> createWrappedTextureProxy(const GrBackendTexture&, GrSurfaceOrigin);

    /*
     * Create a texture proxy that wraps a backend texture and is both texture-able and renderable
     */
    sk_sp<GrTextureProxy> createWrappedTextureProxy(const GrBackendTexture&,
                                                    GrSurfaceOrigin,
                                                    int sampleCnt);

    /*
     * Create a render target proxy that wraps a backend rendertarget
     */
    sk_sp<GrSurfaceProxy> createWrappedRenderTargetProxy(const GrBackendRenderTarget&,
                                                         GrSurfaceOrigin);

    /*
     * Create a render target proxy that wraps a backend texture?
     */
    sk_sp<GrSurfaceProxy> createWrappedRenderTargetProxy(const GrBackendTexture& tex,
                                                         GrSurfaceOrigin origin,
                                                         int sampleCnt);

    using LazyInstantiateCallback = std::function<sk_sp<GrTexture>(GrResourceProvider*,
                                                                   GrSurfaceOrigin* outOrigin)>;

    enum class Renderable : bool {
        kNo = false,
        kYes = true
    };

    /**
     * Creates a texture proxy that will be instantiated by a user-supplied callback during flush.
     * (Stencil is not supported by this method.) The width and height must either both be greater
     * than 0 or both less than or equal to zero. A non-positive value is a signal that the width
     * and height are currently unknown.
     *
     * When called, the callback must be able to cleanup any resources that it captured at creation.
     * It also must support being passed in a null GrResourceProvider. When this happens, the
     * callback should cleanup any resources it captured and return an empty sk_sp<GrTextureProxy>.
     */
    sk_sp<GrTextureProxy> createLazyProxy(LazyInstantiateCallback&&, const GrSurfaceDesc&,
                                          GrMipMapped, SkBackingFit, SkBudgeted);

    sk_sp<GrTextureProxy> createFullyLazyProxy(LazyInstantiateCallback&&,
                                               Renderable, GrPixelConfig);

    // 'proxy' is about to be used as a texture src or drawn to. This query can be used to
    // determine if it is going to need a texture domain or a full clear.
    static bool IsFunctionallyExact(GrSurfaceProxy* proxy);

    /**
     * Either the proxy attached to the unique key is being deleted (in which case we
     * don't want it cluttering up the hash table) or the client has indicated that
     * it will never refer to the unique key again. In either case, remove the key
     * from the hash table.
     * Note: this does not, by itself, alter unique key attached to the underlying GrTexture.
     */
    void processInvalidProxyUniqueKey(const GrUniqueKey&);

    /**
     * Same as above, but you must pass in a GrTextureProxy to save having to search for it. The
     * GrUniqueKey of the proxy must be valid and it must match the passed in key. This function
     * also gives the option to invalidate the GrUniqueKey on the underlying GrTexture.
     */
    void processInvalidProxyUniqueKey(const GrUniqueKey&, GrTextureProxy*, bool invalidateSurface);

    const GrCaps* caps() const { return fCaps.get(); }

    void abandon() {
        fResourceCache = nullptr;
        fResourceProvider = nullptr;
    }

    bool isAbandoned() const {
        SkASSERT(SkToBool(fResourceCache) == SkToBool(fResourceProvider));
        return !SkToBool(fResourceCache);
    }

    int numUniqueKeyProxies_TestOnly() const;

    void removeAllUniqueKeys();

private:
    struct UniquelyKeyedProxyHashTraits {
        static const GrUniqueKey& GetKey(const GrTextureProxy& p) { return p.getUniqueKey(); }

        static uint32_t Hash(const GrUniqueKey& key) { return key.hash(); }
    };
    typedef SkTDynamicHash<GrTextureProxy, GrUniqueKey, UniquelyKeyedProxyHashTraits> UniquelyKeyedProxyHash;

    // This holds the texture proxies that have unique keys. The resourceCache does not get a ref
    // on these proxies but they must send a message to the resourceCache when they are deleted.
    UniquelyKeyedProxyHash fUniquelyKeyedProxies;

    GrResourceProvider*    fResourceProvider;
    GrResourceCache*       fResourceCache;
    sk_sp<const GrCaps>    fCaps;

    // In debug builds we guard against improper thread handling
    SkDEBUGCODE(mutable GrSingleOwner* fSingleOwner;)
};

#endif
