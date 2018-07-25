/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawingManager.h"
#include "GrBackendSemaphore.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrMemoryPool.h"
#include "GrOnFlushResourceProvider.h"
#include "GrOpList.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceAllocator.h"
#include "GrResourceProvider.h"
#include "GrSoftwarePathRenderer.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexture.h"
#include "GrTextureContext.h"
#include "GrTextureOpList.h"
#include "GrTexturePriv.h"
#include "GrTextureProxy.h"
#include "GrTextureProxyPriv.h"
#include "GrTracing.h"
#include "SkDeferredDisplayList.h"
#include "SkSurface_Gpu.h"
#include "SkTTopoSort.h"
#include "ccpr/GrCoverageCountingPathRenderer.h"
#include "text/GrTextContext.h"

GrDrawingManager::GrDrawingManager(GrContext* context,
                                   const GrPathRendererChain::Options& optionsForPathRendererChain,
                                   const GrTextContext::Options& optionsForTextContext,
                                   GrSingleOwner* singleOwner,
                                   bool explicitlyAllocating,
                                   GrContextOptions::Enable sortRenderTargets)
        : fContext(context)
        , fOptionsForPathRendererChain(optionsForPathRendererChain)
        , fOptionsForTextContext(optionsForTextContext)
        , fSingleOwner(singleOwner)
        , fAbandoned(false)
        , fTextContext(nullptr)
        , fPathRendererChain(nullptr)
        , fSoftwarePathRenderer(nullptr)
        , fFlushing(false) {

    if (GrContextOptions::Enable::kNo == sortRenderTargets) {
        fSortRenderTargets = false;
    } else if (GrContextOptions::Enable::kYes == sortRenderTargets) {
        fSortRenderTargets = true;
    } else {
        // By default we always enable sorting when we're explicitly allocating GPU resources
        fSortRenderTargets = explicitlyAllocating;
    }
}

void GrDrawingManager::cleanup() {
    for (int i = 0; i < fOpLists.count(); ++i) {
        // no opList should receive a new command after this
        fOpLists[i]->makeClosed(*fContext->contextPriv().caps());

        // We shouldn't need to do this, but it turns out some clients still hold onto opLists
        // after a cleanup.
        // MDB TODO: is this still true?
        if (!fOpLists[i]->unique()) {
            // TODO: Eventually this should be guaranteed unique.
            // https://bugs.chromium.org/p/skia/issues/detail?id=7111
            fOpLists[i]->endFlush();
        }
    }

    fOpLists.reset();

    fPathRendererChain = nullptr;
    fSoftwarePathRenderer = nullptr;

    fOnFlushCBObjects.reset();
}

GrDrawingManager::~GrDrawingManager() {
    this->cleanup();
}

void GrDrawingManager::abandon() {
    fAbandoned = true;
    this->cleanup();
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
GrSemaphoresSubmitted GrDrawingManager::internalFlush(GrSurfaceProxy*,
                                                      GrResourceCache::FlushType type,
                                                      int numSemaphores,
                                                      GrBackendSemaphore backendSemaphores[]) {
    GR_CREATE_TRACE_MARKER_CONTEXT("GrDrawingManager", "internalFlush", fContext);

    if (fFlushing || this->wasAbandoned()) {
        return GrSemaphoresSubmitted::kNo;
    }
    GrGpu* gpu = fContext->contextPriv().getGpu();
    if (!gpu) {
        return GrSemaphoresSubmitted::kNo; // Can't flush while DDL recording
    }
    fFlushing = true;

    for (int i = 0; i < fOpLists.count(); ++i) {
        // Semi-usually the GrOpLists are already closed at this point, but sometimes Ganesh
        // needs to flush mid-draw. In that case, the SkGpuDevice's GrOpLists won't be closed
        // but need to be flushed anyway. Closing such GrOpLists here will mean new
        // GrOpLists will be created to replace them if the SkGpuDevice(s) write to them again.
        fOpLists[i]->makeClosed(*fContext->contextPriv().caps());
    }

    if (fSortRenderTargets) {
        SkDEBUGCODE(bool result =) SkTTopoSort<GrOpList, GrOpList::TopoSortTraits>(&fOpLists);
        SkASSERT(result);
    }

#ifdef SK_DEBUG
    // This block checks for any unnecessary splits in the opLists. If two sequential opLists
    // share the same backing GrSurfaceProxy it means the opList was artificially split.
    if (fOpLists.count()) {
        GrRenderTargetOpList* prevOpList = fOpLists[0]->asRenderTargetOpList();
        for (int i = 1; i < fOpLists.count(); ++i) {
            GrRenderTargetOpList* curOpList = fOpLists[i]->asRenderTargetOpList();

            if (prevOpList && curOpList) {
                SkASSERT(prevOpList->fTarget.get() != curOpList->fTarget.get());
            }

            prevOpList = curOpList;
        }
    }
#endif

    GrOpFlushState flushState(gpu, fContext->contextPriv().resourceProvider(),
                              &fTokenTracker);

    GrOnFlushResourceProvider onFlushProvider(this);
    // TODO: AFAICT the only reason fFlushState is on GrDrawingManager rather than on the
    // stack here is to preserve the flush tokens.

    // Prepare any onFlush op lists (e.g. atlases).
    if (!fOnFlushCBObjects.empty()) {
        fFlushingOpListIDs.reset(fOpLists.count());
        for (int i = 0; i < fOpLists.count(); ++i) {
            fFlushingOpListIDs[i] = fOpLists[i]->uniqueID();
        }
        SkSTArray<4, sk_sp<GrRenderTargetContext>> renderTargetContexts;
        for (GrOnFlushCallbackObject* onFlushCBObject : fOnFlushCBObjects) {
            onFlushCBObject->preFlush(&onFlushProvider,
                                      fFlushingOpListIDs.begin(), fFlushingOpListIDs.count(),
                                      &renderTargetContexts);
            for (const sk_sp<GrRenderTargetContext>& rtc : renderTargetContexts) {
                sk_sp<GrRenderTargetOpList> onFlushOpList = sk_ref_sp(rtc->getRTOpList());
                if (!onFlushOpList) {
                    continue;   // Odd - but not a big deal
                }
#ifdef SK_DEBUG
                // OnFlush callbacks are already invoked during flush, and are therefore expected to
                // handle resource allocation & usage on their own. (No deferred or lazy proxies!)
                onFlushOpList->visitProxies_debugOnly([](GrSurfaceProxy* p) {
                    SkASSERT(!p->asTextureProxy() || !p->asTextureProxy()->texPriv().isDeferred());
                    SkASSERT(GrSurfaceProxy::LazyState::kNot == p->lazyInstantiationState());
                });
#endif
                onFlushOpList->makeClosed(*fContext->contextPriv().caps());
                onFlushOpList->prepare(&flushState);
                fOnFlushCBOpLists.push_back(std::move(onFlushOpList));
            }
            renderTargetContexts.reset();
        }
    }

#if 0
    // Enable this to print out verbose GrOp information
    for (int i = 0; i < fOpLists.count(); ++i) {
        SkDEBUGCODE(fOpLists[i]->dump();)
    }
#endif

    int startIndex, stopIndex;
    bool flushed = false;

    {
        GrResourceAllocator alloc(fContext->contextPriv().resourceProvider());
        for (int i = 0; i < fOpLists.count(); ++i) {
            fOpLists[i]->gatherProxyIntervals(&alloc);
            alloc.markEndOfOpList(i);
        }

        GrResourceAllocator::AssignError error = GrResourceAllocator::AssignError::kNoError;
        while (alloc.assign(&startIndex, &stopIndex, flushState.uninstantiateProxyTracker(),
                            &error)) {
            if (GrResourceAllocator::AssignError::kFailedProxyInstantiation == error) {
                for (int i = startIndex; i < stopIndex; ++i) {
                    if (fOpLists[i]) {
                        fOpLists[i]->purgeOpsWithUninstantiatedProxies();
                    }
                }
            }

            if (this->executeOpLists(startIndex, stopIndex, &flushState)) {
                flushed = true;
            }
        }
    }

#ifdef SK_DEBUG
    for (const auto& opList : fOpLists) {
        // If there are any remaining opLists at this point, make sure they will not survive the
        // flush. Otherwise we need to call endFlush() on them.
        // http://skbug.com/7111
        SkASSERT(!opList || opList->unique());
    }
#endif
    fOpLists.reset();

#ifdef SK_DEBUG
    // In non-DDL mode this checks that all the flushed ops have been freed from the memory pool.
    // When we move to partial flushes this assert will no longer be valid.
    // In DDL mode this check is somewhat superfluous since the memory for most of the ops/opLists
    // will be stored in the DDL's GrOpMemoryPools.
    GrOpMemoryPool* opMemoryPool = fContext->contextPriv().opMemoryPool();
    opMemoryPool->isEmpty();
#endif

    GrSemaphoresSubmitted result = gpu->finishFlush(numSemaphores, backendSemaphores);

    flushState.uninstantiateProxyTracker()->uninstantiateAllProxies();

    // We always have to notify the cache when it requested a flush so it can reset its state.
    if (flushed || type == GrResourceCache::FlushType::kCacheRequested) {
        fContext->contextPriv().getResourceCache()->notifyFlushOccurred(type);
    }
    for (GrOnFlushCallbackObject* onFlushCBObject : fOnFlushCBObjects) {
        onFlushCBObject->postFlush(fTokenTracker.nextTokenToFlush(), fFlushingOpListIDs.begin(),
                                   fFlushingOpListIDs.count());
    }
    fFlushingOpListIDs.reset();
    fFlushing = false;

    return result;
}

static void end_oplist_flush_if_not_unique(const sk_sp<GrOpList>& opList) {
    if (!opList->unique()) {
        // TODO: Eventually this should be guaranteed unique: http://skbug.com/7111
        opList->endFlush();
    }
}

bool GrDrawingManager::executeOpLists(int startIndex, int stopIndex, GrOpFlushState* flushState) {
    SkASSERT(startIndex <= stopIndex && stopIndex <= fOpLists.count());

#if GR_FLUSH_TIME_OP_SPEW
    SkDebugf("Flushing opLists: %d to %d out of [%d, %d]\n",
                            startIndex, stopIndex, 0, fOpLists.count());
    for (int i = startIndex; i < stopIndex; ++i) {
        fOpLists[i]->dump(true);
    }
#endif

    GrResourceProvider* resourceProvider = fContext->contextPriv().resourceProvider();
    bool anyOpListsExecuted = false;

    for (int i = startIndex; i < stopIndex; ++i) {
        if (!fOpLists[i]) {
             continue;
        }

        if (resourceProvider->explicitlyAllocateGPUResources()) {
            if (!fOpLists[i]->isInstantiated()) {
                // If the backing surface wasn't allocated drop the draw of the entire opList.
                end_oplist_flush_if_not_unique(fOpLists[i]); // http://skbug.com/7111
                fOpLists[i] = nullptr;
                continue;
            }
        } else {
            if (!fOpLists[i]->instantiate(resourceProvider)) {
                SkDebugf("OpList failed to instantiate.\n");
                end_oplist_flush_if_not_unique(fOpLists[i]); // http://skbug.com/7111
                fOpLists[i] = nullptr;
                continue;
            }
        }

        // TODO: handle this instantiation via lazy surface proxies?
        // Instantiate all deferred proxies (being built on worker threads) so we can upload them
        fOpLists[i]->instantiateDeferredProxies(fContext->contextPriv().resourceProvider());
        fOpLists[i]->prepare(flushState);
    }

    // Upload all data to the GPU
    flushState->preExecuteDraws();

    // Execute the onFlush op lists first, if any.
    for (sk_sp<GrOpList>& onFlushOpList : fOnFlushCBOpLists) {
        if (!onFlushOpList->execute(flushState)) {
            SkDebugf("WARNING: onFlushOpList failed to execute.\n");
        }
        SkASSERT(onFlushOpList->unique());
        onFlushOpList = nullptr;
    }
    fOnFlushCBOpLists.reset();

    // Execute the normal op lists.
    for (int i = startIndex; i < stopIndex; ++i) {
        if (!fOpLists[i]) {
            continue;
        }

        if (fOpLists[i]->execute(flushState)) {
            anyOpListsExecuted = true;
        }
    }

    SkASSERT(!flushState->commandBuffer());
    SkASSERT(fTokenTracker.nextDrawToken() == fTokenTracker.nextTokenToFlush());

    // We reset the flush state before the OpLists so that the last resources to be freed are those
    // that are written to in the OpLists. This helps to make sure the most recently used resources
    // are the last to be purged by the resource cache.
    flushState->reset();

    for (int i = startIndex; i < stopIndex; ++i) {
        if (!fOpLists[i]) {
            continue;
        }
        end_oplist_flush_if_not_unique(fOpLists[i]); // http://skbug.com/7111
        fOpLists[i] = nullptr;
    }

    return anyOpListsExecuted;
}

GrSemaphoresSubmitted GrDrawingManager::prepareSurfaceForExternalIO(
        GrSurfaceProxy* proxy, int numSemaphores, GrBackendSemaphore backendSemaphores[]) {
    if (this->wasAbandoned()) {
        return GrSemaphoresSubmitted::kNo;
    }
    SkASSERT(proxy);

    GrGpu* gpu = fContext->contextPriv().getGpu();
    if (!gpu) {
        return GrSemaphoresSubmitted::kNo; // Can't flush while DDL recording
    }

    GrSemaphoresSubmitted result = GrSemaphoresSubmitted::kNo;
    if (proxy->priv().hasPendingIO() || numSemaphores) {
        result = this->flush(proxy, numSemaphores, backendSemaphores);
    }

    if (!proxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return result;
    }

    GrSurface* surface = proxy->priv().peekSurface();
    if (auto* rt = surface->asRenderTarget()) {
        gpu->resolveRenderTarget(rt);
    }
#if 0
    // This is temporarily is disabled. See comment in SkImage_Gpu.cpp,
    // new_wrapped_texture_common().
    if (auto* tex = surface->asTexture()) {
        if (tex->texturePriv().mipMapped() == GrMipMapped::kYes &&
            tex->texturePriv().mipMapsAreDirty()) {
            gpu->regenerateMipMapLevels(tex);
        }
    }
#endif
    return result;
}

void GrDrawingManager::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fOnFlushCBObjects.push_back(onFlushCBObject);
}

void GrDrawingManager::moveOpListsToDDL(SkDeferredDisplayList* ddl) {
    for (int i = 0; i < fOpLists.count(); ++i) {
        // no opList should receive a new command after this
        fOpLists[i]->makeClosed(*fContext->contextPriv().caps());
    }

    SkASSERT(ddl->fOpLists.empty());
    ddl->fOpLists.swap(fOpLists);

    if (fPathRendererChain) {
        if (auto ccpr = fPathRendererChain->getCoverageCountingPathRenderer()) {
            ddl->fPendingPaths = ccpr->detachPendingPaths();
        }
    }
}

void GrDrawingManager::copyOpListsFromDDL(const SkDeferredDisplayList* ddl,
                                          GrRenderTargetProxy* newDest) {
    // Here we jam the proxy that backs the current replay SkSurface into the LazyProxyData.
    // The lazy proxy that references it (in the copied opLists) will steal its GrTexture.
    ddl->fLazyProxyData->fReplayDest = newDest;

    if (ddl->fPendingPaths.size()) {
        GrCoverageCountingPathRenderer* ccpr = this->getCoverageCountingPathRenderer();

        ccpr->mergePendingPaths(ddl->fPendingPaths);
    }
    fOpLists.push_back_n(ddl->fOpLists.count(), ddl->fOpLists.begin());
}

sk_sp<GrRenderTargetOpList> GrDrawingManager::newRTOpList(GrRenderTargetProxy* rtp,
                                                          bool managedOpList) {
    SkASSERT(fContext);

    if (!fOpLists.empty()) {
        // This is  a temporary fix for the partial-MDB world. In that world we're not
        // reordering so ops that (in the single opList world) would've just glommed onto the
        // end of the single opList but referred to a far earlier RT need to appear in their
        // own opList.
        fOpLists.back()->makeClosed(*fContext->contextPriv().caps());
    }

    auto resourceProvider = fContext->contextPriv().resourceProvider();

    sk_sp<GrRenderTargetOpList> opList(new GrRenderTargetOpList(
                                                        resourceProvider,
                                                        fContext->contextPriv().refOpMemoryPool(),
                                                        rtp,
                                                        fContext->contextPriv().getAuditTrail()));
    SkASSERT(rtp->getLastOpList() == opList.get());

    if (managedOpList) {
        fOpLists.push_back() = opList;
    }

    return opList;
}

sk_sp<GrTextureOpList> GrDrawingManager::newTextureOpList(GrTextureProxy* textureProxy) {
    SkASSERT(fContext);

    if (!fOpLists.empty()) {
        // This is  a temporary fix for the partial-MDB world. In that world we're not
        // reordering so ops that (in the single opList world) would've just glommed onto the
        // end of the single opList but referred to a far earlier RT need to appear in their
        // own opList.
        fOpLists.back()->makeClosed(*fContext->contextPriv().caps());
    }

    sk_sp<GrTextureOpList> opList(new GrTextureOpList(fContext->contextPriv().resourceProvider(),
                                                      fContext->contextPriv().refOpMemoryPool(),
                                                      textureProxy,
                                                      fContext->contextPriv().getAuditTrail()));

    SkASSERT(textureProxy->getLastOpList() == opList.get());

    fOpLists.push_back() = opList;

    return opList;
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
                new GrSoftwarePathRenderer(fContext->contextPriv().proxyProvider(),
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
    GrResourceCache* resourceCache = fContext->contextPriv().getResourceCache();
    if (resourceCache && resourceCache->requestsFlush()) {
        this->internalFlush(nullptr, GrResourceCache::kCacheRequested, 0, nullptr);
    }
}

sk_sp<GrRenderTargetContext> GrDrawingManager::makeRenderTargetContext(
                                                            sk_sp<GrSurfaceProxy> sProxy,
                                                            sk_sp<SkColorSpace> colorSpace,
                                                            const SkSurfaceProps* surfaceProps,
                                                            bool managedOpList) {
    if (this->wasAbandoned() || !sProxy->asRenderTargetProxy()) {
        return nullptr;
    }

    // SkSurface catches bad color space usage at creation. This check handles anything that slips
    // by, including internal usage.
    if (!SkSurface_Gpu::Valid(fContext->contextPriv().caps(), sProxy->config(), colorSpace.get())) {
        SkDEBUGFAIL("Invalid config and colorspace combination");
        return nullptr;
    }

    sk_sp<GrRenderTargetProxy> rtp(sk_ref_sp(sProxy->asRenderTargetProxy()));

    return sk_sp<GrRenderTargetContext>(new GrRenderTargetContext(
                                                        fContext, this, std::move(rtp),
                                                        std::move(colorSpace),
                                                        surfaceProps,
                                                        fContext->contextPriv().getAuditTrail(),
                                                        fSingleOwner, managedOpList));
}

sk_sp<GrTextureContext> GrDrawingManager::makeTextureContext(sk_sp<GrSurfaceProxy> sProxy,
                                                             sk_sp<SkColorSpace> colorSpace) {
    if (this->wasAbandoned() || !sProxy->asTextureProxy()) {
        return nullptr;
    }

    // SkSurface catches bad color space usage at creation. This check handles anything that slips
    // by, including internal usage.
    if (!SkSurface_Gpu::Valid(fContext->contextPriv().caps(), sProxy->config(), colorSpace.get())) {
        SkDEBUGFAIL("Invalid config and colorspace combination");
        return nullptr;
    }

    // GrTextureRenderTargets should always be using a GrRenderTargetContext
    SkASSERT(!sProxy->asRenderTargetProxy());

    sk_sp<GrTextureProxy> textureProxy(sk_ref_sp(sProxy->asTextureProxy()));

    return sk_sp<GrTextureContext>(new GrTextureContext(fContext, this, std::move(textureProxy),
                                                        std::move(colorSpace),
                                                        fContext->contextPriv().getAuditTrail(),
                                                        fSingleOwner));
}
