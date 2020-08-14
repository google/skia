/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"

#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTaskGroup.h"
#include "src/gpu/GrClientMappedBufferManager.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrPathRendererChain.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrShaderUtils.h"
#include "src/gpu/GrSoftwarePathRenderer.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/text/GrSDFTOptions.h"
#include "src/gpu/text/GrStrikeCache.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/image/SkImage_GpuBase.h"
#include "src/image/SkSurface_Gpu.h"
#include <atomic>
#include <memory>

#define ASSERT_OWNED_PROXY(P) \
    SkASSERT(!(P) || !((P)->peekTexture()) || (P)->peekTexture()->getContext() == this)

#define ASSERT_OWNED_RESOURCE(R) SkASSERT(!(R) || (R)->getContext() == this)
#define ASSERT_SINGLE_OWNER GR_ASSERT_SINGLE_OWNER(this->singleOwner())
#define RETURN_IF_ABANDONED if (this->abandoned()) { return; }
#define RETURN_FALSE_IF_ABANDONED if (this->abandoned()) { return false; }
#define RETURN_NULL_IF_ABANDONED if (this->abandoned()) { return nullptr; }

////////////////////////////////////////////////////////////////////////////////

GrContext::GrContext(sk_sp<GrContextThreadSafeProxy> proxy) : INHERITED(std::move(proxy)) {
    fResourceCache = nullptr;
    fResourceProvider = nullptr;
}

GrContext::~GrContext() {
    ASSERT_SINGLE_OWNER

    this->destroyDrawingManager();
    fMappedBufferManager.reset();
    delete fResourceProvider;
    delete fResourceCache;
}

bool GrContext::init() {
    ASSERT_SINGLE_OWNER
    SkASSERT(this->proxyProvider());

    if (!INHERITED::init()) {
        return false;
    }

    SkASSERT(this->getTextBlobCache());

    if (fGpu) {
        fStrikeCache = std::make_unique<GrStrikeCache>();
        fResourceCache = new GrResourceCache(this->caps(), this->singleOwner(), this->contextID());
        fResourceProvider = new GrResourceProvider(fGpu.get(), fResourceCache, this->singleOwner());
        fMappedBufferManager = std::make_unique<GrClientMappedBufferManager>(this->contextID());
    }

    if (fResourceCache) {
        fResourceCache->setProxyProvider(this->proxyProvider());
    }

    fDidTestPMConversions = false;

    // DDL TODO: we need to think through how the task group & persistent cache
    // get passed on to/shared between all the DDLRecorders created with this context.
    if (this->options().fExecutor) {
        fTaskGroup = std::make_unique<SkTaskGroup>(*this->options().fExecutor);
    }

    fPersistentCache = this->options().fPersistentCache;
    fShaderErrorHandler = this->options().fShaderErrorHandler;
    if (!fShaderErrorHandler) {
        fShaderErrorHandler = GrShaderUtils::DefaultShaderErrorHandler();
    }

    return true;
}

sk_sp<GrContextThreadSafeProxy> GrContext::threadSafeProxy() {
    return INHERITED::threadSafeProxy();
}

//////////////////////////////////////////////////////////////////////////////

void GrContext::abandonContext() {
    if (INHERITED::abandoned()) {
        return;
    }

    INHERITED::abandonContext();

    fStrikeCache->freeAll();

    fMappedBufferManager->abandon();

    fResourceProvider->abandon();

    // abandon first to so destructors
    // don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->disconnect(GrGpu::DisconnectType::kAbandon);

    fMappedBufferManager.reset();
}

void GrContext::releaseResourcesAndAbandonContext() {
    if (INHERITED::abandoned()) {
        return;
    }

    INHERITED::abandonContext();

    fMappedBufferManager.reset();

    fResourceProvider->abandon();

    // Release all resources in the backend 3D API.
    fResourceCache->releaseAll();

    fGpu->disconnect(GrGpu::DisconnectType::kCleanup);
}

bool GrContext::abandoned() {
    if (INHERITED::abandoned()) {
        return true;
    }

    if (fGpu && fGpu->isDeviceLost()) {
        this->abandonContext();
        return true;
    }
    return false;
}

bool GrContext::oomed() { return fGpu ? fGpu->checkAndResetOOMed() : false; }

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

    if (this->abandoned()) {
        return;
    }

    // TODO: the glyph cache doesn't hold any GpuResources so this call should not be needed here.
    // Some slack in the GrTextBlob's implementation requires it though. That could be fixed.
    fStrikeCache->freeAll();

    this->drawingManager()->freeGpuResources();

    fResourceCache->purgeAllUnlocked();
}

void GrContext::purgeUnlockedResources(bool scratchResourcesOnly) {
    ASSERT_SINGLE_OWNER

    if (this->abandoned()) {
        return;
    }

    fResourceCache->purgeUnlockedResources(scratchResourcesOnly);
    fResourceCache->purgeAsNeeded();

    // The textBlob Cache doesn't actually hold any GPU resource but this is a convenient
    // place to purge stale blobs
    this->getTextBlobCache()->purgeStaleBlobs();
}

void GrContext::performDeferredCleanup(std::chrono::milliseconds msNotUsed) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    ASSERT_SINGLE_OWNER

    if (this->abandoned()) {
        return;
    }

    this->checkAsyncWorkCompletion();
    fMappedBufferManager->process();
    auto purgeTime = GrStdSteadyClock::now() - msNotUsed;

    fResourceCache->purgeAsNeeded();
    fResourceCache->purgeResourcesNotUsedSince(purgeTime);

    if (auto ccpr = this->drawingManager()->getCoverageCountingPathRenderer()) {
        ccpr->purgeCacheEntriesOlderThan(this->proxyProvider(), purgeTime);
    }

    // The textBlob Cache doesn't actually hold any GPU resource but this is a convenient
    // place to purge stale blobs
    this->getTextBlobCache()->purgeStaleBlobs();
}

void GrContext::purgeUnlockedResources(size_t bytesToPurge, bool preferScratchResources) {
    ASSERT_SINGLE_OWNER

    if (this->abandoned()) {
        return;
    }

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

size_t GrContext::ComputeImageSize(sk_sp<SkImage> image, GrMipmapped mipMapped, bool useNextPow2) {
    if (!image->isTextureBacked()) {
        return 0;
    }
    SkImage_GpuBase* gpuImage = static_cast<SkImage_GpuBase*>(as_IB(image.get()));
    GrTextureProxy* proxy = gpuImage->peekProxy();
    if (!proxy) {
        return 0;
    }

    const GrCaps& caps = *gpuImage->context()->priv().caps();
    int colorSamplesPerPixel = 1;
    return GrSurface::ComputeSize(caps, proxy->backendFormat(), image->dimensions(),
                                  colorSamplesPerPixel, mipMapped, useNextPow2);
}

////////////////////////////////////////////////////////////////////////////////

int GrContext::maxTextureSize() const { return this->caps()->maxTextureSize(); }

int GrContext::maxRenderTargetSize() const { return this->caps()->maxRenderTargetSize(); }

bool GrContext::colorTypeSupportedAsImage(SkColorType colorType) const {
    GrBackendFormat format =
            this->caps()->getDefaultBackendFormat(SkColorTypeToGrColorType(colorType),
                                                  GrRenderable::kNo);
    return format.isValid();
}

////////////////////////////////////////////////////////////////////////////////

bool GrContext::wait(int numSemaphores, const GrBackendSemaphore waitSemaphores[],
                     bool deleteSemaphoresAfterWait) {
    if (!fGpu || fGpu->caps()->semaphoreSupport()) {
        return false;
    }
    GrWrapOwnership ownership =
            deleteSemaphoresAfterWait ? kAdopt_GrWrapOwnership : kBorrow_GrWrapOwnership;
    for (int i = 0; i < numSemaphores; ++i) {
        std::unique_ptr<GrSemaphore> sema = fResourceProvider->wrapBackendSemaphore(
                waitSemaphores[i], GrResourceProvider::SemaphoreWrapType::kWillWait, ownership);
        // If we failed to wrap the semaphore it means the client didn't give us a valid semaphore
        // to begin with. Therefore, it is fine to not wait on it.
        if (sema) {
            fGpu->waitSemaphore(sema.get());
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

GrSemaphoresSubmitted GrContext::flush(const GrFlushInfo& info) {
    ASSERT_SINGLE_OWNER
    if (this->abandoned()) {
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        if (info.fSubmittedProc) {
            info.fSubmittedProc(info.fSubmittedContext, false);
        }
        return GrSemaphoresSubmitted::kNo;
    }

    bool flushed = this->drawingManager()->flush(
            nullptr, 0, SkSurface::BackendSurfaceAccess::kNoAccess, info, nullptr);

    if (!flushed || (!this->priv().caps()->semaphoreSupport() && info.fNumSemaphores)) {
        return GrSemaphoresSubmitted::kNo;
    }
    return GrSemaphoresSubmitted::kYes;
}

bool GrContext::submit(bool syncCpu) {
    ASSERT_SINGLE_OWNER
    if (this->abandoned()) {
        return false;
    }

    if (!fGpu) {
        return false;
    }

    return fGpu->submitToGpu(syncCpu);
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::checkAsyncWorkCompletion() {
    if (fGpu) {
        fGpu->checkFinishProcs();
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrContext::storeVkPipelineCacheData() {
    if (fGpu) {
        fGpu->storeVkPipelineCacheData();
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrContext::supportsDistanceFieldText() const {
    return this->caps()->shaderCaps()->supportsDistanceFieldText();
}

//////////////////////////////////////////////////////////////////////////////

void GrContext::getResourceCacheLimits(int* maxResources, size_t* maxResourceBytes) const {
    ASSERT_SINGLE_OWNER
    if (maxResources) {
        *maxResources = -1;
    }
    if (maxResourceBytes) {
        *maxResourceBytes = this->getResourceCacheLimit();
    }
}

size_t GrContext::getResourceCacheLimit() const {
    ASSERT_SINGLE_OWNER
    return fResourceCache->getMaxResourceBytes();
}

void GrContext::setResourceCacheLimits(int unused, size_t maxResourceBytes) {
    ASSERT_SINGLE_OWNER
    this->setResourceCacheLimit(maxResourceBytes);
}

void GrContext::setResourceCacheLimit(size_t maxResourceBytes) {
    ASSERT_SINGLE_OWNER
    fResourceCache->setLimit(maxResourceBytes);
}

//////////////////////////////////////////////////////////////////////////////
void GrContext::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    ASSERT_SINGLE_OWNER
    fResourceCache->dumpMemoryStatistics(traceMemoryDump);
    traceMemoryDump->dumpNumericValue("skia/gr_text_blob_cache", "size", "bytes",
                                      this->getTextBlobCache()->usedBytes());
}

//////////////////////////////////////////////////////////////////////////////
GrBackendTexture GrContext::createBackendTexture(int width, int height,
                                                 const GrBackendFormat& backendFormat,
                                                 GrMipmapped mipMapped,
                                                 GrRenderable renderable,
                                                 GrProtected isProtected) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (!this->asDirectContext()) {
        return GrBackendTexture();
    }

    if (this->abandoned()) {
        return GrBackendTexture();
    }

    return fGpu->createBackendTexture({width, height}, backendFormat, renderable,
                                      mipMapped, isProtected);
}

GrBackendTexture GrContext::createBackendTexture(int width, int height,
                                                 SkColorType skColorType,
                                                 GrMipmapped mipMapped,
                                                 GrRenderable renderable,
                                                 GrProtected isProtected) {
    if (!this->asDirectContext()) {
        return GrBackendTexture();
    }

    if (this->abandoned()) {
        return GrBackendTexture();
    }

    const GrBackendFormat format = this->defaultBackendFormat(skColorType, renderable);

    return this->createBackendTexture(width, height, format, mipMapped, renderable, isProtected);
}

GrBackendTexture GrContext::createBackendTexture(const SkSurfaceCharacterization& c) {
    if (!this->asDirectContext() || !c.isValid()) {
        return GrBackendTexture();
    }

    if (this->abandoned()) {
        return GrBackendTexture();
    }

    if (c.usesGLFBO0()) {
        // If we are making the surface we will never use FBO0.
        return GrBackendTexture();
    }

    if (c.vulkanSecondaryCBCompatible()) {
        return {};
    }

    const GrBackendFormat format = this->defaultBackendFormat(c.colorType(), GrRenderable::kYes);
    if (!format.isValid()) {
        return GrBackendTexture();
    }

    GrBackendTexture result = this->createBackendTexture(c.width(), c.height(), format,
                                                         GrMipmapped(c.isMipMapped()),
                                                         GrRenderable::kYes,
                                                         c.isProtected());
    SkASSERT(c.isCompatible(result));
    return result;
}

static GrBackendTexture create_and_update_backend_texture(
        GrDirectContext* context,
        SkISize dimensions,
        const GrBackendFormat& backendFormat,
        GrMipmapped mipMapped,
        GrRenderable renderable,
        GrProtected isProtected,
        sk_sp<GrRefCntedCallback> finishedCallback,
        const GrGpu::BackendTextureData* data) {
    GrGpu* gpu = context->priv().getGpu();

    GrBackendTexture beTex = gpu->createBackendTexture(dimensions, backendFormat, renderable,
                                                       mipMapped, isProtected);
    if (!beTex.isValid()) {
        return {};
    }

    if (!context->priv().getGpu()->updateBackendTexture(beTex, std::move(finishedCallback), data)) {
        context->deleteBackendTexture(beTex);
        return {};
    }
    return beTex;
}


GrBackendTexture GrContext::createBackendTexture(const SkSurfaceCharacterization& c,
                                                 const SkColor4f& color,
                                                 GrGpuFinishedProc finishedProc,
                                                 GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext() || !c.isValid()) {
        return {};
    }

    if (this->abandoned()) {
        return {};
    }

    if (c.usesGLFBO0()) {
        // If we are making the surface we will never use FBO0.
        return {};
    }

    if (c.vulkanSecondaryCBCompatible()) {
        return {};
    }

    const GrBackendFormat format = this->defaultBackendFormat(c.colorType(), GrRenderable::kYes);
    if (!format.isValid()) {
        return {};
    }

    GrGpu::BackendTextureData data(color);
    GrBackendTexture result = create_and_update_backend_texture(
            this->asDirectContext(), {c.width(), c.height()}, format, GrMipmapped(c.isMipMapped()),
            GrRenderable::kYes, c.isProtected(), std::move(finishedCallback), &data);

    SkASSERT(c.isCompatible(result));
    return result;
}

GrBackendTexture GrContext::createBackendTexture(int width, int height,
                                                 const GrBackendFormat& backendFormat,
                                                 const SkColor4f& color,
                                                 GrMipmapped mipMapped,
                                                 GrRenderable renderable,
                                                 GrProtected isProtected,
                                                 GrGpuFinishedProc finishedProc,
                                                 GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (!this->asDirectContext()) {
        return {};
    }

    if (this->abandoned()) {
        return {};
    }

    GrGpu::BackendTextureData data(color);
    return create_and_update_backend_texture(this->asDirectContext(), {width, height},
                                             backendFormat, mipMapped, renderable, isProtected,
                                             std::move(finishedCallback), &data);
}

GrBackendTexture GrContext::createBackendTexture(int width, int height,
                                                 SkColorType skColorType,
                                                 const SkColor4f& color,
                                                 GrMipmapped mipMapped,
                                                 GrRenderable renderable,
                                                 GrProtected isProtected,
                                                 GrGpuFinishedProc finishedProc,
                                                 GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return {};
    }

    if (this->abandoned()) {
        return {};
    }

    GrBackendFormat format = this->defaultBackendFormat(skColorType, renderable);
    if (!format.isValid()) {
        return {};
    }

    GrColorType grColorType = SkColorTypeToGrColorType(skColorType);
    SkColor4f swizzledColor = this->caps()->getWriteSwizzle(format, grColorType).applyTo(color);

    GrGpu::BackendTextureData data(swizzledColor);
    return create_and_update_backend_texture(this->asDirectContext(), {width, height}, format,
                                             mipMapped, renderable, isProtected,
                                             std::move(finishedCallback), &data);
}

GrBackendTexture GrContext::createBackendTexture(const SkPixmap srcData[], int numProvidedLevels,
                                                 GrRenderable renderable, GrProtected isProtected,
                                                 GrGpuFinishedProc finishedProc,
                                                 GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return {};
    }

    if (this->abandoned()) {
        return {};
    }

    if (!srcData || numProvidedLevels <= 0) {
        return {};
    }

    int baseWidth = srcData[0].width();
    int baseHeight = srcData[0].height();
    SkColorType colorType = srcData[0].colorType();

    GrMipmapped mipMapped = GrMipmapped::kNo;
    int numExpectedLevels = 1;
    if (numProvidedLevels > 1) {
        numExpectedLevels = SkMipmap::ComputeLevelCount(baseWidth, baseHeight) + 1;
        mipMapped = GrMipmapped::kYes;
    }

    if (numProvidedLevels != numExpectedLevels) {
        return {};
    }

    GrBackendFormat backendFormat = this->defaultBackendFormat(colorType, renderable);

    GrGpu::BackendTextureData data(srcData);
    return create_and_update_backend_texture(this->asDirectContext(), {baseWidth, baseHeight},
                                             backendFormat, mipMapped, renderable, isProtected,
                                             std::move(finishedCallback), &data);
}

bool GrContext::updateBackendTexture(const GrBackendTexture& backendTexture,
                                     const SkColor4f& color,
                                     GrGpuFinishedProc finishedProc,
                                     GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    GrGpu::BackendTextureData data(color);
    return fGpu->updateBackendTexture(backendTexture, std::move(finishedCallback), &data);
}

bool GrContext::updateBackendTexture(const GrBackendTexture& backendTexture,
                                     SkColorType skColorType,
                                     const SkColor4f& color,
                                     GrGpuFinishedProc finishedProc,
                                     GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    GrBackendFormat format = backendTexture.getBackendFormat();
    GrColorType grColorType = SkColorTypeAndFormatToGrColorType(this->caps(), skColorType, format);

    if (!this->caps()->areColorTypeAndFormatCompatible(grColorType, format)) {
        return false;
    }

    GrSwizzle swizzle = this->caps()->getWriteSwizzle(format, grColorType);
    GrGpu::BackendTextureData data(swizzle.applyTo(color));

    return fGpu->updateBackendTexture(backendTexture, std::move(finishedCallback), &data);
}

bool GrContext::updateBackendTexture(const GrBackendTexture& backendTexture,
                                     const SkPixmap srcData[],
                                     int numLevels,
                                     GrGpuFinishedProc finishedProc,
                                     GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    if (!srcData || numLevels <= 0) {
        return false;
    }

    int numExpectedLevels = 1;
    if (backendTexture.hasMipmaps()) {
        numExpectedLevels = SkMipmap::ComputeLevelCount(backendTexture.width(),
                                                        backendTexture.height()) + 1;
    }
    if (numLevels != numExpectedLevels) {
        return false;
    }

    GrGpu::BackendTextureData data(srcData);
    return fGpu->updateBackendTexture(backendTexture, std::move(finishedCallback), &data);
}

//////////////////////////////////////////////////////////////////////////////

static GrBackendTexture create_and_update_compressed_backend_texture(
        GrDirectContext* context,
        SkISize dimensions,
        const GrBackendFormat& backendFormat,
        GrMipmapped mipMapped,
        GrProtected isProtected,
        sk_sp<GrRefCntedCallback> finishedCallback,
        const GrGpu::BackendTextureData* data) {
    GrGpu* gpu = context->priv().getGpu();

    GrBackendTexture beTex = gpu->createCompressedBackendTexture(dimensions, backendFormat,
                                                                 mipMapped, isProtected);
    if (!beTex.isValid()) {
        return {};
    }

    if (!context->priv().getGpu()->updateCompressedBackendTexture(
                beTex, std::move(finishedCallback), data)) {
        context->deleteBackendTexture(beTex);
        return {};
    }
    return beTex;
}

GrBackendTexture GrContext::createCompressedBackendTexture(int width, int height,
                                                           const GrBackendFormat& backendFormat,
                                                           const SkColor4f& color,
                                                           GrMipmapped mipMapped,
                                                           GrProtected isProtected,
                                                           GrGpuFinishedProc finishedProc,
                                                           GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return {};
    }

    if (this->abandoned()) {
        return {};
    }

    GrGpu::BackendTextureData data(color);
    return create_and_update_compressed_backend_texture(this->asDirectContext(), {width, height},
                                                        backendFormat, mipMapped, isProtected,
                                                        std::move(finishedCallback), &data);
}

GrBackendTexture GrContext::createCompressedBackendTexture(int width, int height,
                                                           SkImage::CompressionType compression,
                                                           const SkColor4f& color,
                                                           GrMipmapped mipMapped,
                                                           GrProtected isProtected,
                                                           GrGpuFinishedProc finishedProc,
                                                           GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    GrBackendFormat format = this->compressedBackendFormat(compression);
    return this->createCompressedBackendTexture(width, height, format, color,
                                                mipMapped, isProtected, finishedProc,
                                                finishedContext);
}

GrBackendTexture GrContext::createCompressedBackendTexture(int width, int height,
                                                           const GrBackendFormat& backendFormat,
                                                           const void* compressedData,
                                                           size_t dataSize,
                                                           GrMipmapped mipMapped,
                                                           GrProtected isProtected,
                                                           GrGpuFinishedProc finishedProc,
                                                           GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return {};
    }

    if (this->abandoned()) {
        return {};
    }

    GrGpu::BackendTextureData data(compressedData, dataSize);
    return create_and_update_compressed_backend_texture(this->asDirectContext(), {width, height},
                                                        backendFormat, mipMapped, isProtected,
                                                        std::move(finishedCallback), &data);
}

GrBackendTexture GrContext::createCompressedBackendTexture(int width, int height,
                                                           SkImage::CompressionType compression,
                                                           const void* data, size_t dataSize,
                                                           GrMipmapped mipMapped,
                                                           GrProtected isProtected,
                                                           GrGpuFinishedProc finishedProc,
                                                           GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    GrBackendFormat format = this->compressedBackendFormat(compression);
    return this->createCompressedBackendTexture(width, height, format, data, dataSize, mipMapped,
                                                isProtected, finishedProc, finishedContext);
}

bool GrContext::setBackendTextureState(const GrBackendTexture& backendTexture,
                                       const GrBackendSurfaceMutableState& state,
                                       GrGpuFinishedProc finishedProc,
                                       GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> callback;
    if (finishedProc) {
        callback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    return fGpu->setBackendTextureState(backendTexture, state, std::move(callback));
}

bool GrContext::updateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                               const SkColor4f& color,
                                               GrGpuFinishedProc finishedProc,
                                               GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    GrGpu::BackendTextureData data(color);
    return fGpu->updateCompressedBackendTexture(backendTexture, std::move(finishedCallback), &data);
}

bool GrContext::updateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                               const void* compressedData,
                                               size_t dataSize,
                                               GrGpuFinishedProc finishedProc,
                                               GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> finishedCallback;
    if (finishedProc) {
        finishedCallback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    if (!compressedData) {
        return false;
    }

    GrGpu::BackendTextureData data(compressedData, dataSize);

    return fGpu->updateCompressedBackendTexture(backendTexture, std::move(finishedCallback), &data);
}

//////////////////////////////////////////////////////////////////////////////

bool GrContext::setBackendRenderTargetState(const GrBackendRenderTarget& backendRenderTarget,
                                            const GrBackendSurfaceMutableState& state,
                                            GrGpuFinishedProc finishedProc,
                                            GrGpuFinishedContext finishedContext) {
    sk_sp<GrRefCntedCallback> callback;
    if (finishedProc) {
        callback.reset(new GrRefCntedCallback(finishedProc, finishedContext));
    }

    if (!this->asDirectContext()) {
        return false;
    }

    if (this->abandoned()) {
        return false;
    }

    return fGpu->setBackendRenderTargetState(backendRenderTarget, state, std::move(callback));
}

void GrContext::deleteBackendTexture(GrBackendTexture backendTex) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    // For the Vulkan backend we still must destroy the backend texture when the context is
    // abandoned.
    if ((this->abandoned() && this->backend() != GrBackendApi::kVulkan) || !backendTex.isValid()) {
        return;
    }

    fGpu->deleteBackendTexture(backendTex);
}

//////////////////////////////////////////////////////////////////////////////

bool GrContext::precompileShader(const SkData& key, const SkData& data) {
    return fGpu->precompileShader(key, data);
}

#ifdef SK_ENABLE_DUMP_GPU
#include "include/core/SkString.h"
#include "src/utils/SkJSONWriter.h"
SkString GrContext::dump() const {
    SkDynamicMemoryWStream stream;
    SkJSONWriter writer(&stream, SkJSONWriter::Mode::kPretty);
    writer.beginObject();

    writer.appendString("backend", GrBackendApiToStr(this->backend()));

    writer.appendName("caps");
    this->caps()->dumpJSON(&writer);

    writer.appendName("gpu");
    this->fGpu->dumpJSON(&writer);

    writer.appendName("context");
    this->dumpJSON(&writer);

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
