/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrAtlasTextContext.h"
#include "GrDrawContext.h"
#include "GrDrawingManager.h"
#include "GrDrawTarget.h"
#include "GrResourceProvider.h"
#include "GrSoftwarePathRenderer.h"
#include "GrStencilAndCoverTextContext.h"
#include "SkTTopoSort.h"


void GrDrawingManager::cleanup() {
    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->makeClosed();  // no drawTarget should receive a new command after this
        fDrawTargets[i]->clearRT();

        fDrawTargets[i]->unref();
    }

    fDrawTargets.reset();

    delete fNVPRTextContext;
    fNVPRTextContext = nullptr;

    for (int i = 0; i < kNumPixelGeometries; ++i) {
        delete fTextContexts[i][0];
        fTextContexts[i][0] = nullptr;
        delete fTextContexts[i][1];
        fTextContexts[i][1] = nullptr;
    }

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
    SkDEBUGCODE(bool result =) 
                        SkTTopoSort<GrDrawTarget, GrDrawTarget::TopoSortTraits>(&fDrawTargets);
    SkASSERT(result);

#if 0
    for (int i = 0; i < fDrawTargets.count(); ++i) {
        SkDEBUGCODE(fDrawTargets[i]->dump();)
    }
#endif

    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->prepareBatches(&fFlushState);
    }

    // Upload all data to the GPU
    fFlushState.preIssueDraws();

    for (int i = 0; i < fDrawTargets.count(); ++i) {
        fDrawTargets[i]->drawBatches(&fFlushState);
    }

    SkASSERT(fFlushState.lastFlushedToken() == fFlushState.currentToken());

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
}

GrTextContext* GrDrawingManager::textContext(const SkSurfaceProps& props,
                                             GrRenderTarget* rt) {
    if (this->abandoned()) {
        return nullptr;
    }

    SkASSERT(props.pixelGeometry() < kNumPixelGeometries);
    bool useDIF = props.isUseDeviceIndependentFonts();

    if (useDIF && fContext->caps()->shaderCaps()->pathRenderingSupport() &&
        rt->isStencilBufferMultisampled()) {
        GrStencilAttachment* sb = fContext->resourceProvider()->attachStencilAttachment(rt);
        if (sb) {
            if (!fNVPRTextContext) {
                fNVPRTextContext = GrStencilAndCoverTextContext::Create(fContext, props);
            }

            return fNVPRTextContext;
        }
    }

    if (!fTextContexts[props.pixelGeometry()][useDIF]) {
        fTextContexts[props.pixelGeometry()][useDIF] = GrAtlasTextContext::Create(fContext, props);
    }

    return fTextContexts[props.pixelGeometry()][useDIF];
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
                                        fOptionsForDrawTargets);

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
            fSoftwarePathRenderer = new GrSoftwarePathRenderer(fContext);
        }
        pr = fSoftwarePathRenderer;
    }

    return pr;
}

GrDrawContext* GrDrawingManager::drawContext(GrRenderTarget* rt,
                                             const SkSurfaceProps* surfaceProps) {
    if (this->abandoned()) {
        return nullptr;
    }

    return new GrDrawContext(this, rt, surfaceProps);
}
