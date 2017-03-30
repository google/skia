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
#include "GrSurfaceProxyPriv.h"
#include "GrTextureContext.h"
#include "GrTextureOpList.h"
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

// MDB TODO: make use of the 'proxy' parameter.
void GrDrawingManager::internalFlush(GrSurfaceProxy*, GrResourceCache::FlushType type) {
    if (fFlushing || this->wasAbandoned()) {
        return;
    }
    fFlushing = true;
    bool flushed = false;

    for (int i = 0; i < fOpLists.count(); ++i) {
        // Semi-usually the GrOpLists are already closed at this point, but sometimes Ganesh
        // needs to flush mid-draw. In that case, the SkGpuDevice's GrOpLists won't be closed
        // but need to be flushed anyway. Closing such GrOpLists here will mean new
        // GrOpLists will be created to replace them if the SkGpuDevice(s) write to them again.
        fOpLists[i]->makeClosed();
    }

    SkDEBUGCODE(bool result =)
                        SkTTopoSort<GrOpList, GrOpList::TopoSortTraits>(&fOpLists);
    SkASSERT(result);

    GrPreFlushResourceProvider preFlushProvider(this);

    if (fPreFlushCBObjects.count()) {
        // MDB TODO: pre-MDB '1' is the correct pre-allocated size. Post-MDB it will need
        // to be larger.
        SkAutoSTArray<1, uint32_t> opListIds(fOpLists.count());
        for (int i = 0; i < fOpLists.count(); ++i) {
            opListIds[i] = fOpLists[i]->uniqueID();
        }

        SkSTArray<1, sk_sp<GrRenderTargetContext>> renderTargetContexts;
        for (int i = 0; i < fPreFlushCBObjects.count(); ++i) {
            fPreFlushCBObjects[i]->preFlush(&preFlushProvider,
                                            opListIds.get(), opListIds.count(),
                                            &renderTargetContexts);
            if (!renderTargetContexts.count()) {
                continue;       // This is fine. No atlases of this type are required for this flush
            }

            for (int j = 0; j < renderTargetContexts.count(); ++j) {
                GrRenderTargetOpList* opList = renderTargetContexts[j]->getOpList();
                if (!opList) {
                    continue;   // Odd - but not a big deal
                }
                SkDEBUGCODE(opList->validateTargetsSingleRenderTarget());
                opList->prepareOps(&fFlushState);
                if (!opList->executeOps(&fFlushState)) {
                    continue;         // This is bad
                }
            }
            renderTargetContexts.reset();
        }
    }

    for (int i = 0; i < fOpLists.count(); ++i) {
        fOpLists[i]->prepareOps(&fFlushState);
    }

#if 0
    // Enable this to print out verbose GrOp information
    for (int i = 0; i < fOpLists.count(); ++i) {
        SkDEBUGCODE(fOpLists[i]->dump();)
    }
#endif

    // Upload all data to the GPU
    fFlushState.preIssueDraws();

    for (int i = 0; i < fOpLists.count(); ++i) {
        if (fOpLists[i]->executeOps(&fFlushState)) {
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

void GrDrawingManager::prepareSurfaceForExternalIO(GrSurfaceProxy* proxy) {
    if (this->wasAbandoned()) {
        return;
    }
    SkASSERT(proxy);

    if (proxy->priv().hasPendingIO()) {
        this->flush(proxy);
    }

    GrSurface* surface = proxy->instantiate(fContext->resourceProvider());
    if (!surface) {
        return;
    }

    if (fContext->getGpu() && surface->asRenderTarget()) {
        fContext->getGpu()->resolveRenderTarget(surface->asRenderTarget());
    }
}

void GrDrawingManager::addPreFlushCallbackObject(sk_sp<GrPreFlushCallbackObject> preFlushCBObject) {
    fPreFlushCBObjects.push_back(preFlushCBObject);
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

GrTextureOpList* GrDrawingManager::newOpList(GrTextureProxy* textureProxy) {
    SkASSERT(fContext);

    GrTextureOpList* opList = new GrTextureOpList(textureProxy, fContext->getGpu(),
                                                  fContext->getAuditTrail());

#ifndef ENABLE_MDB
    // When MDB is disabled we still create a new GrOpList, but don't store or ref it - we rely
    // on the caller to immediately execute and free it.
    return opList;
#else
    *fOpLists.append() = opList;

    // Drawing manager gets the creation ref - this ref is for the caller
    return SkRef(opList);
#endif
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
                    new GrSoftwarePathRenderer(fContext->resourceProvider(),
                                               fOptionsForPathRendererChain.fAllowPathMaskCaching);
        }
        if (fSoftwarePathRenderer->canDrawPath(args)) {
            pr = fSoftwarePathRenderer;
        }
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
        rtp->isStencilBufferMultisampled()) {
        // TODO: defer stencil buffer attachment for PathRenderingDrawContext
        sk_sp<GrRenderTarget> rt(sk_ref_sp(rtp->instantiate(fContext->resourceProvider())));
        if (!rt) {
            return nullptr;
        }
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
