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

sk_sp<GrOpMemoryPool> GrContextPriv::refOpMemoryPool() {
    return fContext->refOpMemoryPool();
}

void GrContextPriv::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fContext->addOnFlushCallbackObject(onFlushCBObject);
}

sk_sp<GrSurfaceContext> GrContextPriv::makeWrappedSurfaceContext(
                                                    sk_sp<GrSurfaceProxy> proxy,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    const SkSurfaceProps* props) {
    return fContext->makeWrappedSurfaceContext(std::move(proxy), std::move(colorSpace), props);
}

sk_sp<GrSurfaceContext> GrContextPriv::makeDeferredSurfaceContext(
                                                    const GrBackendFormat& format,
                                                    const GrSurfaceDesc& dstDesc,
                                                    GrSurfaceOrigin origin,
                                                    GrMipMapped mipMapped,
                                                    SkBackingFit fit,
                                                    SkBudgeted isDstBudgeted,
                                                    sk_sp<SkColorSpace> colorSpace,
                                                    const SkSurfaceProps* props) {
    return fContext->makeDeferredSurfaceContext(format, dstDesc, origin, mipMapped, fit,
                                                isDstBudgeted, std::move(colorSpace), props);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeDeferredRenderTargetContext(
        const GrBackendFormat& format, SkBackingFit fit, int width, int height,
        GrPixelConfig config, sk_sp<SkColorSpace> colorSpace, int sampleCnt, GrMipMapped mipMapped,
        GrSurfaceOrigin origin, const SkSurfaceProps* surfaceProps, SkBudgeted budgeted,
        GrProtected isProtected) {
    return fContext->makeDeferredRenderTargetContext(format, fit, width, height, config,
                                                     std::move(colorSpace), sampleCnt, mipMapped,
                                                     origin, surfaceProps, budgeted, isProtected);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeDeferredRenderTargetContextWithFallback(
                                        const GrBackendFormat& format,
                                        SkBackingFit fit,
                                        int width, int height,
                                        GrPixelConfig config,
                                        sk_sp<SkColorSpace> colorSpace,
                                        int sampleCnt,
                                        GrMipMapped mipMapped,
                                        GrSurfaceOrigin origin,
                                        const SkSurfaceProps* surfaceProps,
                                        SkBudgeted budgeted) {
    return fContext->makeDeferredRenderTargetContextWithFallback(format, fit, width, height, config,
                                                                 std::move(colorSpace), sampleCnt,
                                                                 mipMapped, origin, surfaceProps,
                                                                 budgeted);
}

sk_sp<GrTextureContext> GrContextPriv::makeBackendTextureContext(const GrBackendTexture& tex,
                                                                 GrSurfaceOrigin origin,
                                                                 sk_sp<SkColorSpace> colorSpace) {
    ASSERT_SINGLE_OWNER

    sk_sp<GrSurfaceProxy> proxy = this->proxyProvider()->wrapBackendTexture(
            tex, origin, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, kRW_GrIOType);
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeTextureContext(std::move(proxy), std::move(colorSpace));
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendTextureRenderTargetContext(
                                                                   const GrBackendTexture& tex,
                                                                   GrSurfaceOrigin origin,
                                                                   int sampleCnt,
                                                                   sk_sp<SkColorSpace> colorSpace,
                                                                   const SkSurfaceProps* props,
                                                                   ReleaseProc releaseProc,
                                                                   ReleaseContext releaseCtx) {
    ASSERT_SINGLE_OWNER
    SkASSERT(sampleCnt > 0);

    sk_sp<GrTextureProxy> proxy(this->proxyProvider()->wrapRenderableBackendTexture(
            tex, origin, sampleCnt, kBorrow_GrWrapOwnership, GrWrapCacheable::kNo, releaseProc,
            releaseCtx));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace), props);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendRenderTargetRenderTargetContext(
                                                const GrBackendRenderTarget& backendRT,
                                                GrSurfaceOrigin origin,
                                                sk_sp<SkColorSpace> colorSpace,
                                                const SkSurfaceProps* surfaceProps,
                                                ReleaseProc releaseProc,
                                                ReleaseContext releaseCtx) {
    ASSERT_SINGLE_OWNER

    sk_sp<GrSurfaceProxy> proxy = this->proxyProvider()->wrapBackendRenderTarget(
            backendRT, origin, releaseProc, releaseCtx);
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           surfaceProps);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeBackendTextureAsRenderTargetRenderTargetContext(
                                                     const GrBackendTexture& tex,
                                                     GrSurfaceOrigin origin,
                                                     int sampleCnt,
                                                     sk_sp<SkColorSpace> colorSpace,
                                                     const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER
    SkASSERT(sampleCnt > 0);
    sk_sp<GrSurfaceProxy> proxy(
            this->proxyProvider()->wrapBackendTextureAsRenderTarget(tex, origin, sampleCnt));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
                                                           std::move(colorSpace),
                                                           props);
}

sk_sp<GrRenderTargetContext> GrContextPriv::makeVulkanSecondaryCBRenderTargetContext(
        const SkImageInfo& imageInfo, const GrVkDrawableInfo& vkInfo, const SkSurfaceProps* props) {
    ASSERT_SINGLE_OWNER
    sk_sp<GrSurfaceProxy> proxy(
            this->proxyProvider()->wrapVulkanSecondaryCBAsRenderTarget(imageInfo, vkInfo));
    if (!proxy) {
        return nullptr;
    }

    return this->drawingManager()->makeRenderTargetContext(std::move(proxy),
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

void GrContextPriv::moveOpListsToDDL(SkDeferredDisplayList* ddl) {
    fContext->drawingManager()->moveOpListsToDDL(ddl);
}

void GrContextPriv::copyOpListsFromDDL(const SkDeferredDisplayList* ddl,
                                       GrRenderTargetProxy* newDest) {
    fContext->drawingManager()->copyOpListsFromDDL(ddl, newDest);
}

//////////////////////////////////////////////////////////////////////////////
#ifdef SK_ENABLE_DUMP_GPU
#include "src/utils/SkJSONWriter.h"
SkString GrContextPriv::dump() const {
    SkDynamicMemoryWStream stream;
    SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);
    writer.beginObject();

    static const char* kBackendStr[] = {
        "Metal",
        "OpenGL",
        "Vulkan",
        "Mock",
    };
    GR_STATIC_ASSERT(0 == (unsigned)GrBackendApi::kMetal);
    GR_STATIC_ASSERT(1 == (unsigned)GrBackendApi::kOpenGL);
    GR_STATIC_ASSERT(2 == (unsigned)GrBackendApi::kVulkan);
    GR_STATIC_ASSERT(3 == (unsigned)GrBackendApi::kMock);
    writer.appendString("backend", kBackendStr[(unsigned)fContext->backend()]);

    writer.appendName("caps");
    fContext->caps()->dumpJSON(&writer);

    writer.appendName("gpu");
    fContext->fGpu->dumpJSON(&writer);

    // Flush JSON to the memory stream
    writer.endObject();
    writer.flush();

    // Null terminate the JSON data in the memory stream
    stream.write8(0);

    // Allocate a string big enough to hold all the data, then copy out of the stream
    SkString result(stream.bytesWritten());
    stream.copyToAndReset(result.writable_str());
    return result;
}
#endif

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
    const sk_sp<GrTextureProxy>* proxies = atlasManager->getProxies(format, &numActiveProxies);
    if (index >= numActiveProxies || !proxies || !proxies[index]) {
        return nullptr;
    }

    SkASSERT(proxies[index]->priv().isExact());
    sk_sp<SkImage> image(new SkImage_Gpu(sk_ref_sp(fContext), kNeedNewImageUniqueID,
                                         kPremul_SkAlphaType, proxies[index], nullptr));
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

//////////////////////////////////////////////////////////////////////////////

#include "src/core/SkMipMap.h"

GrBackendTexture GrContextPriv::createBackendTexture(const SkPixmap srcData[], int numLevels,
                                                     GrRenderable renderable) {
    if (!fContext->asDirectContext()) {
        return {};
    }

    if (this->abandoned()) {
        return {};
    }

    if (!srcData || !numLevels) {
        return {};
    }

    int baseWidth = srcData[0].width();
    int baseHeight = srcData[0].height();
    SkColorType colorType = srcData[0].colorType();

    if (numLevels > 1) {
        if (numLevels != SkMipMap::ComputeLevelCount(baseWidth, baseHeight) + 1) {
            return {};
        }

        int currentWidth = baseWidth;
        int currentHeight = baseHeight;
        for (int i = 1; i < numLevels; ++i) {
            currentWidth = SkTMax(1, currentWidth / 2);
            currentHeight = SkTMax(1, currentHeight / 2);

            if (srcData[i].colorType() != colorType) {
                return {};
            }

            if (srcData[i].width() != currentWidth || srcData[i].height() != currentHeight) {
                return {};
            }
        }
    }

    GrBackendFormat backendFormat = this->caps()->getBackendFormatFromColorType(colorType);
    if (!backendFormat.isValid()) {
        return {};
    }

    GrGpu* gpu = fContext->fGpu.get();

    // TODO: propagate the array of pixmaps interface to GrGpu
    return gpu->createBackendTexture(baseWidth, baseHeight, backendFormat,
                                     GrMipMapped::kNo, // TODO: use real mipmap setting here
                                     renderable, srcData[0].addr(), srcData[0].rowBytes(),
                                     nullptr, GrProtected::kNo);
}
