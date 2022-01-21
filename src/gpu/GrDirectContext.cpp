/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/GrDirectContext.h"

#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrContextThreadSafeProxy.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrBackendUtils.h"
#include "src/gpu/GrClientMappedBufferManager.h"
#include "src/gpu/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrThreadSafePipelineBuilder.h"
#include "src/gpu/SurfaceContext.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/mock/GrMockGpu.h"
#include "src/gpu/text/GrAtlasManager.h"
#include "src/gpu/text/GrStrikeCache.h"
#include "src/image/SkImage_GpuBase.h"
#include "src/utils/SkShaderUtils.h"
#if SK_GPU_V1
#include "src/gpu/ops/SmallPathAtlasMgr.h"
#else
// A vestigial definition for v2 that will never be instantiated
namespace skgpu::v1 {
class SmallPathAtlasMgr {
public:
    SmallPathAtlasMgr() { SkASSERT(0); }
    void reset() { SkASSERT(0); }
};
}
#endif
#ifdef SK_GL
#include "src/gpu/gl/GrGLGpu.h"
#endif
#ifdef SK_METAL
#include "include/gpu/mtl/GrMtlBackendContext.h"
#include "src/gpu/mtl/GrMtlTrampoline.h"
#endif
#ifdef SK_VULKAN
#include "src/gpu/vk/GrVkGpu.h"
#endif
#ifdef SK_DIRECT3D
#include "src/gpu/d3d/GrD3DGpu.h"
#endif
#ifdef SK_DAWN
#include "src/gpu/dawn/GrDawnGpu.h"
#endif
#include <memory>

#if GR_TEST_UTILS
#   include "include/utils/SkRandom.h"
#   if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
#       include <sanitizer/lsan_interface.h>
#   endif
#endif

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())

GrDirectContext::DirectContextID GrDirectContext::DirectContextID::Next() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);
    return DirectContextID(id);
}

GrDirectContext::GrDirectContext(GrBackendApi backend, const GrContextOptions& options)
        : INHERITED(GrContextThreadSafeProxyPriv::Make(backend, options), false)
        , fDirectContextID(DirectContextID::Next()) {
}

GrDirectContext::~GrDirectContext() {
    ASSERT_SINGLE_OWNER
    // this if-test protects against the case where the context is being destroyed
    // before having been fully created
    if (fGpu) {
        this->flushAndSubmit();
    }

    // We need to make sure all work is finished on the gpu before we start releasing resources.
    this->syncAllOutstandingGpuWork(/*shouldExecuteWhileAbandoned=*/false);

    this->destroyDrawingManager();

    // Ideally we could just let the ptr drop, but resource cache queries this ptr in releaseAll.
    if (fResourceCache) {
        fResourceCache->releaseAll();
    }
    // This has to be after GrResourceCache::releaseAll so that other threads that are holding
    // async pixel result don't try to destroy buffers off thread.
    fMappedBufferManager.reset();
}

sk_sp<GrContextThreadSafeProxy> GrDirectContext::threadSafeProxy() {
    return INHERITED::threadSafeProxy();
}

void GrDirectContext::resetGLTextureBindings() {
    if (this->abandoned() || this->backend() != GrBackendApi::kOpenGL) {
        return;
    }
    fGpu->resetTextureBindings();
}

void GrDirectContext::resetContext(uint32_t state) {
    ASSERT_SINGLE_OWNER
    fGpu->markContextDirty(state);
}

void GrDirectContext::abandonContext() {
    if (INHERITED::abandoned()) {
        return;
    }

    INHERITED::abandonContext();

    // We need to make sure all work is finished on the gpu before we start releasing resources.
    this->syncAllOutstandingGpuWork(this->caps()->mustSyncGpuDuringAbandon());

    fStrikeCache->freeAll();

    fMappedBufferManager->abandon();

    fResourceProvider->abandon();

    // abandon first so destructors don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->disconnect(GrGpu::DisconnectType::kAbandon);

    // Must be after GrResourceCache::abandonAll().
    fMappedBufferManager.reset();

    if (fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr->reset();
    }
    fAtlasManager->freeAll();
}

bool GrDirectContext::abandoned() {
    if (INHERITED::abandoned()) {
        return true;
    }

    if (fGpu && fGpu->isDeviceLost()) {
        this->abandonContext();
        return true;
    }
    return false;
}

bool GrDirectContext::oomed() { return fGpu ? fGpu->checkAndResetOOMed() : false; }

void GrDirectContext::releaseResourcesAndAbandonContext() {
    if (INHERITED::abandoned()) {
        return;
    }

    INHERITED::abandonContext();

    // We need to make sure all work is finished on the gpu before we start releasing resources.
    this->syncAllOutstandingGpuWork(/*shouldExecuteWhileAbandoned=*/true);

    fResourceProvider->abandon();

    // Release all resources in the backend 3D API.
    fResourceCache->releaseAll();

    // Must be after GrResourceCache::releaseAll().
    fMappedBufferManager.reset();

    fGpu->disconnect(GrGpu::DisconnectType::kCleanup);
    if (fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr->reset();
    }
    fAtlasManager->freeAll();
}

void GrDirectContext::freeGpuResources() {
    ASSERT_SINGLE_OWNER

    if (this->abandoned()) {
        return;
    }

    this->flushAndSubmit();
    if (fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr->reset();
    }
    fAtlasManager->freeAll();

    // TODO: the glyph cache doesn't hold any GpuResources so this call should not be needed here.
    // Some slack in the GrTextBlob's implementation requires it though. That could be fixed.
    fStrikeCache->freeAll();

    this->drawingManager()->freeGpuResources();

    fResourceCache->purgeUnlockedResources();
}

bool GrDirectContext::init() {
    ASSERT_SINGLE_OWNER
    if (!fGpu) {
        return false;
    }

    fThreadSafeProxy->priv().init(fGpu->refCaps(), fGpu->refPipelineBuilder());
    if (!INHERITED::init()) {
        return false;
    }

    SkASSERT(this->getTextBlobRedrawCoordinator());
    SkASSERT(this->threadSafeCache());

    fStrikeCache = std::make_unique<GrStrikeCache>();
    fResourceCache = std::make_unique<GrResourceCache>(this->singleOwner(),
                                                       this->directContextID(),
                                                       this->contextID());
    fResourceCache->setProxyProvider(this->proxyProvider());
    fResourceCache->setThreadSafeCache(this->threadSafeCache());
#if GR_TEST_UTILS
    if (this->options().fResourceCacheLimitOverride != -1) {
        this->setResourceCacheLimit(this->options().fResourceCacheLimitOverride);
    }
#endif
    fResourceProvider = std::make_unique<GrResourceProvider>(fGpu.get(), fResourceCache.get(),
                                                             this->singleOwner());
    fMappedBufferManager = std::make_unique<GrClientMappedBufferManager>(this->directContextID());

    fDidTestPMConversions = false;

    // DDL TODO: we need to think through how the task group & persistent cache
    // get passed on to/shared between all the DDLRecorders created with this context.
    if (this->options().fExecutor) {
        fTaskGroup = std::make_unique<SkTaskGroup>(*this->options().fExecutor);
    }

    fPersistentCache = this->options().fPersistentCache;

    GrDrawOpAtlas::AllowMultitexturing allowMultitexturing;
    if (GrContextOptions::Enable::kNo == this->options().fAllowMultipleGlyphCacheTextures ||
        // multitexturing supported only if range can represent the index + texcoords fully
        !(this->caps()->shaderCaps()->floatIs32Bits() ||
        this->caps()->shaderCaps()->integerSupport())) {
        allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kNo;
    } else {
        allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kYes;
    }

    GrProxyProvider* proxyProvider = this->priv().proxyProvider();

    fAtlasManager = std::make_unique<GrAtlasManager>(proxyProvider,
                                                     this->options().fGlyphCacheTextureMaximumBytes,
                                                     allowMultitexturing);
    this->priv().addOnFlushCallbackObject(fAtlasManager.get());

    return true;
}

void GrDirectContext::getResourceCacheUsage(int* resourceCount, size_t* resourceBytes) const {
    ASSERT_SINGLE_OWNER

    if (resourceCount) {
        *resourceCount = fResourceCache->getBudgetedResourceCount();
    }
    if (resourceBytes) {
        *resourceBytes = fResourceCache->getBudgetedResourceBytes();
    }
}

size_t GrDirectContext::getResourceCachePurgeableBytes() const {
    ASSERT_SINGLE_OWNER
    return fResourceCache->getPurgeableBytes();
}

void GrDirectContext::getResourceCacheLimits(int* maxResources, size_t* maxResourceBytes) const {
    ASSERT_SINGLE_OWNER
    if (maxResources) {
        *maxResources = -1;
    }
    if (maxResourceBytes) {
        *maxResourceBytes = this->getResourceCacheLimit();
    }
}

size_t GrDirectContext::getResourceCacheLimit() const {
    ASSERT_SINGLE_OWNER
    return fResourceCache->getMaxResourceBytes();
}

void GrDirectContext::setResourceCacheLimits(int unused, size_t maxResourceBytes) {
    ASSERT_SINGLE_OWNER
    this->setResourceCacheLimit(maxResourceBytes);
}

void GrDirectContext::setResourceCacheLimit(size_t maxResourceBytes) {
    ASSERT_SINGLE_OWNER
    fResourceCache->setLimit(maxResourceBytes);
}

void GrDirectContext::purgeUnlockedResources(bool scratchResourcesOnly) {
    ASSERT_SINGLE_OWNER

    if (this->abandoned()) {
        return;
    }

    fResourceCache->purgeUnlockedResources(scratchResourcesOnly);
    fResourceCache->purgeAsNeeded();

    // The textBlob Cache doesn't actually hold any GPU resource but this is a convenient
    // place to purge stale blobs
    this->getTextBlobRedrawCoordinator()->purgeStaleBlobs();

    fGpu->releaseUnlockedBackendObjects();
}

void GrDirectContext::performDeferredCleanup(std::chrono::milliseconds msNotUsed,
                                             bool scratchResourcesOnly) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    ASSERT_SINGLE_OWNER

    if (this->abandoned()) {
        return;
    }

    this->checkAsyncWorkCompletion();
    fMappedBufferManager->process();
    auto purgeTime = GrStdSteadyClock::now() - msNotUsed;

    fResourceCache->purgeAsNeeded();
    fResourceCache->purgeResourcesNotUsedSince(purgeTime, scratchResourcesOnly);

    // The textBlob Cache doesn't actually hold any GPU resource but this is a convenient
    // place to purge stale blobs
    this->getTextBlobRedrawCoordinator()->purgeStaleBlobs();
}

void GrDirectContext::purgeUnlockedResources(size_t bytesToPurge, bool preferScratchResources) {
    ASSERT_SINGLE_OWNER

    if (this->abandoned()) {
        return;
    }

    fResourceCache->purgeUnlockedResources(bytesToPurge, preferScratchResources);
}

////////////////////////////////////////////////////////////////////////////////
bool GrDirectContext::wait(int numSemaphores, const GrBackendSemaphore waitSemaphores[],
                           bool deleteSemaphoresAfterWait) {
    if (!fGpu || !fGpu->caps()->semaphoreSupport()) {
        return false;
    }
    GrWrapOwnership ownership =
            deleteSemaphoresAfterWait ? kAdopt_GrWrapOwnership : kBorrow_GrWrapOwnership;
    for (int i = 0; i < numSemaphores; ++i) {
        std::unique_ptr<GrSemaphore> sema = fResourceProvider->wrapBackendSemaphore(
                waitSemaphores[i], GrSemaphoreWrapType::kWillWait, ownership);
        // If we failed to wrap the semaphore it means the client didn't give us a valid semaphore
        // to begin with. Therefore, it is fine to not wait on it.
        if (sema) {
            fGpu->waitSemaphore(sema.get());
        }
    }
    return true;
}

skgpu::v1::SmallPathAtlasMgr* GrDirectContext::onGetSmallPathAtlasMgr() {
#if SK_GPU_V1
    if (!fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr = std::make_unique<skgpu::v1::SmallPathAtlasMgr>();

        this->priv().addOnFlushCallbackObject(fSmallPathAtlasMgr.get());
    }

    if (!fSmallPathAtlasMgr->initAtlas(this->proxyProvider(), this->caps())) {
        return nullptr;
    }
#endif

    return fSmallPathAtlasMgr.get();
}

////////////////////////////////////////////////////////////////////////////////

GrSemaphoresSubmitted GrDirectContext::flush(const GrFlushInfo& info) {
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

    return this->drawingManager()->flushSurfaces({}, SkSurface::BackendSurfaceAccess::kNoAccess,
                                                 info, nullptr);
}

bool GrDirectContext::submit(bool syncCpu) {
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

void GrDirectContext::checkAsyncWorkCompletion() {
    if (fGpu) {
        fGpu->checkFinishProcs();
    }
}

void GrDirectContext::syncAllOutstandingGpuWork(bool shouldExecuteWhileAbandoned) {
    if (fGpu && (!this->abandoned() || shouldExecuteWhileAbandoned)) {
        fGpu->finishOutstandingGpuWork();
        this->checkAsyncWorkCompletion();
    }
}

////////////////////////////////////////////////////////////////////////////////

void GrDirectContext::storeVkPipelineCacheData() {
    if (fGpu) {
        fGpu->storeVkPipelineCacheData();
    }
}

////////////////////////////////////////////////////////////////////////////////

bool GrDirectContext::supportsDistanceFieldText() const {
    return this->caps()->shaderCaps()->supportsDistanceFieldText();
}

//////////////////////////////////////////////////////////////////////////////

void GrDirectContext::dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const {
    ASSERT_SINGLE_OWNER
    fResourceCache->dumpMemoryStatistics(traceMemoryDump);
    traceMemoryDump->dumpNumericValue("skia/gr_text_blob_cache", "size", "bytes",
                                      this->getTextBlobRedrawCoordinator()->usedBytes());
}

GrBackendTexture GrDirectContext::createBackendTexture(int width, int height,
                                                       const GrBackendFormat& backendFormat,
                                                       GrMipmapped mipMapped,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (this->abandoned()) {
        return GrBackendTexture();
    }

    return fGpu->createBackendTexture({width, height}, backendFormat, renderable,
                                      mipMapped, isProtected);
}

GrBackendTexture GrDirectContext::createBackendTexture(int width, int height,
                                                       SkColorType skColorType,
                                                       GrMipmapped mipMapped,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected) {
    if (this->abandoned()) {
        return GrBackendTexture();
    }

    const GrBackendFormat format = this->defaultBackendFormat(skColorType, renderable);

    return this->createBackendTexture(width, height, format, mipMapped, renderable, isProtected);
}

static GrBackendTexture create_and_clear_backend_texture(GrDirectContext* dContext,
                                                         SkISize dimensions,
                                                         const GrBackendFormat& backendFormat,
                                                         GrMipmapped mipMapped,
                                                         GrRenderable renderable,
                                                         GrProtected isProtected,
                                                         sk_sp<GrRefCntedCallback> finishedCallback,
                                                         std::array<float, 4> color) {
    GrGpu* gpu = dContext->priv().getGpu();
    GrBackendTexture beTex = gpu->createBackendTexture(dimensions, backendFormat, renderable,
                                                       mipMapped, isProtected);
    if (!beTex.isValid()) {
        return {};
    }

    if (!dContext->priv().getGpu()->clearBackendTexture(beTex,
                                                        std::move(finishedCallback),
                                                        color)) {
        dContext->deleteBackendTexture(beTex);
        return {};
    }
    return beTex;
}

static bool update_texture_with_pixmaps(GrDirectContext* context,
                                        const SkPixmap src[],
                                        int numLevels,
                                        const GrBackendTexture& backendTexture,
                                        GrSurfaceOrigin textureOrigin,
                                        sk_sp<GrRefCntedCallback> finishedCallback) {
    GrColorType ct = SkColorTypeToGrColorType(src[0].colorType());
    const GrBackendFormat& format = backendTexture.getBackendFormat();

    if (!context->priv().caps()->areColorTypeAndFormatCompatible(ct, format)) {
        return false;
    }

    auto proxy = context->priv().proxyProvider()->wrapBackendTexture(backendTexture,
                                                                     kBorrow_GrWrapOwnership,
                                                                     GrWrapCacheable::kNo,
                                                                     kRW_GrIOType,
                                                                     std::move(finishedCallback));
    if (!proxy) {
        return false;
    }

    GrSwizzle swizzle = context->priv().caps()->getReadSwizzle(format, ct);
    GrSurfaceProxyView view(std::move(proxy), textureOrigin, swizzle);
    skgpu::SurfaceContext surfaceContext(context, std::move(view), src[0].info().colorInfo());
    SkAutoSTArray<15, GrCPixmap> tmpSrc(numLevels);
    for (int i = 0; i < numLevels; ++i) {
        tmpSrc[i] = src[i];
    }
    if (!surfaceContext.writePixels(context, tmpSrc.get(), numLevels)) {
        return false;
    }

    GrSurfaceProxy* p = surfaceContext.asSurfaceProxy();
    GrFlushInfo info;
    context->priv().drawingManager()->flushSurfaces({&p, 1},
                                                    SkSurface::BackendSurfaceAccess::kNoAccess,
                                                    info,
                                                    nullptr);
    return true;
}

GrBackendTexture GrDirectContext::createBackendTexture(int width, int height,
                                                       const GrBackendFormat& backendFormat,
                                                       const SkColor4f& color,
                                                       GrMipmapped mipMapped,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       GrGpuFinishedProc finishedProc,
                                                       GrGpuFinishedContext finishedContext) {
    auto finishedCallback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (this->abandoned()) {
        return {};
    }

    return create_and_clear_backend_texture(this,
                                            {width, height},
                                            backendFormat,
                                            mipMapped,
                                            renderable,
                                            isProtected,
                                            std::move(finishedCallback),
                                            color.array());
}

GrBackendTexture GrDirectContext::createBackendTexture(int width, int height,
                                                       SkColorType skColorType,
                                                       const SkColor4f& color,
                                                       GrMipmapped mipMapped,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       GrGpuFinishedProc finishedProc,
                                                       GrGpuFinishedContext finishedContext) {
    auto finishedCallback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return {};
    }

    GrBackendFormat format = this->defaultBackendFormat(skColorType, renderable);
    if (!format.isValid()) {
        return {};
    }

    GrColorType grColorType = SkColorTypeToGrColorType(skColorType);
    SkColor4f swizzledColor = this->caps()->getWriteSwizzle(format, grColorType).applyTo(color);

    return create_and_clear_backend_texture(this,
                                            {width, height},
                                            format,
                                            mipMapped,
                                            renderable,
                                            isProtected,
                                            std::move(finishedCallback),
                                            swizzledColor.array());
}

GrBackendTexture GrDirectContext::createBackendTexture(const SkPixmap srcData[],
                                                       int numProvidedLevels,
                                                       GrSurfaceOrigin textureOrigin,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       GrGpuFinishedProc finishedProc,
                                                       GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    auto finishedCallback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return {};
    }

    if (!srcData || numProvidedLevels <= 0) {
        return {};
    }

    SkColorType colorType = srcData[0].colorType();

    GrMipmapped mipMapped = GrMipmapped::kNo;
    if (numProvidedLevels > 1) {
        mipMapped = GrMipmapped::kYes;
    }

    GrBackendFormat backendFormat = this->defaultBackendFormat(colorType, renderable);
    GrBackendTexture beTex = this->createBackendTexture(srcData[0].width(),
                                                        srcData[0].height(),
                                                        backendFormat,
                                                        mipMapped,
                                                        renderable,
                                                        isProtected);
    if (!beTex.isValid()) {
        return {};
    }
    if (!update_texture_with_pixmaps(this,
                                     srcData,
                                     numProvidedLevels,
                                     beTex,
                                     textureOrigin,
                                     std::move(finishedCallback))) {
        this->deleteBackendTexture(beTex);
        return {};
    }
    return beTex;
}

bool GrDirectContext::updateBackendTexture(const GrBackendTexture& backendTexture,
                                           const SkColor4f& color,
                                           GrGpuFinishedProc finishedProc,
                                           GrGpuFinishedContext finishedContext) {
    auto finishedCallback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    return fGpu->clearBackendTexture(backendTexture, std::move(finishedCallback), color.array());
}

bool GrDirectContext::updateBackendTexture(const GrBackendTexture& backendTexture,
                                           SkColorType skColorType,
                                           const SkColor4f& color,
                                           GrGpuFinishedProc finishedProc,
                                           GrGpuFinishedContext finishedContext) {
    auto finishedCallback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    GrBackendFormat format = backendTexture.getBackendFormat();
    GrColorType grColorType = SkColorTypeToGrColorType(skColorType);

    if (!this->caps()->areColorTypeAndFormatCompatible(grColorType, format)) {
        return false;
    }

    GrSwizzle swizzle = this->caps()->getWriteSwizzle(format, grColorType);
    SkColor4f swizzledColor = swizzle.applyTo(color);

    return fGpu->clearBackendTexture(backendTexture,
                                     std::move(finishedCallback),
                                     swizzledColor.array());
}

bool GrDirectContext::updateBackendTexture(const GrBackendTexture& backendTexture,
                                           const SkPixmap srcData[],
                                           int numLevels,
                                           GrSurfaceOrigin textureOrigin,
                                           GrGpuFinishedProc finishedProc,
                                           GrGpuFinishedContext finishedContext) {
    auto finishedCallback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    if (!srcData || numLevels <= 0) {
        return false;
    }

    // If the texture has MIP levels then we require that the full set is overwritten.
    int numExpectedLevels = 1;
    if (backendTexture.hasMipmaps()) {
        numExpectedLevels = SkMipmap::ComputeLevelCount(backendTexture.width(),
                                                        backendTexture.height()) + 1;
    }
    if (numLevels != numExpectedLevels) {
        return false;
    }
    return update_texture_with_pixmaps(this,
                                       srcData,
                                       numLevels,
                                       backendTexture,
                                       textureOrigin,
                                       std::move(finishedCallback));
}

//////////////////////////////////////////////////////////////////////////////

static GrBackendTexture create_and_update_compressed_backend_texture(
        GrDirectContext* dContext,
        SkISize dimensions,
        const GrBackendFormat& backendFormat,
        GrMipmapped mipMapped,
        GrProtected isProtected,
        sk_sp<GrRefCntedCallback> finishedCallback,
        const void* data,
        size_t size) {
    GrGpu* gpu = dContext->priv().getGpu();

    GrBackendTexture beTex = gpu->createCompressedBackendTexture(dimensions, backendFormat,
                                                                 mipMapped, isProtected);
    if (!beTex.isValid()) {
        return {};
    }

    if (!dContext->priv().getGpu()->updateCompressedBackendTexture(
                beTex, std::move(finishedCallback), data, size)) {
        dContext->deleteBackendTexture(beTex);
        return {};
    }
    return beTex;
}

GrBackendTexture GrDirectContext::createCompressedBackendTexture(
        int width, int height,
        const GrBackendFormat& backendFormat,
        const SkColor4f& color,
        GrMipmapped mipmapped,
        GrProtected isProtected,
        GrGpuFinishedProc finishedProc,
        GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    auto finishedCallback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return {};
    }

    SkImage::CompressionType compression = GrBackendFormatToCompressionType(backendFormat);
    if (compression == SkImage::CompressionType::kNone) {
        return {};
    }

    size_t size = SkCompressedDataSize(compression,
                                       {width, height},
                                       nullptr,
                                       mipmapped == GrMipmapped::kYes);
    auto storage = std::make_unique<char[]>(size);
    GrFillInCompressedData(compression, {width, height}, mipmapped, storage.get(), color);
    return create_and_update_compressed_backend_texture(this,
                                                        {width, height},
                                                        backendFormat,
                                                        mipmapped,
                                                        isProtected,
                                                        std::move(finishedCallback),
                                                        storage.get(),
                                                        size);
}

GrBackendTexture GrDirectContext::createCompressedBackendTexture(
        int width, int height,
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

GrBackendTexture GrDirectContext::createCompressedBackendTexture(
        int width, int height,
        const GrBackendFormat& backendFormat,
        const void* compressedData,
        size_t dataSize,
        GrMipmapped mipMapped,
        GrProtected isProtected,
        GrGpuFinishedProc finishedProc,
        GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    auto finishedCallback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return {};
    }

    return create_and_update_compressed_backend_texture(this,
                                                        {width, height},
                                                        backendFormat,
                                                        mipMapped,
                                                        isProtected,
                                                        std::move(finishedCallback),
                                                        compressedData,
                                                        dataSize);
}

GrBackendTexture GrDirectContext::createCompressedBackendTexture(
        int width, int height,
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

bool GrDirectContext::updateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                                     const SkColor4f& color,
                                                     GrGpuFinishedProc finishedProc,
                                                     GrGpuFinishedContext finishedContext) {
    auto finishedCallback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    SkImage::CompressionType compression =
            GrBackendFormatToCompressionType(backendTexture.getBackendFormat());
    if (compression == SkImage::CompressionType::kNone) {
        return {};
    }
    size_t size = SkCompressedDataSize(compression,
                                       backendTexture.dimensions(),
                                       nullptr,
                                       backendTexture.hasMipmaps());
    SkAutoMalloc storage(size);
    GrFillInCompressedData(compression,
                           backendTexture.dimensions(),
                           backendTexture.mipmapped(),
                           static_cast<char*>(storage.get()),
                           color);
    return fGpu->updateCompressedBackendTexture(backendTexture,
                                                std::move(finishedCallback),
                                                storage.get(),
                                                size);
}

bool GrDirectContext::updateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                                     const void* compressedData,
                                                     size_t dataSize,
                                                     GrGpuFinishedProc finishedProc,
                                                     GrGpuFinishedContext finishedContext) {
    auto finishedCallback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    if (!compressedData) {
        return false;
    }

    return fGpu->updateCompressedBackendTexture(backendTexture,
                                                std::move(finishedCallback),
                                                compressedData,
                                                dataSize);
}

//////////////////////////////////////////////////////////////////////////////

bool GrDirectContext::setBackendTextureState(const GrBackendTexture& backendTexture,
                                             const GrBackendSurfaceMutableState& state,
                                             GrBackendSurfaceMutableState* previousState,
                                             GrGpuFinishedProc finishedProc,
                                             GrGpuFinishedContext finishedContext) {
    auto callback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    return fGpu->setBackendTextureState(backendTexture, state, previousState, std::move(callback));
}


bool GrDirectContext::setBackendRenderTargetState(const GrBackendRenderTarget& backendRenderTarget,
                                                  const GrBackendSurfaceMutableState& state,
                                                  GrBackendSurfaceMutableState* previousState,
                                                  GrGpuFinishedProc finishedProc,
                                                  GrGpuFinishedContext finishedContext) {
    auto callback = GrRefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    return fGpu->setBackendRenderTargetState(backendRenderTarget, state, previousState,
                                             std::move(callback));
}

void GrDirectContext::deleteBackendTexture(GrBackendTexture backendTex) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    // For the Vulkan backend we still must destroy the backend texture when the context is
    // abandoned.
    if ((this->abandoned() && this->backend() != GrBackendApi::kVulkan) || !backendTex.isValid()) {
        return;
    }

    fGpu->deleteBackendTexture(backendTex);
}

//////////////////////////////////////////////////////////////////////////////

bool GrDirectContext::precompileShader(const SkData& key, const SkData& data) {
    return fGpu->precompileShader(key, data);
}

#ifdef SK_ENABLE_DUMP_GPU
#include "include/core/SkString.h"
#include "src/utils/SkJSONWriter.h"
SkString GrDirectContext::dump() const {
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

#ifdef SK_GL

/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeGL(sk_sp<const GrGLInterface> glInterface) {
    GrContextOptions defaultOptions;
    return MakeGL(std::move(glInterface), defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeGL(const GrContextOptions& options) {
    return MakeGL(nullptr, options);
}

sk_sp<GrDirectContext> GrDirectContext::MakeGL() {
    GrContextOptions defaultOptions;
    return MakeGL(nullptr, defaultOptions);
}

#if GR_TEST_UTILS
GrGLFunction<GrGLGetErrorFn> make_get_error_with_random_oom(GrGLFunction<GrGLGetErrorFn> original) {
    // A SkRandom and a GrGLFunction<GrGLGetErrorFn> are too big to be captured by a
    // GrGLFunction<GrGLGetError> (surprise, surprise). So we make a context object and
    // capture that by pointer. However, GrGLFunction doesn't support calling a destructor
    // on the thing it captures. So we leak the context.
    struct GetErrorContext {
        SkRandom fRandom;
        GrGLFunction<GrGLGetErrorFn> fGetError;
    };

    auto errorContext = new GetErrorContext;

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
    __lsan_ignore_object(errorContext);
#endif

    errorContext->fGetError = original;

    return GrGLFunction<GrGLGetErrorFn>([errorContext]() {
        GrGLenum error = errorContext->fGetError();
        if (error == GR_GL_NO_ERROR && (errorContext->fRandom.nextU() % 300) == 0) {
            error = GR_GL_OUT_OF_MEMORY;
        }
        return error;
    });
}
#endif

sk_sp<GrDirectContext> GrDirectContext::MakeGL(sk_sp<const GrGLInterface> glInterface,
                                               const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kOpenGL, options));
#if GR_TEST_UTILS
    if (options.fRandomGLOOM) {
        auto copy = sk_make_sp<GrGLInterface>(*glInterface);
        copy->fFunctions.fGetError =
                make_get_error_with_random_oom(glInterface->fFunctions.fGetError);
#if GR_GL_CHECK_ERROR
        // Suppress logging GL errors since we'll be synthetically generating them.
        copy->suppressErrorLogging();
#endif
        glInterface = std::move(copy);
    }
#endif
    direct->fGpu = GrGLGpu::Make(std::move(glInterface), options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }
    return direct;
}
#endif

/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeMock(const GrMockOptions* mockOptions) {
    GrContextOptions defaultOptions;
    return MakeMock(mockOptions, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeMock(const GrMockOptions* mockOptions,
                                                 const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kMock, options));

    direct->fGpu = GrMockGpu::Make(mockOptions, options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}

#ifdef SK_VULKAN
/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeVulkan(const GrVkBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeVulkan(backendContext, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeVulkan(const GrVkBackendContext& backendContext,
                                                   const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kVulkan, options));

    direct->fGpu = GrVkGpu::Make(backendContext, options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}
#endif

#ifdef SK_METAL
/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeMetal(const GrMtlBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeMetal(backendContext, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeMetal(const GrMtlBackendContext& backendContext,
                                                     const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kMetal, options));

    direct->fGpu = GrMtlTrampoline::MakeGpu(backendContext, options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}

// deprecated
sk_sp<GrDirectContext> GrDirectContext::MakeMetal(void* device, void* queue) {
    GrContextOptions defaultOptions;
    return MakeMetal(device, queue, defaultOptions);
}

// deprecated
// remove include/gpu/mtl/GrMtlBackendContext.h, above, when removed
sk_sp<GrDirectContext> GrDirectContext::MakeMetal(void* device, void* queue,
                                                  const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kMetal, options));
    GrMtlBackendContext backendContext = {};
    backendContext.fDevice.reset(device);
    backendContext.fQueue.reset(queue);

    return GrDirectContext::MakeMetal(backendContext, options);
}
#endif

#ifdef SK_DIRECT3D
/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeDirect3D(const GrD3DBackendContext& backendContext) {
    GrContextOptions defaultOptions;
    return MakeDirect3D(backendContext, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeDirect3D(const GrD3DBackendContext& backendContext,
                                                     const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kDirect3D, options));

    direct->fGpu = GrD3DGpu::Make(backendContext, options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}
#endif

#ifdef SK_DAWN
/*************************************************************************************************/
sk_sp<GrDirectContext> GrDirectContext::MakeDawn(const wgpu::Device& device) {
    GrContextOptions defaultOptions;
    return MakeDawn(device, defaultOptions);
}

sk_sp<GrDirectContext> GrDirectContext::MakeDawn(const wgpu::Device& device,
                                                 const GrContextOptions& options) {
    sk_sp<GrDirectContext> direct(new GrDirectContext(GrBackendApi::kDawn, options));

    direct->fGpu = GrDawnGpu::Make(device, options, direct.get());
    if (!direct->init()) {
        return nullptr;
    }

    return direct;
}

#endif
