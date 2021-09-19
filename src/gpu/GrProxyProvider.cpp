/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrProxyProvider.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/GrImageContext.h"
#include "include/private/GrResourceKey.h"
#include "include/private/GrSingleOwner.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrImageContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxy.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxyCacheAccess.h"
#include "src/gpu/GrTextureRenderTargetProxy.h"
#include "src/gpu/SkGr.h"
#include "src/image/SkImage_Base.h"

#ifdef SK_VULKAN
#include "include/gpu/vk/GrVkTypes.h"
#endif

#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(fImageContext->priv().singleOwner())

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

    // Only the proxyProvider that created a proxy should be assigning unique keys to it.
    SkASSERT(this->isDDLProvider() == proxy->creatingProvider());

#ifdef SK_DEBUG
    {
        auto direct = fImageContext->asDirectContext();
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

sk_sp<GrTextureProxy> GrProxyProvider::findProxyByUniqueKey(const GrUniqueKey& key) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    GrTextureProxy* proxy = fUniquelyKeyedProxies.find(key);
    if (proxy) {
        return sk_ref_sp(proxy);
    }
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS
sk_sp<GrTextureProxy> GrProxyProvider::testingOnly_createInstantiatedProxy(
        SkISize dimensions,
        const GrBackendFormat& format,
        GrRenderable renderable,
        int renderTargetSampleCnt,
        SkBackingFit fit,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    auto direct = fImageContext->asDirectContext();
    if (!direct) {
        return nullptr;
    }

    if (this->caps()->isFormatCompressed(format)) {
        // TODO: Allow this to go to GrResourceProvider::createCompressedTexture() once we no longer
        // rely on GrColorType to get a swizzle for the proxy.
        return nullptr;
    }

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();
    sk_sp<GrTexture> tex;

    if (SkBackingFit::kApprox == fit) {
        tex = resourceProvider->createApproxTexture(dimensions,
                                                    format,
                                                    format.textureType(),
                                                    renderable,
                                                    renderTargetSampleCnt,
                                                    isProtected);
    } else {
        tex = resourceProvider->createTexture(dimensions,
                                              format,
                                              format.textureType(),
                                              renderable,
                                              renderTargetSampleCnt,
                                              GrMipmapped::kNo,
                                              budgeted,
                                              isProtected);
    }
    if (!tex) {
        return nullptr;
    }

    return this->createWrapped(std::move(tex), UseAllocator::kYes);
}

sk_sp<GrTextureProxy> GrProxyProvider::testingOnly_createInstantiatedProxy(
        SkISize dimensions,
        GrColorType colorType,
        GrRenderable renderable,
        int renderTargetSampleCnt,
        SkBackingFit fit,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    auto format = this->caps()->getDefaultBackendFormat(colorType, renderable);
    return this->testingOnly_createInstantiatedProxy(dimensions,
                                                     format,
                                                     renderable,
                                                     renderTargetSampleCnt,
                                                     fit,
                                                     budgeted,
                                                     isProtected);
}

sk_sp<GrTextureProxy> GrProxyProvider::testingOnly_createWrapped(sk_sp<GrTexture> tex) {
    return this->createWrapped(std::move(tex), UseAllocator::kYes);
}
#endif

sk_sp<GrTextureProxy> GrProxyProvider::createWrapped(sk_sp<GrTexture> tex,
                                                     UseAllocator useAllocator) {
#ifdef SK_DEBUG
    if (tex->getUniqueKey().isValid()) {
        SkASSERT(!this->findProxyByUniqueKey(tex->getUniqueKey()));
    }
#endif

    if (tex->asRenderTarget()) {
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(std::move(tex), useAllocator,
                                                                    this->isDDLProvider()));
    } else {
        return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex), useAllocator,
                                                        this->isDDLProvider()));
    }
}

sk_sp<GrTextureProxy> GrProxyProvider::findOrCreateProxyByUniqueKey(const GrUniqueKey& key,
                                                                    UseAllocator useAllocator) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> result = this->findProxyByUniqueKey(key);
    if (result) {
        return result;
    }

    auto direct = fImageContext->asDirectContext();
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

    result = this->createWrapped(std::move(texture), useAllocator);
    SkASSERT(result->getUniqueKey() == key);
    // createWrapped should've added this for us
    SkASSERT(fUniquelyKeyedProxies.find(key));
    return result;
}

GrSurfaceProxyView GrProxyProvider::findCachedProxyWithColorTypeFallback(const GrUniqueKey& key,
                                                                         GrSurfaceOrigin origin,
                                                                         GrColorType ct,
                                                                         int sampleCnt) {
    auto proxy = this->findOrCreateProxyByUniqueKey(key);
    if (!proxy) {
        return {};
    }
    const GrCaps* caps = fImageContext->priv().caps();

    // Assume that we used a fallback color type if and only if the proxy is renderable.
    if (proxy->asRenderTargetProxy()) {
        GrBackendFormat expectedFormat;
        std::tie(ct, expectedFormat) = caps->getFallbackColorTypeAndFormat(ct, sampleCnt);
        SkASSERT(expectedFormat == proxy->backendFormat());
    }
    GrSwizzle swizzle = caps->getReadSwizzle(proxy->backendFormat(), ct);
    return {std::move(proxy), origin, swizzle};
}

sk_sp<GrTextureProxy> GrProxyProvider::createProxyFromBitmap(const SkBitmap& bitmap,
                                                             GrMipmapped mipMapped,
                                                             SkBackingFit fit,
                                                             SkBudgeted budgeted) {
    ASSERT_SINGLE_OWNER
    SkASSERT(fit == SkBackingFit::kExact || mipMapped == GrMipmapped::kNo);

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!SkImageInfoIsValid(bitmap.info())) {
        return nullptr;
    }

    ATRACE_ANDROID_FRAMEWORK("Upload %sTexture [%ux%u]",
                             GrMipmapped::kYes == mipMapped ? "MipMap " : "",
                             bitmap.width(), bitmap.height());

    // In non-ddl we will always instantiate right away. Thus we never want to copy the SkBitmap
    // even if its mutable. In ddl, if the bitmap is mutable then we must make a copy since the
    // upload of the data to the gpu can happen at anytime and the bitmap may change by then.
    SkBitmap copyBitmap = bitmap;
    if (!this->renderingDirectly() && !bitmap.isImmutable()) {
        copyBitmap.allocPixels();
        if (!bitmap.readPixels(copyBitmap.pixmap())) {
            return nullptr;
        }
        copyBitmap.setImmutable();
    }

    sk_sp<GrTextureProxy> proxy;
    if (mipMapped == GrMipmapped::kNo ||
        0 == SkMipmap::ComputeLevelCount(copyBitmap.width(), copyBitmap.height())) {
        proxy = this->createNonMippedProxyFromBitmap(copyBitmap, fit, budgeted);
    } else {
        proxy = this->createMippedProxyFromBitmap(copyBitmap, budgeted);
    }

    if (!proxy) {
        return nullptr;
    }

    auto direct = fImageContext->asDirectContext();
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

sk_sp<GrTextureProxy> GrProxyProvider::createNonMippedProxyFromBitmap(const SkBitmap& bitmap,
                                                                      SkBackingFit fit,
                                                                      SkBudgeted budgeted) {
    auto dims = bitmap.dimensions();

    auto colorType = SkColorTypeToGrColorType(bitmap.colorType());
    GrBackendFormat format = this->caps()->getDefaultBackendFormat(colorType, GrRenderable::kNo);
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [bitmap](GrResourceProvider* resourceProvider, const LazySurfaceDesc& desc) {
                SkASSERT(desc.fMipmapped == GrMipmapped::kNo);
                GrMipLevel mipLevel = {bitmap.getPixels(), bitmap.rowBytes(), nullptr};
                auto colorType = SkColorTypeToGrColorType(bitmap.colorType());
                return LazyCallbackResult(resourceProvider->createTexture(
                        desc.fDimensions,
                        desc.fFormat,
                        desc.fTextureType,
                        colorType,
                        desc.fRenderable,
                        desc.fSampleCnt,
                        desc.fBudgeted,
                        desc.fFit,
                        desc.fProtected,
                        mipLevel));
            },
            format, dims, GrMipmapped::kNo, GrMipmapStatus::kNotAllocated,
            GrInternalSurfaceFlags::kNone, fit, budgeted, GrProtected::kNo, UseAllocator::kYes);

    if (!proxy) {
        return nullptr;
    }
    SkASSERT(proxy->dimensions() == bitmap.dimensions());
    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::createMippedProxyFromBitmap(const SkBitmap& bitmap,
                                                                   SkBudgeted budgeted) {
    SkASSERT(this->caps()->mipmapSupport());

    auto colorType = SkColorTypeToGrColorType(bitmap.colorType());
    GrBackendFormat format = this->caps()->getDefaultBackendFormat(colorType, GrRenderable::kNo);
    if (!format.isValid()) {
        return nullptr;
    }

    sk_sp<SkMipmap> mipmaps(SkMipmap::Build(bitmap.pixmap(), nullptr));
    if (!mipmaps) {
        return nullptr;
    }

    auto dims = bitmap.dimensions();

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [bitmap, mipmaps](GrResourceProvider* resourceProvider, const LazySurfaceDesc& desc) {
                const int mipLevelCount = mipmaps->countLevels() + 1;
                std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipLevelCount]);
                auto colorType = SkColorTypeToGrColorType(bitmap.colorType());

                texels[0].fPixels = bitmap.getPixels();
                texels[0].fRowBytes = bitmap.rowBytes();

                for (int i = 1; i < mipLevelCount; ++i) {
                    SkMipmap::Level generatedMipLevel;
                    mipmaps->getLevel(i - 1, &generatedMipLevel);
                    texels[i].fPixels = generatedMipLevel.fPixmap.addr();
                    texels[i].fRowBytes = generatedMipLevel.fPixmap.rowBytes();
                    SkASSERT(texels[i].fPixels);
                    SkASSERT(generatedMipLevel.fPixmap.colorType() == bitmap.colorType());
                }
                return LazyCallbackResult(resourceProvider->createTexture(
                        desc.fDimensions,
                        desc.fFormat,
                        desc.fTextureType,
                        colorType,
                        GrRenderable::kNo,
                        1,
                        desc.fBudgeted,
                        GrMipMapped::kYes,
                        GrProtected::kNo,
                        texels.get()));
            },
            format, dims, GrMipmapped::kYes, GrMipmapStatus::kValid, GrInternalSurfaceFlags::kNone,
            SkBackingFit::kExact, budgeted, GrProtected::kNo, UseAllocator::kYes);

    if (!proxy) {
        return nullptr;
    }

    SkASSERT(proxy->dimensions() == bitmap.dimensions());

    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::createProxy(const GrBackendFormat& format,
                                                   SkISize dimensions,
                                                   GrRenderable renderable,
                                                   int renderTargetSampleCnt,
                                                   GrMipmapped mipMapped,
                                                   SkBackingFit fit,
                                                   SkBudgeted budgeted,
                                                   GrProtected isProtected,
                                                   GrInternalSurfaceFlags surfaceFlags,
                                                   GrSurfaceProxy::UseAllocator useAllocator) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }

    const GrCaps* caps = this->caps();

    if (caps->isFormatCompressed(format)) {
        // Deferred proxies for compressed textures are not supported.
        return nullptr;
    }

    if (GrMipmapped::kYes == mipMapped) {
        // SkMipmap doesn't include the base level in the level count so we have to add 1
        int mipCount = SkMipmap::ComputeLevelCount(dimensions.fWidth, dimensions.fHeight) + 1;
        if (1 == mipCount) {
            mipMapped = GrMipmapped::kNo;
        }
    }

    if (!caps->validateSurfaceParams(dimensions,
                                     format,
                                     renderable,
                                     renderTargetSampleCnt,
                                     mipMapped,
                                     GrTextureType::k2D)) {
        return nullptr;
    }
    GrMipmapStatus mipmapStatus = (GrMipmapped::kYes == mipMapped)
            ? GrMipmapStatus::kDirty
            : GrMipmapStatus::kNotAllocated;
    if (renderable == GrRenderable::kYes) {
        renderTargetSampleCnt = caps->getRenderTargetSampleCount(renderTargetSampleCnt, format);
        SkASSERT(renderTargetSampleCnt);
        GrInternalSurfaceFlags extraFlags = caps->getExtraSurfaceFlagsForDeferredRT();
        // We know anything we instantiate later from this deferred path will be
        // both texturable and renderable
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(
                *caps, format, dimensions, renderTargetSampleCnt, mipMapped, mipmapStatus, fit,
                budgeted, isProtected, surfaceFlags | extraFlags, useAllocator,
                this->isDDLProvider()));
    }

    return sk_sp<GrTextureProxy>(new GrTextureProxy(format, dimensions, mipMapped, mipmapStatus,
                                                    fit, budgeted, isProtected, surfaceFlags,
                                                    useAllocator, this->isDDLProvider()));
}

sk_sp<GrTextureProxy> GrProxyProvider::createCompressedTextureProxy(
        SkISize dimensions, SkBudgeted budgeted, GrMipmapped mipMapped, GrProtected isProtected,
        SkImage::CompressionType compressionType, sk_sp<SkData> data) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }

    GrBackendFormat format = this->caps()->getBackendFormatFromCompressionType(compressionType);

    if (!this->caps()->isFormatTexturable(format, GrTextureType::k2D)) {
        return nullptr;
    }

    GrMipmapStatus mipmapStatus = (GrMipmapped::kYes == mipMapped) ? GrMipmapStatus::kValid
                                                                   : GrMipmapStatus::kNotAllocated;

    sk_sp<GrTextureProxy> proxy = this->createLazyProxy(
            [data](GrResourceProvider* resourceProvider, const LazySurfaceDesc& desc) {
                return LazyCallbackResult(resourceProvider->createCompressedTexture(
                        desc.fDimensions, desc.fFormat, desc.fBudgeted, desc.fMipmapped,
                        desc.fProtected, data.get()));
            },
            format, dimensions, mipMapped, mipmapStatus,GrInternalSurfaceFlags::kReadOnly,
            SkBackingFit::kExact, SkBudgeted::kYes, GrProtected::kNo, UseAllocator::kYes);

    if (!proxy) {
        return nullptr;
    }

    auto direct = fImageContext->asDirectContext();
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
                                                          GrWrapOwnership ownership,
                                                          GrWrapCacheable cacheable,
                                                          GrIOType ioType,
                                                          sk_sp<GrRefCntedCallback> releaseHelper) {
    SkASSERT(ioType != kWrite_GrIOType);

    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    auto direct = fImageContext->asDirectContext();
    if (!direct) {
        return nullptr;
    }

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sk_sp<GrTexture> tex =
            resourceProvider->wrapBackendTexture(backendTex, ownership, cacheable, ioType);
    if (!tex) {
        return nullptr;
    }

    if (releaseHelper) {
        tex->setRelease(std::move(releaseHelper));
    }

    SkASSERT(!tex->asRenderTarget());  // Strictly a GrTexture
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(GrBudgetedType::kBudgeted != tex->resourcePriv().budgetedType());

    return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex), UseAllocator::kNo,
                                                    this->isDDLProvider()));
}

sk_sp<GrTextureProxy> GrProxyProvider::wrapCompressedBackendTexture(
        const GrBackendTexture& beTex,
        GrWrapOwnership ownership,
        GrWrapCacheable cacheable,
        sk_sp<GrRefCntedCallback> releaseHelper) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    auto direct = fImageContext->asDirectContext();
    if (!direct) {
        return nullptr;
    }

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sk_sp<GrTexture> tex = resourceProvider->wrapCompressedBackendTexture(beTex, ownership,
                                                                          cacheable);
    if (!tex) {
        return nullptr;
    }

    if (releaseHelper) {
        tex->setRelease(std::move(releaseHelper));
    }

    SkASSERT(!tex->asRenderTarget());  // Strictly a GrTexture
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(GrBudgetedType::kBudgeted != tex->resourcePriv().budgetedType());

    return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(tex), UseAllocator::kNo,
                                                    this->isDDLProvider()));
}

sk_sp<GrTextureProxy> GrProxyProvider::wrapRenderableBackendTexture(
        const GrBackendTexture& backendTex,
        int sampleCnt,
        GrWrapOwnership ownership,
        GrWrapCacheable cacheable,
        sk_sp<GrRefCntedCallback> releaseHelper) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    auto direct = fImageContext->asDirectContext();
    if (!direct) {
        return nullptr;
    }

    const GrCaps* caps = this->caps();

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sampleCnt = caps->getRenderTargetSampleCount(sampleCnt, backendTex.getBackendFormat());
    SkASSERT(sampleCnt);

    sk_sp<GrTexture> tex = resourceProvider->wrapRenderableBackendTexture(
            backendTex, sampleCnt, ownership, cacheable);
    if (!tex) {
        return nullptr;
    }

    if (releaseHelper) {
        tex->setRelease(std::move(releaseHelper));
    }

    SkASSERT(tex->asRenderTarget());  // A GrTextureRenderTarget
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(GrBudgetedType::kBudgeted != tex->resourcePriv().budgetedType());

    return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(std::move(tex), UseAllocator::kNo,
                                                                this->isDDLProvider()));
}

sk_sp<GrSurfaceProxy> GrProxyProvider::wrapBackendRenderTarget(
        const GrBackendRenderTarget& backendRT,
        sk_sp<GrRefCntedCallback> releaseHelper) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    auto direct = fImageContext->asDirectContext();
    if (!direct) {
        return nullptr;
    }

    GrResourceProvider* resourceProvider = direct->priv().resourceProvider();

    sk_sp<GrRenderTarget> rt = resourceProvider->wrapBackendRenderTarget(backendRT);
    if (!rt) {
        return nullptr;
    }

    if (releaseHelper) {
        rt->setRelease(std::move(releaseHelper));
    }

    SkASSERT(!rt->asTexture());  // A GrRenderTarget that's not textureable
    SkASSERT(!rt->getUniqueKey().isValid());
    // Make sure we match how we created the proxy with SkBudgeted::kNo
    SkASSERT(GrBudgetedType::kBudgeted != rt->resourcePriv().budgetedType());

    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(std::move(rt), UseAllocator::kNo));
}

#ifdef SK_VULKAN
sk_sp<GrRenderTargetProxy> GrProxyProvider::wrapVulkanSecondaryCBAsRenderTarget(
        const SkImageInfo& imageInfo, const GrVkDrawableInfo& vkInfo) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    // This is only supported on a direct GrContext.
    auto direct = fImageContext->asDirectContext();
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

    if (!this->caps()->isFormatAsColorTypeRenderable(
            colorType, GrBackendFormat::MakeVk(vkInfo.fFormat), /*sampleCount=*/1)) {
        return nullptr;
    }

    return sk_sp<GrRenderTargetProxy>(new GrRenderTargetProxy(
            std::move(rt), UseAllocator::kNo, GrRenderTargetProxy::WrapsVkSecondaryCB::kYes));
}
#else
sk_sp<GrRenderTargetProxy> GrProxyProvider::wrapVulkanSecondaryCBAsRenderTarget(
        const SkImageInfo&, const GrVkDrawableInfo&) {
    return nullptr;
}
#endif

sk_sp<GrTextureProxy> GrProxyProvider::CreatePromiseProxy(GrContextThreadSafeProxy* threadSafeProxy,
                                                          LazyInstantiateCallback&& callback,
                                                          const GrBackendFormat& format,
                                                          SkISize dimensions,
                                                          GrMipmapped mipMapped) {
    if (threadSafeProxy->priv().abandoned()) {
        return nullptr;
    }
    SkASSERT((dimensions.fWidth <= 0 && dimensions.fHeight <= 0) ||
             (dimensions.fWidth >  0 && dimensions.fHeight >  0));

    if (dimensions.fWidth > threadSafeProxy->priv().caps()->maxTextureSize() ||
        dimensions.fHeight > threadSafeProxy->priv().caps()->maxTextureSize()) {
        return nullptr;
    }
    // Ganesh assumes that, when wrapping a mipmapped backend texture from a client, that its
    // mipmaps are fully fleshed out.
    GrMipmapStatus mipmapStatus = (GrMipmapped::kYes == mipMapped) ? GrMipmapStatus::kValid
                                                                   : GrMipmapStatus::kNotAllocated;

    // We pass kReadOnly here since we should treat content of the client's texture as immutable.
    // The promise API provides no way for the client to indicate that the texture is protected.
    auto proxy = sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(callback),
                                                          format,
                                                          dimensions,
                                                          mipMapped,
                                                          mipmapStatus,
                                                          SkBackingFit::kExact,
                                                          SkBudgeted::kNo,
                                                          GrProtected::kNo,
                                                          GrInternalSurfaceFlags::kReadOnly,
                                                          GrSurfaceProxy::UseAllocator::kYes,
                                                          GrDDLProvider::kYes));
    proxy->priv().setIsPromiseProxy();
    return proxy;
}

sk_sp<GrTextureProxy> GrProxyProvider::createLazyProxy(LazyInstantiateCallback&& callback,
                                                       const GrBackendFormat& format,
                                                       SkISize dimensions,
                                                       GrMipmapped mipMapped,
                                                       GrMipmapStatus mipmapStatus,
                                                       GrInternalSurfaceFlags surfaceFlags,
                                                       SkBackingFit fit,
                                                       SkBudgeted budgeted,
                                                       GrProtected isProtected,
                                                       GrSurfaceProxy::UseAllocator useAllocator) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    SkASSERT((dimensions.fWidth <= 0 && dimensions.fHeight <= 0) ||
             (dimensions.fWidth >  0 && dimensions.fHeight >  0));

    if (!format.isValid() || format.backend() != fImageContext->backend()) {
        return nullptr;
    }

    if (dimensions.fWidth > this->caps()->maxTextureSize() ||
        dimensions.fHeight > this->caps()->maxTextureSize()) {
        return nullptr;
    }

    return sk_sp<GrTextureProxy>(new GrTextureProxy(std::move(callback),
                                                    format,
                                                    dimensions,
                                                    mipMapped,
                                                    mipmapStatus,
                                                    fit,
                                                    budgeted,
                                                    isProtected,
                                                    surfaceFlags,
                                                    useAllocator,
                                                    this->isDDLProvider()));
}

sk_sp<GrRenderTargetProxy> GrProxyProvider::createLazyRenderTargetProxy(
        LazyInstantiateCallback&& callback,
        const GrBackendFormat& format,
        SkISize dimensions,
        int sampleCnt,
        GrInternalSurfaceFlags surfaceFlags,
        const TextureInfo* textureInfo,
        GrMipmapStatus mipmapStatus,
        SkBackingFit fit,
        SkBudgeted budgeted,
        GrProtected isProtected,
        bool wrapsVkSecondaryCB,
        UseAllocator useAllocator) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    SkASSERT((dimensions.fWidth <= 0 && dimensions.fHeight <= 0) ||
             (dimensions.fWidth >  0 && dimensions.fHeight >  0));

    if (dimensions.fWidth > this->caps()->maxRenderTargetSize() ||
        dimensions.fHeight > this->caps()->maxRenderTargetSize()) {
        return nullptr;
    }

    if (textureInfo) {
        // Wrapped vulkan secondary command buffers don't support texturing since we won't have an
        // actual VkImage to texture from.
        SkASSERT(!wrapsVkSecondaryCB);
        return sk_sp<GrRenderTargetProxy>(new GrTextureRenderTargetProxy(
                *this->caps(), std::move(callback), format, dimensions, sampleCnt,
                textureInfo->fMipmapped, mipmapStatus, fit, budgeted, isProtected, surfaceFlags,
                useAllocator, this->isDDLProvider()));
    }

    GrRenderTargetProxy::WrapsVkSecondaryCB vkSCB =
            wrapsVkSecondaryCB ? GrRenderTargetProxy::WrapsVkSecondaryCB::kYes
                               : GrRenderTargetProxy::WrapsVkSecondaryCB::kNo;

    return sk_sp<GrRenderTargetProxy>(
            new GrRenderTargetProxy(std::move(callback), format, dimensions, sampleCnt, fit,
                                    budgeted, isProtected, surfaceFlags, useAllocator, vkSCB));
}

sk_sp<GrTextureProxy> GrProxyProvider::MakeFullyLazyProxy(LazyInstantiateCallback&& callback,
                                                          const GrBackendFormat& format,
                                                          GrRenderable renderable,
                                                          int renderTargetSampleCnt,
                                                          GrProtected isProtected,
                                                          const GrCaps& caps,
                                                          UseAllocator useAllocator) {
    if (!format.isValid()) {
        return nullptr;
    }

    SkASSERT(renderTargetSampleCnt == 1 || renderable == GrRenderable::kYes);
    // TODO: If we ever have callers requesting specific surface flags then we shouldn't use the
    // extra deferred flags here. Instead those callers should all pass in exactly what they want.
    // However, as of today all uses of this essentially create a deferred proxy in the end.
    GrInternalSurfaceFlags surfaceFlags = caps.getExtraSurfaceFlagsForDeferredRT();

    // MakeFullyLazyProxy is only called at flush time so we know these texture proxies are
    // not being created by a DDL provider.
    static constexpr SkISize kLazyDims = {-1, -1};
    if (GrRenderable::kYes == renderable) {
        return sk_sp<GrTextureProxy>(new GrTextureRenderTargetProxy(
                caps, std::move(callback), format, kLazyDims, renderTargetSampleCnt,
                GrMipmapped::kNo, GrMipmapStatus::kNotAllocated, SkBackingFit::kApprox,
                SkBudgeted::kYes, isProtected, surfaceFlags, useAllocator, GrDDLProvider::kNo));
    } else {
        return sk_sp<GrTextureProxy>(
                new GrTextureProxy(std::move(callback), format, kLazyDims, GrMipmapped::kNo,
                                   GrMipmapStatus::kNotAllocated, SkBackingFit::kApprox,
                                   SkBudgeted::kYes, isProtected, surfaceFlags, useAllocator,
                                   GrDDLProvider::kNo));
    }
}

void GrProxyProvider::processInvalidUniqueKey(const GrUniqueKey& key, GrTextureProxy* proxy,
                                              InvalidateGPUResource invalidateGPUResource) {
    this->processInvalidUniqueKeyImpl(key, proxy, invalidateGPUResource, RemoveTableEntry::kYes);
}

void GrProxyProvider::processInvalidUniqueKeyImpl(const GrUniqueKey& key, GrTextureProxy* proxy,
                                                  InvalidateGPUResource invalidateGPUResource,
                                                  RemoveTableEntry removeTableEntry) {
    SkASSERT(key.isValid());

    if (!proxy) {
        proxy = fUniquelyKeyedProxies.find(key);
    }
    SkASSERT(!proxy || proxy->getUniqueKey() == key);

    // Locate the corresponding GrGpuResource (if it needs to be invalidated) before clearing the
    // proxy's unique key. We must do it in this order because 'key' may alias the proxy's key.
    sk_sp<GrGpuResource> invalidGpuResource;
    if (InvalidateGPUResource::kYes == invalidateGPUResource) {
        auto direct = fImageContext->asDirectContext();
        if (direct) {
            GrResourceProvider* resourceProvider = direct->priv().resourceProvider();
            invalidGpuResource = resourceProvider->findByUniqueKey<GrGpuResource>(key);
        }
        SkASSERT(!invalidGpuResource || invalidGpuResource->getUniqueKey() == key);
    }

    // Note: this method is called for the whole variety of GrGpuResources so often 'key'
    // will not be in 'fUniquelyKeyedProxies'.
    if (proxy) {
        if (removeTableEntry == RemoveTableEntry::kYes) {
            fUniquelyKeyedProxies.remove(key);
        }
        proxy->cacheAccess().clearUniqueKey();
    }

    if (invalidGpuResource) {
        invalidGpuResource->resourcePriv().removeUniqueKey();
    }
}

GrDDLProvider GrProxyProvider::isDDLProvider() const {
    return fImageContext->asDirectContext() ? GrDDLProvider::kNo : GrDDLProvider::kYes;
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
    fUniquelyKeyedProxies.foreach([&](GrTextureProxy* proxy){
        proxy->fProxyProvider = nullptr;
    });
}

void GrProxyProvider::removeAllUniqueKeys() {
    fUniquelyKeyedProxies.foreach([&](GrTextureProxy* proxy){
        // It's not safe to remove table entries while iterating with foreach(),
        // but since we're going to remove them all anyway, simply save that for the end.
        this->processInvalidUniqueKeyImpl(proxy->getUniqueKey(), proxy,
                                          InvalidateGPUResource::kNo,
                                          RemoveTableEntry::kNo);
    });
    // Removing all those table entries is safe now.
    fUniquelyKeyedProxies.reset();
}

bool GrProxyProvider::renderingDirectly() const {
    return fImageContext->asDirectContext();
}
