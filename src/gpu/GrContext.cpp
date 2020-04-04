/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/private/SkDeferredDisplayList.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkMipMap.h"
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
#include "src/gpu/GrSkSLFPFactoryCache.h"
#include "src/gpu/GrSoftwarePathRenderer.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/text/GrTextBlobCache.h"
#include "src/gpu/text/GrTextContext.h"
#include "src/image/SkImage_GpuBase.h"
#include "src/image/SkSurface_Gpu.h"
#include <atomic>

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
}

GrContext::~GrContext() {
    ASSERT_SINGLE_OWNER

    if (this->drawingManager()) {
        this->drawingManager()->cleanup();
    }
    delete fResourceProvider;
    delete fResourceCache;
}

bool GrContext::init(sk_sp<const GrCaps> caps, sk_sp<GrSkSLFPFactoryCache> FPFactoryCache) {
    ASSERT_SINGLE_OWNER
    SkASSERT(fThreadSafeProxy); // needs to have been initialized by derived classes
    SkASSERT(this->proxyProvider());

    if (!INHERITED::init(std::move(caps), std::move(FPFactoryCache))) {
        return false;
    }

    SkASSERT(this->caps());
    SkASSERT(this->getGrStrikeCache());
    SkASSERT(this->getTextBlobCache());

    if (fGpu) {
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
    return fThreadSafeProxy;
}

//////////////////////////////////////////////////////////////////////////////

void GrContext::abandonContext() {
    if (this->abandoned()) {
        return;
    }

    INHERITED::abandonContext();

    fMappedBufferManager->abandon();

    fResourceProvider->abandon();

    // Need to cleanup the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    this->drawingManager()->cleanup();

    // abandon first to so destructors
    // don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->disconnect(GrGpu::DisconnectType::kAbandon);

    fMappedBufferManager.reset();
}

void GrContext::releaseResourcesAndAbandonContext() {
    if (this->abandoned()) {
        return;
    }

    INHERITED::abandonContext();

    fMappedBufferManager.reset();

    fResourceProvider->abandon();

    // Need to cleanup the drawing manager first so all the render targets
    // will be released/forgotten before they too are abandoned.
    this->drawingManager()->cleanup();

    // Release all resources in the backend 3D API.
    fResourceCache->releaseAll();

    fGpu->disconnect(GrGpu::DisconnectType::kCleanup);
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

    // TODO: the glyph cache doesn't hold any GpuResources so this call should not be needed here.
    // Some slack in the GrTextBlob's implementation requires it though. That could be fixed.
    this->getGrStrikeCache()->freeAll();

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

size_t GrContext::ComputeImageSize(sk_sp<SkImage> image, GrMipMapped mipMapped, bool useNextPow2) {
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

int GrContext::maxSurfaceSampleCountForColorType(SkColorType colorType) const {
    GrBackendFormat format =
            this->caps()->getDefaultBackendFormat(SkColorTypeToGrColorType(colorType),
                                                  GrRenderable::kYes);
    return this->caps()->maxRenderTargetSampleCount(format);
}

////////////////////////////////////////////////////////////////////////////////

bool GrContext::wait(int numSemaphores, const GrBackendSemaphore waitSemaphores[]) {
    if (!fGpu || fGpu->caps()->semaphoreSupport()) {
        return false;
    }
    for (int i = 0; i < numSemaphores; ++i) {
        std::unique_ptr<GrSemaphore> sema = fResourceProvider->wrapBackendSemaphore(
                waitSemaphores[i], GrResourceProvider::SemaphoreWrapType::kWillWait,
                kAdopt_GrWrapOwnership);
        fGpu->waitSemaphore(sema.get());
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

GrSemaphoresSubmitted GrContext::flush(const GrFlushInfo& info,
                                       const GrPrepareForExternalIORequests& externalRequests) {
    ASSERT_SINGLE_OWNER
    if (this->abandoned()) {
        return GrSemaphoresSubmitted::kNo;
    }

    return this->drawingManager()->flush(nullptr, 0, SkSurface::BackendSurfaceAccess::kNoAccess,
                                         info, externalRequests);
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
                                                 GrMipMapped mipMapped,
                                                 GrRenderable renderable,
                                                 GrProtected isProtected) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (!this->asDirectContext()) {
        return GrBackendTexture();
    }

    if (this->abandoned()) {
        return GrBackendTexture();
    }

    int numMipLevels = 1;
    if (mipMapped == GrMipMapped::kYes) {
        numMipLevels = SkMipMap::ComputeLevelCount(width, height) + 1;
    }

    return fGpu->createBackendTexture({width, height}, backendFormat, renderable, nullptr,
                                      numMipLevels, isProtected);
}

GrBackendTexture GrContext::createBackendTexture(int width, int height,
                                                 SkColorType skColorType,
                                                 GrMipMapped mipMapped,
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
                                                         GrMipMapped(c.isMipMapped()),
                                                         GrRenderable::kYes,
                                                         c.isProtected());
    SkASSERT(c.isCompatible(result));
    return result;
}

GrBackendTexture GrContext::createBackendTexture(const SkSurfaceCharacterization& c,
                                                 const SkColor4f& color) {
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

    GrBackendTexture result = this->createBackendTexture(c.width(), c.height(), format, color,
                                                         GrMipMapped(c.isMipMapped()),
                                                         GrRenderable::kYes,
                                                         c.isProtected());
    SkASSERT(c.isCompatible(result));
    return result;
}

GrBackendTexture GrContext::createBackendTexture(int width, int height,
                                                 const GrBackendFormat& backendFormat,
                                                 const SkColor4f& color,
                                                 GrMipMapped mipMapped,
                                                 GrRenderable renderable,
                                                 GrProtected isProtected) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (!this->asDirectContext()) {
        return GrBackendTexture();
    }

    if (this->abandoned()) {
        return GrBackendTexture();
    }

    int numMipLevels = 1;
    if (mipMapped == GrMipMapped::kYes) {
        numMipLevels = SkMipMap::ComputeLevelCount(width, height) + 1;
    }
    GrGpu::BackendTextureData data(color);
    return fGpu->createBackendTexture({width, height}, backendFormat, renderable, &data,
                                      numMipLevels, isProtected);
}

GrBackendTexture GrContext::createBackendTexture(int width, int height,
                                                 SkColorType skColorType,
                                                 const SkColor4f& color,
                                                 GrMipMapped mipMapped,
                                                 GrRenderable renderable,
                                                 GrProtected isProtected) {
    if (!this->asDirectContext()) {
        return GrBackendTexture();
    }

    if (this->abandoned()) {
        return GrBackendTexture();
    }

    GrBackendFormat format = this->defaultBackendFormat(skColorType, renderable);
    if (!format.isValid()) {
        return GrBackendTexture();
    }

    GrColorType grColorType = SkColorTypeToGrColorType(skColorType);
    SkColor4f swizzledColor = this->caps()->getOutputSwizzle(format, grColorType).applyTo(color);

    return this->createBackendTexture(width, height, format, swizzledColor, mipMapped, renderable,
                                      isProtected);
}

GrBackendTexture GrContext::createBackendTexture(const SkPixmap srcData[], int numLevels,
                                                 GrRenderable renderable, GrProtected isProtected) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    if (!this->asDirectContext()) {
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

    GrBackendFormat backendFormat = this->defaultBackendFormat(colorType, renderable);

    GrGpu::BackendTextureData data(srcData);
    return fGpu->createBackendTexture({baseWidth, baseHeight}, backendFormat, renderable, &data,
                                      numLevels, isProtected);
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

bool GrContext::precompileShader(const SkData& key, const SkData& data) {
    return fGpu->precompileShader(key, data);
}

#ifdef SK_ENABLE_DUMP_GPU
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
