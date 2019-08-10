/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDrawingManager.h"

#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrTexture.h"
#include "include/private/GrRecordingContext.h"
#include "include/private/SkDeferredDisplayList.h"
#include "src/core/SkTTopoSort.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrRenderTask.h"
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSoftwarePathRenderer.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTextureContext.h"
#include "src/gpu/GrTextureOpList.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/GrTextureResolveRenderTask.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/text/GrTextContext.h"
#include "src/image/SkSurface_Gpu.h"

GrDrawingManager::RenderTaskDAG::RenderTaskDAG(bool sortRenderTasks)
        : fSortRenderTasks(sortRenderTasks) {}

GrDrawingManager::RenderTaskDAG::~RenderTaskDAG() {}

void GrDrawingManager::RenderTaskDAG::gatherIDs(SkSTArray<8, uint32_t, true>* idArray) const {
    idArray->reset(fRenderTasks.count());
    for (int i = 0; i < fRenderTasks.count(); ++i) {
        if (fRenderTasks[i]) {
            (*idArray)[i] = fRenderTasks[i]->uniqueID();
        }
    }
}

void GrDrawingManager::RenderTaskDAG::reset() {
    fRenderTasks.reset();
}

void GrDrawingManager::RenderTaskDAG::removeRenderTask(int index) {
    if (!fRenderTasks[index]->unique()) {
        // TODO: Eventually this should be guaranteed unique: http://skbug.com/7111
        fRenderTasks[index]->endFlush();
    }

    fRenderTasks[index] = nullptr;
}

void GrDrawingManager::RenderTaskDAG::removeRenderTasks(int startIndex, int stopIndex) {
    for (int i = startIndex; i < stopIndex; ++i) {
        if (!fRenderTasks[i]) {
            continue;
        }
        this->removeRenderTask(i);
    }
}

bool GrDrawingManager::RenderTaskDAG::isUsed(GrSurfaceProxy* proxy) const {
    for (int i = 0; i < fRenderTasks.count(); ++i) {
        if (fRenderTasks[i] && fRenderTasks[i]->isUsed(proxy)) {
            return true;
        }
    }

    return false;
}

GrRenderTask* GrDrawingManager::RenderTaskDAG::add(sk_sp<GrRenderTask> renderTask) {
    return fRenderTasks.emplace_back(std::move(renderTask)).get();
}

GrRenderTask* GrDrawingManager::RenderTaskDAG::addBeforeLast(sk_sp<GrRenderTask> renderTask) {
    SkASSERT(!fRenderTasks.empty());
    // Release 'fRenderTasks.back()' and grab the raw pointer. If we pass in a reference to an array
    // element, and the array grows during emplace_back, then the reference will become invalidated.
    fRenderTasks.emplace_back(fRenderTasks.back().release());
    return (fRenderTasks[fRenderTasks.count() - 2] = std::move(renderTask)).get();
}

void GrDrawingManager::RenderTaskDAG::add(const SkTArray<sk_sp<GrRenderTask>>& opLists) {
    fRenderTasks.push_back_n(opLists.count(), opLists.begin());
}

void GrDrawingManager::RenderTaskDAG::swap(SkTArray<sk_sp<GrRenderTask>>* opLists) {
    SkASSERT(opLists->empty());
    opLists->swap(fRenderTasks);
}

void GrDrawingManager::RenderTaskDAG::prepForFlush() {
    if (fSortRenderTasks) {
        SkDEBUGCODE(bool result =) SkTTopoSort<GrRenderTask, GrRenderTask::TopoSortTraits>(
                &fRenderTasks);
        SkASSERT(result);
    }

#ifdef SK_DEBUG
    // This block checks for any unnecessary splits in the opLists. If two sequential opLists
    // share the same backing GrSurfaceProxy it means the opList was artificially split.
    if (fRenderTasks.count()) {
        GrRenderTargetOpList* prevOpList = fRenderTasks[0]->asRenderTargetOpList();
        for (int i = 1; i < fRenderTasks.count(); ++i) {
            GrRenderTargetOpList* curOpList = fRenderTasks[i]->asRenderTargetOpList();

            if (prevOpList && curOpList) {
                SkASSERT(prevOpList->fTarget.get() != curOpList->fTarget.get());
            }

            prevOpList = curOpList;
        }
    }
#endif
}

void GrDrawingManager::RenderTaskDAG::closeAll(const GrCaps* caps) {
    for (int i = 0; i < fRenderTasks.count(); ++i) {
        if (fRenderTasks[i]) {
            fRenderTasks[i]->makeClosed(*caps);
        }
    }
}

void GrDrawingManager::RenderTaskDAG::cleanup(const GrCaps* caps) {
    for (int i = 0; i < fRenderTasks.count(); ++i) {
        if (!fRenderTasks[i]) {
            continue;
        }

        // no renderTask should receive a dependency
        fRenderTasks[i]->makeClosed(*caps);

        // We shouldn't need to do this, but it turns out some clients still hold onto opLists
        // after a cleanup.
        // MDB TODO: is this still true?
        if (!fRenderTasks[i]->unique()) {
            // TODO: Eventually this should be guaranteed unique.
            // https://bugs.chromium.org/p/skia/issues/detail?id=7111
            fRenderTasks[i]->endFlush();
        }
    }

    fRenderTasks.reset();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GrDrawingManager::GrDrawingManager(GrRecordingContext* context,
                                   const GrPathRendererChain::Options& optionsForPathRendererChain,
                                   const GrTextContext::Options& optionsForTextContext,
                                   bool sortRenderTasks,
                                   bool reduceOpListSplitting)
        : fContext(context)
        , fOptionsForPathRendererChain(optionsForPathRendererChain)
        , fOptionsForTextContext(optionsForTextContext)
        , fDAG(sortRenderTasks)
        , fTextContext(nullptr)
        , fPathRendererChain(nullptr)
        , fSoftwarePathRenderer(nullptr)
        , fFlushing(false)
        , fReduceOpListSplitting(reduceOpListSplitting) {
}

void GrDrawingManager::cleanup() {
    fDAG.cleanup(fContext->priv().caps());

    fPathRendererChain = nullptr;
    fSoftwarePathRenderer = nullptr;

    fOnFlushCBObjects.reset();
}

GrDrawingManager::~GrDrawingManager() {
    this->cleanup();
}

bool GrDrawingManager::wasAbandoned() const {
    return fContext->priv().abandoned();
}

void GrDrawingManager::freeGpuResources() {
    for (int i = fOnFlushCBObjects.count() - 1; i >= 0; --i) {
        if (!fOnFlushCBObjects[i]->retainOnFreeGpuResources()) {
            // it's safe to just do this because we're iterating in reverse
            fOnFlushCBObjects.removeShuffle(i);
        }
    }

    // a path renderer may be holding onto resources
    fPathRendererChain = nullptr;
    fSoftwarePathRenderer = nullptr;
}

// MDB TODO: make use of the 'proxy' parameter.
GrSemaphoresSubmitted GrDrawingManager::flush(GrSurfaceProxy* proxies[], int numProxies,
        SkSurface::BackendSurfaceAccess access, const GrFlushInfo& info,
        const GrPrepareForExternalIORequests& externalRequests) {
    SkASSERT(numProxies >= 0);
    SkASSERT(!numProxies || proxies);
    GR_CREATE_TRACE_MARKER_CONTEXT("GrDrawingManager", "flush", fContext);

    if (fFlushing || this->wasAbandoned()) {
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return GrSemaphoresSubmitted::kNo;
    }

    SkDEBUGCODE(this->validate());

    if (kNone_GrFlushFlags == info.fFlags && !info.fNumSemaphores && !info.fFinishedProc &&
            !externalRequests.hasRequests()) {
        bool canSkip = numProxies > 0;
        for (int i = 0; i < numProxies && canSkip; ++i) {
            canSkip = !fDAG.isUsed(proxies[i]) && !this->isDDLTarget(proxies[i]);
        }
        if (canSkip) {
            return GrSemaphoresSubmitted::kNo;
        }
    }

    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return GrSemaphoresSubmitted::kNo; // Can't flush while DDL recording
    }

    GrGpu* gpu = direct->priv().getGpu();
    if (!gpu) {
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return GrSemaphoresSubmitted::kNo; // Can't flush while DDL recording
    }

    fFlushing = true;

    auto resourceProvider = direct->priv().resourceProvider();
    auto resourceCache = direct->priv().getResourceCache();

    // Semi-usually the GrRenderTasks are already closed at this point, but sometimes Ganesh needs
    // to flush mid-draw. In that case, the SkGpuDevice's opLists won't be closed but need to be
    // flushed anyway. Closing such opLists here will mean new ones will be created to replace them
    // if the SkGpuDevice(s) write to them again.
    fDAG.closeAll(fContext->priv().caps());
    fActiveOpList = nullptr;

    fDAG.prepForFlush();
    if (!fCpuBufferCache) {
        // We cache more buffers when the backend is using client side arrays. Otherwise, we
        // expect each pool will use a CPU buffer as a staging buffer before uploading to a GPU
        // buffer object. Each pool only requires one staging buffer at a time.
        int maxCachedBuffers = fContext->priv().caps()->preferClientSideDynamicBuffers() ? 2 : 6;
        fCpuBufferCache = GrBufferAllocPool::CpuBufferCache::Make(maxCachedBuffers);
    }

    GrOpFlushState flushState(gpu, resourceProvider, &fTokenTracker, fCpuBufferCache);

    GrOnFlushResourceProvider onFlushProvider(this);
    // TODO: AFAICT the only reason fFlushState is on GrDrawingManager rather than on the
    // stack here is to preserve the flush tokens.

    // Prepare any onFlush op lists (e.g. atlases).
    if (!fOnFlushCBObjects.empty()) {
        fDAG.gatherIDs(&fFlushingRenderTaskIDs);

        SkSTArray<4, sk_sp<GrRenderTargetContext>> renderTargetContexts;
        for (GrOnFlushCallbackObject* onFlushCBObject : fOnFlushCBObjects) {
            onFlushCBObject->preFlush(&onFlushProvider, fFlushingRenderTaskIDs.begin(),
                                      fFlushingRenderTaskIDs.count(), &renderTargetContexts);
            for (const sk_sp<GrRenderTargetContext>& rtc : renderTargetContexts) {
                sk_sp<GrRenderTargetOpList> onFlushOpList = sk_ref_sp(rtc->getRTOpList());
                if (!onFlushOpList) {
                    continue;   // Odd - but not a big deal
                }
#ifdef SK_DEBUG
                // OnFlush callbacks are already invoked during flush, and are therefore expected to
                // handle resource allocation & usage on their own. (No deferred or lazy proxies!)
                onFlushOpList->visitProxies_debugOnly([](GrSurfaceProxy* p, GrMipMapped) {
                    SkASSERT(!p->asTextureProxy() || !p->asTextureProxy()->texPriv().isDeferred());
                    SkASSERT(GrSurfaceProxy::LazyState::kNot == p->lazyInstantiationState());
                });
#endif
                onFlushOpList->makeClosed(*fContext->priv().caps());
                onFlushOpList->prepare(&flushState);
                fOnFlushCBOpLists.push_back(std::move(onFlushOpList));
            }
            renderTargetContexts.reset();
        }
    }

#if 0
    // Enable this to print out verbose GrOp information
    for (int i = 0; i < fRenderTasks.count(); ++i) {
        SkDEBUGCODE(fRenderTasks[i]->dump();)
    }
#endif

    int startIndex, stopIndex;
    bool flushed = false;

    {
        GrResourceAllocator alloc(resourceProvider, flushState.deinstantiateProxyTracker()
                                  SkDEBUGCODE(, fDAG.numRenderTasks()));
        for (int i = 0; i < fDAG.numRenderTasks(); ++i) {
            if (fDAG.renderTask(i)) {
                fDAG.renderTask(i)->gatherProxyIntervals(&alloc);
            }
            alloc.markEndOfOpList(i);
        }
        alloc.determineRecyclability();

        GrResourceAllocator::AssignError error = GrResourceAllocator::AssignError::kNoError;
        int numRenderTasksExecuted = 0;
        while (alloc.assign(&startIndex, &stopIndex, &error)) {
            if (GrResourceAllocator::AssignError::kFailedProxyInstantiation == error) {
                for (int i = startIndex; i < stopIndex; ++i) {
                    GrRenderTask* renderTask = fDAG.renderTask(i);
                    if (!renderTask) {
                        continue;
                    }
                    if (!renderTask->isInstantiated()) {
                        // If the backing surface wasn't allocated, drop the entire renderTask.
                        fDAG.removeRenderTask(i);
                        continue;
                    }
                    renderTask->handleInternalAllocationFailure();
                }
            }

            if (this->executeRenderTasks(
                    startIndex, stopIndex, &flushState, &numRenderTasksExecuted)) {
                flushed = true;
            }
        }
    }

#ifdef SK_DEBUG
    for (int i = 0; i < fDAG.numRenderTasks(); ++i) {
        // If there are any remaining opLists at this point, make sure they will not survive the
        // flush. Otherwise we need to call endFlush() on them.
        // http://skbug.com/7111
        SkASSERT(!fDAG.renderTask(i) || fDAG.renderTask(i)->unique());
    }
#endif
    fDAG.reset();
    this->clearDDLTargets();

#ifdef SK_DEBUG
    // In non-DDL mode this checks that all the flushed ops have been freed from the memory pool.
    // When we move to partial flushes this assert will no longer be valid.
    // In DDL mode this check is somewhat superfluous since the memory for most of the ops/opLists
    // will be stored in the DDL's GrOpMemoryPools.
    GrOpMemoryPool* opMemoryPool = fContext->priv().opMemoryPool();
    opMemoryPool->isEmpty();
#endif

    GrSemaphoresSubmitted result = gpu->finishFlush(proxies, numProxies, access, info,
                                                    externalRequests);

    flushState.deinstantiateProxyTracker()->deinstantiateAllProxies();

    // Give the cache a chance to purge resources that become purgeable due to flushing.
    if (flushed) {
        resourceCache->purgeAsNeeded();
        flushed = false;
    }
    for (GrOnFlushCallbackObject* onFlushCBObject : fOnFlushCBObjects) {
        onFlushCBObject->postFlush(fTokenTracker.nextTokenToFlush(), fFlushingRenderTaskIDs.begin(),
                                   fFlushingRenderTaskIDs.count());
        flushed = true;
    }
    if (flushed) {
        resourceCache->purgeAsNeeded();
    }
    fFlushingRenderTaskIDs.reset();
    fFlushing = false;

    return result;
}

bool GrDrawingManager::executeRenderTasks(int startIndex, int stopIndex, GrOpFlushState* flushState,
                                          int* numRenderTasksExecuted) {
    SkASSERT(startIndex <= stopIndex && stopIndex <= fDAG.numRenderTasks());

#if GR_FLUSH_TIME_OP_SPEW
    SkDebugf("Flushing opLists: %d to %d out of [%d, %d]\n",
                            startIndex, stopIndex, 0, fDAG.numRenderTasks());
    for (int i = startIndex; i < stopIndex; ++i) {
        if (fDAG.renderTask(i)) {
            fDAG.renderTask(i)->dump(true);
        }
    }
#endif

    bool anyRenderTasksExecuted = false;

    for (int i = startIndex; i < stopIndex; ++i) {
        if (!fDAG.renderTask(i)) {
             continue;
        }

        GrRenderTask* renderTask = fDAG.renderTask(i);
        SkASSERT(renderTask->isInstantiated());
        SkASSERT(renderTask->deferredProxiesAreInstantiated());

        renderTask->prepare(flushState);
    }

    // Upload all data to the GPU
    flushState->preExecuteDraws();

    // For Vulkan, if we have too many oplists to be flushed we end up allocating a lot of resources
    // for each command buffer associated with the oplists. If this gets too large we can cause the
    // devices to go OOM. In practice we usually only hit this case in our tests, but to be safe we
    // put a cap on the number of oplists we will execute before flushing to the GPU to relieve some
    // memory pressure.
    static constexpr int kMaxRenderTasksBeforeFlush = 100;

    // Execute the onFlush op lists first, if any.
    for (sk_sp<GrOpList>& onFlushOpList : fOnFlushCBOpLists) {
        if (!onFlushOpList->execute(flushState)) {
            SkDebugf("WARNING: onFlushOpList failed to execute.\n");
        }
        SkASSERT(onFlushOpList->unique());
        onFlushOpList = nullptr;
        (*numRenderTasksExecuted)++;
        if (*numRenderTasksExecuted >= kMaxRenderTasksBeforeFlush) {
            flushState->gpu()->finishFlush(nullptr, 0, SkSurface::BackendSurfaceAccess::kNoAccess,
                                           GrFlushInfo(), GrPrepareForExternalIORequests());
            *numRenderTasksExecuted = 0;
        }
    }
    fOnFlushCBOpLists.reset();

    // Execute the normal op lists.
    for (int i = startIndex; i < stopIndex; ++i) {
        if (!fDAG.renderTask(i)) {
            continue;
        }

        if (fDAG.renderTask(i)->execute(flushState)) {
            anyRenderTasksExecuted = true;
        }
        (*numRenderTasksExecuted)++;
        if (*numRenderTasksExecuted >= kMaxRenderTasksBeforeFlush) {
            flushState->gpu()->finishFlush(nullptr, 0, SkSurface::BackendSurfaceAccess::kNoAccess,
                                           GrFlushInfo(), GrPrepareForExternalIORequests());
            *numRenderTasksExecuted = 0;
        }
    }

    SkASSERT(!flushState->commandBuffer());
    SkASSERT(fTokenTracker.nextDrawToken() == fTokenTracker.nextTokenToFlush());

    // We reset the flush state before the RenderTasks so that the last resources to be freed are
    // those that are written to in the RenderTasks. This helps to make sure the most recently used
    // resources are the last to be purged by the resource cache.
    flushState->reset();

    fDAG.removeRenderTasks(startIndex, stopIndex);

    return anyRenderTasksExecuted;
}

GrSemaphoresSubmitted GrDrawingManager::flushSurfaces(GrSurfaceProxy* proxies[], int numProxies,
                                                      SkSurface::BackendSurfaceAccess access,
                                                      const GrFlushInfo& info) {
    if (this->wasAbandoned()) {
        return GrSemaphoresSubmitted::kNo;
    }
    SkDEBUGCODE(this->validate());
    SkASSERT(numProxies >= 0);
    SkASSERT(!numProxies || proxies);

    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        return GrSemaphoresSubmitted::kNo; // Can't flush while DDL recording
    }

    GrGpu* gpu = direct->priv().getGpu();
    if (!gpu) {
        return GrSemaphoresSubmitted::kNo; // Can't flush while DDL recording
    }

    // TODO: It is important to upgrade the drawingmanager to just flushing the
    // portion of the DAG required by 'proxies' in order to restore some of the
    // semantics of this method.
    GrSemaphoresSubmitted result = this->flush(proxies, numProxies, access, info,
                                               GrPrepareForExternalIORequests());
    for (int i = 0; i < numProxies; ++i) {
        if (!proxies[i]->isInstantiated()) {
            return result;
        }
        GrSurface* surface = proxies[i]->peekSurface();
        if (auto* rt = surface->asRenderTarget()) {
            gpu->resolveRenderTarget(rt);
        }
        // If, after a flush, any of the proxies of interest have dirty mipmaps, regenerate them in
        // case their backend textures are being stolen.
        // (This special case is exercised by the ReimportImageTextureWithMipLevels test.)
        // FIXME: It may be more ideal to plumb down a "we're going to steal the backends" flag.
        if (auto* textureProxy = proxies[i]->asTextureProxy()) {
            if (textureProxy->mipMapsAreDirty()) {
                gpu->regenerateMipMapLevels(textureProxy->peekTexture());
                textureProxy->markMipMapsClean();
            }
        }
    }

    SkDEBUGCODE(this->validate());
    return result;
}

void GrDrawingManager::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fOnFlushCBObjects.push_back(onFlushCBObject);
}

#if GR_TEST_UTILS
void GrDrawingManager::testingOnly_removeOnFlushCallbackObject(GrOnFlushCallbackObject* cb) {
    int n = std::find(fOnFlushCBObjects.begin(), fOnFlushCBObjects.end(), cb) -
            fOnFlushCBObjects.begin();
    SkASSERT(n < fOnFlushCBObjects.count());
    fOnFlushCBObjects.removeShuffle(n);
}
#endif

void GrDrawingManager::moveRenderTasksToDDL(SkDeferredDisplayList* ddl) {
    SkDEBUGCODE(this->validate());

    // no renderTask should receive a new command after this
    fDAG.closeAll(fContext->priv().caps());
    fActiveOpList = nullptr;

    fDAG.swap(&ddl->fRenderTasks);

    if (fPathRendererChain) {
        if (auto ccpr = fPathRendererChain->getCoverageCountingPathRenderer()) {
            ddl->fPendingPaths = ccpr->detachPendingPaths();
        }
    }

    SkDEBUGCODE(this->validate());
}

void GrDrawingManager::copyRenderTasksFromDDL(const SkDeferredDisplayList* ddl,
                                          GrRenderTargetProxy* newDest) {
    SkDEBUGCODE(this->validate());

    if (fActiveOpList) {
        // This is  a temporary fix for the partial-MDB world. In that world we're not
        // reordering so ops that (in the single opList world) would've just glommed onto the
        // end of the single opList but referred to a far earlier RT need to appear in their
        // own opList.
        fActiveOpList->makeClosed(*fContext->priv().caps());
        fActiveOpList = nullptr;
    }

    this->addDDLTarget(newDest);

    // Here we jam the proxy that backs the current replay SkSurface into the LazyProxyData.
    // The lazy proxy that references it (in the copied opLists) will steal its GrTexture.
    ddl->fLazyProxyData->fReplayDest = newDest;

    if (ddl->fPendingPaths.size()) {
        GrCoverageCountingPathRenderer* ccpr = this->getCoverageCountingPathRenderer();

        ccpr->mergePendingPaths(ddl->fPendingPaths);
    }

    fDAG.add(ddl->fRenderTasks);

    SkDEBUGCODE(this->validate());
}

#ifdef SK_DEBUG
void GrDrawingManager::validate() const {
    if (fDAG.sortingRenderTasks() && fReduceOpListSplitting) {
        SkASSERT(!fActiveOpList);
    } else {
        if (fActiveOpList) {
            SkASSERT(!fDAG.empty());
            SkASSERT(!fActiveOpList->isClosed());
            SkASSERT(fActiveOpList == fDAG.back());
        }

        for (int i = 0; i < fDAG.numRenderTasks(); ++i) {
            if (fActiveOpList != fDAG.renderTask(i)) {
                SkASSERT(fDAG.renderTask(i)->isClosed());
            }
        }

        if (!fDAG.empty() && !fDAG.back()->isClosed()) {
            SkASSERT(fActiveOpList == fDAG.back());
        }
    }
}
#endif

void GrDrawingManager::closeRenderTasksForNewOpList(GrSurfaceProxy* target) {
    if (fDAG.sortingRenderTasks() && fReduceOpListSplitting) {
        // In this case we need to close all the renderTasks that rely on the current contents of
        // 'target'. That is bc we're going to update the content of the proxy so they need to be
        // split in case they use both the old and new content. (This is a bit of an overkill: they
        // really only need to be split if they ever reference proxy's contents again but that is
        // hard to predict/handle).
        if (GrRenderTask* lastRenderTask = target->getLastRenderTask()) {
            lastRenderTask->closeThoseWhoDependOnMe(*fContext->priv().caps());
        }
    } else if (fActiveOpList) {
        // This is  a temporary fix for the partial-MDB world. In that world we're not
        // reordering so ops that (in the single opList world) would've just glommed onto the
        // end of the single opList but referred to a far earlier RT need to appear in their
        // own opList.
        fActiveOpList->makeClosed(*fContext->priv().caps());
        fActiveOpList = nullptr;
    }
}

sk_sp<GrRenderTargetOpList> GrDrawingManager::newRTOpList(sk_sp<GrRenderTargetProxy> rtp,
                                                          bool managedOpList) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);

    this->closeRenderTasksForNewOpList(rtp.get());

    sk_sp<GrRenderTargetOpList> opList(new GrRenderTargetOpList(
                                                        fContext->priv().refOpMemoryPool(),
                                                        rtp,
                                                        fContext->priv().auditTrail()));
    SkASSERT(rtp->getLastRenderTask() == opList.get());

    if (managedOpList) {
        fDAG.add(opList);

        if (!fDAG.sortingRenderTasks() || !fReduceOpListSplitting) {
            fActiveOpList = opList.get();
        }
    }

    SkDEBUGCODE(this->validate());
    return opList;
}

sk_sp<GrTextureOpList> GrDrawingManager::newTextureOpList(sk_sp<GrTextureProxy> textureProxy) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);

    this->closeRenderTasksForNewOpList(textureProxy.get());

    sk_sp<GrTextureOpList> opList(new GrTextureOpList(fContext->priv().refOpMemoryPool(),
                                                      textureProxy,
                                                      fContext->priv().auditTrail()));

    SkASSERT(textureProxy->getLastRenderTask() == opList.get());

    fDAG.add(opList);
    if (!fDAG.sortingRenderTasks() || !fReduceOpListSplitting) {
        fActiveOpList = opList.get();
    }

    SkDEBUGCODE(this->validate());
    return opList;
}

GrRenderTask* GrDrawingManager::newTextureResolveRenderTask(
        sk_sp<GrTextureProxy> textureProxy, GrTextureResolveFlags flags, const GrCaps& caps) {
    // Unlike in the "new opList" cases, we do not want to close the active opList, nor (if we are
    // in sorting and opList reduction mode) the render tasks that depend on the proxy's current
    // state. This is because those opLists can still receive new ops and because if they refer to
    // the mipmapped version of 'textureProxy', they will then come to depend on the render task
    // being created here.
    // NOTE: In either case, 'textureProxy' should already be closed at this point (i.e., its state
    // is finished))
    SkASSERT(!textureProxy->getLastRenderTask() || textureProxy->getLastRenderTask()->isClosed());
    auto textureResolveTask = GrTextureResolveRenderTask::Make(textureProxy, flags, caps);
    SkASSERT(textureProxy->getLastRenderTask() == textureResolveTask.get());
    // Add the new textureResolveTask before the fActiveOpList (if not in
    // sorting/opList-splitting-reduction mode) because it will depend upon this resolve task.
    // NOTE: Putting it here will also reduce the amount of work required by the topological sort.
    return fDAG.addBeforeLast(std::move(textureResolveTask));
}

GrTextContext* GrDrawingManager::getTextContext() {
    if (!fTextContext) {
        fTextContext = GrTextContext::Make(fOptionsForTextContext);
    }

    return fTextContext.get();
}

/*
 * This method finds a path renderer that can draw the specified path on
 * the provided target.
 * Due to its expense, the software path renderer has split out so it can
 * can be individually allowed/disallowed via the "allowSW" boolean.
 */
GrPathRenderer* GrDrawingManager::getPathRenderer(const GrPathRenderer::CanDrawPathArgs& args,
                                                  bool allowSW,
                                                  GrPathRendererChain::DrawType drawType,
                                                  GrPathRenderer::StencilSupport* stencilSupport) {

    if (!fPathRendererChain) {
        fPathRendererChain.reset(new GrPathRendererChain(fContext, fOptionsForPathRendererChain));
    }

    GrPathRenderer* pr = fPathRendererChain->getPathRenderer(args, drawType, stencilSupport);
    if (!pr && allowSW) {
        auto swPR = this->getSoftwarePathRenderer();
        if (GrPathRenderer::CanDrawPath::kNo != swPR->canDrawPath(args)) {
            pr = swPR;
        }
    }

    return pr;
}

GrPathRenderer* GrDrawingManager::getSoftwarePathRenderer() {
    if (!fSoftwarePathRenderer) {
        fSoftwarePathRenderer.reset(
                new GrSoftwarePathRenderer(fContext->priv().proxyProvider(),
                                           fOptionsForPathRendererChain.fAllowPathMaskCaching));
    }
    return fSoftwarePathRenderer.get();
}

GrCoverageCountingPathRenderer* GrDrawingManager::getCoverageCountingPathRenderer() {
    if (!fPathRendererChain) {
        fPathRendererChain.reset(new GrPathRendererChain(fContext, fOptionsForPathRendererChain));
    }
    return fPathRendererChain->getCoverageCountingPathRenderer();
}

void GrDrawingManager::flushIfNecessary() {
    auto direct = fContext->priv().asDirectContext();
    if (!direct) {
        return;
    }

    auto resourceCache = direct->priv().getResourceCache();
    if (resourceCache && resourceCache->requestsFlush()) {
        this->flush(nullptr, 0, SkSurface::BackendSurfaceAccess::kNoAccess, GrFlushInfo(),
                    GrPrepareForExternalIORequests());
        resourceCache->purgeAsNeeded();
    }
}

sk_sp<GrRenderTargetContext> GrDrawingManager::makeRenderTargetContext(
        sk_sp<GrSurfaceProxy> sProxy,
        GrColorType colorType,
        sk_sp<SkColorSpace> colorSpace,
        const SkSurfaceProps* surfaceProps,
        bool managedOpList) {
    if (this->wasAbandoned() || !sProxy->asRenderTargetProxy()) {
        return nullptr;
    }

    // SkSurface catches bad color space usage at creation. This check handles anything that slips
    // by, including internal usage.
    if (!SkSurface_Gpu::Valid(fContext->priv().caps(), sProxy->backendFormat())) {
        SkDEBUGFAIL("Invalid config and colorspace combination");
        return nullptr;
    }

    sk_sp<GrRenderTargetProxy> renderTargetProxy(sk_ref_sp(sProxy->asRenderTargetProxy()));

    return sk_sp<GrRenderTargetContext>(new GrRenderTargetContext(fContext,
                                                                  std::move(renderTargetProxy),
                                                                  colorType,
                                                                  std::move(colorSpace),
                                                                  surfaceProps,
                                                                  managedOpList));
}

sk_sp<GrTextureContext> GrDrawingManager::makeTextureContext(sk_sp<GrSurfaceProxy> sProxy,
                                                             GrColorType colorType,
                                                             SkAlphaType alphaType,
                                                             sk_sp<SkColorSpace> colorSpace) {
    if (this->wasAbandoned() || !sProxy->asTextureProxy()) {
        return nullptr;
    }

    // SkSurface catches bad color space usage at creation. This check handles anything that slips
    // by, including internal usage.
    if (!SkSurface_Gpu::Valid(fContext->priv().caps(), sProxy->backendFormat())) {
        SkDEBUGFAIL("Invalid config and colorspace combination");
        return nullptr;
    }

    // GrTextureRenderTargets should always be using a GrRenderTargetContext
    SkASSERT(!sProxy->asRenderTargetProxy());

    sk_sp<GrTextureProxy> textureProxy(sk_ref_sp(sProxy->asTextureProxy()));

    return sk_sp<GrTextureContext>(new GrTextureContext(fContext,
                                                        std::move(textureProxy),
                                                        colorType,
                                                        alphaType,
                                                        std::move(colorSpace)));
}
