/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrProxyProvider.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrImageContext.h"
#include "GrImageContextPriv.h"
#include "GrRenderTarget.h"
#include "GrResourceKey.h"
#include "GrResourceProvider.h"
#include "GrSurfaceProxy.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "GrTextureProxyCacheAccess.h"
#include "GrTextureRenderTargetProxy.h"
#include "../private/GrSingleOwner.h"
#include "SkAutoPixmapStorage.h"
#include "SkBitmap.h"
#include "SkGr.h"
#include "SkImage.h"
#include "SkImage_Base.h"
#include "SkImageInfoPriv.h"
#include "SkImagePriv.h"
#include "SkMipMap.h"
#include "SkTraceEvent.h"

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fImageContext->priv().singleOwner());)

GrProxyProvider::GrProxyProvider(GrImageContext* imageContext) : fImageContext(imageContext) {}

GrProxyProvider::~GrProxyProvider() {
    if (this->renderingDirectly()) {
        // In DDL-mode a proxy provider can still have extant uniquely keyed proxies (since
        // they need their unique keys to, potentially, find a cached resource when the
        // DDL is played) but, in non-DDL-mode they should all have been cleaned up by this point.
        SkASSERT(!fUniquelyKeyedProxies.count());
    }
}

bool GrProxyProvider::assignUniqueKeyToProxy(const GrUniqueKey& key, GrTextureProxy* proxy) {
    ASSERT_SINGLE_OWNER
    SkASSERT(key.isValid());
    if (this->isAbandoned() || !proxy) {
        return false;
    }

#ifdef SK_DEBUG
    {
        GrContext* direct = fImageContext->priv().asDirectContext();
        if (direct) {
            GrResourceCache* resourceCache = direct->priv().getResourceCache();
            // If there is already a GrResource with this key then the caller has violated the
            // normal usage pattern of uniquely keyed resources (e.g., they have created one w/o
            // first seeing if it already existed in the cache).
            SkASSERT(!resourceCache->findAndRefUniqueResource(key));
        }
    }
#endif

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

void GrProxyProvider::removeUniqueKeyFromProxy(GrTextureProxy* proxy) {
    ASSERT_SINGLE_OWNER
    SkASSERT(proxy);
    SkASSERT(proxy->getUniqueKey().isValid());

    if (this->isAbandoned()) {
        return;
    }

    this->processInvalidUniqueKey(proxy->getUniqueKey(), proxy, InvalidateGPUResource::kYes);
}

sk_sp<GrTextureProxy> GrProxyProvider::findProxyByUniqueKey(const GrUniqueKey& key,
                                                            GrSurfaceOrigin origin) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    GrTextureProxy* proxy = fUniquelyKeyedProxies.find(key);
    sk_sp<GrTextureProxy> result;
    if (proxy) {
        GrResourceCache* cache = nullptr;
        if (auto directContext = fImageContext->priv().asDirectContext()) {
            cache = directContext->priv().getResourceCache();
        }
        proxy->firstRefAccess().ref(cache);
        result.reset(proxy);
        SkASSERT(result->origin() == origin);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS
sk_sp<GrTextureProxy> GrProxyProvider::testingOnly_createInstantiatedProxy(
        const GrSurfaceDesc& desc, GrSurfaceOrigin origin, SkBackingFit fit, SkBudgeted budgeted) {
    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();
    sk_sp<GrTexture> tex;

    if (SkBackingFit::kApprox == fit) {
        tex = resourceProvider->createApproxTexture(desc, GrResourceProvider::Flags::kNoPendingIO);
    } else {
        tex = resourceProvider->createTexture(desc, budgeted,
                                              GrResourceProvider::Flags::kNoPendingIO);
    }
    if (!tex) {
        return nullptr;
    }

    return this->createWrapped(std::move(tex), origin);
}

sk_sp<GrTextureProxy> GrProxyProvider::testingOnly_createWrapped(sk_sp<GrTexture> tex,
                                                                 GrSurfaceOrigin origin) {
    return this->createWrapped(std::move(tex), origin);
}
#endif

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

    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    GrResourceCache* resourceCache = direct->priv().getResourceCache();

    GrGpuResource* resource = resourceCache->findAndRefUniqueResource(key);
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

sk_sp<GrTextureProxy> GrProxyProvider::createTextureProxy(sk_sp<SkImage> srcImage,
                                                          GrSurfaceDescFlags descFlags,
                                                          int sampleCnt,
                                                          SkBudgeted budgeted,
                                                          SkBackingFit fit,
                                                          GrInternalSurfaceFlags surfaceFlags) {
    ASSERT_SINGLE_OWNER
    SkASSERT(srcImage);

    if (this->isAbandoned()) {
        return nullptr;
    }

    const SkImageInfo& info = srcImage->imageInfo();
    GrPixelConfig config = SkImageInfo2GrPixelConfig(info);

    if (kUnknown_GrPixelConfig == config) {
        return nullptr;
    }

    SkColorType ct = info.colorType();
    if (!this->caps()->isConfigTexturable(config)) {
        SkBitmap copy8888;
        if (!copy8888.tryAllocPixels(info.makeColorType(kRGBA_8888_SkColorType)) ||
            !srcImage->readPixels(copy8888.pixmap(), 0, 0)) {
            return nullptr;
        }
        copy8888.setImmutable();
        srcImage = SkMakeImageFromRasterBitmap(copy8888, kNever_SkCopyPixelsMode);
        config = kRGBA_8888_GrPixelConfig;
        ct = kRGBA_8888_SkColorType;
    }

    GrBackendFormat format = this->caps()->getBackendFormatFromColorType(ct);
    if (!format.isValid()) {
        return nullptr;
    }

    if (SkToBool(descFlags & kRenderTarget_GrSurfaceFlag)) {
        sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, config);
        if (!sampleCnt) {
            return nullptr;
        }
    }

    if (SkToBool(descFlags & kRenderTarget_GrSurfaceFlag)) {
        if (this->caps()->usesMixedSamples() && sampleCnt > 1) {
            surfaceFlags |= GrInternalSurfaceFlags::kMixedSampled;
        }
    }
    if (fImageContext->priv().explicitlyAllocateGPUResources()) {
        surfaceFlags |= GrInternalSurfaceFlags::kNoPendingIO;
    }

    GrSurfaceDesc desc;
    desc.fWidth = srcImage->width();
    desc.fHeight = srcImage->height();
    desc.fFlags = descFlags;
    desc.fSampleCnt = sampleCnt;
    desc.fConfig = config;

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [desc, budgeted, srcImage, fit, surfaceFlags](GrResourceProvider* resourceProvider) {
                SkPixmap pixMap;
                SkAssertResult(srcImage->peekPixels(&pixMap));
                GrMipLevel mipLevel = { pixMap.addr(), pixMap.rowBytes() };

                auto resourceProviderFlags = GrResourceProvider::Flags::kNone;
                if (surfaceFlags & GrInternalSurfaceFlags::kNoPendingIO) {
                    resourceProviderFlags |= GrResourceProvider::Flags::kNoPendingIO;
                }
                return LazyInstantiationResult(resourceProvider->createTexture(
                        desc, budgeted, fit, mipLevel, resourceProviderFlags));
            },
            format, desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, surfaceFlags, fit, budgeted);

    if (!proxy) {
        return nullptr;
    }

    GrContext* direct = fImageContext->priv().asDirectContext();
    if (direct) {
        GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

        // In order to reuse code we always create a lazy proxy. When we aren't in DDL mode however
        // we're better off instantiating the proxy immediately here.
        if (!proxy->priv().doLazyInstantiation(resourceProvider)) {
            return nullptr;
        }
    }

    SkASSERT(proxy->width() == desc.fWidth);
    SkASSERT(proxy->height() == desc.fHeight);
    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::createMipMapProxy(const GrBackendFormat& format,
                                                         const GrSurfaceDesc& desc,
                                                         GrSurfaceOrigin origin,
                                                         SkBudgeted budgeted) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    return this->createProxy(format, desc, origin, GrMipMapped::kYes, SkBackingFit::kExact,
                             budgeted, GrInternalSurfaceFlags::kNone);
}

sk_sp<GrTextureProxy> GrProxyProvider::createProxyFromBitmap(const SkBitmap& bitmap,
                                                             GrMipMapped mipMapped) {
    ASSERT_SINGLE_OWNER
    SkASSERT(GrMipMapped::kNo == mipMapped || this->caps()->mipMapSupport());

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!SkImageInfoIsValid(bitmap.info())) {
        return nullptr;
    }

    ATRACE_ANDROID_FRAMEWORK("Upload %sTexture [%ux%u]",
                             GrMipMapped::kYes == mipMapped ? "MipMap " : "",
                             bitmap.width(), bitmap.height());

    // In non-ddl we will always instantiate right away. Thus we never want to copy the SkBitmap
    // even if its mutable. In ddl, if the bitmap is mutable then we must make a copy since the
    // upload of the data to the gpu can happen at anytime and the bitmap may change by then.
    SkCopyPixelsMode copyMode = this->renderingDirectly() ? kNever_SkCopyPixelsMode
                                                          : kIfMutable_SkCopyPixelsMode;
    sk_sp<SkImage> baseLevel = SkMakeImageFromRasterBitmap(bitmap, copyMode);
    if (!baseLevel) {
        return nullptr;
    }

    // If mips weren't requested (or this was too small to have any), then take the fast path
    if (GrMipMapped::kNo == mipMapped ||
        0 == SkMipMap::ComputeLevelCount(baseLevel->width(), baseLevel->height())) {
        return this->createTextureProxy(std::move(baseLevel), kNone_GrSurfaceFlags, 1,
                                        SkBudgeted::kYes, SkBackingFit::kExact);
    }

    const GrBackendFormat format =
                            this->caps()->getBackendFormatFromColorType(bitmap.info().colorType());
    if (!format.isValid()) {
        return nullptr;
    }

    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(bitmap.info());
    if (!this->caps()->isConfigTexturable(desc.fConfig)) {
        SkBitmap copy8888;
        if (!copy8888.tryAllocPixels(bitmap.info().makeColorType(kRGBA_8888_SkColorType)) ||
            !bitmap.readPixels(copy8888.pixmap())) {
            return nullptr;
        }
        copy8888.setImmutable();
        baseLevel = SkMakeImageFromRasterBitmap(copy8888, kNever_SkCopyPixelsMode);
        desc.fConfig = kRGBA_8888_GrPixelConfig;
    }

    SkPixmap pixmap;
    SkAssertResult(baseLevel->peekPixels(&pixmap));
    sk_sp<SkMipMap> mipmaps(SkMipMap::Build(pixmap, nullptr));
    if (!mipmaps) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [desc, baseLevel, mipmaps](GrResourceProvider* resourceProvider) {
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

                return LazyInstantiationResult(resourceProvider->createTexture(
                        desc, SkBudgeted::kYes, texels.get(), mipLevelCount));
            },
            format, desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kYes, SkBackingFit::kExact,
            SkBudgeted::kYes);

    if (!proxy) {
        return nullptr;
    }

    GrContext* direct = fImageContext->priv().asDirectContext();
    if (direct) {
        GrResourceProvider* resourceProvider = direct->priv().resourceProvider();
        // In order to reuse code we always create a lazy proxy. When we aren't in DDL mode however
        // we're better off instantiating the proxy immediately here.
        if (!proxy->priv().doLazyInstantiation(resourceProvider)) {
            return nullptr;
        }
    }
    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::createProxy(const GrBackendFormat& format,
                                                   const GrSurfaceDesc& desc,
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
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(*this->caps(), format, copyDesc,
                                                                    origin, mipMapped,
                                                                    fit, budgeted, surfaceFlags));
    }

    return sk_sp<GrTextureProxy>(new GrTextureProxy(format, copyDesc, origin, mipMapped,
                                                    fit, budgeted, surfaceFlags));
}

sk_sp<GrTextureProxy> GrProxyProvider::createProxy(sk_sp<SkData> data, const GrSurfaceDesc& desc) {
    if (!this->caps()->isConfigTexturable(desc.fConfig)) {
        return nullptr;
    }

    const GrColorType ct = GrPixelConfigToColorType(desc.fConfig);
    const GrBackendFormat format =
                            this->caps()->getBackendFormatFromGrColorType(ct, GrSRGBEncoded::kNo);

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
        [desc, data](GrResourceProvider* resourceProvider) {
            GrMipLevel texels;
            texels.fPixels = data->data();
            texels.fRowBytes = GrBytesPerPixel(desc.fConfig)*desc.fWidth;
            return LazyInstantiationResult(
                    resourceProvider->createTexture(desc, SkBudgeted::kYes, &texels, 1));
        },
        format, desc, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo, SkBackingFit::kExact,
        SkBudgeted::kYes);

    if (!proxy) {
        return nullptr;
    }

    GrContext* direct = fImageContext->priv().asDirectContext();
    if (direct) {
        GrResourceProvider* resourceProvider = direct->priv().resourceProvider();
        // In order to reuse code we always create a lazy proxy. When we aren't in DDL mode however
        // we're better off instantiating the proxy immediately here.
        if (!proxy->priv().doLazyInstantiation(resourceProvider)) {
            return nullptr;
        }
    }
    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::wrapBackendTexture(const GrBackendTexture& backendTex,
                                                          GrSurfaceOrigin origin,
                                                          GrWrapOwnership ownership,
                                                          GrWrapCacheable cacheable,
                                                          GrIOType ioType,
                                                          ReleaseProc releaseProc,
                                                          ReleaseContext releaseCtx) {
    SkASSERT(ioType != kWrite_GrIOType);
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sk_sp<GrTexture> tex =
            resourceProvider->wrapBackendTexture(backendTex, ownership, cacheable, ioType);
    if (!tex) {
        return nullptr;
    }

    if (releaseProc) {
        tex->setRelease(releaseProc, releaseCtx);
    }

    SkASSERT(!tex->asRenderTarget());  // Strictly a GrTexture
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(GrBudgetedType::kBudgeted != tex->resourcePriv().budgetedType());

    return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex), origin));
}

sk_sp<GrTextureProxy> GrProxyProvider::wrapRenderableBackendTexture(
        const GrBackendTexture& backendTex, GrSurfaceOrigin origin, int sampleCnt,
        GrWrapOwnership ownership, GrWrapCacheable cacheable, ReleaseProc releaseProc,
        ReleaseContext releaseCtx) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, backendTex.config());
    if (!sampleCnt) {
        return nullptr;
    }

    sk_sp<GrTexture> tex = resourceProvider->wrapRenderableBackendTexture(backendTex, sampleCnt,
                                                                          ownership, cacheable);
    if (!tex) {
        return nullptr;
    }

    if (releaseProc) {
        tex->setRelease(releaseProc, releaseCtx);
    }

    SkASSERT(tex->asRenderTarget());  // A GrTextureRenderTarget
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(GrBudgetedType::kBudgeted != tex->resourcePriv().budgetedType());

    return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(std::move(tex), origin));
}

sk_sp<GrSurfaceProxy> GrProxyProvider::wrapBackendRenderTarget(
        const GrBackendRenderTarget& backendRT, GrSurfaceOrigin origin, ReleaseProc releaseProc,
        ReleaseContext releaseCtx) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sk_sp<GrRenderTarget> rt = resourceProvider->wrapBackendRenderTarget(backendRT);
    if (!rt) {
        return nullptr;
    }

    if (releaseProc) {
        rt->setRelease(releaseProc, releaseCtx);
    }

    SkASSERT(!rt->asTexture());  // A GrRenderTarget that's not textureable
    SkASSERT(!rt->getUniqueKey().isValid());
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(GrBudgetedType::kBudgeted != rt->resourcePriv().budgetedType());

    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(std::move(rt), origin));
}

sk_sp<GrSurfaceProxy> GrProxyProvider::wrapBackendTextureAsRenderTarget(
        const GrBackendTexture& backendTex, GrSurfaceOrigin origin, int sampleCnt) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sk_sp<GrRenderTarget> rt =
            resourceProvider->wrapBackendTextureAsRenderTarget(backendTex, sampleCnt);
    if (!rt) {
        return nullptr;
    }
    SkASSERT(!rt->asTexture());  // A GrRenderTarget that's not textureable
    SkASSERT(!rt->getUniqueKey().isValid());
    // This proxy should be unbudgeted because we're just wrapping an external resource
    SkASSERT(GrBudgetedType::kBudgeted != rt->resourcePriv().budgetedType());

    return sk_sp<GrSurfaceProxy>(new GrRenderTargetProxy(std::move(rt), origin));
}

sk_sp<GrRenderTargetProxy> GrProxyProvider::wrapVulkanSecondaryCBAsRenderTarget(
        const SkImageInfo& imageInfo, const GrVkDrawableInfo& vkInfo) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sk_sp<GrRenderTarget> rt = resourceProvider->wrapVulkanSecondaryCBAsRenderTarget(imageInfo,
                                                                                     vkInfo);
    if (!rt) {
        return nullptr;
    }

    SkASSERT(!rt->asTexture());  // A GrRenderTarget that's not textureable
    SkASSERT(!rt->getUniqueKey().isValid());
    // This proxy should be unbudgeted because we're just wrapping an external resource
    SkASSERT(GrBudgetedType::kBudgeted != rt->resourcePriv().budgetedType());

    // All Vulkan surfaces uses top left origins.
    return sk_sp<GrRenderTargetProxy>(
            new GrRenderTargetProxy(std::move(rt),
                                    kTopLeft_GrSurfaceOrigin,
                                    GrRenderTargetProxy::WrapsVkSecondaryCB::kYes));
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrBackendFormat& format,
                                                       const GrSurfaceDesc& desc,
                                                       GrSurfaceOrigin origin,
                                                       GrMipMapped mipMapped,
                                                       SkBackingFit fit,
                                                       SkBudgeted budgeted) {
    return this->createLazyProxy(std::move(callback), format, desc, origin, mipMapped,
                                 GrInternalSurfaceFlags::kNone, fit, budgeted);
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrBackendFormat& format,
                                                       const GrSurfaceDesc& desc,
                                                       GrSurfaceOrigin origin,
                                                       GrMipMapped mipMapped,
                                                       GrInternalSurfaceFlags surfaceFlags,
                                                       SkBackingFit fit,
                                                       SkBudgeted budgeted) {
    // For non-ddl draws always make lazy proxy's single use.
    LazyInstantiationType lazyType = this->renderingDirectly() ? LazyInstantiationType::kSingleUse
                                                               : LazyInstantiationType::kMultipleUse;
    return this->createLazyProxy(std::move(callback), format, desc, origin, mipMapped, surfaceFlags,
                                 fit, budgeted, lazyType);
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrBackendFormat& format,
                                                       const GrSurfaceDesc& desc,
                                                       GrSurfaceOrigin origin,
                                                       GrMipMapped mipMapped,
                                                       GrInternalSurfaceFlags surfaceFlags,
                                                       SkBackingFit fit,
                                                       SkBudgeted budgeted,
                                                       LazyInstantiationType lazyType) {
    SkASSERT((desc.fWidth <= 0 && desc.fHeight <= 0) ||
             (desc.fWidth > 0 && desc.fHeight > 0));

    if (desc.fWidth > this->caps()->maxTextureSize() ||
        desc.fHeight > this->caps()->maxTextureSize()) {
        return nullptr;
    }


#ifdef SK_DEBUG
    if (SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags)) {
        if (SkToBool(surfaceFlags & GrInternalSurfaceFlags::kMixedSampled)) {
            SkASSERT(this->caps()->usesMixedSamples() && desc.fSampleCnt > 1);
        }
    }
#endif

    return sk_sp<GrTextureProxy>(
            SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags)
                    ? new GrTextureRenderTargetProxy(std::move(callback), lazyType, format, desc,
                                                     origin, mipMapped, fit, budgeted, surfaceFlags)
                    : new GrTextureProxy(std::move(callback), lazyType, format, desc, origin,
                                         mipMapped, fit, budgeted, surfaceFlags));
}

sk_sp<GrRenderTargetProxy> GrProxyProvider::createLazyRenderTargetProxy(
        LazyInstantiateCallback&& callback, const GrBackendFormat& format,
        const GrSurfaceDesc& desc, GrSurfaceOrigin origin, GrInternalSurfaceFlags surfaceFlags,
        const TextureInfo* textureInfo, SkBackingFit fit, SkBudgeted budgeted,
        bool wrapsVkSecondaryCB) {
    SkASSERT((desc.fWidth <= 0 && desc.fHeight <= 0) ||
             (desc.fWidth > 0 && desc.fHeight > 0));

    if (desc.fWidth > this->caps()->maxRenderTargetSize() ||
        desc.fHeight > this->caps()->maxRenderTargetSize()) {
        return nullptr;
    }

    SkASSERT(SkToBool(kRenderTarget_GrSurfaceFlag & desc.fFlags));

#ifdef SK_DEBUG
    if (SkToBool(surfaceFlags & GrInternalSurfaceFlags::kMixedSampled)) {
        SkASSERT(this->caps()->usesMixedSamples() && desc.fSampleCnt > 1);
    }
#endif

    using LazyInstantiationType = GrSurfaceProxy::LazyInstantiationType;
    // For non-ddl draws always make lazy proxy's single use.
    LazyInstantiationType lazyType = this->renderingDirectly() ? LazyInstantiationType::kSingleUse
                                                               : LazyInstantiationType::kMultipleUse;

    if (textureInfo) {
        // Wrapped vulkan secondary command buffers don't support texturing since we won't have an
        // actual VkImage to texture from.
        SkASSERT(!wrapsVkSecondaryCB);
        return sk_sp<GrRenderTargetProxy>(new GrTextureRenderTargetProxy(
                std::move(callback), lazyType, format, desc, origin, textureInfo->fMipMapped,
                fit, budgeted, surfaceFlags));
    }

    GrRenderTargetProxy::WrapsVkSecondaryCB vkSCB =
            wrapsVkSecondaryCB ? GrRenderTargetProxy::WrapsVkSecondaryCB::kYes
                               : GrRenderTargetProxy::WrapsVkSecondaryCB::kNo;

    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(
            std::move(callback), lazyType, format, desc, origin, fit, budgeted, surfaceFlags,
            vkSCB));
}

sk_sp<GrTextureProxy> GrProxyProvider::MakeFullyLazyProxy(
        LazyInstantiateCallback&& callback, const GrBackendFormat& format, Renderable renderable,
        GrSurfaceOrigin origin, GrPixelConfig config, const GrCaps& caps, int sampleCnt) {
    GrSurfaceDesc desc;
    GrInternalSurfaceFlags surfaceFlags = GrInternalSurfaceFlags::kNoPendingIO;
    if (Renderable::kYes == renderable) {
        desc.fFlags = kRenderTarget_GrSurfaceFlag;
        if (sampleCnt > 1 && caps.usesMixedSamples()) {
            surfaceFlags |= GrInternalSurfaceFlags::kMixedSampled;
        }
    }
    desc.fWidth = -1;
    desc.fHeight = -1;
    desc.fConfig = config;
    desc.fSampleCnt = sampleCnt;

    return sk_sp<GrTextureProxy>(
            (Renderable::kYes == renderable)
                    ? new GrTextureRenderTargetProxy(
                              std::move(callback), LazyInstantiationType::kSingleUse, format, desc,
                              origin, GrMipMapped::kNo, SkBackingFit::kApprox, SkBudgeted::kYes,
                              surfaceFlags)
                    : new GrTextureProxy(std::move(callback), LazyInstantiationType::kSingleUse,
                                         format, desc, origin, GrMipMapped::kNo,
                                         SkBackingFit::kApprox, SkBudgeted::kYes, surfaceFlags));
}

bool GrProxyProvider::IsFunctionallyExact(GrSurfaceProxy* proxy) {
    const bool isInstantiated = proxy->isInstantiated();
    // A proxy is functionally exact if:
    //   it is exact (obvs)
    //   when it is instantiated it will be exact (i.e., power of two dimensions)
    //   it is already instantiated and the proxy covers the entire backing surface
    return proxy->priv().isExact() ||
           (!isInstantiated && SkIsPow2(proxy->width()) && SkIsPow2(proxy->height())) ||
           (isInstantiated && proxy->worstCaseWidth() == proxy->width() &&
                              proxy->worstCaseHeight() == proxy->height());
}

void GrProxyProvider::processInvalidUniqueKey(const GrUniqueKey& key, GrTextureProxy* proxy,
                                              InvalidateGPUResource invalidateGPUResource) {
    SkASSERT(key.isValid());

    if (!proxy) {
        proxy = fUniquelyKeyedProxies.find(key);
    }
    SkASSERT(!proxy || proxy->getUniqueKey() == key);

    // Locate the corresponding GrGpuResource (if it needs to be invalidated) before clearing the
    // proxy's unique key. We must do it in this order because 'key' may alias the proxy's key.
    sk_sp<GrGpuResource> invalidGpuResource;
    if (InvalidateGPUResource::kYes == invalidateGPUResource) {
        GrContext* direct = fImageContext->priv().asDirectContext();
        if (direct) {
            GrResourceProvider* resourceProvider = direct->priv().resourceProvider();
            invalidGpuResource = resourceProvider->findByUniqueKey<GrGpuResource>(key);
        }
        SkASSERT(!invalidGpuResource || invalidGpuResource->getUniqueKey() == key);
    }

    // Note: this method is called for the whole variety of GrGpuResources so often 'key'
    // will not be in 'fUniquelyKeyedProxies'.
    if (proxy) {
        fUniquelyKeyedProxies.remove(key);
        proxy->cacheAccess().clearUniqueKey();
    }

    if (invalidGpuResource) {
        invalidGpuResource->resourcePriv().removeUniqueKey();
    }
}

uint32_t GrProxyProvider::contextID() const {
    return fImageContext->priv().contextID();
}

const GrCaps* GrProxyProvider::caps() const {
    return fImageContext->priv().caps();
}

sk_sp<const GrCaps> GrProxyProvider::refCaps() const {
    return fImageContext->priv().refCaps();
}

bool GrProxyProvider::isAbandoned() const {
    return fImageContext->priv().abandoned();
}

void GrProxyProvider::orphanAllUniqueKeys() {
    UniquelyKeyedProxyHash::Iter iter(&fUniquelyKeyedProxies);
    for (UniquelyKeyedProxyHash::Iter iter(&fUniquelyKeyedProxies); !iter.done(); ++iter) {
        GrTextureProxy& tmp = *iter;

        tmp.fProxyProvider = nullptr;
    }
}

void GrProxyProvider::removeAllUniqueKeys() {
    UniquelyKeyedProxyHash::Iter iter(&fUniquelyKeyedProxies);
    for (UniquelyKeyedProxyHash::Iter iter(&fUniquelyKeyedProxies); !iter.done(); ++iter) {
        GrTextureProxy& tmp = *iter;

        this->processInvalidUniqueKey(tmp.getUniqueKey(), &tmp, InvalidateGPUResource::kNo);
    }
    SkASSERT(!fUniquelyKeyedProxies.count());
}

bool GrProxyProvider::renderingDirectly() const {
    return fImageContext->priv().asDirectContext();
}
