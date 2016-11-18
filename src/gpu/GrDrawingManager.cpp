/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawingManager.h"

#include "GrContext.h"
#include "GrRenderTargetContext.h"
#include "GrPathRenderingRenderTargetContext.h"
#include "GrRenderTargetProxy.h"
#include "GrResourceProvider.h"
#include "GrSoftwarePathRenderer.h"
#include "GrSurfacePriv.h"
#include "SkSurface_Gpu.h"
#include "SkTTopoSort.h"

#include "text/GrAtlasTextContext.h"
#include "text/GrStencilAndCoverTextContext.h"

void GrDrawingManager::cleanup() {
    for (int i = 0; i < fOpLists.count(); ++i) {
        fOpLists[i]->makeClosed();  // no opList should receive a new command after this
        fOpLists[i]->clearTarget();

        // We shouldn't need to do this, but it turns out some clients still hold onto opLists
        // after a cleanup
        fOpLists[i]->reset();
        fOpLists[i]->unref();
    }

    fOpLists.reset();

    delete fPathRendererChain;
    fPathRendererChain = nullptr;
    SkSafeSetNull(fSoftwarePathRenderer);
}

GrDrawingManager::~GrDrawingManager() {
    this->cleanup();
}

void GrDrawingManager::abandon() {
    fAbandoned = true;
    for (int i = 0; i < fOpLists.count(); ++i) {
        fOpLists[i]->abandonGpuResources();
    }
    this->cleanup();
}

void GrDrawingManager::freeGpuResources() {
    // a path renderer may be holding onto resources
    delete fPathRendererChain;
    fPathRendererChain = nullptr;
    SkSafeSetNull(fSoftwarePathRenderer);
    for (int i = 0; i < fOpLists.count(); ++i) {
        fOpLists[i]->freeGpuResources();
    }
}

void GrDrawingManager::reset() {
    for (int i = 0; i < fOpLists.count(); ++i) {
        fOpLists[i]->reset();
    }
    fFlushState.reset();
}

void GrDrawingManager::internalFlush(GrResourceCache::FlushType type) {
    if (fFlushing || this->wasAbandoned()) {
        return;
    }
    fFlushing = true;
    bool flushed = false;
    SkDEBUGCODE(bool result =)
                        SkTTopoSort<GrOpList, GrOpList::TopoSortTraits>(&fOpLists);
    SkASSERT(result);

    for (int i = 0; i < fOpLists.count(); ++i) {
        fOpLists[i]->prepareBatches(&fFlushState);
    }

    // Enable this to print out verbose batching information
#if 0
    for (int i = 0; i < fOpLists.count(); ++i) {
        SkDEBUGCODE(fOpLists[i]->dump();)
    }
#endif

    // Upload all data to the GPU
    fFlushState.preIssueDraws();

    for (int i = 0; i < fOpLists.count(); ++i) {
        if (fOpLists[i]->drawBatches(&fFlushState)) {
            flushed = true;
        }
    }

    SkASSERT(fFlushState.nextDrawToken() == fFlushState.nextTokenToFlush());

    for (int i = 0; i < fOpLists.count(); ++i) {
        fOpLists[i]->reset();
#ifdef ENABLE_MDB
        fOpLists[i]->unref();
#endif
    }

#ifndef ENABLE_MDB
    // When MDB is disabled we keep reusing the same GrOpList
    if (fOpLists.count()) {
        SkASSERT(fOpLists.count() == 1);
        // Clear out this flag so the topological sort's SkTTopoSort_CheckAllUnmarked check
        // won't bark
        fOpLists[0]->resetFlag(GrOpList::kWasOutput_Flag);
    }
#else
    fOpLists.reset();
#endif

    fFlushState.reset();
    // We always have to notify the cache when it requested a flush so it can reset its state.
    if (flushed || type == GrResourceCache::FlushType::kCacheRequested) {
        fContext->getResourceCache()->notifyFlushOccurred(type);
    }
    fFlushing = false;
}

void GrDrawingManager::prepareSurfaceForExternalIO(GrSurface* surface) {
    if (this->wasAbandoned()) {
        return;
    }
    SkASSERT(surface);
    SkASSERT(surface->getContext() == fContext);

    if (surface->surfacePriv().hasPendingIO()) {
        this->flush();
    }

    GrRenderTarget* rt = surface->asRenderTarget();
    if (fContext->getGpu() && rt) {
        fContext->getGpu()->resolveRenderTarget(rt);
    }
}

GrRenderTargetOpList* GrDrawingManager::newOpList(GrRenderTargetProxy* rtp) {
    SkASSERT(fContext);

#ifndef ENABLE_MDB
    // When MDB is disabled we always just return the single GrOpList
    if (fOpLists.count()) {
        SkASSERT(fOpLists.count() == 1);
        // In the non-MDB-world the same GrOpList gets reused for multiple render targets.
        // Update this pointer so all the asserts are happy
        rtp->setLastOpList(fOpLists[0]);
        // DrawingManager gets the creation ref - this ref is for the caller

        // TODO: although this is true right now it isn't cool
        return SkRef((GrRenderTargetOpList*) fOpLists[0]);
    }
#endif

    GrRenderTargetOpList* opList = new GrRenderTargetOpList(rtp,
                                                            fContext->getGpu(),
                                                            fContext->resourceProvider(),
                                                            fContext->getAuditTrail(),
                                                            fOptionsForOpLists);

    *fOpLists.append() = opList;

    // DrawingManager gets the creation ref - this ref is for the caller
    return SkRef(opList);
}

GrAtlasTextContext* GrDrawingManager::getAtlasTextContext() {
    if (!fAtlasTextContext) {
        fAtlasTextContext.reset(GrAtlasTextContext::Create());
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
                    new GrSoftwarePathRenderer(fContext->textureProvider(),
                                               fOptionsForPathRendererChain.fAllowPathMaskCaching);
        }
        pr = fSoftwarePathRenderer;
    }

    return pr;
}

sk_sp<GrRenderTargetContext> GrDrawingManager::makeRenderTargetContext(
                                                            sk_sp<GrSurfaceProxy> sProxy,
                                                            sk_sp<SkColorSpace> colorSpace,
                                                            const SkSurfaceProps* surfaceProps) {
    if (this->wasAbandoned() || !sProxy->asRenderTargetProxy()) {
        return nullptr;
    }

    sk_sp<GrRenderTargetProxy> rtp(sk_ref_sp(sProxy->asRenderTargetProxy()));

    // SkSurface catches bad color space usage at creation. This check handles anything that slips
    // by, including internal usage. We allow a null color space here, for read/write pixels and
    // other special code paths. If a color space is provided, though, enforce all other rules.
    if (colorSpace && !SkSurface_Gpu::Valid(fContext, rtp->config(), colorSpace.get())) {
        SkDEBUGFAIL("Invalid config and colorspace combination");
        return nullptr;
    }

    bool useDIF = false;
    if (surfaceProps) {
        useDIF = surfaceProps->isUseDeviceIndependentFonts();
    }

    if (useDIF && fContext->caps()->shaderCaps()->pathRenderingSupport() &&
        rtp->isStencilBufferMultisampled()) {
        // TODO: defer stencil buffer attachment for PathRenderingDrawContext
        sk_sp<GrRenderTarget> rt(sk_ref_sp(rtp->instantiate(fContext->textureProvider())));
        GrStencilAttachment* sb = fContext->resourceProvider()->attachStencilAttachment(rt.get());
        if (sb) {
            return sk_sp<GrRenderTargetContext>(new GrPathRenderingRenderTargetContext(
                                                        fContext, this, std::move(rtp),
                                                        std::move(colorSpace), surfaceProps,
                                                        fContext->getAuditTrail(), fSingleOwner));
        }
    }

    return sk_sp<GrRenderTargetContext>(new GrRenderTargetContext(fContext, this, std::move(rtp),
                                                                  std::move(colorSpace),
                                                                  surfaceProps,
                                                                  fContext->getAuditTrail(),
                                                                  fSingleOwner));
}
