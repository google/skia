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
#define RETURN_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED if (fDrawingManager->wasAbandoned()) { return nullptr; }

////////////////////////////////////////////////////////////////////////////////

GrContext::GrContext(GrBackendApi backend, const GrContextOptions& options, int32_t contextID)
        : INHERITED(backend, options, contextID) {
    fResourceCache = nullptr;
    fResourceProvider = nullptr;
//    fProxyProvider = nullptr;
    fGlyphCache = nullptr;
}

bool GrContext::init(sk_sp<const GrCaps> caps, sk_sp<GrSkSLFPFactoryCache> FPFactoryCache) {
    ASSERT_SINGLE_OWNER
    SkASSERT(fThreadSafeProxy); // needs to have been initialized by derived classes
    SkASSERT(this->caps());  // needs to have been initialized by derived classes
    SkASSERT(this->threadSafeProxy()); // needs to have been initialized by derived classes

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
//    delete fProxyProvider;
    delete fGlyphCache;
}

sk_sp<GrContextThreadSafeProxy> GrContext::threadSafeProxy() {
    return fThreadSafeProxy;
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

#if 0
GrContextThreadSafeProxy::GrContextThreadSafeProxy(GrBackendApi backend,
                                                   const GrContextOptions& options,
                                                   uint32_t uniqueID)
        : INHERITED(backend, options, uniqueID) {

}

#if 0
GrContextThreadSafeProxy::GrContextThreadSafeProxy(sk_sp<const GrCaps> caps, uint32_t uniqueID,
                                                   GrBackendApi backend,
                                                   const GrContextOptions& options,
                                                   sk_sp<GrSkSLFPFactoryCache> cache)
        : fCaps(std::move(caps))
        , fContextUniqueID(uniqueID)
        , fBackend(backend)
        , fOptions(options)
        , fFPFactoryCache(std::move(cache)) {}
#endif

GrContextThreadSafeProxy::~GrContextThreadSafeProxy() = default;

//sk_sp<GrContextThreadSafeProxy> GrContext::threadSafeProxy() {
//    return fThreadSafeProxy;
//}

SkSurfaceCharacterization GrContextThreadSafeProxy::createCharacterization(
                                     size_t cacheMaxResourceBytes,
                                     const SkImageInfo& ii, const GrBackendFormat& backendFormat,
                                     int sampleCnt, GrSurfaceOrigin origin,
                                     const SkSurfaceProps& surfaceProps,
                                     bool isMipMapped, bool willUseGLFBO0) {
    if (!backendFormat.isValid()) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (GrBackendApi::kOpenGL != backendFormat.backend() && willUseGLFBO0) {
        // The willUseGLFBO0 flags can only be used for a GL backend.
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    const GrCaps* caps = this->caps();

    if (!caps->mipMapSupport()) {
        isMipMapped = false;
    }

    GrPixelConfig config = caps->getConfigFromBackendFormat(backendFormat, ii.colorType());
    if (config == kUnknown_GrPixelConfig) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    if (!SkSurface_Gpu::Valid(caps, config, ii.colorSpace())) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    sampleCnt = caps->getRenderTargetSampleCount(sampleCnt, config);
    if (!sampleCnt) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    GrFSAAType FSAAType = GrFSAAType::kNone;
    if (sampleCnt > 1) {
        FSAAType = caps->usesMixedSamples() ? GrFSAAType::kMixedSamples : GrFSAAType::kUnifiedMSAA;
    }

    // This surface characterization factory assumes that the resulting characterization is
    // textureable.
    if (!caps->isConfigTexturable(config)) {
        return SkSurfaceCharacterization(); // return an invalid characterization
    }

    return SkSurfaceCharacterization(sk_ref_sp<GrContextThreadSafeProxy>(this),
                                     cacheMaxResourceBytes, ii,
                                     origin, config, FSAAType, sampleCnt,
                                     SkSurfaceCharacterization::Textureable(true),
                                     SkSurfaceCharacterization::MipMapped(isMipMapped),
                                     SkSurfaceCharacterization::UsesGLFBO0(willUseGLFBO0),
                                     SkSurfaceCharacterization::VulkanSecondaryCBCompatible(false),
                                     surfaceProps);
}

#include "SkImage_Gpu.h"
#include "SkImage_GpuYUVA.h"
#include "SkYUVAIndex.h"

sk_sp<SkImage> GrContextThreadSafeProxy::makePromiseTexture(
        const GrBackendFormat& backendFormat,
        int width,
        int height,
        GrMipMapped mipMapped,
        GrSurfaceOrigin origin,
        SkColorType colorType,
        SkAlphaType alphaType,
        sk_sp<SkColorSpace> colorSpace,
        TextureFulfillProc textureFulfillProc,
        TextureReleaseProc textureReleaseProc,
        PromiseDoneProc promiseDoneProc,
        TextureContext textureContext) {
    return SkImage_Gpu::MakePromiseTexture(sk_make_sp<GrContext_Base>(this),
                                           backendFormat,
                                           width,
                                           height,
                                           mipMapped,
                                           origin,
                                           colorType,
                                           alphaType,
                                           std::move(colorSpace),
                                           textureFulfillProc,
                                           textureReleaseProc,
                                           promiseDoneProc,
                                           textureContext);
}

sk_sp<SkImage> GrContextThreadSafeProxy::makeYUVAPromiseTexture(
        SkYUVColorSpace yuvColorSpace,
        const GrBackendFormat yuvaFormats[],
        const SkISize yuvaSizes[],
        const SkYUVAIndex yuvaIndices[4],
        int imageWidth,
        int imageHeight,
        GrSurfaceOrigin imageOrigin,
        sk_sp<SkColorSpace> imageColorSpace,
        TextureFulfillProc textureFulfillProc,
        TextureReleaseProc textureReleaseProc,
        PromiseDoneProc promiseDoneProc,
        TextureContext textureContexts[]) {
    return SkImage_GpuYUVA::MakePromiseYUVATexture(sk_make_sp<GrContext_Base>(this),
                                                   yuvColorSpace,
                                                   yuvaFormats,
                                                   yuvaSizes,
                                                   yuvaIndices,
                                                   imageWidth,
                                                   imageHeight,
                                                   imageOrigin,
                                                   std::move(imageColorSpace),
                                                   textureFulfillProc,
                                                   textureReleaseProc,
                                                   promiseDoneProc,
                                                   textureContexts);
}

//////////////////////////////////////////////////////////////////////////////
#endif

void GrContext::abandonContext() {
    ASSERT_SINGLE_OWNER

    fProxyProvider->abandon();
    fResourceProvider->abandon();

    // Need to abandon the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    fDrawingManager->abandon();

    // abandon first to so destructors
    // don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->disconnect(GrGpu::DisconnectType::kAbandon);

    fGlyphCache->freeAll();
    fTextBlobCache->freeAll();
}

#if 0
bool GrContext::abandoned() const {
    ASSERT_SINGLE_OWNER
    // If called from ~GrContext(), the drawing manager may already be gone.
    return !fDrawingManager || fDrawingManager->wasAbandoned();
}
#endif

void GrContext::releaseResourcesAndAbandonContext() {
    ASSERT_SINGLE_OWNER

    if (this->abandoned1()) {
        return;
    }
    this->proxyProvider()->abandon1();
    fResourceProvider->abandon();

    // Need to abandon the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    fDrawingManager->abandon();

    // Release all resources in the backend 3D API.
    fResourceCache->releaseAll();

    fGpu->disconnect(GrGpu::DisconnectType::kCleanup);

    fGlyphCache->freeAll();
    fTextBlobCache->freeAll();
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
        ccpr->purgeCacheEntriesOlderThan(fProxyProvider, purgeTime);
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
    if (fDrawingManager->wasAbandoned()) { return GrSemaphoresSubmitted::kNo; }

    return fDrawingManager->flush(nullptr, numSemaphores, signalSemaphores);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::storeVkPipelineCacheData() {
    if (fGpu) {
        fGpu->storeVkPipelineCacheData();
    }
}

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

