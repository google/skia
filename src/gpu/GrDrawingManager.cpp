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
#include "src/core/SkTInternalLList.h"
#include "src/gpu/GrClientMappedBufferManager.h"
#include "src/gpu/GrCopyRenderTask.h"
#include "src/gpu/GrDDLTask.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrOnFlushResourceProvider.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetProxy.h"
#include "src/gpu/GrRenderTask.h"
#include "src/gpu/GrRenderTaskCluster.h"
#include "src/gpu/GrResourceAllocator.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrTTopoSort.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrTextureProxyPriv.h"
#include "src/gpu/GrTextureResolveRenderTask.h"
#include "src/gpu/GrTracing.h"
#include "src/gpu/GrTransferFromRenderTask.h"
#include "src/gpu/GrWaitRenderTask.h"
#include "src/gpu/GrWritePixelsRenderTask.h"
#include "src/gpu/text/GrSDFTControl.h"
#include "src/image/SkSurface_Gpu.h"

#if SK_GPU_V1
#include "src/gpu/ops/OpsTask.h"
#include "src/gpu/ops/SoftwarePathRenderer.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
#if SK_GPU_V1
GrDrawingManager::GrDrawingManager(GrRecordingContext* rContext,
                                   const PathRendererChain::Options& optionsForPathRendererChain,
                                   bool reduceOpsTaskSplitting)
        : fContext(rContext)
        , fOptionsForPathRendererChain(optionsForPathRendererChain)
        , fPathRendererChain(nullptr)
        , fSoftwarePathRenderer(nullptr)
        , fReduceOpsTaskSplitting(reduceOpsTaskSplitting) {
}

#else

GrDrawingManager::GrDrawingManager(GrRecordingContext* rContext, bool reduceOpsTaskSplitting)
        : fContext(rContext)
        , fReduceOpsTaskSplitting(reduceOpsTaskSplitting) {
}

#endif

GrDrawingManager::~GrDrawingManager() {
    this->closeAllTasks();
    this->removeRenderTasks();
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

#if SK_GPU_V1
    // a path renderer may be holding onto resources
    fPathRendererChain = nullptr;
    fSoftwarePathRenderer = nullptr;
#endif
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
            return !used;
        });
        if (allUnused) {
            if (info.fSubmittedProc) {
                info.fSubmittedProc(info.fSubmittedContext, true);
            }
            return false;
        }
    }

    auto dContext = fContext->asDirectContext();
    SkASSERT(dContext);
    dContext->priv().clientMappedBufferManager()->process();

    GrGpu* gpu = dContext->priv().getGpu();
    // We have a non abandoned and direct GrContext. It must have a GrGpu.
    SkASSERT(gpu);

    fFlushing = true;

    auto resourceProvider = dContext->priv().resourceProvider();
    auto resourceCache = dContext->priv().getResourceCache();

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
            onFlushCBObject->preFlush(&onFlushProvider, SkMakeSpan(fFlushingRenderTaskIDs));
        }
        for (const auto& onFlushRenderTask : fOnFlushRenderTasks) {
            onFlushRenderTask->makeClosed(fContext);
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

    bool usingReorderedDAG = false;
    GrResourceAllocator resourceAllocator(dContext);
    if (fReduceOpsTaskSplitting) {
        usingReorderedDAG = this->reorderTasks(&resourceAllocator);
        if (!usingReorderedDAG) {
            resourceAllocator.reset();
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

    if (!resourceAllocator.failedInstantiation()) {
        if (!usingReorderedDAG) {
            for (const auto& task : fDAG) {
                SkASSERT(task);
                task->gatherProxyIntervals(&resourceAllocator);
            }
            resourceAllocator.planAssignment();
        }
        resourceAllocator.assign();
    }
    bool flushed = !resourceAllocator.failedInstantiation() &&
                    this->executeRenderTasks(&flushState);
    this->removeRenderTasks();

    gpu->executeFlushInfo(proxies, access, info, newState);

    // Give the cache a chance to purge resources that become purgeable due to flushing.
    if (flushed) {
        resourceCache->purgeAsNeeded();
        flushed = false;
    }
    for (GrOnFlushCallbackObject* onFlushCBObject : fOnFlushCBObjects) {
        onFlushCBObject->postFlush(fTokenTracker.nextTokenToFlush(),
                                   SkMakeSpan(fFlushingRenderTaskIDs));
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

bool GrDrawingManager::executeRenderTasks(GrOpFlushState* flushState) {
#if GR_FLUSH_TIME_OP_SPEW
    SkDebugf("Flushing %d opsTasks\n", fDAG.count());
    for (int i = 0; i < fDAG.count(); ++i) {
        if (fDAG[i]) {
            SkString label;
            label.printf("task %d/%d", i, fDAG.count());
            fDAG[i]->dump(label, {}, true, true);
        }
    }
#endif

    bool anyRenderTasksExecuted = false;

    for (const auto& renderTask : fDAG) {
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
    int numRenderTasksExecuted = 0;

    // Execute the onFlush renderTasks first, if any.
    for (sk_sp<GrRenderTask>& onFlushRenderTask : fOnFlushRenderTasks) {
        if (!onFlushRenderTask->execute(flushState)) {
            SkDebugf("WARNING: onFlushRenderTask failed to execute.\n");
        }
        SkASSERT(onFlushRenderTask->unique());
        onFlushRenderTask->disown(this);
        onFlushRenderTask = nullptr;
        if (++numRenderTasksExecuted >= kMaxRenderTasksBeforeFlush) {
            flushState->gpu()->submitToGpu(false);
            numRenderTasksExecuted = 0;
        }
    }
    fOnFlushRenderTasks.reset();

    // Execute the normal op lists.
    for (const auto& renderTask : fDAG) {
        SkASSERT(renderTask);
        if (!renderTask->isInstantiated()) {
            continue;
        }

        if (renderTask->execute(flushState)) {
            anyRenderTasksExecuted = true;
        }
        if (++numRenderTasksExecuted >= kMaxRenderTasksBeforeFlush) {
            flushState->gpu()->submitToGpu(false);
            numRenderTasksExecuted = 0;
        }
    }

    SkASSERT(!flushState->opsRenderPass());
    SkASSERT(fTokenTracker.nextDrawToken() == fTokenTracker.nextTokenToFlush());

    // We reset the flush state before the RenderTasks so that the last resources to be freed are
    // those that are written to in the RenderTasks. This helps to make sure the most recently used
    // resources are the last to be purged by the resource cache.
    flushState->reset();

    return anyRenderTasksExecuted;
}

void GrDrawingManager::removeRenderTasks() {
    for (const auto& task : fDAG) {
        SkASSERT(task);
        if (!task->unique() || task->requiresExplicitCleanup()) {
            // TODO: Eventually uniqueness should be guaranteed: http://skbug.com/7111.
            // DDLs, however, will always require an explicit notification for when they
            // can clean up resources.
            task->endFlush(this);
        }
        task->disown(this);
    }
    fDAG.reset();
    fLastRenderTasks.reset();
    for (const sk_sp<GrRenderTask>& onFlushRenderTask : fOnFlushRenderTasks) {
        onFlushRenderTask->disown(this);
    }
    fOnFlushRenderTasks.reset();
}

void GrDrawingManager::sortTasks() {
    if (!GrTTopoSort<GrRenderTask, GrRenderTask::TopoSortTraits>(&fDAG)) {
        SkDEBUGFAIL("Render task topo sort failed.");
        return;
    }

#if SK_GPU_V1 && defined(SK_DEBUG)
    // This block checks for any unnecessary splits in the opsTasks. If two sequential opsTasks
    // could have merged it means the opsTask was artificially split.
    if (!fDAG.empty()) {
        auto prevOpsTask = fDAG[0]->asOpsTask();
        for (int i = 1; i < fDAG.count(); ++i) {
            auto curOpsTask = fDAG[i]->asOpsTask();

            if (prevOpsTask && curOpsTask) {
                SkASSERT(!prevOpsTask->canMerge(curOpsTask));
            }

            prevOpsTask = curOpsTask;
        }
    }
#endif
}

// Reorder the array to match the llist without reffing & unreffing sk_sp's.
// Both args must contain the same objects.
// This is basically a shim because clustering uses LList but the rest of drawmgr uses array.
template <typename T>
static void reorder_array_by_llist(const SkTInternalLList<T>& llist, SkTArray<sk_sp<T>>* array) {
    int i = 0;
    for (T* t : llist) {
        // Release the pointer that used to live here so it doesn't get unreffed.
        [[maybe_unused]] T* old = array->at(i).release();
        array->at(i++).reset(t);
    }
    SkASSERT(i == array->count());
}

bool GrDrawingManager::reorderTasks(GrResourceAllocator* resourceAllocator) {
    SkASSERT(fReduceOpsTaskSplitting);
    SkTInternalLList<GrRenderTask> llist;
    bool clustered = GrClusterRenderTasks(SkMakeSpan(fDAG), &llist);
    if (!clustered) {
        return false;
    }

    for (GrRenderTask* task : llist) {
        task->gatherProxyIntervals(resourceAllocator);
    }
    if (!resourceAllocator->planAssignment()) {
        return false;
    }
    if (!resourceAllocator->makeBudgetHeadroom()) {
        auto dContext = fContext->asDirectContext();
        SkASSERT(dContext);
        dContext->priv().getGpu()->stats()->incNumReorderedDAGsOverBudget();
        return false;
    }
    reorder_array_by_llist(llist, &fDAG);

    int newCount = 0;
    for (int i = 0; i < fDAG.count(); i++) {
        sk_sp<GrRenderTask>& task = fDAG[i];
#if SK_GPU_V1
        if (auto opsTask = task->asOpsTask()) {
            size_t remaining = fDAG.size() - i - 1;
            SkSpan<sk_sp<GrRenderTask>> nextTasks{fDAG.end() - remaining, remaining};
            int removeCount = opsTask->mergeFrom(nextTasks);
            for (const auto& removed : nextTasks.first(removeCount)) {
                removed->disown(this);
            }
            i += removeCount;
        }
#endif
        fDAG[newCount++] = std::move(task);
    }
    fDAG.resize_back(newCount);
    return true;
}

void GrDrawingManager::closeAllTasks() {
    for (auto& task : fDAG) {
        if (task) {
            task->makeClosed(fContext);
        }
    }
}

GrRenderTask* GrDrawingManager::insertTaskBeforeLast(sk_sp<GrRenderTask> task) {
    if (!task) {
        return nullptr;
    }
    if (fDAG.empty()) {
        return fDAG.push_back(std::move(task)).get();
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

skgpu::v1::OpsTask* GrDrawingManager::getLastOpsTask(const GrSurfaceProxy* proxy) const {
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

    SkDEBUGCODE(this->validate());
}

void GrDrawingManager::createDDLTask(sk_sp<const SkDeferredDisplayList> ddl,
                                     sk_sp<GrRenderTargetProxy> newDest,
                                     SkIPoint offset) {
    SkDEBUGCODE(this->validate());

#if SK_GPU_V1
    if (fActiveOpsTask) {
        // This is a temporary fix for the partial-MDB world. In that world we're not
        // reordering so ops that (in the single opsTask world) would've just glommed onto the
        // end of the single opsTask but referred to a far earlier RT need to appear in their
        // own opsTask.
        fActiveOpsTask->makeClosed(fContext);
        fActiveOpsTask = nullptr;
    }
#endif

    // Propagate the DDL proxy's state information to the replay target.
    if (ddl->priv().targetProxy()->isMSAADirty()) {
        auto nativeRect = GrNativeRect::MakeIRectRelativeTo(
                ddl->characterization().origin(),
                ddl->priv().targetProxy()->backingStoreDimensions().height(),
                ddl->priv().targetProxy()->msaaDirtyRect());
        newDest->markMSAADirty(nativeRect);
    }
    GrTextureProxy* newTextureProxy = newDest->asTextureProxy();
    if (newTextureProxy && GrMipmapped::kYes == newTextureProxy->mipmapped()) {
        newTextureProxy->markMipmapsDirty();
    }

    // Here we jam the proxy that backs the current replay SkSurface into the LazyProxyData.
    // The lazy proxy that references it (in the DDL opsTasks) will then steal its GrTexture.
    ddl->fLazyProxyData->fReplayDest = newDest.get();

    // Add a task to handle drawing and lifetime management of the DDL.
    SkDEBUGCODE(auto ddlTask =) this->appendTask(sk_make_sp<GrDDLTask>(this,
                                                                       std::move(newDest),
                                                                       std::move(ddl),
                                                                       offset));
    SkASSERT(ddlTask->isClosed());

    SkDEBUGCODE(this->validate());
}

#ifdef SK_DEBUG
void GrDrawingManager::validate() const {
#if SK_GPU_V1
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
            bool isAtlas = fDAG[i]->isSetFlag(GrRenderTask::kAtlas_Flag);
            SkASSERT(isActiveResolveTask || isAtlas || fDAG[i]->isClosed());
        }
    }

    // The active opsTask, if any, should always be at the back of the DAG.
    if (!fDAG.empty()) {
        if (fDAG.back()->isSetFlag(GrRenderTask::kAtlas_Flag)) {
            SkASSERT(fActiveOpsTask == nullptr);
            SkASSERT(!fDAG.back()->isClosed());
        } else if (fDAG.back()->isClosed()) {
            SkASSERT(fActiveOpsTask == nullptr);
        } else {
            SkASSERT(fActiveOpsTask == fDAG.back().get());
        }
    } else {
        SkASSERT(fActiveOpsTask == nullptr);
    }
#endif // SK_GPU_V1
}
#endif // SK_DEBUG

void GrDrawingManager::closeActiveOpsTask() {
#if SK_GPU_V1
    if (fActiveOpsTask) {
        // This is a temporary fix for the partial-MDB world. In that world we're not
        // reordering so ops that (in the single opsTask world) would've just glommed onto the
        // end of the single opsTask but referred to a far earlier RT need to appear in their
        // own opsTask.
        fActiveOpsTask->makeClosed(fContext);
        fActiveOpsTask = nullptr;
    }
#endif
}

#if SK_GPU_V1
sk_sp<skgpu::v1::OpsTask> GrDrawingManager::newOpsTask(GrSurfaceProxyView surfaceView,
                                                       sk_sp<GrArenas> arenas,
                                                       bool flushTimeOpsTask) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);

    this->closeActiveOpsTask();

    sk_sp<skgpu::v1::OpsTask> opsTask(new skgpu::v1::OpsTask(this,
                                                             std::move(surfaceView),
                                                             fContext->priv().auditTrail(),
                                                             std::move(arenas)));

    SkASSERT(this->getLastRenderTask(opsTask->target(0)) == opsTask.get());

    if (flushTimeOpsTask) {
        fOnFlushRenderTasks.push_back(opsTask);
    } else {
        this->appendTask(opsTask);

        fActiveOpsTask = opsTask.get();
    }

    SkDEBUGCODE(this->validate());
    return opsTask;
}

void GrDrawingManager::addAtlasTask(sk_sp<GrRenderTask> atlasTask,
                                    GrRenderTask* previousAtlasTask) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);

    if (previousAtlasTask) {
        previousAtlasTask->makeClosed(fContext);
        for (GrRenderTask* previousAtlasUser : previousAtlasTask->dependents()) {
            // Make the new atlas depend on everybody who used the old atlas, and close their tasks.
            // This guarantees that the previous atlas is totally out of service before we render
            // the next one, meaning there is only ever one atlas active at a time and that they can
            // all share the same texture.
            atlasTask->addDependency(previousAtlasUser);
            previousAtlasUser->makeClosed(fContext);
            if (previousAtlasUser == fActiveOpsTask) {
                fActiveOpsTask = nullptr;
            }
        }
    }

    atlasTask->setFlag(GrRenderTask::kAtlas_Flag);
    this->insertTaskBeforeLast(std::move(atlasTask));

    SkDEBUGCODE(this->validate());
}
#endif // SK_GPU_V1

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

    sk_sp<GrWaitRenderTask> waitTask = sk_make_sp<GrWaitRenderTask>(GrSurfaceProxyView(proxy),
                                                                    std::move(semaphores),
                                                                    numSemaphores);

#if SK_GPU_V1
    if (fActiveOpsTask && (fActiveOpsTask->target(0) == proxy.get())) {
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
    } else
#endif
    {
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
    waitTask->makeClosed(fContext);

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
    task->makeClosed(fContext);

    // We have closed the previous active oplist but since a new oplist isn't being added there
    // shouldn't be an active one.
    SkASSERT(!fActiveOpsTask);
    SkDEBUGCODE(this->validate());
}

sk_sp<GrRenderTask> GrDrawingManager::newCopyRenderTask(sk_sp<GrSurfaceProxy> src,
                                                        SkIRect srcRect,
                                                        sk_sp<GrSurfaceProxy> dst,
                                                        SkIPoint dstPoint,
                                                        GrSurfaceOrigin origin) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);

    // It'd be nicer to check this in GrCopyRenderTask::Make. This gets complicated because of
    // "active ops task" tracking. dst will be the target of our copy task but it might also be the
    // target of the active ops task. We currently require the active ops task to be closed before
    // making a new task that targets the same proxy. However, if we first close the active ops
    // task, then fail to make a copy task, the next active ops task may target the same proxy. This
    // will trip an assert related to unnecessary ops task splitting.
    if (src->framebufferOnly()) {
        return nullptr;
    }

    this->closeActiveOpsTask();

    sk_sp<GrRenderTask> task = GrCopyRenderTask::Make(this,
                                                      src,
                                                      srcRect,
                                                      std::move(dst),
                                                      dstPoint,
                                                      origin);
    if (!task) {
        return nullptr;
    }

    this->appendTask(task);

    const GrCaps& caps = *fContext->priv().caps();
    // We always say GrMipmapped::kNo here since we are always just copying from the base layer to
    // another base layer. We don't need to make sure the whole mip map chain is valid.
    task->addDependency(this, src.get(), GrMipmapped::kNo, GrTextureResolveManager(this), caps);
    task->makeClosed(fContext);

    // We have closed the previous active oplist but since a new oplist isn't being added there
    // shouldn't be an active one.
    SkASSERT(!fActiveOpsTask);
    SkDEBUGCODE(this->validate());
    return task;
}

bool GrDrawingManager::newWritePixelsTask(sk_sp<GrSurfaceProxy> dst,
                                          SkIRect rect,
                                          GrColorType srcColorType,
                                          GrColorType dstColorType,
                                          const GrMipLevel levels[],
                                          int levelCount) {
    SkDEBUGCODE(this->validate());
    SkASSERT(fContext);

    this->closeActiveOpsTask();
    const GrCaps& caps = *fContext->priv().caps();

    // On platforms that prefer flushes over VRAM use (i.e., ANGLE) we're better off forcing a
    // complete flush here.
    if (!caps.preferVRAMUseOverFlushes()) {
        this->flushSurfaces(SkSpan<GrSurfaceProxy*>{},
                            SkSurface::BackendSurfaceAccess::kNoAccess,
                            GrFlushInfo{},
                            nullptr);
    }

    GrRenderTask* task = this->appendTask(GrWritePixelsTask::Make(this,
                                                                  std::move(dst),
                                                                  rect,
                                                                  srcColorType,
                                                                  dstColorType,
                                                                  levels,
                                                                  levelCount));
    if (!task) {
        return false;
    }

    task->makeClosed(fContext);

    // We have closed the previous active oplist but since a new oplist isn't being added there
    // shouldn't be an active one.
    SkASSERT(!fActiveOpsTask);
    SkDEBUGCODE(this->validate());
    return true;
}

#if SK_GPU_V1
/*
 * This method finds a path renderer that can draw the specified path on
 * the provided target.
 * Due to its expense, the software path renderer has split out so it can
 * can be individually allowed/disallowed via the "allowSW" boolean.
 */
skgpu::v1::PathRenderer* GrDrawingManager::getPathRenderer(
        const PathRenderer::CanDrawPathArgs& args,
        bool allowSW,
        PathRendererChain::DrawType drawType,
        PathRenderer::StencilSupport* stencilSupport) {

    if (!fPathRendererChain) {
        fPathRendererChain =
                std::make_unique<PathRendererChain>(fContext, fOptionsForPathRendererChain);
    }

    auto pr = fPathRendererChain->getPathRenderer(args, drawType, stencilSupport);
    if (!pr && allowSW) {
        auto swPR = this->getSoftwarePathRenderer();
        if (PathRenderer::CanDrawPath::kNo != swPR->canDrawPath(args)) {
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

skgpu::v1::PathRenderer* GrDrawingManager::getSoftwarePathRenderer() {
    if (!fSoftwarePathRenderer) {
        fSoftwarePathRenderer.reset(new skgpu::v1::SoftwarePathRenderer(
            fContext->priv().proxyProvider(), fOptionsForPathRendererChain.fAllowPathMaskCaching));
    }
    return fSoftwarePathRenderer.get();
}

skgpu::v1::AtlasPathRenderer* GrDrawingManager::getAtlasPathRenderer() {
    if (!fPathRendererChain) {
        fPathRendererChain = std::make_unique<PathRendererChain>(fContext,
                                                                 fOptionsForPathRendererChain);
    }
    return fPathRendererChain->getAtlasPathRenderer();
}

skgpu::v1::PathRenderer* GrDrawingManager::getTessellationPathRenderer() {
    if (!fPathRendererChain) {
        fPathRendererChain = std::make_unique<PathRendererChain>(fContext,
                                                                 fOptionsForPathRendererChain);
    }
    return fPathRendererChain->getTessellationPathRenderer();
}

#endif // SK_GPU_V1

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
