/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrProxyProvider.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTexture.h"
#include "include/private/GrImageContext.h"
#include "include/private/GrResourceKey.h"
#include "include/private/GrSingleOwner.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMipMap.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureProxyCacheAccess.h"
#include "src/gpu/GrTextureRenderTargetProxy.h"
#include "src/gpu/SkGr.h"
#include "src/image/SkImage_Base.h"

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
    if (proxy) {
        SkASSERT(proxy->origin() == origin);
        return sk_ref_sp(proxy);
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS
sk_sp<GrTextureProxy> GrProxyProvider::testingOnly_createInstantiatedProxy(
        const SkISize& dimensions,
        GrColorType colorType,
        const GrBackendFormat& format,
        GrRenderable renderable,
        int renderTargetSampleCnt,
        GrSurfaceOrigin origin,
        SkBackingFit fit,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    if (this->caps()->isFormatCompressed(format)) {
        // TODO: Allow this to go to GrResourceProvider::createCompressedTexture() once we no longer
        // rely on GrColorType to get to GrPixelConfig. Currently this will cause
        // makeConfigSpecific() to assert because GrColorTypeToPixelConfig() never returns a
        // compressed GrPixelConfig.
        return nullptr;
    }
    GrSurfaceDesc desc;
    desc.fConfig = GrColorTypeToPixelConfig(colorType);
    desc.fConfig = this->caps()->makeConfigSpecific(desc.fConfig, format);
    desc.fWidth = dimensions.width();
    desc.fHeight = dimensions.height();

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();
    sk_sp<GrTexture> tex;

    if (SkBackingFit::kApprox == fit) {
        tex = resourceProvider->createApproxTexture(desc, format, renderable, renderTargetSampleCnt,
                                                    isProtected);
    } else {
        tex = resourceProvider->createTexture(desc, format, renderable, renderTargetSampleCnt,
                                              GrMipMapped::kNo, budgeted, isProtected);
    }
    if (!tex) {
        return nullptr;
    }

    return this->createWrapped(std::move(tex), colorType, origin, UseAllocator::kYes);
}

sk_sp<GrTextureProxy> GrProxyProvider::testingOnly_createInstantiatedProxy(
        const SkISize& dimensions,
        GrColorType colorType,
        GrRenderable renderable,
        int renderTargetSampleCnt,
        GrSurfaceOrigin origin,
        SkBackingFit fit,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    auto format = this->caps()->getDefaultBackendFormat(colorType, renderable);
    return this->testingOnly_createInstantiatedProxy(dimensions,
                                                     colorType,
                                                     format,
                                                     renderable,
                                                     renderTargetSampleCnt,
                                                     origin,
                                                     fit,
                                                     budgeted,
                                                     isProtected);
}

sk_sp<GrTextureProxy> GrProxyProvider::testingOnly_createWrapped(sk_sp<GrTexture> tex,
                                                                 GrColorType colorType,
                                                                 GrSurfaceOrigin origin) {
    return this->createWrapped(std::move(tex), colorType, origin, UseAllocator::kYes);
}
#endif

sk_sp<GrTextureProxy> GrProxyProvider::createWrapped(sk_sp<GrTexture> tex,
                                                     GrColorType colorType,
                                                     GrSurfaceOrigin origin,
                                                     UseAllocator useAllocator) {
#ifdef SK_DEBUG
    if (tex->getUniqueKey().isValid()) {
        SkASSERT(!this->findProxyByUniqueKey(tex->getUniqueKey(), origin));
    }
#endif
    GrSwizzle texSwizzle = this->caps()->getTextureSwizzle(tex->backendFormat(), colorType);

    if (tex->asRenderTarget()) {
        GrSwizzle outSwizzle = this->caps()->getOutputSwizzle(tex->backendFormat(), colorType);
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(
                std::move(tex), origin, texSwizzle, outSwizzle, useAllocator));
    } else {
        return sk_sp<GrTextureProxy>(
                new GrTextureProxy(std::move(tex), origin, texSwizzle, useAllocator));
    }
}

sk_sp<GrTextureProxy> GrProxyProvider::findOrCreateProxyByUniqueKey(const GrUniqueKey& key,
                                                                    GrColorType colorType,
                                                                    GrSurfaceOrigin origin,
                                                                    UseAllocator useAllocator) {
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

    result = this->createWrapped(std::move(texture), colorType, origin, useAllocator);
    SkASSERT(result->getUniqueKey() == key);
    // createWrapped should've added this for us
    SkASSERT(fUniquelyKeyedProxies.find(key));
    SkASSERT(result->textureSwizzle() ==
             this->caps()->getTextureSwizzle(result->backendFormat(), colorType));
    return result;
}

sk_sp<GrTextureProxy> GrProxyProvider::createTextureProxy(sk_sp<SkImage> srcImage,
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
    GrColorType ct = SkColorTypeToGrColorType(info.colorType());

    GrBackendFormat format = this->caps()->getDefaultBackendFormat(ct, GrRenderable::kNo);

    if (!format.isValid()) {
        SkBitmap copy8888;
        if (!copy8888.tryAllocPixels(info.makeColorType(kRGBA_8888_SkColorType)) ||
            !srcImage->readPixels(copy8888.pixmap(), 0, 0)) {
            return nullptr;
        }
        copy8888.setImmutable();
        srcImage = SkMakeImageFromRasterBitmap(copy8888, kNever_SkCopyPixelsMode);
        ct = GrColorType::kRGBA_8888;
        format = this->caps()->getDefaultBackendFormat(ct, GrRenderable::kNo);
        if (!format.isValid()) {
            return nullptr;
        }
    }

    GrPixelConfig config = GrColorTypeToPixelConfig(ct);
    if (kUnknown_GrPixelConfig == config) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fWidth = srcImage->width();
    desc.fHeight = srcImage->height();
    desc.fConfig = config;

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [desc, format, sampleCnt, budgeted, srcImage, fit,
             ct](GrResourceProvider* resourceProvider) {
                SkPixmap pixMap;
                SkAssertResult(srcImage->peekPixels(&pixMap));
                GrMipLevel mipLevel = { pixMap.addr(), pixMap.rowBytes() };

                return LazyCallbackResult(resourceProvider->createTexture(
                        desc, format, ct, GrRenderable::kNo, sampleCnt, budgeted, fit,
                        GrProtected::kNo, mipLevel));
            },
            format, desc, GrRenderable::kNo, sampleCnt, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo,
            GrMipMapsStatus::kNotAllocated, surfaceFlags, fit, budgeted, GrProtected::kNo,
            UseAllocator::kYes);

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
        return this->createTextureProxy(std::move(baseLevel), 1, SkBudgeted::kYes,
                                        SkBackingFit::kExact);
    }

    GrSurfaceDesc desc = GrImageInfoToSurfaceDesc(bitmap.info());

    GrBackendFormat format = this->caps()->getDefaultBackendFormat(
            SkColorTypeToGrColorType(bitmap.info().colorType()), GrRenderable::kNo);
    if (!format.isValid()) {
        SkBitmap copy8888;
        if (!copy8888.tryAllocPixels(bitmap.info().makeColorType(kRGBA_8888_SkColorType)) ||
            !bitmap.readPixels(copy8888.pixmap())) {
            return nullptr;
        }
        copy8888.setImmutable();
        baseLevel = SkMakeImageFromRasterBitmap(copy8888, kNever_SkCopyPixelsMode);
        desc.fConfig = kRGBA_8888_GrPixelConfig;
        format = this->caps()->getDefaultBackendFormat(GrColorType::kRGBA_8888, GrRenderable::kNo);
        if (!format.isValid()) {
            return nullptr;
        }
    }

    SkPixmap pixmap;
    SkAssertResult(baseLevel->peekPixels(&pixmap));
    sk_sp<SkMipMap> mipmaps(SkMipMap::Build(pixmap, nullptr));
    if (!mipmaps) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [desc, format, baseLevel, mipmaps](GrResourceProvider* resourceProvider) {
                const int mipLevelCount = mipmaps->countLevels() + 1;
                std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipLevelCount]);

                SkPixmap pixmap;
                SkAssertResult(baseLevel->peekPixels(&pixmap));

                texels[0].fPixels = pixmap.addr();
                texels[0].fRowBytes = pixmap.rowBytes();

                auto colorType = SkColorTypeToGrColorType(pixmap.colorType());
                for (int i = 1; i < mipLevelCount; ++i) {
                    SkMipMap::Level generatedMipLevel;
                    mipmaps->getLevel(i - 1, &generatedMipLevel);
                    texels[i].fPixels = generatedMipLevel.fPixmap.addr();
                    texels[i].fRowBytes = generatedMipLevel.fPixmap.rowBytes();
                    SkASSERT(texels[i].fPixels);
                    SkASSERT(generatedMipLevel.fPixmap.colorType() == pixmap.colorType());
                }
                return LazyCallbackResult(resourceProvider->createTexture(
                        desc, format, colorType, GrRenderable::kNo, 1, SkBudgeted::kYes,
                        GrProtected::kNo, texels.get(), mipLevelCount));
            },
            format, desc, GrRenderable::kNo, 1, kTopLeft_GrSurfaceOrigin, GrMipMapped::kYes,
            GrMipMapsStatus::kValid, GrInternalSurfaceFlags::kNone, SkBackingFit::kExact,
            SkBudgeted::kYes, GrProtected::kNo, UseAllocator::kYes);

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
                                                   GrRenderable renderable,
                                                   int renderTargetSampleCnt,
                                                   GrSurfaceOrigin origin,
                                                   GrMipMapped mipMapped,
                                                   SkBackingFit fit,
                                                   SkBudgeted budgeted,
                                                   GrProtected isProtected,
                                                   GrInternalSurfaceFlags surfaceFlags,
                                                   GrSurfaceProxy::UseAllocator useAllocator) {
    const GrCaps* caps = this->caps();

    if (caps->isFormatCompressed(format)) {
        // Deferred proxies for compressed textures are not supported.
        return nullptr;
    }

    GrColorType colorType = GrPixelConfigToColorType(desc.fConfig);

    SkASSERT(GrCaps::AreConfigsCompatible(desc.fConfig,
                                          caps->getConfigFromBackendFormat(format, colorType)));

    if (GrMipMapped::kYes == mipMapped) {
        // SkMipMap doesn't include the base level in the level count so we have to add 1
        int mipCount = SkMipMap::ComputeLevelCount(desc.fWidth, desc.fHeight) + 1;
        if (1 == mipCount) {
            mipMapped = GrMipMapped::kNo;
        }
    }

    if (!caps->validateSurfaceParams({desc.fWidth, desc.fHeight}, format, desc.fConfig, renderable,
                                     renderTargetSampleCnt, mipMapped)) {
        return nullptr;
    }
    GrSurfaceDesc copyDesc = desc;
    GrMipMapsStatus mipMapsStatus = (GrMipMapped::kYes == mipMapped)
            ? GrMipMapsStatus::kDirty
            : GrMipMapsStatus::kNotAllocated;
    GrSwizzle texSwizzle = caps->getTextureSwizzle(format, colorType);
    if (renderable == GrRenderable::kYes) {
        renderTargetSampleCnt =
                caps->getRenderTargetSampleCount(renderTargetSampleCnt, format);
        SkASSERT(renderTargetSampleCnt);
        // We know anything we instantiate later from this deferred path will be
        // both texturable and renderable
        GrSwizzle outSwizzle = caps->getOutputSwizzle(format, colorType);
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(
                *caps, format, copyDesc, renderTargetSampleCnt, origin, mipMapped, mipMapsStatus,
                texSwizzle, outSwizzle, fit, budgeted, isProtected, surfaceFlags, useAllocator));
    }

    return sk_sp<GrTextureProxy>(new GrTextureProxy(format, copyDesc, origin, mipMapped,
                                                    mipMapsStatus, texSwizzle, fit, budgeted,
                                                    isProtected, surfaceFlags, useAllocator));
}

sk_sp<GrTextureProxy> GrProxyProvider::createCompressedTextureProxy(
        int width, int height, SkBudgeted budgeted, SkImage::CompressionType compressionType,
        sk_sp<SkData> data) {

    GrSurfaceDesc desc;
    desc.fConfig = GrCompressionTypePixelConfig(compressionType);
    desc.fWidth = width;
    desc.fHeight = height;

    GrBackendFormat format = this->caps()->getBackendFormatFromCompressionType(compressionType);

    if (!this->caps()->isFormatTexturable(format)) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [width, height, format, compressionType, budgeted,
             data](GrResourceProvider* resourceProvider) {
                return LazyCallbackResult(resourceProvider->createCompressedTexture(
                        width, height, format, compressionType, budgeted, data.get()));
            },
            format, desc, GrRenderable::kNo, 1, kTopLeft_GrSurfaceOrigin, GrMipMapped::kNo,
            GrMipMapsStatus::kNotAllocated, GrInternalSurfaceFlags::kReadOnly, SkBackingFit::kExact,
            SkBudgeted::kYes, GrProtected::kNo, UseAllocator::kYes);

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
                                                          GrColorType grColorType,
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

    const GrCaps* caps = this->caps();

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sk_sp<GrTexture> tex =
            resourceProvider->wrapBackendTexture(backendTex, grColorType,
                                                 ownership, cacheable, ioType);
    if (!tex) {
        return nullptr;
    }

    if (releaseProc) {
        tex->setRelease(releaseProc, releaseCtx);
    }

    SkASSERT(!tex->asRenderTarget());  // Strictly a GrTexture
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(GrBudgetedType::kBudgeted != tex->resourcePriv().budgetedType());

    GrSwizzle texSwizzle = caps->getTextureSwizzle(tex->backendFormat(), grColorType);

    return sk_sp<GrTextureProxy>(
            new GrTextureProxy(std::move(tex), origin, texSwizzle, UseAllocator::kNo));
}

sk_sp<GrTextureProxy> GrProxyProvider::wrapRenderableBackendTexture(
        const GrBackendTexture& backendTex, GrSurfaceOrigin origin, int sampleCnt,
        GrColorType colorType, GrWrapOwnership ownership, GrWrapCacheable cacheable,
        ReleaseProc releaseProc, ReleaseContext releaseCtx) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    const GrCaps* caps = this->caps();

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    // TODO: This should have been checked and validated before getting into GrProxyProvider.
    if (!caps->isFormatAsColorTypeRenderable(colorType, backendTex.getBackendFormat(), sampleCnt)) {
        return nullptr;
    }

    sampleCnt = caps->getRenderTargetSampleCount(sampleCnt, backendTex.getBackendFormat());
    SkASSERT(sampleCnt);

    sk_sp<GrTexture> tex = resourceProvider->wrapRenderableBackendTexture(backendTex, sampleCnt,
                                                                          colorType, ownership,
                                                                          cacheable);
    if (!tex) {
        return nullptr;
    }

    if (releaseProc) {
        tex->setRelease(releaseProc, releaseCtx);
    }

    SkASSERT(tex->asRenderTarget());  // A GrTextureRenderTarget
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(GrBudgetedType::kBudgeted != tex->resourcePriv().budgetedType());

    GrSwizzle texSwizzle = caps->getTextureSwizzle(tex->backendFormat(), colorType);
    GrSwizzle outSwizzle = caps->getOutputSwizzle(tex->backendFormat(), colorType);

    return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(std::move(tex), origin, texSwizzle,
                                                                outSwizzle, UseAllocator::kNo));
}

sk_sp<GrSurfaceProxy> GrProxyProvider::wrapBackendRenderTarget(
        const GrBackendRenderTarget& backendRT, GrColorType grColorType,
        GrSurfaceOrigin origin, ReleaseProc releaseProc, ReleaseContext releaseCtx) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    const GrCaps* caps = this->caps();

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sk_sp<GrRenderTarget> rt = resourceProvider->wrapBackendRenderTarget(backendRT, grColorType);
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

    GrSwizzle texSwizzle = caps->getTextureSwizzle(rt->backendFormat(), grColorType);
    GrSwizzle outSwizzle = caps->getOutputSwizzle(rt->backendFormat(), grColorType);

    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(std::move(rt), origin, texSwizzle,
                                                              outSwizzle, UseAllocator::kNo));
}

sk_sp<GrSurfaceProxy> GrProxyProvider::wrapBackendTextureAsRenderTarget(
        const GrBackendTexture& backendTex, GrColorType grColorType,
        GrSurfaceOrigin origin, int sampleCnt) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    GrContext* direct = fImageContext->priv().asDirectContext();
    if (!direct) {
        return nullptr;
    }

    const GrCaps* caps = this->caps();

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sk_sp<GrRenderTarget> rt =
            resourceProvider->wrapBackendTextureAsRenderTarget(backendTex, sampleCnt, grColorType);
    if (!rt) {
        return nullptr;
    }
    SkASSERT(!rt->asTexture());  // A GrRenderTarget that's not textureable
    SkASSERT(!rt->getUniqueKey().isValid());
    // This proxy should be unbudgeted because we're just wrapping an external resource
    SkASSERT(GrBudgetedType::kBudgeted != rt->resourcePriv().budgetedType());

    GrSwizzle texSwizzle = caps->getTextureSwizzle(rt->backendFormat(), grColorType);
    GrSwizzle outSwizzle = caps->getOutputSwizzle(rt->backendFormat(), grColorType);

    return sk_sp<GrSurfaceProxy>(new GrRenderTargetProxy(std::move(rt), origin, texSwizzle,
                                                         outSwizzle, UseAllocator::kNo));
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

    GrColorType colorType = SkColorTypeToGrColorType(imageInfo.colorType());
    GrSwizzle texSwizzle = this->caps()->getTextureSwizzle(rt->backendFormat(), colorType);
    GrSwizzle outSwizzle = this->caps()->getOutputSwizzle(rt->backendFormat(), colorType);

    if (!this->caps()->isFormatAsColorTypeRenderable(colorType, rt->backendFormat(),
                                                     rt->numSamples())) {
        return nullptr;
    }

    // All Vulkan surfaces uses top left origins.
    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(
            std::move(rt), kTopLeft_GrSurfaceOrigin, texSwizzle, outSwizzle, UseAllocator::kNo,
            GrRenderTargetProxy::WrapsVkSecondaryCB::kYes));
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrBackendFormat& format,
                                                       const GrSurfaceDesc& desc,
                                                       GrRenderable renderable,
                                                       int renderTargetSampleCnt,
                                                       GrSurfaceOrigin origin,
                                                       GrMipMapped mipMapped,
                                                       GrMipMapsStatus mipMapsStatus,
                                                       GrInternalSurfaceFlags surfaceFlags,
                                                       SkBackingFit fit,
                                                       SkBudgeted budgeted,
                                                       GrProtected isProtected,
                                                       GrSurfaceProxy::UseAllocator useAllocator) {
    SkASSERT((desc.fWidth <= 0 && desc.fHeight <= 0) ||
             (desc.fWidth > 0 && desc.fHeight > 0));

    if (!format.isValid()) {
        return nullptr;
    }

    if (desc.fWidth > this->caps()->maxTextureSize() ||
        desc.fHeight > this->caps()->maxTextureSize()) {
        return nullptr;
    }

    GrColorType colorType = GrPixelConfigToColorType(desc.fConfig);
    GrSwizzle texSwizzle = this->caps()->getTextureSwizzle(format, colorType);
    GrSwizzle outSwizzle = this->caps()->getOutputSwizzle(format, colorType);

    if (renderable == GrRenderable::kYes) {
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(*this->caps(),
                                                                    std::move(callback),
                                                                    format,
                                                                    desc,
                                                                    renderTargetSampleCnt,
                                                                    origin,
                                                                    mipMapped,
                                                                    mipMapsStatus,
                                                                    texSwizzle,
                                                                    outSwizzle,
                                                                    fit,
                                                                    budgeted,
                                                                    isProtected,
                                                                    surfaceFlags,
                                                                    useAllocator));
    } else {
        return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(callback),
                                                        format,
                                                        desc,
                                                        origin,
                                                        mipMapped,
                                                        mipMapsStatus,
                                                        texSwizzle,
                                                        fit,
                                                        budgeted,
                                                        isProtected,
                                                        surfaceFlags,
                                                        useAllocator));
    }
}

sk_sp<GrRenderTargetProxy> GrProxyProvider::createLazyRenderTargetProxy(
        LazyInstantiateCallback&& callback,
        const GrBackendFormat& format,
        const GrSurfaceDesc& desc,
        int sampleCnt,
        GrSurfaceOrigin origin,
        GrInternalSurfaceFlags surfaceFlags,
        const TextureInfo* textureInfo,
        GrMipMapsStatus mipMapsStatus,
        SkBackingFit fit,
        SkBudgeted budgeted,
        GrProtected isProtected,
        bool wrapsVkSecondaryCB,
        UseAllocator useAllocator) {
    SkASSERT((desc.fWidth <= 0 && desc.fHeight <= 0) ||
             (desc.fWidth > 0 && desc.fHeight > 0));

    if (desc.fWidth > this->caps()->maxRenderTargetSize() ||
        desc.fHeight > this->caps()->maxRenderTargetSize()) {
        return nullptr;
    }

    GrColorType colorType = GrPixelConfigToColorType(desc.fConfig);
    GrSwizzle texSwizzle = this->caps()->getTextureSwizzle(format, colorType);
    GrSwizzle outSwizzle = this->caps()->getOutputSwizzle(format, colorType);

    if (textureInfo) {
        // Wrapped vulkan secondary command buffers don't support texturing since we won't have an
        // actual VkImage to texture from.
        SkASSERT(!wrapsVkSecondaryCB);
        return sk_sp<GrRenderTargetProxy>(new GrTextureRenderTargetProxy(
                *this->caps(), std::move(callback), format, desc, sampleCnt, origin,
                textureInfo->fMipMapped, mipMapsStatus, texSwizzle, outSwizzle, fit, budgeted,
                isProtected, surfaceFlags, useAllocator));
    }

    GrRenderTargetProxy::WrapsVkSecondaryCB vkSCB =
            wrapsVkSecondaryCB ? GrRenderTargetProxy::WrapsVkSecondaryCB::kYes
                               : GrRenderTargetProxy::WrapsVkSecondaryCB::kNo;

    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(
            std::move(callback), format, desc, sampleCnt, origin, texSwizzle, outSwizzle, fit,
            budgeted, isProtected, surfaceFlags, useAllocator, vkSCB));
}

sk_sp<GrTextureProxy> GrProxyProvider::MakeFullyLazyProxy(LazyInstantiateCallback&& callback,
                                                          const GrBackendFormat& format,
                                                          GrRenderable renderable,
                                                          int renderTargetSampleCnt,
                                                          GrProtected isProtected,
                                                          GrSurfaceOrigin origin,
                                                          GrPixelConfig config,
                                                          const GrCaps& caps,
                                                          UseAllocator useAllocator) {
    if (!format.isValid()) {
        return nullptr;
    }

    SkASSERT(renderTargetSampleCnt == 1 || renderable == GrRenderable::kYes);
    GrSurfaceDesc desc;
    GrInternalSurfaceFlags surfaceFlags = GrInternalSurfaceFlags::kNone;
    desc.fWidth = -1;
    desc.fHeight = -1;
    desc.fConfig = config;

    GrColorType colorType = GrPixelConfigToColorType(desc.fConfig);
    GrSwizzle texSwizzle = caps.getTextureSwizzle(format, colorType);
    GrSwizzle outSwizzle = caps.getOutputSwizzle(format, colorType);

    if (GrRenderable::kYes == renderable) {
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(
                caps, std::move(callback), format, desc, renderTargetSampleCnt, origin,
                GrMipMapped::kNo, GrMipMapsStatus::kNotAllocated, texSwizzle, outSwizzle,
                SkBackingFit::kApprox, SkBudgeted::kYes, isProtected, surfaceFlags, useAllocator));
    } else {
        return sk_sp<GrTextureProxy>(new GrTextureProxy(
                std::move(callback), format, desc, origin, GrMipMapped::kNo,
                GrMipMapsStatus::kNotAllocated, texSwizzle, SkBackingFit::kApprox, SkBudgeted::kYes,
                isProtected, surfaceFlags, useAllocator));
    }
}

bool GrProxyProvider::IsFunctionallyExact(GrSurfaceProxy* proxy) {
    const bool isInstantiated = proxy->isInstantiated();
    // A proxy is functionally exact if:
    //   it is exact (obvs)
    //   when it is instantiated it will be exact (i.e., power of two dimensions)
    //   when it is instantiated the content rect will cover the entire backing surface
    return proxy->priv().isExact() ||
           (!isInstantiated && SkIsPow2(proxy->width()) && SkIsPow2(proxy->height())) ||
           (proxy->backingStoreDimensions() == proxy->dimensions());
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
