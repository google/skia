/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProxyProvider.h"

#include "GrCaps.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "GrTextureProxyCacheAccess.h"
#include "../private/GrSingleOwner.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fSingleOwner);)

GrProxyProvider::GrProxyProvider(GrResourceProvider* resourceProvider,
                                 GrResourceCache* resourceCache,
                                 sk_sp<const GrCaps> caps,
                                 GrSingleOwner* owner)
        : fResourceProvider(resourceProvider)
        , fResourceCache(resourceCache)
        , fCaps(caps)
#ifdef SK_DEBUG
        , fSingleOwner(owner)
#endif
{

}

GrProxyProvider::~GrProxyProvider() {
    SkASSERT(!fUniquelyKeyedProxies.count());
}

void GrProxyProvider::assignUniqueKeyToProxy(const GrUniqueKey& key, GrTextureProxy* proxy) {
    ASSERT_SINGLE_OWNER
    SkASSERT(key.isValid());
    if (this->isAbandoned() || !proxy) {
        return;
    }

    // If there is already a GrResource with this key then the caller has violated the normal
    // usage pattern of uniquely keyed resources (e.g., they have created one w/o first seeing
    // if it already existed in the cache).
    SkASSERT(!fResourceCache->findAndRefUniqueResource(key));

    // Uncached resources can never have a unique key, unless they're wrapped resources. Wrapped
    // resources are a special case: the unique keys give us a weak ref so that we can reuse the
    // same resource (rather than re-wrapping). When a wrapped resource is no longer referenced,
    // it will always be released - it is never converted to a scratch resource.
    if (SkBudgeted::kNo == proxy->isBudgeted() &&
                    (!proxy->priv().isInstantiated() ||
                     !proxy->priv().peekSurface()->resourcePriv().refsWrappedObjects())) {
        return;
    }

    SkASSERT(!fUniquelyKeyedProxies.find(key));     // multiple proxies can't get the same key

    proxy->cacheAccess().setUniqueKey(this, key);
    SkASSERT(proxy->getUniqueKey() == key);
    fUniquelyKeyedProxies.add(proxy);
}

void GrProxyProvider::adoptUniqueKeyFromSurface(GrTextureProxy* proxy, const GrSurface* surf) {
    SkASSERT(surf->getUniqueKey().isValid());
    proxy->cacheAccess().setUniqueKey(this, surf->getUniqueKey());
    SkASSERT(proxy->getUniqueKey() == surf->getUniqueKey());
    // multiple proxies can't get the same key
    SkASSERT(!fUniquelyKeyedProxies.find(surf->getUniqueKey()));
    fUniquelyKeyedProxies.add(proxy);
}

void GrProxyProvider::removeUniqueKeyFromProxy(const GrUniqueKey& key, GrTextureProxy* proxy) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned() || !proxy) {
        return;
    }
    this->processInvalidProxyUniqueKey(key, proxy, true);
}

sk_sp<GrTextureProxy> GrProxyProvider::findProxyByUniqueKey(const GrUniqueKey& key,
                                                            GrSurfaceOrigin origin) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> result = sk_ref_sp(fUniquelyKeyedProxies.find(key));
    if (result) {
        SkASSERT(result->origin() == origin);
    }
    return result;
}

sk_sp<GrTextureProxy> GrProxyProvider::findOrCreateProxyByUniqueKey(const GrUniqueKey& key,
                                                                    GrSurfaceOrigin origin) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> result = this->findProxyByUniqueKey(key, origin);
    if (result) {
        return result;
    }

    GrGpuResource* resource = fResourceCache->findAndRefUniqueResource(key);
    if (!resource) {
        return nullptr;
    }

    sk_sp<GrTexture> texture(static_cast<GrSurface*>(resource)->asTexture());
    SkASSERT(texture);

    result = GrSurfaceProxy::MakeWrapped(std::move(texture), origin);
    SkASSERT(result->getUniqueKey() == key);
    // MakeWrapped should've added this for us
    SkASSERT(fUniquelyKeyedProxies.find(key));
    return result;
}

sk_sp<GrTextureProxy> GrProxyProvider::createTextureProxy(const GrSurfaceDesc& desc,
                                                          SkBudgeted budgeted,
                                                          const GrMipLevel& mipLevel) {
    ASSERT_SINGLE_OWNER

    sk_sp<GrTexture> tex = fResourceProvider->createTexture(desc, budgeted, mipLevel);
    if (!tex) {
        return nullptr;
    }

    return GrSurfaceProxy::MakeWrapped(std::move(tex), desc.fOrigin);
}

sk_sp<GrTextureProxy> GrProxyProvider::createTextureProxy(
                                                    const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                                    const GrMipLevel texels[], int mipLevelCount,
                                                    SkDestinationSurfaceColorMode mipColorMode) {
    sk_sp<GrTexture> tex(fResourceProvider->createTexture(desc, budgeted,
                                                          texels, mipLevelCount,
                                                          mipColorMode));
    if (!tex) {
        return nullptr;
    }

    return GrSurfaceProxy::MakeWrapped(std::move(tex), desc.fOrigin);
}

bool GrProxyProvider::IsFunctionallyExact(GrSurfaceProxy* proxy) {
    return proxy->priv().isExact() || (SkIsPow2(proxy->width()) && SkIsPow2(proxy->height()));
}

void GrProxyProvider::processInvalidProxyUniqueKey(const GrUniqueKey& key) {
    // Note: this method is called for the whole variety of GrGpuResources so often 'key'
    // will not be in 'fUniquelyKeyedProxies'.
    GrTextureProxy* proxy = fUniquelyKeyedProxies.find(key);
    if (proxy) {
        this->processInvalidProxyUniqueKey(key, proxy, false);
    }
}

void GrProxyProvider::processInvalidProxyUniqueKey(const GrUniqueKey& key, GrTextureProxy* proxy,
                                                   bool invalidateSurface) {
    SkASSERT(proxy);
    SkASSERT(proxy->getUniqueKey().isValid());
    SkASSERT(proxy->getUniqueKey() == key);

    fUniquelyKeyedProxies.remove(key);
    proxy->cacheAccess().clearUniqueKey();

    if (invalidateSurface && proxy->priv().isInstantiated()) {
        GrSurface* surface = proxy->priv().peekSurface();
        if (surface) {
            surface->resourcePriv().removeUniqueKey();
        }
    }
}

void GrProxyProvider::removeAllUniqueKeys() {
    UniquelyKeyedProxyHash::Iter iter(&fUniquelyKeyedProxies);
    for (UniquelyKeyedProxyHash::Iter iter(&fUniquelyKeyedProxies); !iter.done(); ++iter) {
        GrTextureProxy& tmp = *iter;

        this->processInvalidProxyUniqueKey(tmp.getUniqueKey(), &tmp, false);
    }
    SkASSERT(!fUniquelyKeyedProxies.count());
}
