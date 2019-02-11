/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrBackendSemaphore.h"
#include "GrDrawingManager.h"
#include "GrGpu.h"
#include "GrMemoryPool.h"
#include "GrPathRendererChain.h"
#include "GrProxyProvider.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceCache.h"
#include "GrResourceProvider.h"
#include "GrSemaphore.h"
#include "GrSoftwarePathRenderer.h"
#include "GrTracing.h"
#include "SkDeferredDisplayList.h"
#include "SkGr.h"
#include "SkImageInfoPriv.h"
#include "SkMakeUnique.h"
#include "SkSurface_Gpu.h"
#include "SkTaskGroup.h"
#include "SkTraceMemoryDump.h"
#include "effects/GrConfigConversionEffect.h"
#include "effects/GrSkSLFP.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"
#include "text/GrTextBlobCache.h"
#include "text/GrTextContext.h"
#include <atomic>
#include <unordered_map>

#define ASSERT_OWNED_PROXY(P) \
    SkASSERT(!(P) || !((P)->peekTexture()) || (P)->peekTexture()->getContext() == this)

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this)
#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(this->singleOwner());)
#define RETURN_IF_ABANDONED if (this->abandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED if (this->abandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED if (this->abandoned()) { return nullptr; }

////////////////////////////////////////////////////////////////////////////////

GrContext::GrContext(GrBackendApi backend, const GrContextOptions& options, int32_t contextID)
        : INHERITED(backend, options, contextID) {
    fResourceCache = nullptr;
    fResourceProvider = nullptr;
    fGlyphCache = nullptr;
}

bool GrContext::init(sk_sp<const GrCaps> caps, sk_sp<GrSkSLFPFactoryCache> FPFactoryCache) {
    ASSERT_SINGLE_OWNER
    SkASSERT(fThreadSafeProxy); // needs to have been initialized by derived classes
    SkASSERT(this->proxyProvider());

    if (!INHERITED::init(std::move(caps), std::move(FPFactoryCache))) {
        return false;
    }

    SkASSERT(this->caps());

    if (fGpu) {
        fResourceCache = new GrResourceCache(this->caps(), this->singleOwner(), this->contextID());
        fResourceProvider = new GrResourceProvider(fGpu.get(), fResourceCache, this->singleOwner(),
                                                   this->options().fExplicitlyAllocateGPUResources);
    }

    if (fResourceCache) {
        fResourceCache->setProxyProvider(this->proxyProvider());
    }

    fDidTestPMConversions = false;

    GrPathRendererChain::Options prcOptions;
    prcOptions.fAllowPathMaskCaching = this->options().fAllowPathMaskCaching;
#if GR_TEST_UTILS
    prcOptions.fGpuPathRenderers = this->options().fGpuPathRenderers;
#endif
    if (this->options().fDisableCoverageCountingPaths) {
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kCoverageCounting;
    }
    if (this->options().fDisableDistanceFieldPaths) {
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
    }

    if (!fResourceCache) {
        // DDL TODO: remove this crippling of the path renderer chain
        // Disable the small path renderer bc of the proxies in the atlas. They need to be
        // unified when the opLists are added back to the destination drawing manager.
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kSmall;
        prcOptions.fGpuPathRenderers &= ~GpuPathRenderers::kStencilAndCover;
    }

    GrTextContext::Options textContextOptions;
    textContextOptions.fMaxDistanceFieldFontSize = this->options().fGlyphsAsPathsFontSize;
    textContextOptions.fMinDistanceFieldFontSize = this->options().fMinDistanceFieldFontSize;
    textContextOptions.fDistanceFieldVerticesAlwaysHaveW = false;
#if SK_SUPPORT_ATLAS_TEXT
    if (GrContextOptions::Enable::kYes == this->options().fDistanceFieldGlyphVerticesAlwaysHaveW) {
        textContextOptions.fDistanceFieldVerticesAlwaysHaveW = true;
    }
#endif

    bool explicitlyAllocatingResources = fResourceProvider
                                            ? fResourceProvider->explicitlyAllocateGPUResources()
                                            : false;
    fDrawingManager.reset(new GrDrawingManager(this, prcOptions, textContextOptions,
                                               this->singleOwner(), explicitlyAllocatingResources,
                                               this->options().fSortRenderTargets,
                                               this->options().fReduceOpListSplitting));

    fGlyphCache = new GrStrikeCache(this->caps(), this->options().fGlyphCacheTextureMaximumBytes);

    fTextBlobCache.reset(new GrTextBlobCache(TextBlobCacheOverBudgetCB, this, this->contextID()));

    // DDL TODO: we need to think through how the task group & persistent cache
    // get passed on to/shared between all the DDLRecorders created with this context.
    if (this->options().fExecutor) {
        fTaskGroup = skstd::make_unique<SkTaskGroup>(*this->options().fExecutor);
    }

    fPersistentCache = this->options().fPersistentCache;

    return true;
}

GrContext::~GrContext() {
    ASSERT_SINGLE_OWNER

    if (fDrawingManager) {
        fDrawingManager->cleanup();
    }
    delete fResourceProvider;
    delete fResourceCache;
    delete fGlyphCache;
}

sk_sp<GrContextThreadSafeProxy> GrContext::threadSafeProxy() {
    return fThreadSafeProxy;
}

//////////////////////////////////////////////////////////////////////////////

void GrContext::abandonContext() {
    if (this->abandoned()) {
        return;
    }

    INHERITED::abandonContext();

    fResourceProvider->abandon();

    // Need to cleanup the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    fDrawingManager->cleanup();

    // abandon first to so destructors
    // don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->disconnect(GrGpu::DisconnectType::kAbandon);

    fGlyphCache->freeAll();
    fTextBlobCache->freeAll();

}

void GrContext::releaseResourcesAndAbandonContext() {
    if (this->abandoned()) {
        return;
    }

    INHERITED::abandonContext();

    fResourceProvider->abandon();

    // Need to cleanup the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    fDrawingManager->cleanup();

    // Release all resources in the backend 3D API.
    fResourceCache->releaseAll();

    fGpu->disconnect(GrGpu::DisconnectType::kCleanup);

    fGlyphCache->freeAll();
    fTextBlobCache->freeAll();
}

void GrContext::resetGLTextureBindings() {
    if (this->abandoned() || this->backend() != GrBackendApi::kOpenGL) {
        return;
    }
    fGpu->resetTextureBindings();
}

void GrContext::resetContext(uint32_t state) {
    ASSERT_SINGLE_OWNER
    fGpu->markContextDirty(state);
}

void GrContext::freeGpuResources() {
    ASSERT_SINGLE_OWNER

    fGlyphCache->freeAll();

    fDrawingManager->freeGpuResources();

    fResourceCache->purgeAllUnlocked();
}

void GrContext::purgeUnlockedResources(bool scratchResourcesOnly) {
    ASSERT_SINGLE_OWNER
    fResourceCache->purgeUnlockedResources(scratchResourcesOnly);
    fResourceCache->purgeAsNeeded();
    fTextBlobCache->purgeStaleBlobs();
}

void GrContext::performDeferredCleanup(std::chrono::milliseconds msNotUsed) {
    ASSERT_SINGLE_OWNER

    auto purgeTime = GrStdSteadyClock::now() - msNotUsed;

    fResourceCache->purgeAsNeeded();
    fResourceCache->purgeResourcesNotUsedSince(purgeTime);

    if (auto ccpr = fDrawingManager->getCoverageCountingPathRenderer()) {
        ccpr->purgeCacheEntriesOlderThan(this->proxyProvider(), purgeTime);
    }

    fTextBlobCache->purgeStaleBlobs();
}

void GrContext::purgeUnlockedResources(size_t bytesToPurge, bool preferScratchResources) {
    ASSERT_SINGLE_OWNER
    fResourceCache->purgeUnlockedResources(bytesToPurge, preferScratchResources);
}

void GrContext::getResourceCacheUsage(int* resourceCount, size_t* resourceBytes) const {
    ASSERT_SINGLE_OWNER

    if (resourceCount) {
        *resourceCount = fResourceCache->getBudgetedResourceCount();
    }
    if (resourceBytes) {
        *resourceBytes = fResourceCache->getBudgetedResourceBytes();
    }
}

size_t GrContext::getResourceCachePurgeableBytes() const {
    ASSERT_SINGLE_OWNER
    return fResourceCache->getPurgeableBytes();
}

////////////////////////////////////////////////////////////////////////////////

int GrContext::maxTextureSize() const { return this->caps()->maxTextureSize(); }

int GrContext::maxRenderTargetSize() const { return this->caps()->maxRenderTargetSize(); }

bool GrContext::colorTypeSupportedAsImage(SkColorType colorType) const {
    GrPixelConfig config = SkColorType2GrPixelConfig(colorType);
    return this->caps()->isConfigTexturable(config);
}

int GrContext::maxSurfaceSampleCountForColorType(SkColorType colorType) const {
    GrPixelConfig config = SkColorType2GrPixelConfig(colorType);
    return this->caps()->maxRenderTargetSampleCount(config);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::TextBlobCacheOverBudgetCB(void* data) {
    SkASSERT(data);
    // TextBlobs are drawn at the SkGpuDevice level, therefore they cannot rely on
    // GrRenderTargetContext to perform a necessary flush.  The solution is to move drawText calls
    // to below the GrContext level, but this is not trivial because they call drawPath on
    // SkGpuDevice.
    GrContext* context = reinterpret_cast<GrContext*>(data);
    context->flush();
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::flush() {
    ASSERT_SINGLE_OWNER
    RETURN_IF_ABANDONED

    fDrawingManager->flush(nullptr);
}

GrSemaphoresSubmitted GrContext::flushAndSignalSemaphores(int numSemaphores,
                                                          GrBackendSemaphore signalSemaphores[]) {
    ASSERT_SINGLE_OWNER
    if (this->abandoned()) {
        return GrSemaphoresSubmitted::kNo;
    }

    return fDrawingManager->flush(nullptr, numSemaphores, signalSemaphores);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::storeVkPipelineCacheData() {
    if (fGpu) {
        fGpu->storeVkPipelineCacheData();
    }
}

////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrContext::createPMToUPMEffect(
        std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    // We should have already called this->validPMUPMConversionExists() in this case
    SkASSERT(fDidTestPMConversions);
    // ...and it should have succeeded
    SkASSERT(this->validPMUPMConversionExists());

    return GrConfigConversionEffect::Make(std::move(fp), PMConversion::kToUnpremul);
}

std::unique_ptr<GrFragmentProcessor> GrContext::createUPMToPMEffect(
        std::unique_ptr<GrFragmentProcessor> fp) {
    ASSERT_SINGLE_OWNER
    // We should have already called this->validPMUPMConversionExists() in this case
    SkASSERT(fDidTestPMConversions);
    // ...and it should have succeeded
    SkASSERT(this->validPMUPMConversionExists());

    return GrConfigConversionEffect::Make(std::move(fp), PMConversion::kToPremul);
}

bool GrContext::validPMUPMConversionExists() {
    ASSERT_SINGLE_OWNER
    if (!fDidTestPMConversions) {
        fPMUPMConversionsRoundTrip = GrConfigConversionEffect::TestForPreservingPMConversions(this);
        fDidTestPMConversions = true;
    }

    // The PM<->UPM tests fail or succeed together so we only need to check one.
    return fPMUPMConversionsRoundTrip;
}

bool GrContext::supportsDistanceFieldText() const {
    return this->caps()->shaderCaps()->supportsDistanceFieldText();
}

//////////////////////////////////////////////////////////////////////////////

// DDL TODO: remove 'maxResources'
void GrContext::getResourceCacheLimits(int* maxResources, size_t* maxResourceBytes) const {
    ASSERT_SINGLE_OWNER
    if (maxResources) {
        *maxResources = fResourceCache->getMaxResourceCount();
    }
    if (maxResourceBytes) {
        *maxResourceBytes = fResourceCache->getMaxResourceBytes();
    }
}

void GrContext::setResourceCacheLimits(int maxResources, size_t maxResourceBytes) {
    ASSERT_SINGLE_OWNER
    fResourceCache->setLimits(maxResources, maxResourceBytes);
}

//////////////////////////////////////////////////////////////////////////////
void GrContext::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    ASSERT_SINGLE_OWNER
    fResourceCache->dumpMemoryStatistics(traceMemoryDump);
    traceMemoryDump->dumpNumericValue("skia/gr_text_blob_cache", "size", "bytes",
                                      fTextBlobCache->usedBytes());
}

