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
#include "GrOnFlushResourceProvider.h"
#include "GrOpList.h"
#include "GrRenderTargetContext.h"
#include "GrPathRenderingRenderTargetContext.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceAllocator.h"
#include "GrResourceProvider.h"
#include "GrSoftwarePathRenderer.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTextureContext.h"
#include "GrTextureOpList.h"
#include "GrTextureProxy.h"
#include "GrTextureProxyPriv.h"
#include "SkSurface_Gpu.h"
#include "SkTTopoSort.h"

#include "GrTracing.h"
#include "text/GrAtlasTextContext.h"
#include "text/GrStencilAndCoverTextContext.h"

void GrDrawingManager::cleanup() {
    for (int i = 0; i < fOpLists.count(); ++i) {
        // no opList should receive a new command after this
        fOpLists[i]->makeClosed(*fContext->caps());

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

    delete fPathRendererChain;
    fPathRendererChain = nullptr;
    SkSafeSetNull(fSoftwarePathRenderer);

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
    delete fPathRendererChain;
    fPathRendererChain = nullptr;
    SkSafeSetNull(fSoftwarePathRenderer);
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
    fFlushing = true;

    for (int i = 0; i < fOpLists.count(); ++i) {
        // Semi-usually the GrOpLists are already closed at this point, but sometimes Ganesh
        // needs to flush mid-draw. In that case, the SkGpuDevice's GrOpLists won't be closed
        // but need to be flushed anyway. Closing such GrOpLists here will mean new
        // GrOpLists will be created to replace them if the SkGpuDevice(s) write to them again.
        fOpLists[i]->makeClosed(*fContext->caps());
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

#ifndef SK_DISABLE_RENDER_TARGET_SORTING
    SkDEBUGCODE(bool result =)
                        SkTTopoSort<GrOpList, GrOpList::TopoSortTraits>(&fOpLists);
    SkASSERT(result);
#endif

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
                onFlushOpList->makeClosed(*fContext->caps());
                onFlushOpList->prepare(&fFlushState);
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

#ifdef SK_DISABLE_EXPLICIT_GPU_RESOURCE_ALLOCATION
        startIndex = 0;
        stopIndex = fOpLists.count();
#else
        while (alloc.assign(&startIndex, &stopIndex))
#endif
        {
            if (this->executeOpLists(startIndex, stopIndex, &fFlushState)) {
                flushed = true;
            }
        }
    }

    fOpLists.reset();

    GrSemaphoresSubmitted result = fContext->getGpu()->finishFlush(numSemaphores,
                                                                   backendSemaphores);

    // We always have to notify the cache when it requested a flush so it can reset its state.
    if (flushed || type == GrResourceCache::FlushType::kCacheRequested) {
        fContext->contextPriv().getResourceCache()->notifyFlushOccurred(type);
    }
    for (GrOnFlushCallbackObject* onFlushCBObject : fOnFlushCBObjects) {
        onFlushCBObject->postFlush(fFlushState.nextTokenToFlush(), fFlushingOpListIDs.begin(),
                                   fFlushingOpListIDs.count());
    }
    fFlushingOpListIDs.reset();
    fFlushing = false;

    return result;
}

bool GrDrawingManager::executeOpLists(int startIndex, int stopIndex, GrOpFlushState* flushState) {
    SkASSERT(startIndex <= stopIndex && stopIndex <= fOpLists.count());

    bool anyOpListsExecuted = false;

    for (int i = startIndex; i < stopIndex; ++i) {
        if (!fOpLists[i]) {
             continue;
        }

#ifdef SK_DISABLE_EXPLICIT_GPU_RESOURCE_ALLOCATION
        if (!fOpLists[i]->instantiate(fContext->contextPriv().resourceProvider())) {
            SkDebugf("OpList failed to instantiate.\n");
            fOpLists[i] = nullptr;
            continue;
        }
#else
        SkASSERT(fOpLists[i]->isInstantiated());
#endif

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
    SkASSERT(flushState->nextDrawToken() == flushState->nextTokenToFlush());

    // We reset the flush state before the OpLists so that the last resources to be freed are those
    // that are written to in the OpLists. This helps to make sure the most recently used resources
    // are the last to be purged by the resource cache.
    flushState->reset();

    for (int i = startIndex; i < stopIndex; ++i) {
        if (!fOpLists[i]) {
            continue;
        }
        if (!fOpLists[i]->unique()) {
            // TODO: Eventually this should be guaranteed unique.
            // https://bugs.chromium.org/p/skia/issues/detail?id=7111
            fOpLists[i]->endFlush();
        }
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

    GrSemaphoresSubmitted result = GrSemaphoresSubmitted::kNo;
    if (proxy->priv().hasPendingIO() || numSemaphores) {
        result = this->flush(proxy, numSemaphores, backendSemaphores);
    }

    if (!proxy->instantiate(fContext->contextPriv().resourceProvider())) {
        return result;
    }

    GrSurface* surface = proxy->priv().peekSurface();

    if (fContext->getGpu() && surface->asRenderTarget()) {
        fContext->getGpu()->resolveRenderTarget(surface->asRenderTarget(), proxy->origin());
    }
    return result;
}

void GrDrawingManager::addOnFlushCallbackObject(GrOnFlushCallbackObject* onFlushCBObject) {
    fOnFlushCBObjects.push_back(onFlushCBObject);
}

sk_sp<GrRenderTargetOpList> GrDrawingManager::newRTOpList(GrRenderTargetProxy* rtp,
                                                          bool managedOpList) {
    SkASSERT(fContext);

    // This is  a temporary fix for the partial-MDB world. In that world we're not reordering
    // so ops that (in the single opList world) would've just glommed onto the end of the single
    // opList but referred to a far earlier RT need to appear in their own opList.
    if (!fOpLists.empty()) {
        fOpLists.back()->makeClosed(*fContext->caps());
    }

    sk_sp<GrRenderTargetOpList> opList(new GrRenderTargetOpList(rtp,
                                                                fContext->getGpu(),
                                                                fContext->getAuditTrail()));
    SkASSERT(rtp->getLastOpList() == opList.get());

    if (managedOpList) {
        fOpLists.push_back() = opList;
    }

    return opList;
}

sk_sp<GrTextureOpList> GrDrawingManager::newTextureOpList(GrTextureProxy* textureProxy) {
    SkASSERT(fContext);

    // This is  a temporary fix for the partial-MDB world. In that world we're not reordering
    // so ops that (in the single opList world) would've just glommed onto the end of the single
    // opList but referred to a far earlier RT need to appear in their own opList.
    if (!fOpLists.empty()) {
        fOpLists.back()->makeClosed(*fContext->caps());
    }

    sk_sp<GrTextureOpList> opList(new GrTextureOpList(fContext->contextPriv().resourceProvider(),
                                                      textureProxy,
                                                      fContext->getAuditTrail()));

    SkASSERT(textureProxy->getLastOpList() == opList.get());

    fOpLists.push_back() = opList;

    return opList;
}

GrAtlasTextContext* GrDrawingManager::getAtlasTextContext() {
    if (!fAtlasTextContext) {
        fAtlasTextContext = GrAtlasTextContext::Make(fOptionsForAtlasTextContext);
    }

    return fAtlasTextContext.get();
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
        fPathRendererChain = new GrPathRendererChain(fContext, fOptionsForPathRendererChain);
    }

    GrPathRenderer* pr = fPathRendererChain->getPathRenderer(args, drawType, stencilSupport);
    if (!pr && allowSW) {
        if (!fSoftwarePathRenderer) {
            fSoftwarePathRenderer =
                    new GrSoftwarePathRenderer(fContext->contextPriv().proxyProvider(),
                                               fOptionsForPathRendererChain.fAllowPathMaskCaching);
        }
        if (GrPathRenderer::CanDrawPath::kNo != fSoftwarePathRenderer->canDrawPath(args)) {
            pr = fSoftwarePathRenderer;
        }
    }

    return pr;
}

GrCoverageCountingPathRenderer* GrDrawingManager::getCoverageCountingPathRenderer() {
    if (!fPathRendererChain) {
        fPathRendererChain = new GrPathRendererChain(fContext, fOptionsForPathRendererChain);
    }
    return fPathRendererChain->getCoverageCountingPathRenderer();
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
    // by, including internal usage. We allow a null color space here, for read/write pixels and
    // other special code paths. If a color space is provided, though, enforce all other rules.
    if (colorSpace && !SkSurface_Gpu::Valid(fContext, sProxy->config(), colorSpace.get())) {
        SkDEBUGFAIL("Invalid config and colorspace combination");
        return nullptr;
    }

    sk_sp<GrRenderTargetProxy> rtp(sk_ref_sp(sProxy->asRenderTargetProxy()));

    bool useDIF = false;
    if (surfaceProps) {
        useDIF = surfaceProps->isUseDeviceIndependentFonts();
    }

    if (useDIF && fContext->caps()->shaderCaps()->pathRenderingSupport() &&
        GrFSAAType::kNone != rtp->fsaaType()) {

        return sk_sp<GrRenderTargetContext>(new GrPathRenderingRenderTargetContext(
                                                    fContext, this, std::move(rtp),
                                                    std::move(colorSpace), surfaceProps,
                                                    fContext->getAuditTrail(), fSingleOwner));
    }

    return sk_sp<GrRenderTargetContext>(new GrRenderTargetContext(fContext, this, std::move(rtp),
                                                                  std::move(colorSpace),
                                                                  surfaceProps,
                                                                  fContext->getAuditTrail(),
                                                                  fSingleOwner, managedOpList));
}

sk_sp<GrTextureContext> GrDrawingManager::makeTextureContext(sk_sp<GrSurfaceProxy> sProxy,
                                                             sk_sp<SkColorSpace> colorSpace) {
    if (this->wasAbandoned() || !sProxy->asTextureProxy()) {
        return nullptr;
    }

    // SkSurface catches bad color space usage at creation. This check handles anything that slips
    // by, including internal usage. We allow a null color space here, for read/write pixels and
    // other special code paths. If a color space is provided, though, enforce all other rules.
    if (colorSpace && !SkSurface_Gpu::Valid(fContext, sProxy->config(), colorSpace.get())) {
        SkDEBUGFAIL("Invalid config and colorspace combination");
        return nullptr;
    }

    // GrTextureRenderTargets should always be using GrRenderTargetContext
    SkASSERT(!sProxy->asRenderTargetProxy());

    sk_sp<GrTextureProxy> textureProxy(sk_ref_sp(sProxy->asTextureProxy()));

    return sk_sp<GrTextureContext>(new GrTextureContext(fContext, this, std::move(textureProxy),
                                                        std::move(colorSpace),
                                                        fContext->getAuditTrail(),
                                                        fSingleOwner));
}
