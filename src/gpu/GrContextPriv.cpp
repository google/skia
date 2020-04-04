/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrContextPriv.h"

#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/gpu/GrTexture.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrSkSLFPFactoryCache.h"
#include "src/gpu/GrSurfaceContextPriv.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTextureContext.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/generated/GrConfigConversionEffect.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"

#define ASSERT_OWNED_PROXY(P) \
    SkASSERT(!(P) || !((P)->peekTexture()) || (P)->peekTexture()->getContext() == fContext)
#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fContext->singleOwner());)
#define RETURN_VALUE_IF_ABANDONED(value) if (fContext->abandoned()) { return (value); }
#define RETURN_IF_ABANDONED RETURN_VALUE_IF_ABANDONED(void)

sk_sp<const GrCaps> GrContextPriv::refCaps() const {
    return fContext->refCaps();
}

sk_sp<GrSkSLFPFactoryCache> GrContextPriv::fpFactoryCache() {
    return fContext->fpFactoryCache();
}

void GrContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fContext->addOnFlushCallbackObject(onFlushCBObject);
}

std::unique_ptr<GrSurfaceContext> GrContextPriv::makeWrappedSurfaceContext(
        sk_sp<GrSurfaceProxy> proxy,
        GrColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* props) {
    return fContext->makeWrappedSurfaceContext(std::move(proxy), colorType, alphaType,
                                               std::move(colorSpace), props);
}

std::unique_ptr<GrTextureContext> GrContextPriv::makeDeferredTextureContext(
        SkBackingFit fit,
        int width,
        int height,
        GrColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    return fContext->makeDeferredTextureContext(fit, width, height, colorType, alphaType,
                                                std::move(colorSpace), mipMapped, origin, budgeted,
                                                isProtected);
}

std::unique_ptr<GrRenderTargetContext> GrContextPriv::makeDeferredRenderTargetContext(
        SkBackingFit fit,
        int width,
        int height,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        int sampleCnt,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        const SkSurfaceProps* surfaceProps,
        SkBudgeted budgeted,
        GrProtected isProtected) {
    return fContext->makeDeferredRenderTargetContext(fit, width, height, colorType,
                                                     std::move(colorSpace), sampleCnt, mipMapped,
                                                     origin, surfaceProps, budgeted, isProtected);
}

std::unique_ptr<GrRenderTargetContext> GrContextPriv::makeDeferredRenderTargetContextWithFallback(
        SkBackingFit fit, int width, int height, GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace, int sampleCnt, GrMipMapped mipMapped,
        GrSurfaceOrigin origin, const SkSurfaceProps* surfaceProps, SkBudgeted budgeted,
        GrProtected isProtected) {
    return fContext->makeDeferredRenderTargetContextWithFallback(
            fit, width, height, colorType, std::move(colorSpace), sampleCnt, mipMapped, origin,
            surfaceProps, budgeted, isProtected);
}

std::unique_ptr<GrTextureContext> GrContextPriv::makeBackendTextureContext(
        const GrBackendTexture& tex,
        GrSurfaceOrigin origin,
        GrColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace) {
    ASSERT_SINGLE_OWNER

    sk_sp<GrSurfaceProxy> proxy = this->proxyProvider()->wrapBackendTexture(
            tex, colorType, origin, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType);
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeTextureContext(std::move(proxy), colorType, alphaType,
                                                      std::move(colorSpace));
}

std::unique_ptr<GrRenderTargetContext> GrContextPriv::makeBackendTextureRenderTargetContext(
        const GrBackendTexture& tex,
        GrSurfaceOrigin origin,
        int sampleCnt,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* props,
        ReleaseProc releaseProc,
        ReleaseContext releaseCtx) {
    ASSERT_SINGLE_OWNER
    SkASSERT(sampleCnt > 0);

    sk_sp<GrTextureProxy> proxy(this->proxyProvider()->wrapRenderableBackendTexture(
            tex, origin, sampleCnt, colorType, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo,
            releaseProc, releaseCtx));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy), colorType,
                                                           std::move(colorSpace), props);
}

std::unique_ptr<GrRenderTargetContext> GrContextPriv::makeBackendRenderTargetRenderTargetContext(
        const GrBackendRenderTarget& backendRT,
        GrSurfaceOrigin origin,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* surfaceProps,
        ReleaseProc releaseProc,
        ReleaseContext releaseCtx) {
    ASSERT_SINGLE_OWNER

    sk_sp<GrSurfaceProxy> proxy = this->proxyProvider()->wrapBackendRenderTarget(
            backendRT, colorType, origin, releaseProc, releaseCtx);
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy), colorType,
                                                           std::move(colorSpace), surfaceProps);
}

std::unique_ptr<GrRenderTargetContext>
GrContextPriv::makeBackendTextureAsRenderTargetRenderTargetContext(const GrBackendTexture& tex,
                                                                   GrSurfaceOrigin origin,
                                                                   int sampleCnt,
                                                                   GrColorType colorType,
                                                                   sk_sp<SkColorSpace> colorSpace,
                                                                   const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER
    SkASSERT(sampleCnt > 0);
    sk_sp<GrSurfaceProxy> proxy(
            this->proxyProvider()->wrapBackendTextureAsRenderTarget(tex, colorType,
                                                                    origin, sampleCnt));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy), colorType,
                                                           std::move(colorSpace), props);
}

std::unique_ptr<GrRenderTargetContext> GrContextPriv::makeVulkanSecondaryCBRenderTargetContext(
        const SkImageInfo& imageInfo, const GrVkDrawableInfo& vkInfo, const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER
    sk_sp<GrSurfaceProxy> proxy(
            this->proxyProvider()->wrapVulkanSecondaryCBAsRenderTarget(imageInfo, vkInfo));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(
            std::move(proxy),
            SkColorTypeToGrColorType(imageInfo.colorType()),
            imageInfo.refColorSpace(),
            props);
}

GrSemaphoresSubmitted GrContextPriv::flushSurfaces(GrSurfaceProxy* proxies[], int numProxies,
                                                   const GrFlushInfo& info) {
    ASSERT_SINGLE_OWNER
    RETURN_VALUE_IF_ABANDONED(GrSemaphoresSubmitted::kNo)
    GR_CREATE_TRACE_MARKER_CONTEXT("GrContextPriv", "flushSurfaces", fContext);
    SkASSERT(numProxies >= 0);
    SkASSERT(!numProxies || proxies);
    for (int i = 0; i < numProxies; ++i) {
        SkASSERT(proxies[i]);
        ASSERT_OWNED_PROXY(proxies[i]);
    }
    return fContext->drawingManager()->flushSurfaces(
            proxies, numProxies, SkSurface::BackendSurfaceAccess::kNoAccess, info);
}

void GrContextPriv::flushSurface(GrSurfaceProxy* proxy) {
    this->flushSurfaces(proxy ? &proxy : nullptr, proxy ? 1 : 0, {});
}

void GrContextPriv::moveRenderTasksToDDL(SkDeferredDisplayList* ddl) {
    fContext->drawingManager()->moveRenderTasksToDDL(ddl);
}

void GrContextPriv::copyRenderTasksFromDDL(const SkDeferredDisplayList* ddl,
                                           GrRenderTargetProxy* newDest) {
    fContext->drawingManager()->copyRenderTasksFromDDL(ddl, newDest);
}

//////////////////////////////////////////////////////////////////////////////

#if GR_TEST_UTILS
void GrContextPriv::resetGpuStats() const {
#if GR_GPU_STATS
    fContext->fGpu->stats()->reset();
#endif
}

void GrContextPriv::dumpCacheStats(SkString* out) const {
#if GR_CACHE_STATS
    fContext->fResourceCache->dumpStats(out);
#endif
}

void GrContextPriv::dumpCacheStatsKeyValuePairs(SkTArray<SkString>* keys,
                                                SkTArray<double>* values) const {
#if GR_CACHE_STATS
    fContext->fResourceCache->dumpStatsKeyValuePairs(keys, values);
#endif
}

void GrContextPriv::printCacheStats() const {
    SkString out;
    this->dumpCacheStats(&out);
    SkDebugf("%s", out.c_str());
}

void GrContextPriv::dumpGpuStats(SkString* out) const {
#if GR_GPU_STATS
    return fContext->fGpu->stats()->dump(out);
#endif
}

void GrContextPriv::dumpGpuStatsKeyValuePairs(SkTArray<SkString>* keys,
                                              SkTArray<double>* values) const {
#if GR_GPU_STATS
    return fContext->fGpu->stats()->dumpKeyValuePairs(keys, values);
#endif
}

void GrContextPriv::printGpuStats() const {
    SkString out;
    this->dumpGpuStats(&out);
    SkDebugf("%s", out.c_str());
}

void GrContextPriv::testingOnly_setTextBlobCacheLimit(size_t bytes) {
    fContext->priv().getTextBlobCache()->setBudget(bytes);
}

sk_sp<SkImage> GrContextPriv::testingOnly_getFontAtlasImage(GrMaskFormat format, unsigned int index) {
    auto atlasManager = this->getAtlasManager();
    if (!atlasManager) {
        return nullptr;
    }

    unsigned int numActiveProxies;
    const GrSurfaceProxyView* views = atlasManager->getViews(format, &numActiveProxies);
    if (index >= numActiveProxies || !views || !views[index].proxy()) {
        return nullptr;
    }

    SkASSERT(views[index].proxy()->priv().isExact());
    sk_sp<SkImage> image(new SkImage_Gpu(sk_ref_sp(fContext), kNeedNewImageUniqueID,
                                         kPremul_SkAlphaType, views[index], nullptr));
    return image;
}

void GrContextPriv::testingOnly_purgeAllUnlockedResources() {
    fContext->fResourceCache->purgeAllUnlocked();
}

void GrContextPriv::testingOnly_flushAndRemoveOnFlushCallbackObject(GrOnFlushCallbackObject* cb) {
    fContext->flush();
    fContext->drawingManager()->testingOnly_removeOnFlushCallbackObject(cb);
}
#endif

bool GrContextPriv::validPMUPMConversionExists() {
    ASSERT_SINGLE_OWNER
    if (!fContext->fDidTestPMConversions) {
        fContext->fPMUPMConversionsRoundTrip =
                GrConfigConversionEffect::TestForPreservingPMConversions(fContext);
        fContext->fDidTestPMConversions = true;
    }

    // The PM<->UPM tests fail or succeed together so we only need to check one.
    return fContext->fPMUPMConversionsRoundTrip;
}

std::unique_ptr<GrFragmentProcessor> GrContextPriv::createPMToUPMEffect(
        std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    // We should have already called this->priv().validPMUPMConversionExists() in this case
    SkASSERT(fContext->fDidTestPMConversions);
    // ...and it should have succeeded
    SkASSERT(this->validPMUPMConversionExists());

    return GrConfigConversionEffect::Make(std::move(fp), PMConversion::kToUnpremul);
}

std::unique_ptr<GrFragmentProcessor> GrContextPriv::createUPMToPMEffect(
        std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    // We should have already called this->priv().validPMUPMConversionExists() in this case
    SkASSERT(fContext->fDidTestPMConversions);
    // ...and it should have succeeded
    SkASSERT(this->validPMUPMConversionExists());

    return GrConfigConversionEffect::Make(std::move(fp), PMConversion::kToPremul);
}
