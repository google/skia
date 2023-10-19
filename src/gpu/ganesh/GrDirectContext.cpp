/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/GrDirectContext.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextureCompressionType.h"
#include "include/core/SkTraceMemoryDump.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextThreadSafeProxy.h"
#include "include/private/base/SingleOwner.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkAutoMalloc.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkTaskGroup.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GpuTypesPriv.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/Device.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrClientMappedBufferManager.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrContextThreadSafeProxyPriv.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrDrawOpAtlas.h"
#include "src/gpu/ganesh/GrDrawingManager.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrThreadSafePipelineBuilder.h" // IWYU pragma: keep
#include "src/gpu/ganesh/SurfaceContext.h"
#include "src/gpu/ganesh/image/SkImage_GaneshBase.h"
#include "src/gpu/ganesh/mock/GrMockGpu.h"
#include "src/gpu/ganesh/ops/SmallPathAtlasMgr.h"
#include "src/gpu/ganesh/surface/SkSurface_Ganesh.h"
#include "src/gpu/ganesh/text/GrAtlasManager.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkSurface_Base.h"
#include "src/text/gpu/StrikeCache.h"
#include "src/text/gpu/TextBlobRedrawCoordinator.h"

#include <array>
#include <atomic>
#include <forward_list>
#include <memory>
#include <utility>

class GrSemaphore;

#ifdef SK_METAL
#include "include/gpu/mtl/GrMtlBackendContext.h"
#include "src/gpu/ganesh/mtl/GrMtlTrampoline.h"
#endif
#ifdef SK_DIRECT3D
#include "src/gpu/ganesh/d3d/GrD3DGpu.h"
#endif

using namespace skia_private;

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(this->singleOwner())

using StrikeCache = sktext::gpu::StrikeCache;

GrDirectContext::DirectContextID GrDirectContext::DirectContextID::Next() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID.fetch_add(1, std::memory_order_relaxed);
    } while (id == SK_InvalidUniqueID);
    return DirectContextID(id);
}

GrDirectContext::GrDirectContext(GrBackendApi backend, const GrContextOptions& options)
        : GrRecordingContext(GrContextThreadSafeProxyPriv::Make(backend, options), false)
        , fDeleteCallbackHelper(new DeleteCallbackHelper(options.fContextDeleteContext,
                                                         options.fContextDeleteProc))
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
    return GrRecordingContext::threadSafeProxy();
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
    if (GrRecordingContext::abandoned()) {
        return;
    }

    if (fInsideReleaseProcCnt) {
        SkDEBUGFAIL("Calling GrDirectContext::abandonContext() while inside a ReleaseProc is not "
                    "allowed");
        return;
    }

    GrRecordingContext::abandonContext();

    // We need to make sure all work is finished on the gpu before we start releasing resources.
    this->syncAllOutstandingGpuWork(this->caps()->mustSyncGpuDuringAbandon());

    fStrikeCache->freeAll();

    fMappedBufferManager->abandon();

    fResourceProvider->abandon();

    // abandon first so destructors don't try to free the resources in the API.
    fResourceCache->abandonAll();

    fGpu->disconnect(GrGpu::DisconnectType::kAbandon);

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
    if (fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr->reset();
    }
#endif
    fAtlasManager->freeAll();
}

bool GrDirectContext::abandoned() {
    if (GrRecordingContext::abandoned()) {
        return true;
    }

    if (fGpu && fGpu->isDeviceLost()) {
        this->abandonContext();
        return true;
    }
    return false;
}

bool GrDirectContext::isDeviceLost() {
    if (fGpu && fGpu->isDeviceLost()) {
        if (!GrRecordingContext::abandoned()) {
            this->abandonContext();
        }
        return true;
    }
    return false;
}

bool GrDirectContext::oomed() { return fGpu ? fGpu->checkAndResetOOMed() : false; }

void GrDirectContext::releaseResourcesAndAbandonContext() {
    if (GrRecordingContext::abandoned()) {
        return;
    }

    GrRecordingContext::abandonContext();

    // We need to make sure all work is finished on the gpu before we start releasing resources.
    this->syncAllOutstandingGpuWork(/*shouldExecuteWhileAbandoned=*/true);

    fResourceProvider->abandon();

    // Release all resources in the backend 3D API.
    fResourceCache->releaseAll();

    // Must be after GrResourceCache::releaseAll().
    fMappedBufferManager.reset();

    fGpu->disconnect(GrGpu::DisconnectType::kCleanup);
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
    if (fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr->reset();
    }
#endif
    fAtlasManager->freeAll();
}

void GrDirectContext::freeGpuResources() {
    ASSERT_SINGLE_OWNER

    if (this->abandoned()) {
        return;
    }

    this->flushAndSubmit();
#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
    if (fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr->reset();
    }
#endif
    fAtlasManager->freeAll();

    // TODO: the glyph cache doesn't hold any GpuResources so this call should not be needed here.
    // Some slack in the GrTextBlob's implementation requires it though. That could be fixed.
    fStrikeCache->freeAll();

    this->drawingManager()->freeGpuResources();

    fResourceCache->purgeUnlockedResources(GrPurgeResourceOptions::kAllResources);
}

bool GrDirectContext::init() {
    ASSERT_SINGLE_OWNER
    if (!fGpu) {
        return false;
    }

    fThreadSafeProxy->priv().init(fGpu->refCaps(), fGpu->refPipelineBuilder());
    if (!GrRecordingContext::init()) {
        return false;
    }

    SkASSERT(this->getTextBlobRedrawCoordinator());
    SkASSERT(this->threadSafeCache());

    fStrikeCache = std::make_unique<StrikeCache>();
    fResourceCache = std::make_unique<GrResourceCache>(this->singleOwner(),
                                                       this->directContextID(),
                                                       this->contextID());
    fResourceCache->setProxyProvider(this->proxyProvider());
    fResourceCache->setThreadSafeCache(this->threadSafeCache());
#if defined(GR_TEST_UTILS)
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
        !(this->caps()->shaderCaps()->fFloatIs32Bits ||
        this->caps()->shaderCaps()->fIntegerSupport)) {
        allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kNo;
    } else {
        allowMultitexturing = GrDrawOpAtlas::AllowMultitexturing::kYes;
    }

    GrProxyProvider* proxyProvider = this->priv().proxyProvider();

    fAtlasManager = std::make_unique<GrAtlasManager>(proxyProvider,
                                                     this->options().fGlyphCacheTextureMaximumBytes,
                                                     allowMultitexturing,
                                                     this->options().fSupportBilerpFromGlyphAtlas);
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

void GrDirectContext::purgeUnlockedResources(GrPurgeResourceOptions opts) {
    ASSERT_SINGLE_OWNER

    if (this->abandoned()) {
        return;
    }

    fResourceCache->purgeUnlockedResources(opts);
    fResourceCache->purgeAsNeeded();

    // The textBlob Cache doesn't actually hold any GPU resource but this is a convenient
    // place to purge stale blobs
    this->getTextBlobRedrawCoordinator()->purgeStaleBlobs();

    fGpu->releaseUnlockedBackendObjects();
}

void GrDirectContext::performDeferredCleanup(std::chrono::milliseconds msNotUsed,
                                             GrPurgeResourceOptions opts) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    ASSERT_SINGLE_OWNER

    if (this->abandoned()) {
        return;
    }

    this->checkAsyncWorkCompletion();
    fMappedBufferManager->process();
    auto purgeTime = skgpu::StdSteadyClock::now() - msNotUsed;

    fResourceCache->purgeAsNeeded();
    fResourceCache->purgeResourcesNotUsedSince(purgeTime, opts);

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
    if (!fGpu || !fGpu->caps()->backendSemaphoreSupport()) {
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

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
skgpu::ganesh::SmallPathAtlasMgr* GrDirectContext::onGetSmallPathAtlasMgr() {
    if (!fSmallPathAtlasMgr) {
        fSmallPathAtlasMgr = std::make_unique<skgpu::ganesh::SmallPathAtlasMgr>();

        this->priv().addOnFlushCallbackObject(fSmallPathAtlasMgr.get());
    }

    if (!fSmallPathAtlasMgr->initAtlas(this->proxyProvider(), this->caps())) {
        return nullptr;
    }

    return fSmallPathAtlasMgr.get();
}
#endif

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

    return this->drawingManager()->flushSurfaces(
            {}, SkSurfaces::BackendSurfaceAccess::kNoAccess, info, nullptr);
}

bool GrDirectContext::submit(GrSyncCpu sync) {
    ASSERT_SINGLE_OWNER
    if (this->abandoned()) {
        return false;
    }

    if (!fGpu) {
        return false;
    }

    return fGpu->submitToGpu(sync);
}

GrSemaphoresSubmitted GrDirectContext::flush(const sk_sp<const SkImage>& image,
                                             const GrFlushInfo& flushInfo) {
    if (!image) {
        return GrSemaphoresSubmitted::kNo;
    }
    auto ib = as_IB(image);
    if (!ib->isGaneshBacked()) {
        return GrSemaphoresSubmitted::kNo;
    }
    auto igb = static_cast<const SkImage_GaneshBase*>(image.get());
    return igb->flush(this, flushInfo);
}

void GrDirectContext::flush(const sk_sp<const SkImage>& image) {
    this->flush(image, {});
}

void GrDirectContext::flushAndSubmit(const sk_sp<const SkImage>& image) {
    this->flush(image, {});
    this->submit();
}

GrSemaphoresSubmitted GrDirectContext::flush(SkSurface* surface,
                                             SkSurfaces::BackendSurfaceAccess access,
                                             const GrFlushInfo& info) {
    if (!surface) {
        return GrSemaphoresSubmitted::kNo;
    }
    auto sb = asSB(surface);
    if (!sb->isGaneshBacked()) {
        return GrSemaphoresSubmitted::kNo;
    }

    auto gs = static_cast<SkSurface_Ganesh*>(surface);
    SkASSERT(this->priv().matches(gs->getDevice()->recordingContext()->asDirectContext()));
    GrRenderTargetProxy* rtp = gs->getDevice()->targetProxy();

    return this->priv().flushSurface(rtp, access, info, nullptr);
}

GrSemaphoresSubmitted GrDirectContext::flush(SkSurface* surface,
                                             const GrFlushInfo& info,
                                             const skgpu::MutableTextureState* newState) {
    if (!surface) {
        return GrSemaphoresSubmitted::kNo;
    }
    auto sb = asSB(surface);
    if (!sb->isGaneshBacked()) {
        return GrSemaphoresSubmitted::kNo;
    }

    auto gs = static_cast<SkSurface_Ganesh*>(surface);
    SkASSERT(this->priv().matches(gs->getDevice()->recordingContext()->asDirectContext()));
    GrRenderTargetProxy* rtp = gs->getDevice()->targetProxy();

    return this->priv().flushSurface(
            rtp, SkSurfaces::BackendSurfaceAccess::kNoAccess, info, newState);
}

void GrDirectContext::flushAndSubmit(SkSurface* surface, GrSyncCpu sync) {
    this->flush(surface, SkSurfaces::BackendSurfaceAccess::kNoAccess, GrFlushInfo());
    this->submit(sync);
}

void GrDirectContext::flush(SkSurface* surface) {
    this->flush(surface, GrFlushInfo(), nullptr);
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

GrBackendTexture GrDirectContext::createBackendTexture(int width,
                                                       int height,
                                                       const GrBackendFormat& backendFormat,
                                                       skgpu::Mipmapped mipmapped,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       std::string_view label) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (this->abandoned()) {
        return GrBackendTexture();
    }

    return fGpu->createBackendTexture({width, height}, backendFormat, renderable,
                                      mipmapped, isProtected, label);
}

GrBackendTexture GrDirectContext::createBackendTexture(const SkPixmap& srcData,
                                                       GrSurfaceOrigin textureOrigin,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       GrGpuFinishedProc finishedProc,
                                                       GrGpuFinishedContext finishedContext,
                                                       std::string_view label) {
     return this->createBackendTexture(&srcData, 1, textureOrigin, renderable, isProtected,
                                       finishedProc, finishedContext, label);
}

GrBackendTexture GrDirectContext::createBackendTexture(const SkPixmap& srcData,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       GrGpuFinishedProc finishedProc,
                                                       GrGpuFinishedContext finishedContext,
                                                       std::string_view label) {
    return this->createBackendTexture(&srcData,
                                      1,
                                      renderable,
                                      isProtected,
                                      finishedProc,
                                      finishedContext,
                                      label);
}

GrBackendTexture GrDirectContext::createBackendTexture(const SkPixmap srcData[],
                                                       int numLevels,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       GrGpuFinishedProc finishedProc,
                                                       GrGpuFinishedContext finishedContext,
                                                       std::string_view label) {
    return this->createBackendTexture(srcData,
                                      numLevels,
                                      kTopLeft_GrSurfaceOrigin,
                                      renderable,
                                      isProtected,
                                      finishedProc,
                                      finishedContext,
                                      label);
}

GrBackendTexture GrDirectContext::createBackendTexture(int width,
                                                       int height,
                                                       SkColorType skColorType,
                                                       skgpu::Mipmapped mipmapped,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       std::string_view label) {
    if (this->abandoned()) {
        return GrBackendTexture();
    }

    const GrBackendFormat format = this->defaultBackendFormat(skColorType, renderable);

    return this->createBackendTexture(
            width, height, format, mipmapped, renderable, isProtected, label);
}

static GrBackendTexture create_and_clear_backend_texture(
        GrDirectContext* dContext,
        SkISize dimensions,
        const GrBackendFormat& backendFormat,
        skgpu::Mipmapped mipmapped,
        GrRenderable renderable,
        GrProtected isProtected,
        sk_sp<skgpu::RefCntedCallback> finishedCallback,
        std::array<float, 4> color,
        std::string_view label) {
    GrGpu* gpu = dContext->priv().getGpu();
    GrBackendTexture beTex = gpu->createBackendTexture(dimensions, backendFormat, renderable,
                                                       mipmapped, isProtected, label);
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
                                        sk_sp<skgpu::RefCntedCallback> finishedCallback) {
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

    skgpu::Swizzle swizzle = context->priv().caps()->getReadSwizzle(format, ct);
    GrSurfaceProxyView view(std::move(proxy), textureOrigin, swizzle);
    skgpu::ganesh::SurfaceContext surfaceContext(
            context, std::move(view), src[0].info().colorInfo());
    AutoSTArray<15, GrCPixmap> tmpSrc(numLevels);
    for (int i = 0; i < numLevels; ++i) {
        tmpSrc[i] = src[i];
    }
    if (!surfaceContext.writePixels(context, tmpSrc.get(), numLevels)) {
        return false;
    }

    GrSurfaceProxy* p = surfaceContext.asSurfaceProxy();
    GrFlushInfo info;
    context->priv().drawingManager()->flushSurfaces(
            {&p, 1}, SkSurfaces::BackendSurfaceAccess::kNoAccess, info, nullptr);
    return true;
}

GrBackendTexture GrDirectContext::createBackendTexture(int width,
                                                       int height,
                                                       const GrBackendFormat& backendFormat,
                                                       const SkColor4f& color,
                                                       skgpu::Mipmapped mipmapped,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       GrGpuFinishedProc finishedProc,
                                                       GrGpuFinishedContext finishedContext,
                                                       std::string_view label) {
    auto finishedCallback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    if (this->abandoned()) {
        return {};
    }

    return create_and_clear_backend_texture(this,
                                            {width, height},
                                            backendFormat,
                                            mipmapped,
                                            renderable,
                                            isProtected,
                                            std::move(finishedCallback),
                                            color.array(),
                                            label);
}

GrBackendTexture GrDirectContext::createBackendTexture(int width,
                                                       int height,
                                                       SkColorType skColorType,
                                                       const SkColor4f& color,
                                                       skgpu::Mipmapped mipmapped,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       GrGpuFinishedProc finishedProc,
                                                       GrGpuFinishedContext finishedContext,
                                                       std::string_view label) {
    auto finishedCallback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

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
                                            mipmapped,
                                            renderable,
                                            isProtected,
                                            std::move(finishedCallback),
                                            swizzledColor.array(),
                                            label);
}

GrBackendTexture GrDirectContext::createBackendTexture(const SkPixmap srcData[],
                                                       int numProvidedLevels,
                                                       GrSurfaceOrigin textureOrigin,
                                                       GrRenderable renderable,
                                                       GrProtected isProtected,
                                                       GrGpuFinishedProc finishedProc,
                                                       GrGpuFinishedContext finishedContext,
                                                       std::string_view label) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    auto finishedCallback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return {};
    }

    if (!srcData || numProvidedLevels <= 0) {
        return {};
    }

    SkColorType colorType = srcData[0].colorType();

    skgpu::Mipmapped mipmapped = skgpu::Mipmapped::kNo;
    if (numProvidedLevels > 1) {
        mipmapped = skgpu::Mipmapped::kYes;
    }

    GrBackendFormat backendFormat = this->defaultBackendFormat(colorType, renderable);
    GrBackendTexture beTex = this->createBackendTexture(srcData[0].width(),
                                                        srcData[0].height(),
                                                        backendFormat,
                                                        mipmapped,
                                                        renderable,
                                                        isProtected,
                                                        label);
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

bool GrDirectContext::updateBackendTexture(const GrBackendTexture& texture,
                                           const SkPixmap srcData[],
                                           int numLevels,
                                           GrGpuFinishedProc finishedProc,
                                           GrGpuFinishedContext finishedContext) {
    return this->updateBackendTexture(texture,
                                      srcData,
                                      numLevels,
                                      kTopLeft_GrSurfaceOrigin,
                                      finishedProc,
                                      finishedContext);
}

bool GrDirectContext::updateBackendTexture(const GrBackendTexture& backendTexture,
                                           const SkColor4f& color,
                                           GrGpuFinishedProc finishedProc,
                                           GrGpuFinishedContext finishedContext) {
    auto finishedCallback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

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
    auto finishedCallback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    GrBackendFormat format = backendTexture.getBackendFormat();
    GrColorType grColorType = SkColorTypeToGrColorType(skColorType);

    if (!this->caps()->areColorTypeAndFormatCompatible(grColorType, format)) {
        return false;
    }

    skgpu::Swizzle swizzle = this->caps()->getWriteSwizzle(format, grColorType);
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
    auto finishedCallback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

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
        skgpu::Mipmapped mipmapped,
        GrProtected isProtected,
        sk_sp<skgpu::RefCntedCallback> finishedCallback,
        const void* data,
        size_t size) {
    GrGpu* gpu = dContext->priv().getGpu();

    GrBackendTexture beTex = gpu->createCompressedBackendTexture(dimensions, backendFormat,
                                                                 mipmapped, isProtected);
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
        int width,
        int height,
        const GrBackendFormat& backendFormat,
        const SkColor4f& color,
        skgpu::Mipmapped mipmapped,
        GrProtected isProtected,
        GrGpuFinishedProc finishedProc,
        GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    auto finishedCallback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return {};
    }

    SkTextureCompressionType compression = GrBackendFormatToCompressionType(backendFormat);
    if (compression == SkTextureCompressionType::kNone) {
        return {};
    }

    size_t size = SkCompressedDataSize(
            compression, {width, height}, nullptr, mipmapped == skgpu::Mipmapped::kYes);
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
        int width,
        int height,
        SkTextureCompressionType compression,
        const SkColor4f& color,
        skgpu::Mipmapped mipmapped,
        GrProtected isProtected,
        GrGpuFinishedProc finishedProc,
        GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    GrBackendFormat format = this->compressedBackendFormat(compression);
    return this->createCompressedBackendTexture(width, height, format, color,
                                                mipmapped, isProtected, finishedProc,
                                                finishedContext);
}

GrBackendTexture GrDirectContext::createCompressedBackendTexture(
        int width,
        int height,
        const GrBackendFormat& backendFormat,
        const void* compressedData,
        size_t dataSize,
        skgpu::Mipmapped mipmapped,
        GrProtected isProtected,
        GrGpuFinishedProc finishedProc,
        GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    auto finishedCallback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return {};
    }

    return create_and_update_compressed_backend_texture(this,
                                                        {width, height},
                                                        backendFormat,
                                                        mipmapped,
                                                        isProtected,
                                                        std::move(finishedCallback),
                                                        compressedData,
                                                        dataSize);
}

GrBackendTexture GrDirectContext::createCompressedBackendTexture(
        int width,
        int height,
        SkTextureCompressionType compression,
        const void* data,
        size_t dataSize,
        skgpu::Mipmapped mipmapped,
        GrProtected isProtected,
        GrGpuFinishedProc finishedProc,
        GrGpuFinishedContext finishedContext) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    GrBackendFormat format = this->compressedBackendFormat(compression);
    return this->createCompressedBackendTexture(width, height, format, data, dataSize, mipmapped,
                                                isProtected, finishedProc, finishedContext);
}

bool GrDirectContext::updateCompressedBackendTexture(const GrBackendTexture& backendTexture,
                                                     const SkColor4f& color,
                                                     GrGpuFinishedProc finishedProc,
                                                     GrGpuFinishedContext finishedContext) {
    auto finishedCallback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    SkTextureCompressionType compression =
            GrBackendFormatToCompressionType(backendTexture.getBackendFormat());
    if (compression == SkTextureCompressionType::kNone) {
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
    auto finishedCallback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

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
                                             const skgpu::MutableTextureState& state,
                                             skgpu::MutableTextureState* previousState,
                                             GrGpuFinishedProc finishedProc,
                                             GrGpuFinishedContext finishedContext) {
    auto callback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    return fGpu->setBackendTextureState(backendTexture, state, previousState, std::move(callback));
}


bool GrDirectContext::setBackendRenderTargetState(const GrBackendRenderTarget& backendRenderTarget,
                                                  const skgpu::MutableTextureState& state,
                                                  skgpu::MutableTextureState* previousState,
                                                  GrGpuFinishedProc finishedProc,
                                                  GrGpuFinishedContext finishedContext) {
    auto callback = skgpu::RefCntedCallback::Make(finishedProc, finishedContext);

    if (this->abandoned()) {
        return false;
    }

    return fGpu->setBackendRenderTargetState(backendRenderTarget, state, previousState,
                                             std::move(callback));
}

void GrDirectContext::deleteBackendTexture(const GrBackendTexture& backendTex) {
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

    writer.appendCString("backend", GrBackendApiToStr(this->backend()));

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
    stream.copyToAndReset(result.data());
    return result;
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
