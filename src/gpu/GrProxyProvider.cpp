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
#include "SkBitmap.h"
#include "SkGr.h"
#include "SkImage.h"
#include "SkImage_Base.h"
#include "SkImageInfoPriv.h"
#include "SkImagePriv.h"
#include "SkMipMap.h"
#include "SkTraceEvent.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fSingleOwner);)

GrProxyProvider::GrProxyProvider(GrResourceProvider* resourceProvider,
                                 GrResourceCache* resourceCache,
                                 sk_sp<const GrCaps> caps,
                                 GrSingleOwner* owner)
        : fResourceProvider(resourceProvider)
        , fResourceCache(resourceCache)
        , fAbandoned(false)
        , fCaps(caps)
        , fContextUniqueID(resourceCache->contextUniqueID())
#ifdef SK_DEBUG
        , fSingleOwner(owner)
#endif
{
    SkASSERT(fResourceProvider);
    SkASSERT(fResourceCache);
    SkASSERT(fCaps);
    SkASSERT(fSingleOwner);
}

GrProxyProvider::GrProxyProvider(uint32_t contextUniqueID,
                                 sk_sp<const GrCaps> caps,
                                 GrSingleOwner* owner)
        : fResourceProvider(nullptr)
        , fResourceCache(nullptr)
        , fAbandoned(false)
        , fCaps(caps)
        , fContextUniqueID(contextUniqueID)
#ifdef SK_DEBUG
        , fSingleOwner(owner)
#endif
{
    SkASSERT(fContextUniqueID != SK_InvalidUniqueID);
    SkASSERT(fCaps);
    SkASSERT(fSingleOwner);
}

GrProxyProvider::~GrProxyProvider() {
    SkASSERT(!fUniquelyKeyedProxies.count());
}

bool GrProxyProvider::assignUniqueKeyToProxy(const GrUniqueKey& key, GrTextureProxy* proxy) {
    ASSERT_SINGLE_OWNER
    SkASSERT(key.isValid());
    if (this->isAbandoned() || !proxy) {
        return false;
    }

    // If there is already a GrResource with this key then the caller has violated the normal
    // usage pattern of uniquely keyed resources (e.g., they have created one w/o first seeing
    // if it already existed in the cache).
    SkASSERT(!fResourceCache || !fResourceCache->findAndRefUniqueResource(key));

    // Uncached resources can never have a unique key, unless they're wrapped resources. Wrapped
    // resources are a special case: the unique keys give us a weak ref so that we can reuse the
    // same resource (rather than re-wrapping). When a wrapped resource is no longer referenced,
    // it will always be released - it is never converted to a scratch resource.
    if (SkBudgeted::kNo == proxy->isBudgeted() &&
                    (!proxy->priv().isInstantiated() ||
                     !proxy->priv().peekSurface()->resourcePriv().refsWrappedObjects())) {
        return false;
    }

    SkASSERT(!fUniquelyKeyedProxies.find(key));     // multiple proxies can't get the same key

    proxy->cacheAccess().setUniqueKey(this, key);
    SkASSERT(proxy->getUniqueKey() == key);
    fUniquelyKeyedProxies.add(proxy);
    return true;
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

sk_sp<GrTextureProxy> GrProxyProvider::createWrapped(sk_sp<GrTexture> tex, GrSurfaceOrigin origin) {
#ifdef SK_DEBUG
    if (tex->getUniqueKey().isValid()) {
        SkASSERT(!this->findProxyByUniqueKey(tex->getUniqueKey(), origin));
    }
#endif

    if (tex->asRenderTarget()) {
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(std::move(tex), origin));
    } else {
        return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex), origin));
    }
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

    if (!fResourceCache) {
        return nullptr;
    }

    GrGpuResource* resource = fResourceCache->findAndRefUniqueResource(key);
    if (!resource) {
        return nullptr;
    }

    sk_sp<GrTexture> texture(static_cast<GrSurface*>(resource)->asTexture());
    SkASSERT(texture);

    result = this->createWrapped(std::move(texture), origin);
    SkASSERT(result->getUniqueKey() == key);
    // createWrapped should've added this for us
    SkASSERT(fUniquelyKeyedProxies.find(key));
    return result;
}

sk_sp<GrTextureProxy> GrProxyProvider::createInstantiatedProxy(const GrSurfaceDesc& desc,
                                                               GrSurfaceOrigin origin,
                                                               SkBackingFit fit,
                                                               SkBudgeted budgeted,
                                                               GrSurfaceDescFlags descFlags) {
    sk_sp<GrTexture> tex;

    if (SkBackingFit::kApprox == fit) {
        tex = fResourceProvider->createApproxTexture(desc, descFlags);
    } else {
        tex = fResourceProvider->createTexture(desc, budgeted, descFlags);
    }
    if (!tex) {
        return nullptr;
    }

    return this->createWrapped(std::move(tex), origin);
}

sk_sp<GrTextureProxy> GrProxyProvider::createTextureProxy(const GrSurfaceDesc& desc,
                                                          SkBudgeted budgeted, const void* srcData,
                                                          size_t rowBytes) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (srcData) {
        GrMipLevel mipLevel = { srcData, rowBytes };

        sk_sp<GrTexture> tex =
                fResourceProvider->createTexture(desc, budgeted, SkBackingFit::kExact, mipLevel);
        if (!tex) {
            return nullptr;
        }

        return this->createWrapped(std::move(tex), kTopLeft_GrSurfaceOrigin);
    }

    return this->createProxy(desc, kTopLeft_GrSurfaceOrigin, SkBackingFit::kExact, budgeted);
}

sk_sp<GrTextureProxy> GrProxyProvider::createTextureProxy(sk_sp<SkImage> srcImage,
                                                          GrSurfaceDescFlags descFlags,
                                                          int sampleCnt,
                                                          SkBudgeted budgeted,
                                                          SkBackingFit fit) {
    ASSERT_SINGLE_OWNER
    SkASSERT(srcImage);

    if (this->isAbandoned()) {
        return nullptr;
    }

    GrPixelConfig config = SkImageInfo2GrPixelConfig(as_IB(srcImage)->onImageInfo());

    if (kUnknown_GrPixelConfig == config) {
        return nullptr;
    }

    if (SkToBool(descFlags & kRenderTarget_GrSurfaceFlag)) {
        sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, config);
        if (!sampleCnt) {
            return nullptr;
        }
    }

    GrInternalSurfaceFlags surfaceFlags = GrInternalSurfaceFlags::kNone;
    if (SkToBool(descFlags & kRenderTarget_GrSurfaceFlag)) {
        if (fCaps->usesMixedSamples() && sampleCnt > 1) {
            surfaceFlags |= GrInternalSurfaceFlags::kMixedSampled;
        }
        if (fCaps->maxWindowRectangles() > 0) {
            surfaceFlags |= GrInternalSurfaceFlags::kWindowRectsSupport;
        }
    }

    GrSurfaceDesc desc;
    desc.fWidth = srcImage->width();
    desc.fHeight = srcImage->height();
    desc.fFlags = descFlags;
    desc.fSampleCnt = sampleCnt;
    desc.fConfig = config;

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [desc, budgeted, srcImage, fit](GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    // Nothing to clean up here. Once the proxy (and thus lambda) is deleted the ref
                    // on srcImage will be released.
                    return sk_sp<GrTexture>();
                }
                SkPixmap pixMap;
                SkAssertResult(srcImage->peekPixels(&pixMap));
                GrMipLevel mipLevel = { pixMap.addr(), pixMap.rowBytes() };

                return resourceProvider->createTexture(desc, budgeted, fit, mipLevel);
            },
            desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, surfaceFlags, fit, budgeted);

    if (!proxy) {
        return nullptr;
    }

    if (fResourceProvider) {
        // In order to reuse code we always create a lazy proxy. When we aren't in DDL mode however
        // we're better off instantiating the proxy immediately here.
        if (!proxy->priv().doLazyInstantiation(fResourceProvider)) {
            return nullptr;
        }
    }

    SkASSERT(proxy->width() == desc.fWidth);
    SkASSERT(proxy->height() == desc.fHeight);
    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::createMipMapProxy(const GrSurfaceDesc& desc,
                                                         GrSurfaceOrigin origin,
                                                         SkBudgeted budgeted) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    return this->createProxy(desc, origin, GrMipMapped::kYes, SkBackingFit::kExact, budgeted,
                             GrInternalSurfaceFlags::kNone);
}

sk_sp<GrTextureProxy> GrProxyProvider::createMipMapProxyFromBitmap(const SkBitmap& bitmap) {
    if (!SkImageInfoIsValid(bitmap.info())) {
        return nullptr;
    }

    SkPixmap pixmap;
    if (!bitmap.peekPixels(&pixmap)) {
        return nullptr;
    }

    ATRACE_ANDROID_FRAMEWORK("Upload MipMap Texture [%ux%u]", pixmap.width(), pixmap.height());
    sk_sp<SkMipMap> mipmaps(SkMipMap::Build(pixmap, nullptr));
    if (!mipmaps) {
        return nullptr;
    }

    if (mipmaps->countLevels() < 0) {
        return nullptr;
    }

    // In non-ddl we will always instantiate right away. Thus we never want to copy the SkBitmap
    // even if its mutable. In ddl, if the bitmap is mutable then we must make a copy since the
    // upload of the data to the gpu can happen at anytime and the bitmap may change by then.
    SkCopyPixelsMode copyMode = this->recordingDDL() ? kIfMutable_SkCopyPixelsMode
                                                     : kNever_SkCopyPixelsMode;
    sk_sp<SkImage> baseLevel = SkMakeImageFromRasterBitmap(bitmap, copyMode);

    if (!baseLevel) {
        return nullptr;
    }

    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(pixmap.info());

    if (0 == mipmaps->countLevels()) {
        return this->createTextureProxy(baseLevel, kNone_GrSurfaceFlags, 1, SkBudgeted::kYes,
                                        SkBackingFit::kExact);
    }

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [desc, baseLevel, mipmaps](GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    return sk_sp<GrTexture>();
                }

                const int mipLevelCount = mipmaps->countLevels() + 1;
                std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipLevelCount]);

                SkPixmap pixmap;
                SkAssertResult(baseLevel->peekPixels(&pixmap));

                // DDL TODO: Instead of copying all this info into GrMipLevels we should just plumb
                // the use of SkMipMap down through Ganesh.
                texels[0].fPixels = pixmap.addr();
                texels[0].fRowBytes = pixmap.rowBytes();

                for (int i = 1; i < mipLevelCount; ++i) {
                    SkMipMap::Level generatedMipLevel;
                    mipmaps->getLevel(i - 1, &generatedMipLevel);
                    texels[i].fPixels = generatedMipLevel.fPixmap.addr();
                    texels[i].fRowBytes = generatedMipLevel.fPixmap.rowBytes();
                    SkASSERT(texels[i].fPixels);
                }

                return resourceProvider->createTexture(desc, SkBudgeted::kYes, texels.get(),
                                                       mipLevelCount);
            },
            desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kYes, SkBackingFit::kExact,
            SkBudgeted::kYes);

    if (!proxy) {
        return nullptr;
    }

    if (fResourceProvider) {
        // In order to reuse code we always create a lazy proxy. When we aren't in DDL mode however
        // we're better off instantiating the proxy immediately here.
        if (!proxy->priv().doLazyInstantiation(fResourceProvider)) {
            return nullptr;
        }
    }
    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::createProxy(const GrSurfaceDesc& desc,
                                                   GrSurfaceOrigin origin,
                                                   GrMipMapped mipMapped,
                                                   SkBackingFit fit,
                                                   SkBudgeted budgeted,
                                                   GrInternalSurfaceFlags surfaceFlags) {
    if (GrMipMapped::kYes == mipMapped) {
        // SkMipMap doesn't include the base level in the level count so we have to add 1
        int mipCount = SkMipMap::ComputeLevelCount(desc.fWidth, desc.fHeight) + 1;
        if (1 == mipCount) {
            mipMapped = GrMipMapped::kNo;
        }
    }

    if (!this->caps()->validateSurfaceDesc(desc, mipMapped)) {
        return nullptr;
    }
    GrSurfaceDesc copyDesc = desc;
    if (desc.fFlags & kRenderTarget_GrSurfaceFlag) {
        copyDesc.fSampleCnt =
                this->caps()->getRenderTargetSampleCount(desc.fSampleCnt, desc.fConfig);
    }

    if (copyDesc.fFlags & kRenderTarget_GrSurfaceFlag) {
        // We know anything we instantiate later from this deferred path will be
        // both texturable and renderable
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(
                *this->caps(), copyDesc, origin, mipMapped, fit, budgeted, surfaceFlags));
    }

    return sk_sp<GrTextureProxy>(
            new GrTextureProxy(copyDesc, origin, mipMapped, fit, budgeted, surfaceFlags));
}

sk_sp<GrTextureProxy> GrProxyProvider::wrapBackendTexture(const GrBackendTexture& backendTex,
                                                          GrSurfaceOrigin origin,
                                                          GrWrapOwnership ownership,
                                                          ReleaseProc releaseProc,
                                                          ReleaseContext releaseCtx) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    if (!fResourceProvider) {
        return nullptr;
    }

    sk_sp<GrTexture> tex = fResourceProvider->wrapBackendTexture(backendTex, ownership);
    if (!tex) {
        return nullptr;
    }

    sk_sp<GrReleaseProcHelper> releaseHelper;
    if (releaseProc) {
        releaseHelper.reset(new GrReleaseProcHelper(releaseProc, releaseCtx));
        // This gives the texture a ref on the releaseHelper
        tex->setRelease(releaseHelper);
    }

    SkASSERT(!tex->asRenderTarget());  // Strictly a GrTexture
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(SkBudgeted::kNo == tex->resourcePriv().isBudgeted());

    return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex), origin));
}

sk_sp<GrTextureProxy> GrProxyProvider::wrapRenderableBackendTexture(
        const GrBackendTexture& backendTex, GrSurfaceOrigin origin, int sampleCnt,
        GrWrapOwnership ownership) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    if (!fResourceProvider) {
        return nullptr;
    }

    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, backendTex.config());
    if (!sampleCnt) {
        return nullptr;
    }

    sk_sp<GrTexture> tex =
            fResourceProvider->wrapRenderableBackendTexture(backendTex, sampleCnt, ownership);
    if (!tex) {
        return nullptr;
    }

    SkASSERT(tex->asRenderTarget());  // A GrTextureRenderTarget
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(SkBudgeted::kNo == tex->resourcePriv().isBudgeted());

    return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(std::move(tex), origin));
}

sk_sp<GrSurfaceProxy> GrProxyProvider::wrapBackendRenderTarget(
        const GrBackendRenderTarget& backendRT, GrSurfaceOrigin origin) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    if (!fResourceProvider) {
        return nullptr;
    }

    sk_sp<GrRenderTarget> rt = fResourceProvider->wrapBackendRenderTarget(backendRT);
    if (!rt) {
        return nullptr;
    }
    SkASSERT(!rt->asTexture());  // A GrRenderTarget that's not textureable
    SkASSERT(!rt->getUniqueKey().isValid());
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(SkBudgeted::kNo == rt->resourcePriv().isBudgeted());

    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(std::move(rt), origin));
}

sk_sp<GrSurfaceProxy> GrProxyProvider::wrapBackendTextureAsRenderTarget(
        const GrBackendTexture& backendTex, GrSurfaceOrigin origin, int sampleCnt) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    if (!fResourceProvider) {
        return nullptr;
    }

    sk_sp<GrRenderTarget> rt =
            fResourceProvider->wrapBackendTextureAsRenderTarget(backendTex, sampleCnt);
    if (!rt) {
        return nullptr;
    }
    SkASSERT(!rt->asTexture());  // A GrRenderTarget that's not textureable
    SkASSERT(!rt->getUniqueKey().isValid());
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(SkBudgeted::kNo == rt->resourcePriv().isBudgeted());

    return sk_sp<GrSurfaceProxy>(new GrRenderTargetProxy(std::move(rt), origin));
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrSurfaceDesc& desc,
                                                       GrSurfaceOrigin origin,
                                                       GrMipMapped mipMapped, SkBackingFit fit,
                                                       SkBudgeted budgeted) {
    return this->createLazyProxy(std::move(callback), desc, origin, mipMapped,
                                 GrInternalSurfaceFlags::kNone, fit, budgeted);
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrSurfaceDesc& desc,
                                                       GrSurfaceOrigin origin,
                                                       GrMipMapped mipMapped,
                                                       GrInternalSurfaceFlags surfaceFlags,
                                                       SkBackingFit fit, SkBudgeted budgeted) {
    // For non-ddl draws always make lazy proxy's single use.
    LazyInstantiationType lazyType = fResourceProvider ? LazyInstantiationType::kSingleUse
                                                       : LazyInstantiationType::kMultipleUse;
    return this->createLazyProxy(std::move(callback), desc, origin, mipMapped, surfaceFlags,
                                 fit, budgeted, lazyType);
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrSurfaceDesc& desc,
                                                       GrSurfaceOrigin origin,
                                                       GrMipMapped mipMapped,
                                                       GrInternalSurfaceFlags surfaceFlags,
                                                       SkBackingFit fit, SkBudgeted budgeted,
                                                       LazyInstantiationType lazyType) {
    SkASSERT((desc.fWidth <= 0 && desc.fHeight <= 0) ||
             (desc.fWidth > 0 && desc.fHeight > 0));

    if (desc.fWidth > fCaps->maxTextureSize() || desc.fHeight > fCaps->maxTextureSize()) {
        return nullptr;
    }


#ifdef SK_DEBUG
    if (SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags)) {
        if (SkToBool(surfaceFlags & GrInternalSurfaceFlags::kMixedSampled)) {
            SkASSERT(fCaps->usesMixedSamples() && desc.fSampleCnt > 1);
        }
        if (SkToBool(surfaceFlags & GrInternalSurfaceFlags::kWindowRectsSupport)) {
            SkASSERT(fCaps->maxWindowRectangles() > 0);
        }
    }
#endif

    return sk_sp<GrTextureProxy>(
            SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags)
                    ? new GrTextureRenderTargetProxy(std::move(callback), lazyType, desc, origin,
                                                     mipMapped, fit, budgeted, surfaceFlags)
                    : new GrTextureProxy(std::move(callback), lazyType, desc, origin, mipMapped,
                                         fit, budgeted, surfaceFlags));
}

sk_sp<GrRenderTargetProxy> GrProxyProvider::createLazyRenderTargetProxy(
        LazyInstantiateCallback&& callback, const GrSurfaceDesc& desc, GrSurfaceOrigin origin,
        GrInternalSurfaceFlags surfaceFlags, Textureable textureable, GrMipMapped mipMapped,
        SkBackingFit fit, SkBudgeted budgeted) {
    SkASSERT((desc.fWidth <= 0 && desc.fHeight <= 0) ||
             (desc.fWidth > 0 && desc.fHeight > 0));

    if (desc.fWidth > fCaps->maxRenderTargetSize() || desc.fHeight > fCaps->maxRenderTargetSize()) {
        return nullptr;
    }

    SkASSERT(SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags));

#ifdef SK_DEBUG
    if (SkToBool(surfaceFlags & GrInternalSurfaceFlags::kMixedSampled)) {
        SkASSERT(fCaps->usesMixedSamples() && desc.fSampleCnt > 1);
    }
    if (SkToBool(surfaceFlags & GrInternalSurfaceFlags::kWindowRectsSupport)) {
        SkASSERT(fCaps->maxWindowRectangles() > 0);
    }
#endif

    using LazyInstantiationType = GrSurfaceProxy::LazyInstantiationType;
    // For non-ddl draws always make lazy proxy's single use.
    LazyInstantiationType lazyType = fResourceProvider ? LazyInstantiationType::kSingleUse
                                                       : LazyInstantiationType::kMultipleUse;

    if (Textureable::kYes == textureable) {
        return sk_sp<GrRenderTargetProxy>(
                new GrTextureRenderTargetProxy(std::move(callback), lazyType, desc, origin,
                                               mipMapped, fit, budgeted, surfaceFlags));
    }

    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(
            std::move(callback), lazyType, desc, origin, fit, budgeted, surfaceFlags));
}

sk_sp<GrTextureProxy> GrProxyProvider::MakeFullyLazyProxy(LazyInstantiateCallback&& callback,
                                                          Renderable renderable,
                                                          GrSurfaceOrigin origin,
                                                          GrPixelConfig config,
                                                          const GrCaps& caps) {
    GrSurfaceDesc desc;
    GrInternalSurfaceFlags surfaceFlags = GrInternalSurfaceFlags::kNoPendingIO;
    if (Renderable::kYes == renderable) {
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        if (caps.maxWindowRectangles() > 0) {
            surfaceFlags |= GrInternalSurfaceFlags::kWindowRectsSupport;
        }
    }
    desc.fWidth = -1;
    desc.fHeight = -1;
    desc.fConfig = config;
    desc.fSampleCnt = 1;

    return sk_sp<GrTextureProxy>(
            (Renderable::kYes == renderable)
                    ? new GrTextureRenderTargetProxy(std::move(callback),
                                                     LazyInstantiationType::kSingleUse, desc,
                                                     origin, GrMipMapped::kNo,
                                                     SkBackingFit::kApprox, SkBudgeted::kYes,
                                                     surfaceFlags)
                    : new GrTextureProxy(std::move(callback), LazyInstantiationType::kSingleUse,
                                         desc, origin, GrMipMapped::kNo, SkBackingFit::kApprox,
                                         SkBudgeted::kYes, surfaceFlags));
}

bool GrProxyProvider::IsFunctionallyExact(GrSurfaceProxy* proxy) {
    const bool isInstantiated = proxy->priv().isInstantiated();
    // A proxy is functionally exact if:
    //   it is exact (obvs)
    //   when it is instantiated it will be exact (i.e., power of two dimensions)
    //   it is already instantiated and the proxy covers the entire backing surface
    return proxy->priv().isExact() ||
           (!isInstantiated && SkIsPow2(proxy->width()) && SkIsPow2(proxy->height())) ||
           (isInstantiated && proxy->worstCaseWidth() == proxy->width() &&
                              proxy->worstCaseHeight() == proxy->height());
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
