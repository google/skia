/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProxyProvider.h"

#include "GrCaps.h"
#include "GrRenderTarget.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "GrTextureProxyCacheAccess.h"
#include "GrTextureRenderTargetProxy.h"
#include "../private/GrSingleOwner.h"
#include "SkMipMap.h"

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

sk_sp<GrTextureProxy> GrProxyProvider::createInstantiatedProxy(const GrSurfaceDesc& desc,
                                                               SkBackingFit fit,
                                                               SkBudgeted budgeted,
                                                               uint32_t flags) {
    sk_sp<GrTexture> tex;

    if (SkBackingFit::kApprox == fit) {
        tex = fResourceProvider->createApproxTexture(desc, flags);
    } else {
        tex = fResourceProvider->createTexture(desc, budgeted, flags);
    }
    if (!tex) {
        return nullptr;
    }

    SkASSERT(!tex->getUniqueKey().isValid());

    if (tex->asRenderTarget()) {
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(std::move(tex), desc.fOrigin));
    }

    return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex), desc.fOrigin));
}

sk_sp<GrTextureProxy> GrProxyProvider::createTextureProxy(const GrSurfaceDesc& desc,
                                                          SkBudgeted budgeted,
                                                          const void* srcData, size_t rowBytes) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (srcData) {
        GrMipLevel mipLevel = { srcData, rowBytes };

        sk_sp<GrTexture> tex = fResourceProvider->createTexture(desc, budgeted, mipLevel);
        if (!tex) {
            return nullptr;
        }

        return GrSurfaceProxy::MakeWrapped(std::move(tex), desc.fOrigin);
    }

    return this->createProxy(desc, SkBackingFit::kExact, budgeted);
}

sk_sp<GrTextureProxy> GrProxyProvider::createMipMapProxy(
                                                    const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                                    const GrMipLevel texels[], int mipLevelCount,
                                                    SkDestinationSurfaceColorMode mipColorMode) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!mipLevelCount) {
        if (texels) {
            return nullptr;
        }
        return this->createProxy(desc, SkBackingFit::kExact, budgeted);
    }
    if (!texels) {
        return nullptr;
    }

    if (1 == mipLevelCount) {
        return this->createTextureProxy(desc, budgeted, texels[0].fPixels, texels[0].fRowBytes);
    }

#ifdef SK_DEBUG
    // There are only three states we want to be in when uploading data to a mipped surface.
    // 1) We have data to upload to all layers
    // 2) We are not uploading data to any layers
    // 3) We are only uploading data to the base layer
    // We check here to make sure we do not have any other state.
    bool firstLevelHasData = SkToBool(texels[0].fPixels);
    bool allOtherLevelsHaveData = true, allOtherLevelsLackData = true;
    for  (int i = 1; i < mipLevelCount; ++i) {
        if (texels[i].fPixels) {
            allOtherLevelsLackData = false;
        } else {
            allOtherLevelsHaveData = false;
        }
    }
    SkASSERT((firstLevelHasData && allOtherLevelsHaveData) || allOtherLevelsLackData);
#endif

    sk_sp<GrTexture> tex(fResourceProvider->createTexture(desc, budgeted,
                                                          texels, mipLevelCount,
                                                          mipColorMode));
    if (!tex) {
        return nullptr;
    }

    return GrSurfaceProxy::MakeWrapped(std::move(tex), desc.fOrigin);
}

sk_sp<GrTextureProxy> GrProxyProvider::createMipMapProxy(const GrSurfaceDesc& desc,
                                                         SkBudgeted budgeted) {
    // SkMipMap doesn't include the base level in the level count so we have to add 1
    int mipCount = SkMipMap::ComputeLevelCount(desc.fWidth, desc.fHeight) + 1;

    std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipCount]);

    // We don't want to upload any texel data
    for (int i = 0; i < mipCount; i++) {
        texels[i].fPixels = nullptr;
        texels[i].fRowBytes = 0;
    }

    return this->createMipMapProxy(desc, budgeted, texels.get(), mipCount,
                                   SkDestinationSurfaceColorMode::kLegacy);
}

sk_sp<GrTextureProxy> GrProxyProvider::createProxy(const GrSurfaceDesc& desc,
                                                   SkBackingFit fit,
                                                   SkBudgeted budgeted,
                                                   uint32_t flags) {
    SkASSERT(0 == flags || GrResourceProvider::kNoPendingIO_Flag == flags);

    const GrCaps* caps = this->caps();

    // TODO: move this logic into GrResourceProvider!
    // TODO: share this testing code with check_texture_creation_params
    if (!caps->isConfigTexturable(desc.fConfig)) {
        return nullptr;
    }

    bool willBeRT = SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag);
    if (willBeRT && !caps->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return nullptr;
    }

    // We currently do not support multisampled textures
    if (!willBeRT && desc.fSampleCnt > 0) {
        return nullptr;
    }

    int maxSize;
    if (willBeRT) {
        maxSize = caps->maxRenderTargetSize();
    } else {
        maxSize = caps->maxTextureSize();
    }

    if (desc.fWidth > maxSize || desc.fHeight > maxSize || desc.fWidth <= 0 || desc.fHeight <= 0) {
        return nullptr;
    }

    GrSurfaceDesc copyDesc = desc;
    copyDesc.fSampleCnt = caps->getSampleCount(desc.fSampleCnt, desc.fConfig);

#ifdef SK_DISABLE_DEFERRED_PROXIES
    // Temporarily force instantiation for crbug.com/769760 and crbug.com/769898
    sk_sp<GrTexture> tex;

    if (SkBackingFit::kApprox == fit) {
        tex = resourceProvider->createApproxTexture(copyDesc, flags);
    } else {
        tex = resourceProvider->createTexture(copyDesc, budgeted, flags);
    }

    if (!tex) {
        return nullptr;
    }

    return GrSurfaceProxy::MakeWrapped(std::move(tex), copyDesc.fOrigin);
#else
    if (willBeRT) {
        // We know anything we instantiate later from this deferred path will be
        // both texturable and renderable
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(*caps, copyDesc, fit,
                                                                    budgeted, flags));
    }

    return sk_sp<GrTextureProxy>(new GrTextureProxy(copyDesc, fit, budgeted, nullptr, 0, flags));
#endif
}

sk_sp<GrTextureProxy> GrProxyProvider::createWrappedTextureProxy(const GrBackendTexture& backendTex,
                                                                 GrSurfaceOrigin origin) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    sk_sp<GrTexture> texture(fResourceProvider->wrapBackendTexture(backendTex));
    if (!texture) {
        return nullptr;
    }
    SkASSERT(!texture->asRenderTarget());   // Strictly a GrTexture

    return GrSurfaceProxy::MakeWrapped(std::move(texture), origin);
}

sk_sp<GrTextureProxy> GrProxyProvider::createWrappedTextureProxy(const GrBackendTexture& tex,
                                                                 GrSurfaceOrigin origin,
                                                                 int sampleCnt) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    sk_sp<GrTexture> texture(fResourceProvider->wrapRenderableBackendTexture(tex, sampleCnt));
    if (!texture) {
        return nullptr;
    }
    SkASSERT(texture->asRenderTarget());  // A GrTextureRenderTarget

    return GrSurfaceProxy::MakeWrapped(std::move(texture), origin);
}

sk_sp<GrSurfaceProxy> GrProxyProvider::createWrappedRenderTargetProxy(
                                                             const GrBackendRenderTarget& backendRT,
                                                             GrSurfaceOrigin origin) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    sk_sp<GrRenderTarget> rt(fResourceProvider->wrapBackendRenderTarget(backendRT));
    if (!rt) {
        return nullptr;
    }
    SkASSERT(!rt->asTexture()); // Strictly a GrRenderTarget
    SkASSERT(!rt->getUniqueKey().isValid());

    return sk_sp<GrSurfaceProxy>(new GrRenderTargetProxy(std::move(rt), origin));
}

sk_sp<GrSurfaceProxy> GrProxyProvider::createWrappedRenderTargetProxy(const GrBackendTexture& tex,
                                                                      GrSurfaceOrigin origin,
                                                                      int sampleCnt) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    sk_sp<GrRenderTarget> rt(fResourceProvider->wrapBackendTextureAsRenderTarget(tex, sampleCnt));
    if (!rt) {
        return nullptr;
    }
    SkASSERT(!rt->asTexture()); // Strictly a GrRenderTarget
    SkASSERT(!rt->getUniqueKey().isValid());

    return sk_sp<GrSurfaceProxy>(new GrRenderTargetProxy(std::move(rt), origin));
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrSurfaceDesc& desc,
                                                       GrMipMapped mipMapped,
                                                       SkBackingFit fit, SkBudgeted budgeted) {
    SkASSERT((desc.fWidth <= 0 && desc.fHeight <= 0) ||
             (desc.fWidth > 0 && desc.fHeight > 0));
    uint32_t flags = GrResourceProvider::kNoPendingIO_Flag;
    return sk_sp<GrTextureProxy>(SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags) ?
                                 new GrTextureRenderTargetProxy(std::move(callback), desc,
                                                                mipMapped, fit, budgeted, flags) :
                                 new GrTextureProxy(std::move(callback), desc, mipMapped, fit,
                                                    budgeted, flags));

}

sk_sp<GrTextureProxy> GrProxyProvider::createFullyLazyProxy(LazyInstantiateCallback&& callback,
                                                            Renderable renderable,
                                                            GrPixelConfig config) {
    GrSurfaceDesc desc;
    if (Renderable::kYes == renderable) {
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
    }
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth = -1;
    desc.fHeight = -1;
    desc.fConfig = config;
    desc.fSampleCnt = 0;

    return this->createLazyProxy(std::move(callback), desc, GrMipMapped::kNo,
                                 SkBackingFit::kApprox, SkBudgeted::kYes);

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
