/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawingManager_DEFINED
#define GrDrawingManager_DEFINED

#include "GrOpFlushState.h"
#include "GrPathRenderer.h"
#include "GrPathRendererChain.h"
#include "GrPreFlushResourceProvider.h"
#include "GrRenderTargetOpList.h"
#include "GrResourceCache.h"
#include "SkTArray.h"
#include "text/GrAtlasTextContext.h"

class GrContext;
class GrRenderTargetContext;
class GrRenderTargetProxy;
class GrSingleOWner;
class GrSoftwarePathRenderer;
class GrTextureContext;
class GrTextureOpList;

// The GrDrawingManager allocates a new GrRenderTargetContext for each GrRenderTarget
// but all of them still land in the same GrOpList!
//
// In the future this class will allocate a new GrRenderTargetContext for
// each GrRenderTarget/GrOpList and manage the DAG.
class GrDrawingManager {
public:
    ~GrDrawingManager();

    bool wasAbandoned() const { return fAbandoned; }
    void freeGpuResources();

    sk_sp<GrRenderTargetContext> makeRenderTargetContext(sk_sp<GrSurfaceProxy>,
                                                         sk_sp<SkColorSpace>,
                                                         const SkSurfaceProps*);
    sk_sp<GrTextureContext> makeTextureContext(sk_sp<GrSurfaceProxy>, sk_sp<SkColorSpace>);

    // The caller automatically gets a ref on the returned opList. It must
    // be balanced by an unref call.
    GrRenderTargetOpList* newOpList(GrRenderTargetProxy* rtp);
    GrTextureOpList* newOpList(GrTextureProxy* textureProxy);

    GrContext* getContext() { return fContext; }

    GrAtlasTextContext* getAtlasTextContext();

    GrPathRenderer* getPathRenderer(const GrPathRenderer::CanDrawPathArgs& args,
                                    bool allowSW,
                                    GrPathRendererChain::DrawType drawType,
                                    GrPathRenderer::StencilSupport* stencilSupport = NULL);

    void flushIfNecessary() {
        if (fContext->getResourceCache()->requestsFlush()) {
            this->internalFlush(nullptr, GrResourceCache::kCacheRequested);
        } else if (fIsImmediateMode) {
            this->internalFlush(nullptr, GrResourceCache::kImmediateMode);
        }
    }

    static bool ProgramUnitTest(GrContext* context, int maxStages);

    void prepareSurfaceForExternalIO(GrSurfaceProxy*);

    void addPreFlushCallbackObject(sk_sp<GrPreFlushCallbackObject> preFlushCBObject);

private:
    GrDrawingManager(GrContext* context,
                     const GrRenderTargetOpList::Options& optionsForOpLists,
                     const GrPathRendererChain::Options& optionsForPathRendererChain,
                     bool isImmediateMode, GrSingleOwner* singleOwner)
        : fContext(context)
        , fOptionsForOpLists(optionsForOpLists)
        , fOptionsForPathRendererChain(optionsForPathRendererChain)
        , fSingleOwner(singleOwner)
        , fAbandoned(false)
        , fAtlasTextContext(nullptr)
        , fPathRendererChain(nullptr)
        , fSoftwarePathRenderer(nullptr)
        , fFlushState(context->getGpu(), context->resourceProvider())
        , fFlushing(false)
        , fIsImmediateMode(isImmediateMode) {
    }

    void abandon();
    void cleanup();
    void reset();
    void flush(GrSurfaceProxy* proxy) {
        this->internalFlush(proxy, GrResourceCache::FlushType::kExternal);
    }
    void internalFlush(GrSurfaceProxy*, GrResourceCache::FlushType);

    friend class GrContext;  // for access to: ctor, abandon, reset & flush
    friend class GrContextPriv; // access to: flush
    friend class GrPreFlushResourceProvider; // this is just a shallow wrapper around this class

    static const int kNumPixelGeometries = 5; // The different pixel geometries
    static const int kNumDFTOptions = 2;      // DFT or no DFT

    GrContext*                        fContext;
    GrRenderTargetOpList::Options     fOptionsForOpLists;
    GrPathRendererChain::Options      fOptionsForPathRendererChain;

    // In debug builds we guard against improper thread handling
    GrSingleOwner*                    fSingleOwner;

    bool                              fAbandoned;
    SkTDArray<GrOpList*>              fOpLists;

    std::unique_ptr<GrAtlasTextContext> fAtlasTextContext;

    GrPathRendererChain*              fPathRendererChain;
    GrSoftwarePathRenderer*           fSoftwarePathRenderer;

    GrOpFlushState                    fFlushState;
    bool                              fFlushing;

    bool                              fIsImmediateMode;

    SkTArray<sk_sp<GrPreFlushCallbackObject>> fPreFlushCBObjects;
};

#endif
