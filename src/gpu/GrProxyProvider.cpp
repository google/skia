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
#ifdef SK_DEBUG
        , fSingleOwner(owner)
#endif
{

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

    return this->createWrapped(std::move(tex), desc.fOrigin);
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

        sk_sp<GrTexture> tex = fResourceProvider->createTexture(desc, budgeted,
                                                                SkBackingFit::kExact, mipLevel);
        if (!tex) {
            return nullptr;
        }

        return this->createWrapped(std::move(tex), desc.fOrigin);
    }

    return this->createProxy(desc, SkBackingFit::kExact, budgeted);
}

sk_sp<GrTextureProxy> GrProxyProvider::createTextureProxy(sk_sp<SkImage> srcImage,
                                                          GrSurfaceFlags flags,
                                                          GrSurfaceOrigin origin,
                                                          int sampleCnt,
                                                          SkBudgeted budgeted,
                                                          SkBackingFit fit) {
    ASSERT_SINGLE_OWNER
    SkASSERT(srcImage);

    if (this->isAbandoned()) {
        return nullptr;
    }

    GrPixelConfig config = SkImageInfo2GrPixelConfig(as_IB(srcImage)->onImageInfo(),
                                                     *this->caps());

    if (kUnknown_GrPixelConfig == config) {
        return nullptr;
    }

    if (SkToBool(flags & kRenderTarget_GrSurfaceFlag)) {
        sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, config);
        if (!sampleCnt) {
            return nullptr;
        }
    }

    GrRenderTargetFlags renderTargetFlags = GrRenderTargetFlags::kNone;
    if (SkToBool(flags & kRenderTarget_GrSurfaceFlag)) {
        if (fCaps->usesMixedSamples() && sampleCnt > 1) {
            renderTargetFlags |= GrRenderTargetFlags::kMixedSampled;
        }
        if (fCaps->maxWindowRectangles() > 0) {
            renderTargetFlags |= GrRenderTargetFlags::kWindowRectsSupport;
        }
    }

    GrSurfaceDesc desc;
    desc.fWidth = srcImage->width();
    desc.fHeight = srcImage->height();
    desc.fFlags = flags;
    desc.fOrigin = origin;
    desc.fSampleCnt = sampleCnt;
    desc.fConfig = config;

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [desc, budgeted, srcImage, fit]
            (GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    // Nothing to clean up here. Once the proxy (and thus lambda) is deleted the ref
                    // on srcImage will be released.
                    return sk_sp<GrTexture>();
                }
                SkPixmap pixMap;
                SkAssertResult(srcImage->peekPixels(&pixMap));
                GrMipLevel mipLevel = { pixMap.addr(), pixMap.rowBytes() };

                return resourceProvider->createTexture(desc, budgeted, fit, mipLevel);
            }, desc, GrMipMapped::kNo, renderTargetFlags, fit, budgeted);

    if (fResourceProvider) {
        // In order to reuse code we always create a lazy proxy. When we aren't in DDL mode however
        // we're better off instantiating the proxy immediately here.
        if (!proxy->priv().doLazyInstantiation(fResourceProvider)) {
            return nullptr;
        }
    }
    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::createMipMapProxy(const GrSurfaceDesc& desc,
                                                         SkBudgeted budgeted) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    return this->createProxy(desc, GrMipMapped::kYes, SkBackingFit::kExact, budgeted, 0);
}

sk_sp<GrTextureProxy> GrProxyProvider::createMipMapProxyFromBitmap(const SkBitmap& bitmap,
                                                                   SkColorSpace* dstColorSpace) {
    SkDestinationSurfaceColorMode mipColorMode = dstColorSpace
        ? SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware
        : SkDestinationSurfaceColorMode::kLegacy;

    if (!SkImageInfoIsValid(bitmap.info(), mipColorMode)) {
        return nullptr;
    }

    SkPixmap pixmap;
    if (!bitmap.peekPixels(&pixmap)) {
        return nullptr;
    }

    ATRACE_ANDROID_FRAMEWORK("Upload MipMap Texture [%ux%u]", pixmap.width(), pixmap.height());
    sk_sp<SkMipMap> mipmaps(SkMipMap::Build(pixmap, mipColorMode, nullptr));
    if (!mipmaps) {
        return nullptr;
    }

    if (mipmaps->countLevels() < 0) {
        return nullptr;
    }

    // In non-ddl we will always instantiate right away. Thus we never want to copy the SkBitmap
    // even if its mutable. In ddl, if the bitmap is mutable then we must make a copy since the
    // upload of the data to the gpu can happen at anytime and the bitmap may change by then.
    SkCopyPixelsMode copyMode = this->mutableBitmapsNeedCopy() ? kIfMutable_SkCopyPixelsMode
                                                               : kNever_SkCopyPixelsMode;
    sk_sp<SkImage> baseLevel = SkMakeImageFromRasterBitmap(bitmap, copyMode);

    if (!baseLevel) {
        return nullptr;
    }

    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(pixmap.info(), *this->caps());

    if (0 == mipmaps->countLevels()) {
        return this->createTextureProxy(baseLevel, kNone_GrSurfaceFlags, kTopLeft_GrSurfaceOrigin,
                                        1, SkBudgeted::kYes, SkBackingFit::kExact);

    }

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [desc, baseLevel, mipmaps, mipColorMode]
            (GrResourceProvider* resourceProvider) {
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
                                                       mipLevelCount, mipColorMode);
            }, desc, GrMipMapped::kYes, SkBackingFit::kExact, SkBudgeted::kYes);

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
                                                   GrMipMapped mipMapped,
                                                   SkBackingFit fit,
                                                   SkBudgeted budgeted,
                                                   uint32_t flags) {
    SkASSERT(0 == flags || GrResourceProvider::kNoPendingIO_Flag == flags);

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
        return sk_sp<GrTextureProxy>(
                new GrTextureRenderTargetProxy(*this->caps(), copyDesc, mipMapped, fit, budgeted,
                                               flags));
    }

    return sk_sp<GrTextureProxy>(new GrTextureProxy(copyDesc, mipMapped, fit, budgeted, nullptr, 0,
                                                    flags));
}

sk_sp<GrTextureProxy> GrProxyProvider::createWrappedTextureProxy(
                                                         const GrBackendTexture& backendTex,
                                                         GrSurfaceOrigin origin,
                                                         GrWrapOwnership ownership,
                                                         ReleaseProc releaseProc,
                                                         ReleaseContext releaseCtx) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fOrigin = origin;
    desc.fWidth = backendTex.width();
    desc.fHeight = backendTex.height();
    desc.fConfig = backendTex.config();
    GrMipMapped mipMapped = backendTex.hasMipMaps() ? GrMipMapped::kYes : GrMipMapped::kNo;

    sk_sp<GrReleaseProcHelper> releaseHelper;
    if (releaseProc) {
        releaseHelper.reset(new GrReleaseProcHelper(releaseProc, releaseCtx));
    }

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [backendTex, ownership, releaseHelper]
            (GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    // If this had a releaseHelper it will get unrefed when we delete this lambda
                    // and will call the release proc so that the client knows they can free the
                    // underlying backend object.
                    return sk_sp<GrTexture>();
                }

                sk_sp<GrTexture> tex = resourceProvider->wrapBackendTexture(backendTex,
                                                                            ownership);
                if (!tex) {
                    return sk_sp<GrTexture>();
                }
                if (releaseHelper) {
                    // This gives the texture a ref on the releaseHelper
                    tex->setRelease(releaseHelper);
                }
                SkASSERT(!tex->asRenderTarget());   // Strictly a GrTexture
                // Make sure we match how we created the proxy with SkBudgeted::kNo
                SkASSERT(SkBudgeted::kNo == tex->resourcePriv().isBudgeted());

                return tex;
            }, desc, mipMapped, SkBackingFit::kExact, SkBudgeted::kNo);

    if (fResourceProvider) {
        // In order to reuse code we always create a lazy proxy. When we aren't in DDL mode however,
        // we're better off instantiating the proxy immediately here.
        if (!proxy->priv().doLazyInstantiation(fResourceProvider)) {
            return nullptr;
        }
    }
    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::createWrappedTextureProxy(const GrBackendTexture& backendTex,
                                                                 GrSurfaceOrigin origin,
                                                                 int sampleCnt) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, backendTex.config());
    if (!sampleCnt) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fOrigin = origin;
    desc.fWidth = backendTex.width();
    desc.fHeight = backendTex.height();
    desc.fConfig = backendTex.config();
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fSampleCnt = sampleCnt;
    GrMipMapped mipMapped = backendTex.hasMipMaps() ? GrMipMapped::kYes : GrMipMapped::kNo;

    GrRenderTargetFlags renderTargetFlags = GrRenderTargetFlags::kNone;
    if (fCaps->usesMixedSamples() && sampleCnt > 1) {
        renderTargetFlags |= GrRenderTargetFlags::kMixedSampled;
    }
    if (fCaps->maxWindowRectangles() > 0) {
        renderTargetFlags |= GrRenderTargetFlags::kWindowRectsSupport;
    }

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [backendTex, sampleCnt] (GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    return sk_sp<GrTexture>();
                }

                sk_sp<GrTexture> tex = resourceProvider->wrapRenderableBackendTexture(backendTex,
                                                                                      sampleCnt);
                if (!tex) {
                    return sk_sp<GrTexture>();
                }
                SkASSERT(tex->asRenderTarget());   // A GrTextureRenderTarget
                // Make sure we match how we created the proxy with SkBudgeted::kNo
                SkASSERT(SkBudgeted::kNo == tex->resourcePriv().isBudgeted());

                return tex;
            }, desc, mipMapped, renderTargetFlags, SkBackingFit::kExact, SkBudgeted::kNo);

    if (fResourceProvider) {
        // In order to reuse code we always create a lazy proxy. When we aren't in DDL mode however,
        // we're better off instantiating the proxy immediately here.
        if (!proxy->priv().doLazyInstantiation(fResourceProvider)) {
            return nullptr;
        }
    }
    return proxy;
}

sk_sp<GrSurfaceProxy> GrProxyProvider::createWrappedRenderTargetProxy(
                                                             const GrBackendRenderTarget& backendRT,
                                                             GrSurfaceOrigin origin) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fOrigin = origin;
    desc.fWidth = backendRT.width();
    desc.fHeight = backendRT.height();
    desc.fConfig = backendRT.config();
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fSampleCnt = backendRT.sampleCnt();

    GrRenderTargetFlags renderTargetFlags = GrRenderTargetFlags::kNone;
    if (fCaps->isMixedSamplesSupportedForRT(backendRT) && backendRT.sampleCnt() > 1) {
        renderTargetFlags |= GrRenderTargetFlags::kMixedSampled;
    }
    if (fCaps->isWindowRectanglesSupportedForRT(backendRT)) {
        renderTargetFlags |= GrRenderTargetFlags::kWindowRectsSupport;
    }

    sk_sp<GrRenderTargetProxy> proxy = this->createLazyRenderTargetProxy(
            [backendRT] (GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    return sk_sp<GrRenderTarget>();
                }

                sk_sp<GrRenderTarget> rt = resourceProvider->wrapBackendRenderTarget(backendRT);
                if (!rt) {
                    return sk_sp<GrRenderTarget>();
                }
                SkASSERT(!rt->asTexture());   // A GrRenderTarget that's not textureable
                SkASSERT(!rt->getUniqueKey().isValid());
                // Make sure we match how we created the proxy with SkBudgeted::kNo
                SkASSERT(SkBudgeted::kNo == rt->resourcePriv().isBudgeted());

                return rt;
            }, desc, renderTargetFlags, Textureable::kNo, GrMipMapped::kNo, SkBackingFit::kExact,
               SkBudgeted::kNo);

    if (fResourceProvider) {
        // In order to reuse code we always create a lazy proxy. When we aren't in DDL mode however,
        // we're better off instantiating the proxy immediately here.
        if (!proxy->priv().doLazyInstantiation(fResourceProvider)) {
            return nullptr;
        }
    }
    return proxy;
}

sk_sp<GrSurfaceProxy> GrProxyProvider::createWrappedRenderTargetProxy(
                                                                 const GrBackendTexture& backendTex,
                                                                 GrSurfaceOrigin origin,
                                                                 int sampleCnt) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, backendTex.config());
    if (!sampleCnt) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fOrigin = origin;
    desc.fWidth = backendTex.width();
    desc.fHeight = backendTex.height();
    desc.fConfig = backendTex.config();
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fSampleCnt = sampleCnt;

    GrRenderTargetFlags renderTargetFlags = GrRenderTargetFlags::kNone;
    if (fCaps->usesMixedSamples() && sampleCnt > 1) {
        renderTargetFlags |= GrRenderTargetFlags::kMixedSampled;
    }
    if (fCaps->maxWindowRectangles() > 0) {
        renderTargetFlags |= GrRenderTargetFlags::kWindowRectsSupport;
    }

    sk_sp<GrRenderTargetProxy> proxy = this->createLazyRenderTargetProxy(
            [backendTex, sampleCnt] (GrResourceProvider* resourceProvider) {
                if (!resourceProvider) {
                    return sk_sp<GrRenderTarget>();
                }

                sk_sp<GrRenderTarget> rt = resourceProvider->wrapBackendTextureAsRenderTarget(
                        backendTex, sampleCnt);
                if (!rt) {
                    return sk_sp<GrRenderTarget>();
                }
                SkASSERT(!rt->asTexture());   // A GrRenderTarget that's not textureable
                SkASSERT(!rt->getUniqueKey().isValid());
                // Make sure we match how we created the proxy with SkBudgeted::kNo
                SkASSERT(SkBudgeted::kNo == rt->resourcePriv().isBudgeted());

                return rt;
            }, desc, renderTargetFlags, Textureable::kNo, GrMipMapped::kNo, SkBackingFit::kExact,
               SkBudgeted::kNo);

    if (fResourceProvider) {
        // In order to reuse code we always create a lazy proxy. When we aren't in DDL mode however,
        // we're better off instantiating the proxy immediately here.
        if (!proxy->priv().doLazyInstantiation(fResourceProvider)) {
            return nullptr;
        }
    }
    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrSurfaceDesc& desc,
                                                       GrMipMapped mipMapped,
                                                       SkBackingFit fit, SkBudgeted budgeted) {
    return this->createLazyProxy(std::move(callback), desc, mipMapped, GrRenderTargetFlags::kNone,
                                 fit, budgeted);
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrSurfaceDesc& desc,
                                                       GrMipMapped mipMapped,
                                                       GrRenderTargetFlags renderTargetFlags,
                                                       SkBackingFit fit, SkBudgeted budgeted) {
    SkASSERT((desc.fWidth <= 0 && desc.fHeight <= 0) ||
             (desc.fWidth > 0 && desc.fHeight > 0));
    uint32_t flags = GrResourceProvider::kNoPendingIO_Flag;

#ifdef SK_DEBUG
    if (SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags)) {
        if (SkToBool(renderTargetFlags & GrRenderTargetFlags::kMixedSampled)) {
            SkASSERT(fCaps->usesMixedSamples() && desc.fSampleCnt > 1);
        }
        if (SkToBool(renderTargetFlags & GrRenderTargetFlags::kWindowRectsSupport)) {
            SkASSERT(fCaps->maxWindowRectangles() > 0);
        }
    }
#endif

    using LazyInstantiationType = GrSurfaceProxy::LazyInstantiationType;
    // For non-ddl draws always make lazy proxy's single use.
    LazyInstantiationType lazyType = fResourceProvider ? LazyInstantiationType::kSingleUse
                                                       : LazyInstantiationType::kMultipleUse;

    return sk_sp<GrTextureProxy>(SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags) ?
                                 new GrTextureRenderTargetProxy(std::move(callback), lazyType, desc,
                                                                mipMapped, fit, budgeted, flags,
                                                                renderTargetFlags) :
                                 new GrTextureProxy(std::move(callback), lazyType, desc, mipMapped,
                                                    fit, budgeted, flags));
}

sk_sp<GrRenderTargetProxy> GrProxyProvider::createLazyRenderTargetProxy(
                                                LazyInstantiateCallback&& callback,
                                                const GrSurfaceDesc& desc,
                                                GrRenderTargetFlags renderTargetFlags,
                                                Textureable textureable,
                                                GrMipMapped mipMapped,
                                                SkBackingFit fit, SkBudgeted budgeted) {
    SkASSERT((desc.fWidth <= 0 && desc.fHeight <= 0) ||
             (desc.fWidth > 0 && desc.fHeight > 0));
    SkASSERT(SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags));
    uint32_t flags = GrResourceProvider::kNoPendingIO_Flag;

#ifdef SK_DEBUG
    if (SkToBool(renderTargetFlags & GrRenderTargetFlags::kMixedSampled)) {
        SkASSERT(fCaps->usesMixedSamples() && desc.fSampleCnt > 1);
    }
    if (SkToBool(renderTargetFlags & GrRenderTargetFlags::kWindowRectsSupport)) {
        SkASSERT(fCaps->maxWindowRectangles() > 0);
    }
#endif

    using LazyInstantiationType = GrSurfaceProxy::LazyInstantiationType;
    // For non-ddl draws always make lazy proxy's single use.
    LazyInstantiationType lazyType = fResourceProvider ? LazyInstantiationType::kSingleUse
                                                       : LazyInstantiationType::kMultipleUse;

    if (Textureable::kYes == textureable) {
        return sk_sp<GrRenderTargetProxy>(new GrTextureRenderTargetProxy(std::move(callback),
                                                                         lazyType, desc, mipMapped,
                                                                         fit, budgeted, flags,
                                                                         renderTargetFlags));
    }

    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(std::move(callback), lazyType, desc,
                                                              fit, budgeted, flags,
                                                              renderTargetFlags));
}

sk_sp<GrTextureProxy> GrProxyProvider::createFullyLazyProxy(LazyInstantiateCallback&& callback,
                                                            Renderable renderable,
                                                            GrSurfaceOrigin origin,
                                                            GrPixelConfig config) {
    GrSurfaceDesc desc;
    GrRenderTargetFlags renderTargetFlags = GrRenderTargetFlags::kNone;
    if (Renderable::kYes == renderable) {
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        if (fCaps->maxWindowRectangles() > 0) {
            renderTargetFlags |= GrRenderTargetFlags::kWindowRectsSupport;
        }
    }
    desc.fOrigin = origin;
    desc.fWidth = -1;
    desc.fHeight = -1;
    desc.fConfig = config;
    desc.fSampleCnt = 1;

    return this->createLazyProxy(std::move(callback), desc, GrMipMapped::kNo, renderTargetFlags,
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
