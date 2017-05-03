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

class StatsCollector {
public:
    StatsCollector(const char* title, int expected)
        : fTitle(title)
        , fNum(0)
        , fMin(INT_MAX)
        , fMax(-1) {
        fXs.setReserve(expected);
    }

    void add(int x) {
        SkASSERT(x >= 0);

        if (x >= fXs.count()) {
            for (int i = fXs.count(); i < x+1; ++i) {
                fXs.appendClear();
            }
            SkASSERT(fXs.count() == x+1);
            SkASSERT(0 == fXs[x]);
        }

        fNum++;
        fXs[x]++;
        if (fMin > x) {
            fMin = x;
        }
        if (fMax < x) {
            fMax = x;
        }
    }

    void done() {
        float mean = 0.0f;

        for (int i = 0; i < fXs.count(); ++i) {
            mean += i * fXs[i];
        }
        mean /= fNum;

        float stdDev = 0.0;
        for (int i = 0; i < fXs.count(); ++i) {
            stdDev += fXs[i]*(i-mean)*(i-mean);
        }
        stdDev /= fNum;
        stdDev = sqrt(stdDev);

        SkDebugf("-------------------- %s: mean %f stdDev: %f min %d max %d\n", fTitle, mean, stdDev, fMin, fMax);

        fNum = 0;
        fXs.rewind();
    }

private:
    const char*    fTitle;
    int            fNum;
    SkTDArray<int> fXs;
    int            fMin;
    int            fMax;
};

static void test() {
    StatsCollector stats("test", 1);

    stats.add(4);
    stats.add(36);
    stats.add(45);
    stats.add(50);
    stats.add(75);
    stats.done(); // expect mean of 42

    stats.add(2);
    stats.add(4);
    stats.add(4);
    stats.add(4);
    stats.add(5);
    stats.add(5);
    stats.add(7);
    stats.add(9);
    stats.done(); // expect mean of 5 & stdDev of 2
}

StatsCollector gOpStats("ops/opList", 100);
StatsCollector gClipStats("clips/opList", 100);

void done26() {
    gOpStats.done();
    gClipStats.done();
}

void GrDrawingManager::cleanup1() {
    for (int i = 0; i < fOpLists1.count(); ++i) {
        // no opList should receive a new command after this
        fOpLists1[i]->makeClosed(*fContext->caps());

        // We shouldn't need to do this, but it turns out some clients still hold onto opLists
        // after a cleanup.
        // MDB TODO: is this still true?
        fOpLists1[i]->reset1();
    }

    fOpLists1.reset();

    delete fPathRendererChain;
    fPathRendererChain = nullptr;
    SkSafeSetNull(fSoftwarePathRenderer);
}

GrDrawingManager::~GrDrawingManager() {
    this->cleanup1();
}

void GrDrawingManager::abandon() {
    fAbandoned = true;
    for (int i = 0; i < fOpLists1.count(); ++i) {
        fOpLists1[i]->abandonGpuResources();
    }
    this->cleanup1();
}

void GrDrawingManager::freeGpuResources() {
    // a path renderer may be holding onto resources
    delete fPathRendererChain;
    fPathRendererChain = nullptr;
    SkSafeSetNull(fSoftwarePathRenderer);
    for (int i = 0; i < fOpLists1.count(); ++i) {
        fOpLists1[i]->freeGpuResources();
    }
}

void GrDrawingManager::reset1() {
    for (int i = 0; i < fOpLists1.count(); ++i) {
        fOpLists1[i]->reset1();
    }
    fFlushState.reset();
}

gr_instanced::OpAllocator* GrDrawingManager::instancingAllocator() {
    if (fInstancingAllocator) {
        return fInstancingAllocator.get();
    }

    fInstancingAllocator = fContext->getGpu()->createInstancedRenderingAllocator();
    return fInstancingAllocator.get();
}

void GrDrawingManager::recycle(GrRenderTargetOpList* rtol) {
    fSpares.push(SkRef(rtol));
}

// MDB TODO: make use of the 'proxy' parameter.
void GrDrawingManager::internalFlush(GrSurfaceProxy*, GrResourceCache::FlushType type) {

    if (fFlushing || this->wasAbandoned()) {
        return;
    }
    fFlushing = true;
    bool flushed = false;

    int numOps = 0;
    for (int i = 0; i < fOpLists1.count(); ++i) {
        // Semi-usually the GrOpLists are already closed at this point, but sometimes Ganesh
        // needs to flush mid-draw. In that case, the SkGpuDevice's GrOpLists won't be closed
        // but need to be flushed anyway. Closing such GrOpLists here will mean new
        // GrOpLists will be created to replace them if the SkGpuDevice(s) write to them again.
        fOpLists1[i]->makeClosed(*fContext->caps());
        SkDEBUGCODE(fOpLists1[i]->validateTargetsSingleRenderTarget());
        numOps += fOpLists1[i]->numOps();
        gOpStats.add(fOpLists1[i]->numOps());
        gClipStats.add(fOpLists1[i]->numClips());
    }

    SkDebugf("numOpLists: %d numOps: %d\n", fOpLists1.count(), numOps);

#ifdef ENABLE_MDB
    SkDEBUGCODE(bool result =)
                        SkTTopoSort<GrOpList, GrOpList::TopoSortTraits>(&fOpLists);
    SkASSERT(result);
#endif

    GrPreFlushResourceProvider preFlushProvider(this);

    if (fPreFlushCBObjects.count()) {
        // MDB TODO: pre-MDB '1' is the correct pre-allocated size. Post-MDB it will need
        // to be larger.
        SkAutoSTArray<1, uint32_t> opListIds(fOpLists1.count());
        for (int i = 0; i < fOpLists1.count(); ++i) {
            opListIds[i] = fOpLists1[i]->uniqueID();
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
                opList->makeClosed(*fContext->caps());
                SkDEBUGCODE(opList->validateTargetsSingleRenderTarget());
                opList->prepareOps(&fFlushState);
                if (!opList->executeOps(&fFlushState)) {
                    continue;         // This is bad
                }
            }
            renderTargetContexts.reset();
        }
    }

    for (int i = 0; i < fOpLists1.count(); ++i) {
        fOpLists1[i]->prepareOps(&fFlushState);
    }

#if 0
    // Enable this to print out verbose GrOp information
    for (int i = 0; i < fOpLists.count(); ++i) {
        SkDEBUGCODE(fOpLists[i]->dump();)
    }
#endif

    // Upload all data to the GPU
    fFlushState.preIssueDraws();

    for (int i = 0; i < fOpLists1.count(); ++i) {
        if (fOpLists1[i]->executeOps(&fFlushState)) {
            flushed = true;
        }
    }

    SkASSERT(fFlushState.nextDrawToken() == fFlushState.nextTokenToFlush());

    for (int i = 0; i < fOpLists1.count(); ++i) {
        fOpLists1[i]->reset1();

        if (1 == fOpLists1[i]->getRefCnt() && fOpLists1[i]->asRenderTargetOpList()) {
            this->recycle(fOpLists1[i]->asRenderTargetOpList());
            fOpLists1[i] = nullptr;
        }
    }

    fOpLists1.reset();

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

sk_sp<GrRenderTargetOpList> GrDrawingManager::newRTOpList(sk_sp<GrRenderTargetProxy> rtp) {
    SkASSERT(fContext);

    // This is  a temporary fix for the partial-MDB world. In that world we're not reordering
    // so ops that (in the single opList world) would've just glommed onto the end of the single
    // opList but referred to a far earlier RT need to appear in their own opList.
    if (!fOpLists1.empty()) {
        fOpLists1.back()->makeClosed(*fContext->caps());
    }

    if (fSpares.count()) {
        sk_sp<GrRenderTargetOpList> tmp(fSpares.top());
        fSpares.pop();
        SkASSERT(tmp->getRefCnt() == 1);
        tmp->init(std::move(rtp), fContext->getGpu(), fContext->getAuditTrail());
        return tmp;
    }

    sk_sp<GrRenderTargetOpList> opList(new GrRenderTargetOpList(rtp,
                                                                fContext->getGpu(),
                                                                fContext->getAuditTrail()));
    SkASSERT(rtp->getLastOpList() == opList.get());

    fOpLists1.push_back() = opList;

    return opList;
}

sk_sp<GrTextureOpList> GrDrawingManager::newTextureOpList(sk_sp<GrTextureProxy> textureProxy) {
    SkASSERT(fContext);

    // This is  a temporary fix for the partial-MDB world. In that world we're not reordering
    // so ops that (in the single opList world) would've just glommed onto the end of the single
    // opList but referred to a far earlier RT need to appear in their own opList.
    if (!fOpLists1.empty()) {
        fOpLists1.back()->makeClosed(*fContext->caps());
    }

    sk_sp<GrTextureOpList> opList(new GrTextureOpList(textureProxy, fContext->getGpu(),
                                                      fContext->getAuditTrail()));

    SkASSERT(textureProxy->getLastOpList() == opList.get());

    fOpLists1.push_back() = opList;

    return opList;
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
