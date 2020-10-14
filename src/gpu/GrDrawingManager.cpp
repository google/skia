/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDrawingManager.h"

#include <memory>

#include "include/core/SkDeferredDisplayList.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkDeferredDisplayListPriv.h"
#include "src/core/SkTTopoSort.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrClientMappedBufferManager.h"
#include "src/gpu/GrCopyRenderTask.h"
#include "src/gpu/GrDirectContextPriv.h"
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
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/GrTextureResolveRenderTask.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/GrTransferFromRenderTask.h"
#include "src/gpu/GrUnrefDDLTask.h"
#include "src/gpu/GrWaitRenderTask.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/text/GrSDFTOptions.h"
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

void GrDrawingManager::RenderTaskDAG::rawRemoveRenderTasks(int startIndex, int stopIndex) {
    for (int i = startIndex; i < stopIndex; ++i) {
        fRenderTasks[i] = nullptr;
    }
}

bool GrDrawingManager::RenderTaskDAG::isUsed(GrSurfaceProxy* proxy) const {
    for (const auto& task : fRenderTasks) {
        if (task && task->isUsed(proxy)) {
            return true;
        }
    }

    return false;
}

GrRenderTask* GrDrawingManager::RenderTaskDAG::add(sk_sp<GrRenderTask> renderTask) {
    if (renderTask) {
        return fRenderTasks.emplace_back(std::move(renderTask)).get();
    }
    return nullptr;
}

GrRenderTask* GrDrawingManager::RenderTaskDAG::addBeforeLast(sk_sp<GrRenderTask> renderTask) {
    SkASSERT(!fRenderTasks.empty());
    if (renderTask) {
        // Release 'fRenderTasks.back()' and grab the raw pointer, in case the SkTArray grows
        // and reallocates during emplace_back.
        fRenderTasks.emplace_back(fRenderTasks.back().release());
        return (fRenderTasks[fRenderTasks.count() - 2] = std::move(renderTask)).get();
    }
    return nullptr;
}

void GrDrawingManager::RenderTaskDAG::add(const SkTArray<sk_sp<GrRenderTask>>& renderTasks) {
#ifdef SK_DEBUG
    for (auto& renderTask : renderTasks) {
        SkASSERT(renderTask->unique());
    }
#endif

    fRenderTasks.push_back_n(renderTasks.count(), renderTasks.begin());
}

void GrDrawingManager::RenderTaskDAG::swap(SkTArray<sk_sp<GrRenderTask>>* renderTasks) {
    SkASSERT(renderTasks->empty());
    renderTasks->swap(fRenderTasks);
}

void GrDrawingManager::RenderTaskDAG::prepForFlush() {
    if (fSortRenderTasks) {
        SkDEBUGCODE(bool result =) SkTTopoSort<GrRenderTask, GrRenderTask::TopoSortTraits>(
                &fRenderTasks);
        SkASSERT(result);
    }

#ifdef SK_DEBUG
    // This block checks for any unnecessary splits in the opsTasks. If two sequential opsTasks
    // share the same backing GrSurfaceProxy it means the opsTask was artificially split.
    if (fRenderTasks.count()) {
        GrOpsTask* prevOpsTask = fRenderTasks[0]->asOpsTask();
        for (int i = 1; i < fRenderTasks.count(); ++i) {
            GrOpsTask* curOpsTask = fRenderTasks[i]->asOpsTask();

            if (prevOpsTask && curOpsTask) {
                SkASSERT(prevOpsTask->target(0).proxy() != curOpsTask->target(0).proxy());
            }

            prevOpsTask = curOpsTask;
        }
    }
#endif
}

void GrDrawingManager::RenderTaskDAG::closeAll(const GrCaps* caps) {
    for (auto& task : fRenderTasks) {
        if (task) {
            task->makeClosed(*caps);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
GrDrawingManager::GrDrawingManager(GrRecordingContext* context,
                                   const GrPathRendererChain::Options& optionsForPathRendererChain,
                                   bool sortRenderTasks,
                                   bool reduceOpsTaskSplitting)
        : fContext(context)
        , fOptionsForPathRendererChain(optionsForPathRendererChain)
        , fDAG(sortRenderTasks)
        , fPathRendererChain(nullptr)
        , fSoftwarePathRenderer(nullptr)
        , fFlushing(false)
        , fReduceOpsTaskSplitting(reduceOpsTaskSplitting) { }

GrDrawingManager::~GrDrawingManager() {
    fDAG.closeAll(fContext->priv().caps());
    this->removeRenderTasks(0, fDAG.numRenderTasks());
}

bool GrDrawingManager::wasAbandoned() const {
    return fContext->abandoned();
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
bool GrDrawingManager::flush(
        GrSurfaceProxy* proxies[],
        int numProxies,
        SkSurface::BackendSurfaceAccess access,
        const GrFlushInfo& info,
        const GrBackendSurfaceMutableState* newState) {
    SkASSERT(numProxies >= 0);
    SkASSERT(!numProxies || proxies);
    GR_CREATE_TRACE_MARKER_CONTEXT("GrDrawingManager", "flush", fContext);

    if (fFlushing || this->wasAbandoned()) {
        if (info.fSubmittedProc) {
            info.fSubmittedProc(info.fSubmittedContext, false);
        }
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return false;
    }

    SkDEBUGCODE(this->validate());

    if (!info.fNumSemaphores && !info.fFinishedProc &&
        access == SkSurface::BackendSurfaceAccess::kNoAccess && !newState) {
        bool canSkip = numProxies > 0;
        for (int i = 0; i < numProxies && canSkip; ++i) {
            canSkip = !fDAG.isUsed(proxies[i]) && !this->isDDLTarget(proxies[i]);
        }
        if (canSkip) {
            if (info.fSubmittedProc) {
                info.fSubmittedProc(info.fSubmittedContext, true);
            }
            return false;
        }
    }

    auto direct = fContext->asDirectContext();
    if (!direct) {
        if (info.fSubmittedProc) {
            info.fSubmittedProc(info.fSubmittedContext, false);
        }
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return false; // Can't flush while DDL recording
    }
    direct->priv().clientMappedBufferManager()->process();

    GrGpu* gpu = direct->priv().getGpu();
    // We have a non abandoned and direct GrContext. It must have a GrGpu.
    SkASSERT(gpu);

    fFlushing = true;

    auto resourceProvider = direct->priv().resourceProvider();
    auto resourceCache = direct->priv().getResourceCache();

    // Semi-usually the GrRenderTasks are already closed at this point, but sometimes Ganesh needs
    // to flush mid-draw. In that case, the SkGpuDevice's opsTasks won't be closed but need to be
    // flushed anyway. Closing such opsTasks here will mean new ones will be created to replace them
    // if the SkGpuDevice(s) write to them again.
    fDAG.closeAll(fContext->priv().caps());
    fActiveOpsTask = nullptr;

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

    // Prepare any onFlush op lists (e.g. atlases).
    if (!fOnFlushCBObjects.empty()) {
        fDAG.gatherIDs(&fFlushingRenderTaskIDs);

        for (GrOnFlushCallbackObject* onFlushCBObject : fOnFlushCBObjects) {
            onFlushCBObject->preFlush(&onFlushProvider, fFlushingRenderTaskIDs.begin(),
                                      fFlushingRenderTaskIDs.count());
        }
        for (const auto& onFlushRenderTask : fOnFlushRenderTasks) {
            onFlushRenderTask->makeClosed(*fContext->priv().caps());
#ifdef SK_DEBUG
            // OnFlush callbacks are invoked during flush, and are therefore expected to handle
            // resource allocation & usage on their own. (No deferred or lazy proxies!)
            onFlushRenderTask->visitTargetAndSrcProxies_debugOnly(
                    [](GrSurfaceProxy* p, GrMipmapped mipMapped) {
                SkASSERT(!p->asTextureProxy() || !p->asTextureProxy()->texPriv().isDeferred());
                SkASSERT(!p->isLazy());
                if (p->requiresManualMSAAResolve()) {
                    // The onFlush callback is responsible for ensuring MSAA gets resolved.
                    SkASSERT(p->asRenderTargetProxy() && !p->asRenderTargetProxy()->isMSAADirty());
                }
                if (GrMipmapped::kYes == mipMapped) {
                    // The onFlush callback is responsible for regenerating mips if needed.
                    SkASSERT(p->asTextureProxy() && !p->asTextureProxy()->mipmapsAreDirty());
                }
            });
#endif
            onFlushRenderTask->prepare(&flushState);
        }
    }

#if 0
    // Enable this to print out verbose GrOp information
    SkDEBUGCODE(SkDebugf("onFlush renderTasks:"));
    for (const auto& onFlushRenderTask : fOnFlushRenderTasks) {
        SkDEBUGCODE(onFlushRenderTask->dump();)
    }
    SkDEBUGCODE(SkDebugf("Normal renderTasks:"));
    for (int i = 0; i < fRenderTasks.count(); ++i) {
        SkDEBUGCODE(fRenderTasks[i]->dump();)
    }
#endif

    int startIndex, stopIndex;
    bool flushed = false;

    {
        GrResourceAllocator alloc(resourceProvider SkDEBUGCODE(, fDAG.numRenderTasks()));
        for (int i = 0; i < fDAG.numRenderTasks(); ++i) {
            if (fDAG.renderTask(i)) {
                fDAG.renderTask(i)->gatherProxyIntervals(&alloc);
            }
            alloc.markEndOfOpsTask(i);
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
                        // No need to call the renderTask's handleInternalAllocationFailure
                        // since we will already skip executing the renderTask since it is not
                        // instantiated.
                        continue;
                    }
                    renderTask->handleInternalAllocationFailure();
                }
                this->removeRenderTasks(startIndex, stopIndex);
            }

            if (this->executeRenderTasks(
                    startIndex, stopIndex, &flushState, &numRenderTasksExecuted)) {
                flushed = true;
            }
        }
    }

#ifdef SK_DEBUG
    for (int i = 0; i < fDAG.numRenderTasks(); ++i) {
        // All render tasks should have been cleared out by now – we only reset the array below to
        // reclaim storage.
        SkASSERT(!fDAG.renderTask(i));
    }
#endif
    fLastRenderTasks.reset();
    fDAG.reset();
    this->clearDDLTargets();

#ifdef SK_DEBUG
    // In non-DDL mode this checks that all the flushed ops have been freed from the memory pool.
    // When we move to partial flushes this assert will no longer be valid.
    // In DDL mode this check is somewhat superfluous since the memory for most of the ops/opsTasks
    // will be stored in the DDL's GrOpMemoryPools.
    GrOpMemoryPool* opMemoryPool = fContext->priv().opMemoryPool();
    opMemoryPool->isEmpty();
#endif

    gpu->executeFlushInfo(proxies, numProxies, access, info, newState);

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

    return true;
}

bool GrDrawingManager::submitToGpu(bool syncToCpu) {
    if (fFlushing || this->wasAbandoned()) {
        return false;
    }

    auto direct = fContext->asDirectContext();
    if (!direct) {
        return false; // Can't submit while DDL recording
    }
    GrGpu* gpu = direct->priv().getGpu();
    return gpu->submitToGpu(syncToCpu);
}

bool GrDrawingManager::executeRenderTasks(int startIndex, int stopIndex, GrOpFlushState* flushState,
                                          int* numRenderTasksExecuted) {
    SkASSERT(startIndex <= stopIndex && stopIndex <= fDAG.numRenderTasks());

#if GR_FLUSH_TIME_OP_SPEW
    SkDebugf("Flushing opsTask: %d to %d out of [%d, %d]\n",
                            startIndex, stopIndex, 0, fDAG.numRenderTasks());
    for (int i = startIndex; i < stopIndex; ++i) {
        if (fDAG.renderTask(i)) {
            fDAG.renderTask(i)->dump(true);
        }
    }
#endif

    bool anyRenderTasksExecuted = false;

    for (int i = startIndex; i < stopIndex; ++i) {
        GrRenderTask* renderTask = fDAG.renderTask(i);
        if (!renderTask || !renderTask->isInstantiated()) {
             continue;
        }

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

    // Execute the onFlush renderTasks first, if any.
    for (sk_sp<GrRenderTask>& onFlushRenderTask : fOnFlushRenderTasks) {
        if (!onFlushRenderTask->execute(flushState)) {
            SkDebugf("WARNING: onFlushRenderTask failed to execute.\n");
        }
        SkASSERT(onFlushRenderTask->unique());
        onFlushRenderTask->disown(this);
        onFlushRenderTask = nullptr;
        (*numRenderTasksExecuted)++;
        if (*numRenderTasksExecuted >= kMaxRenderTasksBeforeFlush) {
            flushState->gpu()->submitToGpu(false);
            *numRenderTasksExecuted = 0;
        }
    }
    fOnFlushRenderTasks.reset();

    // Execute the normal op lists.
    for (int i = startIndex; i < stopIndex; ++i) {
        GrRenderTask* renderTask = fDAG.renderTask(i);
        if (!renderTask || !renderTask->isInstantiated()) {
            continue;
        }

        if (renderTask->execute(flushState)) {
            anyRenderTasksExecuted = true;
        }
        (*numRenderTasksExecuted)++;
        if (*numRenderTasksExecuted >= kMaxRenderTasksBeforeFlush) {
            flushState->gpu()->submitToGpu(false);
            *numRenderTasksExecuted = 0;
        }
    }

    SkASSERT(!flushState->opsRenderPass());
    SkASSERT(fTokenTracker.nextDrawToken() == fTokenTracker.nextTokenToFlush());

    // We reset the flush state before the RenderTasks so that the last resources to be freed are
    // those that are written to in the RenderTasks. This helps to make sure the most recently used
    // resources are the last to be purged by the resource cache.
    flushState->reset();

    this->removeRenderTasks(startIndex, stopIndex);

    return anyRenderTasksExecuted;
}

void GrDrawingManager::removeRenderTasks(int startIndex, int stopIndex) {
    for (int i = startIndex; i < stopIndex; ++i) {
        GrRenderTask* task = fDAG.renderTask(i);
        if (!task) {
            continue;
        }
        if (!task->unique()) {
            // TODO: Eventually this should be guaranteed unique: http://skbug.com/7111
            task->endFlush(this);
        }
        task->disown(this);
    }
    fDAG.rawRemoveRenderTasks(startIndex, stopIndex);
}

static void resolve_and_mipmap(GrGpu* gpu, GrSurfaceProxy* proxy) {
    if (!proxy->isInstantiated()) {
        return;
    }

    // In the flushSurfaces case, we need to resolve MSAA immediately after flush. This is
    // because clients expect the flushed surface's backing texture to be fully resolved
    // upon return.
    if (proxy->requiresManualMSAAResolve()) {
        auto* rtProxy = proxy->asRenderTargetProxy();
        SkASSERT(rtProxy);
        if (rtProxy->isMSAADirty()) {
            SkASSERT(rtProxy->peekRenderTarget());
            gpu->resolveRenderTarget(rtProxy->peekRenderTarget(), rtProxy->msaaDirtyRect());
            gpu->submitToGpu(false);
            rtProxy->markMSAAResolved();
        }
    }
    // If, after a flush, any of the proxies of interest have dirty mipmaps, regenerate them in
    // case their backend textures are being stolen.
    // (This special case is exercised by the ReimportImageTextureWithMipLevels test.)
    // FIXME: It may be more ideal to plumb down a "we're going to steal the backends" flag.
    if (auto* textureProxy = proxy->asTextureProxy()) {
        if (textureProxy->mipmapsAreDirty()) {
            SkASSERT(textureProxy->peekTexture());
            gpu->regenerateMipMapLevels(textureProxy->peekTexture());
            textureProxy->markMipmapsClean();
        }
    }
}

GrSemaphoresSubmitted GrDrawingManager::flushSurfaces(
        GrSurfaceProxy* proxies[],
        int numProxies,
        SkSurface::BackendSurfaceAccess access,
        const GrFlushInfo& info,
        const GrBackendSurfaceMutableState* newState) {
    if (this->wasAbandoned()) {
        if (info.fSubmittedProc) {
            info.fSubmittedProc(info.fSubmittedContext, false);
        }
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return GrSemaphoresSubmitted::kNo;
    }
    SkDEBUGCODE(this->validate());
    SkASSERT(numProxies >= 0);
    SkASSERT(!numProxies || proxies);

    auto direct = fContext->asDirectContext();
    if (!direct) {
        if (info.fSubmittedProc) {
            info.fSubmittedProc(info.fSubmittedContext, false);
        }
        if (info.fFinishedProc) {
            info.fFinishedProc(info.fFinishedContext);
        }
        return GrSemaphoresSubmitted::kNo; // Can't flush while DDL recording
    }

    GrGpu* gpu = direct->priv().getGpu();
    // We have a non abandoned and direct GrContext. It must have a GrGpu.
    SkASSERT(gpu);

    // TODO: It is important to upgrade the drawingmanager to just flushing the
    // portion of the DAG required by 'proxies' in order to restore some of the
    // semantics of this method.
    bool didFlush = this->flush(proxies, numProxies, access, info, newState);
    for (int i = 0; i < numProxies; ++i) {
        resolve_and_mipmap(gpu, proxies[i]);
    }

    SkDEBUGCODE(this->validate());

    if (!didFlush || (!direct->priv().caps()->semaphoreSupport() && info.fNumSemaphores)) {
        return GrSemaphoresSubmitted::kNo;
    }
    return GrSemaphoresSubmitted::kYes;
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

void GrDrawingManager::setLastRenderTask(const GrSurfaceProxy* proxy, GrRenderTask* task) {
#ifdef SK_DEBUG
    if (GrRenderTask* prior = this->getLastRenderTask(proxy)) {
        SkASSERT(prior->isClosed());
    }
#endif
    uint32_t key = proxy->uniqueID().asUInt();
    if (task) {
        fLastRenderTasks.set(key, task);
    } else if (fLastRenderTasks.find(key)) {
        fLastRenderTasks.remove(key);
    }
}

GrRenderTask* GrDrawingManager::getLastRenderTask(const GrSurfaceProxy* proxy) const {
    auto entry = fLastRenderTasks.find(proxy->uniqueID().asUInt());
    return entry ? *entry : nullptr;
}

GrOpsTask* GrDrawingManager::getLastOpsTask(const GrSurfaceProxy* proxy) const {
    GrRenderTask* task = this->getLastRenderTask(proxy);
    return task ? task->asOpsTask() : nullptr;
}


void GrDrawingManager::moveRenderTasksToDDL(SkDeferredDisplayList* ddl) {
    SkDEBUGCODE(this->validate());

    // no renderTask should receive a new command after this
    fDAG.closeAll(fContext->priv().caps());
    fActiveOpsTask = nullptr;

    fDAG.swap(&ddl->fRenderTasks);
    SkASSERT(!fDAG.numRenderTasks());

    for (auto& renderTask : ddl->fRenderTasks) {
        renderTask->disown(this);
        renderTask->prePrepare(fContext);
    }

    ddl->fArenas = std::move(fContext->priv().detachArenas());

    fContext->priv().detachProgramData(&ddl->fProgramData);

    if (fPathRendererChain) {
        if (auto ccpr = fPathRendererChain->getCoverageCountingPathRenderer()) {
            ddl->fPendingPaths = ccpr->detachPendingPaths();
        }
    }

    SkDEBUGCODE(this->validate());
}

void GrDrawingManager::copyRenderTasksFromDDL(sk_sp<const SkDeferredDisplayList> ddl,
                                              GrRenderTargetProxy* newDest) {
    SkDEBUGCODE(this->validate());

    if (fActiveOpsTask) {
        // This is  a temporary fix for the partial-MDB world. In that world we're not
        // reordering so ops that (in the single opsTask world) would've just glommed onto the
        // end of the single opsTask but referred to a far earlier RT need to appear in their
        // own opsTask.
        fActiveOpsTask->makeClosed(*fContext->priv().caps());
        fActiveOpsTask = nullptr;
    }

    // Propagate the DDL proxy's state information to the replaying DDL.
    if (ddl->priv().targetProxy()->isMSAADirty()) {
        newDest->markMSAADirty(ddl->priv().targetProxy()->msaaDirtyRect(),
                               ddl->characterization().origin());
    }
    GrTextureProxy* newTextureProxy = newDest->asTextureProxy();
    if (newTextureProxy && GrMipmapped::kYes == newTextureProxy->mipmapped()) {
        newTextureProxy->markMipmapsDirty();
    }

    this->addDDLTarget(newDest, ddl->priv().targetProxy());

    // Here we jam the proxy that backs the current replay SkSurface into the LazyProxyData.
    // The lazy proxy that references it (in the copied opsTasks) will steal its GrTexture.
    ddl->fLazyProxyData->fReplayDest = newDest;

    if (ddl->fPendingPaths.size()) {
        GrCoverageCountingPathRenderer* ccpr = this->getCoverageCountingPathRenderer();

        ccpr->mergePendingPaths(ddl->fPendingPaths);
    }

    fDAG.add(ddl->fRenderTasks);

    // Add a task to unref the DDL after flush.
    GrRenderTask* unrefTask = fDAG.add(sk_make_sp<GrUnrefDDLTask>(std::move(ddl)));
    unrefTask->makeClosed(*fContext->priv().caps());

    SkDEBUGCODE(this->validate());
}

#ifdef SK_DEBUG
void GrDrawingManager::validate() const {
    if (fDAG.sortingRenderTasks() && fReduceOpsTaskSplitting) {
        SkASSERT(!fActiveOpsTask);
    } else {
        if (fActiveOpsTask) {
            SkASSERT(!fDAG.empty());
            SkASSERT(!fActiveOpsTask->isClosed());
            SkASSERT(fActiveOpsTask == fDAG.back());
        }

        for (int i = 0; i < fDAG.numRenderTasks(); ++i) {
            if (fActiveOpsTask != fDAG.renderTask(i)) {
                // The resolveTask associated with the activeTask remains open for as long as the
                // activeTask does.
                bool isActiveResolveTask =
                        fActiveOpsTask && fActiveOpsTask->fTextureResolveTask == fDAG.renderTask(i);
                SkASSERT(isActiveResolveTask || fDAG.renderTask(i)->isClosed());
            }
        }

        if (!fDAG.empty() && !fDAG.back()->isClosed()) {
            SkASSERT(fActiveOpsTask == fDAG.back());
        }
    }
}
#endif

void GrDrawingManager::closeRenderTasksForNewRenderTask(GrSurfaceProxy* target) {
    if (target && fDAG.sortingRenderTasks() && fReduceOpsTaskSplitting) {
        // In this case we need to close all the renderTasks that rely on the current contents of
        // 'target'. That is bc we're going to update the content of the proxy so they need to be
        // split in case they use both the old and new content. (This is a bit of an overkill: they
        // really only need to be split if they ever reference proxy's contents again but that is
        // hard to predict/handle).
        if (GrRenderTask* lastRenderTask = this->getLastRenderTask(target)) {
            lastRenderTask->closeThoseWhoDependOnMe(*fContext->priv().caps());
        }
    } else if (fActiveOpsTask) {
        // This is  a temporary fix for the partial-MDB world. In that world we're not
        // reordering so ops that (in the single opsTask world) would've just glommed onto the
        // end of the single opsTask but referred to a far earlier RT need to appear in their
        // own opsTask.
        fActiveOpsTask->makeClosed(*fContext->priv().caps());
        fActiveOpsTask = nullptr;
    }
}

sk_sp<GrOpsTask> GrDrawingManager::newOpsTask(GrSurfaceProxyView surfaceView,
                                              bool managedOpsTask) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);

    GrSurfaceProxy* proxy = surfaceView.proxy();
    this->closeRenderTasksForNewRenderTask(proxy);

    sk_sp<GrOpsTask> opsTask(new GrOpsTask(this, fContext->priv().arenas(),
                                           std::move(surfaceView),
                                           fContext->priv().auditTrail()));
    SkASSERT(this->getLastRenderTask(proxy) == opsTask.get());

    if (managedOpsTask) {
        fDAG.add(opsTask);

        if (!fDAG.sortingRenderTasks() || !fReduceOpsTaskSplitting) {
            fActiveOpsTask = opsTask.get();
        }
    }

    SkDEBUGCODE(this->validate());
    return opsTask;
}

GrTextureResolveRenderTask* GrDrawingManager::newTextureResolveRenderTask(const GrCaps& caps) {
    // Unlike in the "new opsTask" case, we do not want to close the active opsTask, nor (if we are
    // in sorting and opsTask reduction mode) the render tasks that depend on any proxy's current
    // state. This is because those opsTasks can still receive new ops and because if they refer to
    // the mipmapped version of 'proxy', they will then come to depend on the render task being
    // created here.
    //
    // Add the new textureResolveTask before the fActiveOpsTask (if not in
    // sorting/opsTask-splitting-reduction mode) because it will depend upon this resolve task.
    // NOTE: Putting it here will also reduce the amount of work required by the topological sort.
    return static_cast<GrTextureResolveRenderTask*>(fDAG.addBeforeLast(
            sk_make_sp<GrTextureResolveRenderTask>()));
}

void GrDrawingManager::newWaitRenderTask(sk_sp<GrSurfaceProxy> proxy,
                                         std::unique_ptr<std::unique_ptr<GrSemaphore>[]> semaphores,
                                         int numSemaphores) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);

    const GrCaps& caps = *fContext->priv().caps();

    sk_sp<GrWaitRenderTask> waitTask = sk_make_sp<GrWaitRenderTask>(GrSurfaceProxyView(proxy),
                                                                    std::move(semaphores),
                                                                    numSemaphores);
    if (fReduceOpsTaskSplitting) {
        GrRenderTask* lastTask = this->getLastRenderTask(proxy.get());
        if (lastTask && !lastTask->isClosed()) {
            // We directly make the currently open renderTask depend on waitTask instead of using
            // the proxy version of addDependency. The waitTask will never need to trigger any
            // resolves or mip map generation which is the main advantage of going through the proxy
            // version. Additionally we would've had to temporarily set the wait task as the
            // lastRenderTask on the proxy, add the dependency, and then reset the lastRenderTask to
            // lastTask. Additionally we add all dependencies of lastTask to waitTask so that the
            // waitTask doesn't get reordered before them and unnecessarily block those tasks.
            // Note: Any previous Ops already in lastTask will get blocked by the wait semaphore
            // even though they don't need to be for correctness.

            // Make sure we add the dependencies of lastTask to waitTask first or else we'll get a
            // circular self dependency of waitTask on waitTask.
            waitTask->addDependenciesFromOtherTask(lastTask);
            lastTask->addDependency(waitTask.get());
        } else {
            // If there is a last task we set the waitTask to depend on it so that it doesn't get
            // reordered in front of the lastTask causing the lastTask to be blocked by the
            // semaphore. Again we directly just go through adding the dependency to the task and
            // not the proxy since we don't need to worry about resolving anything.
            if (lastTask) {
                waitTask->addDependency(lastTask);
            }
            this->setLastRenderTask(proxy.get(), waitTask.get());
        }
        fDAG.add(waitTask);
    } else {
        if (fActiveOpsTask && (fActiveOpsTask->target(0).proxy() == proxy.get())) {
            SkASSERT(this->getLastRenderTask(proxy.get()) == fActiveOpsTask);
            fDAG.addBeforeLast(waitTask);
            // In this case we keep the current renderTask open but just insert the new waitTask
            // before it in the list. The waitTask will never need to trigger any resolves or mip
            // map generation which is the main advantage of going through the proxy version.
            // Additionally we would've had to temporarily set the wait task as the lastRenderTask
            // on the proxy, add the dependency, and then reset the lastRenderTask to
            // fActiveOpsTask. Additionally we make the waitTask depend on all of fActiveOpsTask
            // dependencies so that we don't unnecessarily reorder the waitTask before them.
            // Note: Any previous Ops already in fActiveOpsTask will get blocked by the wait
            // semaphore even though they don't need to be for correctness.

            // Make sure we add the dependencies of fActiveOpsTask to waitTask first or else we'll
            // get a circular self dependency of waitTask on waitTask.
            waitTask->addDependenciesFromOtherTask(fActiveOpsTask);
            fActiveOpsTask->addDependency(waitTask.get());
        } else {
            // In this case we just close the previous RenderTask and start and append the waitTask
            // to the DAG. Since it is the last task now we call setLastRenderTask on the proxy. If
            // there is a lastTask on the proxy we make waitTask depend on that task. This
            // dependency isn't strictly needed but it does keep the DAG from reordering the
            // waitTask earlier and blocking more tasks.
            if (GrRenderTask* lastTask = this->getLastRenderTask(proxy.get())) {
                waitTask->addDependency(lastTask);
            }
            this->setLastRenderTask(proxy.get(), waitTask.get());
            this->closeRenderTasksForNewRenderTask(proxy.get());
            fDAG.add(waitTask);
        }
    }
    waitTask->makeClosed(caps);

    SkDEBUGCODE(this->validate());
}

void GrDrawingManager::newTransferFromRenderTask(sk_sp<GrSurfaceProxy> srcProxy,
                                                 const SkIRect& srcRect,
                                                 GrColorType surfaceColorType,
                                                 GrColorType dstColorType,
                                                 sk_sp<GrGpuBuffer> dstBuffer,
                                                 size_t dstOffset) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);
    // This copies from srcProxy to dstBuffer so it doesn't have a real target.
    this->closeRenderTasksForNewRenderTask(nullptr);

    GrRenderTask* task = fDAG.add(sk_make_sp<GrTransferFromRenderTask>(
            srcProxy, srcRect, surfaceColorType, dstColorType,
            std::move(dstBuffer), dstOffset));

    const GrCaps& caps = *fContext->priv().caps();

    // We always say GrMipmapped::kNo here since we are always just copying from the base layer. We
    // don't need to make sure the whole mip map chain is valid.
    task->addDependency(this, srcProxy.get(), GrMipmapped::kNo,
                        GrTextureResolveManager(this), caps);
    task->makeClosed(caps);

    // We have closed the previous active oplist but since a new oplist isn't being added there
    // shouldn't be an active one.
    SkASSERT(!fActiveOpsTask);
    SkDEBUGCODE(this->validate());
}

bool GrDrawingManager::newCopyRenderTask(GrSurfaceProxyView srcView,
                                         const SkIRect& srcRect,
                                         GrSurfaceProxyView dstView,
                                         const SkIPoint& dstPoint) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);

    this->closeRenderTasksForNewRenderTask(dstView.proxy());
    const GrCaps& caps = *fContext->priv().caps();

    GrSurfaceProxy* srcProxy = srcView.proxy();

    GrRenderTask* task =
            fDAG.add(GrCopyRenderTask::Make(this, std::move(srcView), srcRect, std::move(dstView),
                                            dstPoint, &caps));
    if (!task) {
        return false;
    }

    // We always say GrMipmapped::kNo here since we are always just copying from the base layer to
    // another base layer. We don't need to make sure the whole mip map chain is valid.
    task->addDependency(this, srcProxy, GrMipmapped::kNo, GrTextureResolveManager(this), caps);
    task->makeClosed(caps);

    // We have closed the previous active oplist but since a new oplist isn't being added there
    // shouldn't be an active one.
    SkASSERT(!fActiveOpsTask);
    SkDEBUGCODE(this->validate());
    return true;
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
        fPathRendererChain =
                std::make_unique<GrPathRendererChain>(fContext, fOptionsForPathRendererChain);
    }

    GrPathRenderer* pr = fPathRendererChain->getPathRenderer(args, drawType, stencilSupport);
    if (!pr && allowSW) {
        auto swPR = this->getSoftwarePathRenderer();
        if (GrPathRenderer::CanDrawPath::kNo != swPR->canDrawPath(args)) {
            pr = swPR;
        }
    }

#if GR_PATH_RENDERER_SPEW
    if (pr) {
        SkDebugf("getPathRenderer: %s\n", pr->name());
    }
#endif

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
        fPathRendererChain = std::make_unique<GrPathRendererChain>(fContext, fOptionsForPathRendererChain);
    }
    return fPathRendererChain->getCoverageCountingPathRenderer();
}

void GrDrawingManager::flushIfNecessary() {
    auto direct = fContext->asDirectContext();
    if (!direct) {
        return;
    }

    auto resourceCache = direct->priv().getResourceCache();
    if (resourceCache && resourceCache->requestsFlush()) {
        if (this->flush(nullptr, 0, SkSurface::BackendSurfaceAccess::kNoAccess, GrFlushInfo(),
                        nullptr)) {
            this->submitToGpu(false);
        }
        resourceCache->purgeAsNeeded();
    }
}

