/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrDrawingManager.h"

#include <algorithm>
#include <memory>

#include "include/core/SkDeferredDisplayList.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkDeferredDisplayListPriv.h"
#include "src/gpu/GrAuditTrail.h"
#include "src/gpu/GrClientMappedBufferManager.h"
#include "src/gpu/GrCopyRenderTask.h"
#include "src/gpu/GrDDLTask.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrRenderTask.h"
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSoftwarePathRenderer.h"
#include "src/gpu/GrSurfaceContext.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTTopoSort.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/GrTextureResolveRenderTask.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/GrTransferFromRenderTask.h"
#include "src/gpu/GrWaitRenderTask.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/text/GrSDFTOptions.h"
#include "src/image/SkSurface_Gpu.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
GrDrawingManager::GrDrawingManager(GrRecordingContext* context,
                                   const GrPathRendererChain::Options& optionsForPathRendererChain,
                                   bool reduceOpsTaskSplitting)
        : fContext(context)
        , fOptionsForPathRendererChain(optionsForPathRendererChain)
        , fPathRendererChain(nullptr)
        , fSoftwarePathRenderer(nullptr)
        , fFlushing(false)
        , fReduceOpsTaskSplitting(reduceOpsTaskSplitting) { }

GrDrawingManager::~GrDrawingManager() {
    this->closeAllTasks();
    this->removeRenderTasks(0, fDAG.count());
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

// MDB TODO: make use of the 'proxies' parameter.
bool GrDrawingManager::flush(
        SkSpan<GrSurfaceProxy*> proxies,
        SkSurface::BackendSurfaceAccess access,
        const GrFlushInfo& info,
        const GrBackendSurfaceMutableState* newState) {
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

    // As of now we only short-circuit if we got an explicit list of surfaces to flush.
    if (!proxies.empty() && !info.fNumSemaphores && !info.fFinishedProc &&
        access == SkSurface::BackendSurfaceAccess::kNoAccess && !newState) {
        bool allUnused = std::all_of(proxies.begin(), proxies.end(), [&](GrSurfaceProxy* proxy) {
            bool used = std::any_of(fDAG.begin(), fDAG.end(), [&](auto& task) {
                return task && task->isUsed(proxy);
            });
            return !used && !this->isDDLTarget(proxy);
        });
        if (allUnused) {
            if (info.fSubmittedProc) {
                info.fSubmittedProc(info.fSubmittedContext, true);
            }
            return false;
        }
    }

    auto direct = fContext->asDirectContext();
    SkASSERT(direct);
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
    this->closeAllTasks();
    fActiveOpsTask = nullptr;

    this->sortTasks();
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
        fFlushingRenderTaskIDs.reserve_back(fDAG.count());
        for (const auto& task : fDAG) {
            if (task) {
                task->gatherIDs(&fFlushingRenderTaskIDs);
            }
        }

        for (GrOnFlushCallbackObject* onFlushCBObject : fOnFlushCBObjects) {
            onFlushCBObject->preFlush(&onFlushProvider, fFlushingRenderTaskIDs);
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
    SkDEBUGCODE(SkDebugf("onFlush renderTasks (%d):\n", fOnFlushRenderTasks.count()));
    for (const auto& onFlushRenderTask : fOnFlushRenderTasks) {
        SkDEBUGCODE(onFlushRenderTask->dump(/* printDependencies */ true);)
    }
    SkDEBUGCODE(SkDebugf("Normal renderTasks (%d):\n", fDAG.count()));
    for (const auto& task : fDAG) {
        SkDEBUGCODE(task->dump(/* printDependencies */ true);)
    }
#endif

    int startIndex, stopIndex;
    bool flushed = false;

    {
        GrResourceAllocator alloc(resourceProvider SkDEBUGCODE(, fDAG.count()));
        for (int i = 0; i < fDAG.count(); ++i) {
            if (fDAG[i]) {
                fDAG[i]->gatherProxyIntervals(&alloc);
            }
            alloc.markEndOfOpsTask(i);
        }
        alloc.determineRecyclability();

        GrResourceAllocator::AssignError error = GrResourceAllocator::AssignError::kNoError;
        int numRenderTasksExecuted = 0;
        while (alloc.assign(&startIndex, &stopIndex, &error)) {
            if (GrResourceAllocator::AssignError::kFailedProxyInstantiation == error) {
                for (int i = startIndex; i < stopIndex; ++i) {
                    GrRenderTask* renderTask = fDAG[i].get();
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
    for (const auto& task : fDAG) {
        // All render tasks should have been cleared out by now – we only reset the array below to
        // reclaim storage.
        SkASSERT(!task);
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
    GrMemoryPool* opMemoryPool = fContext->priv().opMemoryPool();
    opMemoryPool->isEmpty();
#endif

    gpu->executeFlushInfo(proxies, access, info, newState);

    // Give the cache a chance to purge resources that become purgeable due to flushing.
    if (flushed) {
        resourceCache->purgeAsNeeded();
        flushed = false;
    }
    for (GrOnFlushCallbackObject* onFlushCBObject : fOnFlushCBObjects) {
        onFlushCBObject->postFlush(fTokenTracker.nextTokenToFlush(), fFlushingRenderTaskIDs);
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
    SkASSERT(startIndex <= stopIndex && stopIndex <= fDAG.count());

#if GR_FLUSH_TIME_OP_SPEW
    SkDebugf("Flushing opsTask: %d to %d out of [%d, %d]\n",
                            startIndex, stopIndex, 0, fDAG.count());
    for (int i = startIndex; i < stopIndex; ++i) {
        if (fDAG[i]) {
            fDAG[i]->dump(true);
        }
    }
#endif

    bool anyRenderTasksExecuted = false;

    for (int i = startIndex; i < stopIndex; ++i) {
        GrRenderTask* renderTask = fDAG[i].get();
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
        GrRenderTask* renderTask = fDAG[i].get();
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
        GrRenderTask* task = fDAG[i].get();
        if (!task) {
            continue;
        }
        if (!task->unique() || task->requiresExplicitCleanup()) {
            // TODO: Eventually uniqueness should be guaranteed: http://skbug.com/7111.
            // DDLs, however, will always require an explicit notification for when they
            // can clean up resources.
            task->endFlush(this);
        }
        task->disown(this);

        // This doesn't cleanup any referring pointers (e.g. dependency pointers in the DAG).
        // It works right now bc this is only called after the topological sort is complete
        // (so the dangling pointers aren't used).
        fDAG[i] = nullptr;
    }
}

void GrDrawingManager::sortTasks() {
    if (!GrTTopoSort<GrRenderTask, GrRenderTask::TopoSortTraits>(&fDAG)) {
        SkDEBUGFAIL("Render task topo sort failed.");
        return;
    }

#ifdef SK_DEBUG
    // This block checks for any unnecessary splits in the opsTasks. If two sequential opsTasks
    // share the same backing GrSurfaceProxy it means the opsTask was artificially split.
    if (!fDAG.empty()) {
        GrOpsTask* prevOpsTask = fDAG[0]->asOpsTask();
        for (int i = 1; i < fDAG.count(); ++i) {
            GrOpsTask* curOpsTask = fDAG[i]->asOpsTask();

            if (prevOpsTask && curOpsTask) {
                SkASSERT(prevOpsTask->target(0).proxy() != curOpsTask->target(0).proxy());
            }

            prevOpsTask = curOpsTask;
        }
    }
#endif
}

void GrDrawingManager::closeAllTasks() {
    const GrCaps& caps = *fContext->priv().caps();
    for (auto& task : fDAG) {
        if (task) {
            task->makeClosed(caps);
        }
    }
}

GrRenderTask* GrDrawingManager::insertTaskBeforeLast(sk_sp<GrRenderTask> task) {
    SkASSERT(!fDAG.empty());
    if (!task) {
        return nullptr;
    }
    // Release 'fDAG.back()' and grab the raw pointer, in case the SkTArray grows
    // and reallocates during emplace_back.
    // TODO: Either use std::vector that can do this for us, or use SkSTArray to get the
    // perf win.
    fDAG.emplace_back(fDAG.back().release());
    return (fDAG[fDAG.count() - 2] = std::move(task)).get();
}

GrRenderTask* GrDrawingManager::appendTask(sk_sp<GrRenderTask> task) {
    if (!task) {
        return nullptr;
    }
    return fDAG.push_back(std::move(task)).get();
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
        SkSpan<GrSurfaceProxy*> proxies,
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

    auto direct = fContext->asDirectContext();
    SkASSERT(direct);
    GrGpu* gpu = direct->priv().getGpu();
    // We have a non abandoned and direct GrContext. It must have a GrGpu.
    SkASSERT(gpu);

    // TODO: It is important to upgrade the drawingmanager to just flushing the
    // portion of the DAG required by 'proxies' in order to restore some of the
    // semantics of this method.
    bool didFlush = this->flush(proxies, access, info, newState);
    for (GrSurfaceProxy* proxy : proxies) {
        resolve_and_mipmap(gpu, proxy);
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
    if (auto prior = this->getLastRenderTask(proxy)) {
        SkASSERT(prior->isClosed() || prior == task);
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
    this->closeAllTasks();
    fActiveOpsTask = nullptr;

    this->sortTasks();

    fDAG.swap(ddl->fRenderTasks);
    SkASSERT(fDAG.empty());

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

void GrDrawingManager::createDDLTask(sk_sp<const SkDeferredDisplayList> ddl,
                                     GrRenderTargetProxy* newDest,
                                     SkIPoint offset) {
    SkDEBUGCODE(this->validate());

    if (fActiveOpsTask) {
        // This is  a temporary fix for the partial-MDB world. In that world we're not
        // reordering so ops that (in the single opsTask world) would've just glommed onto the
        // end of the single opsTask but referred to a far earlier RT need to appear in their
        // own opsTask.
        fActiveOpsTask->makeClosed(*fContext->priv().caps());
        fActiveOpsTask = nullptr;
    }

    // Propagate the DDL proxy's state information to the replay target.
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
    // The lazy proxy that references it (in the DDL opsTasks) will then steal its GrTexture.
    ddl->fLazyProxyData->fReplayDest = newDest;

    if (ddl->fPendingPaths.size()) {
        GrCoverageCountingPathRenderer* ccpr = this->getCoverageCountingPathRenderer();

        ccpr->mergePendingPaths(ddl->fPendingPaths);
    }

    // Add a task to handle drawing and lifetime management of the DDL.
    SkDEBUGCODE(auto ddlTask =) this->appendTask(sk_make_sp<GrDDLTask>(this,
                                                                       sk_ref_sp(newDest),
                                                                       std::move(ddl),
                                                                       offset));
    SkASSERT(ddlTask->isClosed());

    SkDEBUGCODE(this->validate());
}

#ifdef SK_DEBUG
void GrDrawingManager::validate() const {
    if (fActiveOpsTask) {
        SkASSERT(!fDAG.empty());
        SkASSERT(!fActiveOpsTask->isClosed());
        SkASSERT(fActiveOpsTask == fDAG.back().get());
    }

    for (int i = 0; i < fDAG.count(); ++i) {
        if (fActiveOpsTask != fDAG[i].get()) {
            // The resolveTask associated with the activeTask remains open for as long as the
            // activeTask does.
            bool isActiveResolveTask =
                fActiveOpsTask && fActiveOpsTask->fTextureResolveTask == fDAG[i].get();
            SkASSERT(isActiveResolveTask || fDAG[i]->isClosed());
        }
    }

    if (!fDAG.empty() && !fDAG.back()->isClosed()) {
        SkASSERT(fActiveOpsTask == fDAG.back().get());
    }
}
#endif

void GrDrawingManager::closeActiveOpsTask() {
    if (fActiveOpsTask) {
        // This is a temporary fix for the partial-MDB world. In that world we're not
        // reordering so ops that (in the single opsTask world) would've just glommed onto the
        // end of the single opsTask but referred to a far earlier RT need to appear in their
        // own opsTask.
        fActiveOpsTask->makeClosed(*fContext->priv().caps());
        fActiveOpsTask = nullptr;
    }
}

sk_sp<GrOpsTask> GrDrawingManager::newOpsTask(GrSurfaceProxyView surfaceView,
                                              bool flushTimeOpsTask) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);

    this->closeActiveOpsTask();

    sk_sp<GrOpsTask> opsTask(new GrOpsTask(this, fContext->priv().arenas(),
                                           std::move(surfaceView),
                                           fContext->priv().auditTrail()));
    SkASSERT(this->getLastRenderTask(opsTask->target(0).proxy()) == opsTask.get());

    if (flushTimeOpsTask) {
        fOnFlushRenderTasks.push_back(opsTask);
    } else {
        this->appendTask(opsTask);

        fActiveOpsTask = opsTask.get();
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
    GrRenderTask* task = this->insertTaskBeforeLast(sk_make_sp<GrTextureResolveRenderTask>());
    return static_cast<GrTextureResolveRenderTask*>(task);
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

    if (fActiveOpsTask && (fActiveOpsTask->target(0).proxy() == proxy.get())) {
        SkASSERT(this->getLastRenderTask(proxy.get()) == fActiveOpsTask);
        this->insertTaskBeforeLast(waitTask);
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
        this->closeActiveOpsTask();
        this->appendTask(waitTask);
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
    this->closeActiveOpsTask();

    GrRenderTask* task = this->appendTask(sk_make_sp<GrTransferFromRenderTask>(
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

    this->closeActiveOpsTask();
    const GrCaps& caps = *fContext->priv().caps();

    GrSurfaceProxy* srcProxy = srcView.proxy();

    GrRenderTask* task =
            this->appendTask(GrCopyRenderTask::Make(this, std::move(srcView), srcRect,
                                                    std::move(dstView), dstPoint, &caps));
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
        fPathRendererChain = std::make_unique<GrPathRendererChain>(fContext,
                                                                   fOptionsForPathRendererChain);
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
        if (this->flush({}, SkSurface::BackendSurfaceAccess::kNoAccess, GrFlushInfo(), nullptr)) {
            this->submitToGpu(false);
        }
        resourceCache->purgeAsNeeded();
    }
}
