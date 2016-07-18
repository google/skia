/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawContext.h"
#include "GrDrawingManager.h"
#include "GrDrawTarget.h"
#include "GrPathRenderingDrawContext.h"
#include "GrResourceProvider.h"
#include "GrSoftwarePathRenderer.h"
#include "SkTTopoSort.h"

#include "text/GrAtlasTextContext.h"
#include "text/GrStencilAndCoverTextContext.h"

void GrDrawingManager::cleanup() {
    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->makeClosed();  // no drawTarget should receive a new command after this
        fDrawTargets[i]->clearRT();

        // We shouldn't need to do this, but it turns out some clients still hold onto drawtargets
        // after a cleanup
        fDrawTargets[i]->reset();
        fDrawTargets[i]->unref();
    }

    fDrawTargets.reset();

    delete fPathRendererChain;
    fPathRendererChain = nullptr;
    SkSafeSetNull(fSoftwarePathRenderer);
}

GrDrawingManager::~GrDrawingManager() {
    this->cleanup();
}

void GrDrawingManager::abandon() {
    fAbandoned = true;
    this->cleanup();
}

void GrDrawingManager::freeGpuResources() {
    // a path renderer may be holding onto resources
    delete fPathRendererChain;
    fPathRendererChain = nullptr;
    SkSafeSetNull(fSoftwarePathRenderer);
}

void GrDrawingManager::reset() {
    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->reset();
    }
    fFlushState.reset();
}

void GrDrawingManager::flush() {
    if (fFlushing || this->wasAbandoned()) {
        return;
    }
    fFlushing = true;

    SkDEBUGCODE(bool result =)
                        SkTTopoSort<GrDrawTarget, GrDrawTarget::TopoSortTraits>(&fDrawTargets);
    SkASSERT(result);

    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->prepareBatches(&fFlushState);
    }

    // Enable this to print out verbose batching information
#if 0
    for (int i = 0; i < fDrawTargets.count(); ++i) {
        SkDEBUGCODE(fDrawTargets[i]->dump();)
    }
#endif

    // Upload all data to the GPU
    fFlushState.preIssueDraws();

    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->drawBatches(&fFlushState);
    }

    SkASSERT(fFlushState.nextDrawToken() == fFlushState.nextTokenToFlush());

    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->reset();
#ifdef ENABLE_MDB
        fDrawTargets[i]->unref();
#endif
    }

#ifndef ENABLE_MDB
    // When MDB is disabled we keep reusing the same drawTarget
    if (fDrawTargets.count()) {
        SkASSERT(fDrawTargets.count() == 1);
        // Clear out this flag so the topological sort's SkTTopoSort_CheckAllUnmarked check
        // won't bark
        fDrawTargets[0]->resetFlag(GrDrawTarget::kWasOutput_Flag);
    }
#else
    fDrawTargets.reset();
#endif

    fFlushState.reset();
    fFlushing = false;
}

GrDrawTarget* GrDrawingManager::newDrawTarget(GrRenderTarget* rt) {
    SkASSERT(fContext);

#ifndef ENABLE_MDB
    // When MDB is disabled we always just return the single drawTarget
    if (fDrawTargets.count()) {
        SkASSERT(fDrawTargets.count() == 1);
        // In the non-MDB-world the same drawTarget gets reused for multiple render targets.
        // Update this pointer so all the asserts are happy
        rt->setLastDrawTarget(fDrawTargets[0]);
        // DrawingManager gets the creation ref - this ref is for the caller
        return SkRef(fDrawTargets[0]);
    }
#endif

    GrDrawTarget* dt = new GrDrawTarget(rt, fContext->getGpu(), fContext->resourceProvider(),
                                        fContext->getAuditTrail(), fOptionsForDrawTargets);

    *fDrawTargets.append() = dt;

    // DrawingManager gets the creation ref - this ref is for the caller
    return SkRef(dt);
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
        fPathRendererChain = new GrPathRendererChain(fContext);
    }

    GrPathRenderer* pr = fPathRendererChain->getPathRenderer(args, drawType, stencilSupport);
    if (!pr && allowSW) {
        if (!fSoftwarePathRenderer) {
            fSoftwarePathRenderer = new GrSoftwarePathRenderer(fContext->textureProvider());
        }
        pr = fSoftwarePathRenderer;
    }

    return pr;
}

sk_sp<GrDrawContext> GrDrawingManager::drawContext(sk_sp<GrRenderTarget> rt,
                                                   const SkSurfaceProps* surfaceProps) {
    if (this->wasAbandoned()) {
        return nullptr;
    }


    bool useDIF = false;
    if (surfaceProps) {
        useDIF = surfaceProps->isUseDeviceIndependentFonts();
    }

    if (useDIF && fContext->caps()->shaderCaps()->pathRenderingSupport() &&
        rt->isStencilBufferMultisampled()) {
        GrStencilAttachment* sb = fContext->resourceProvider()->attachStencilAttachment(rt.get());
        if (sb) {
            return sk_sp<GrDrawContext>(new GrPathRenderingDrawContext(
                                                        fContext, this, std::move(rt), 
                                                        surfaceProps,
                                                        fContext->getAuditTrail(), fSingleOwner));
        }
    }

    return sk_sp<GrDrawContext>(new GrDrawContext(fContext, this, std::move(rt), surfaceProps,
                                                  fContext->getAuditTrail(),
                                                  fSingleOwner));
}
