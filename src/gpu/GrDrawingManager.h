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
#include "GrRenderTargetOpList.h"
#include "GrResourceCache.h"
#include "SkTArray.h"
#include "instanced/InstancedRendering.h"
#include "text/GrAtlasTextContext.h"

class GrContext;
class GrOnFlushCallbackObject;
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

    gr_instanced::OpAllocator* instancingAllocator();

    sk_sp<GrRenderTargetContext> makeRenderTargetContext(sk_sp<GrSurfaceProxy>,
                                                         sk_sp<SkColorSpace>,
                                                         const SkSurfaceProps*,
                                                         bool managedOpList = true);
    sk_sp<GrTextureContext> makeTextureContext(sk_sp<GrSurfaceProxy>, sk_sp<SkColorSpace>);

    // The caller automatically gets a ref on the returned opList. It must
    // be balanced by an unref call.
    // A managed opList is controlled by the drawing manager (i.e., sorted & flushed with the
    // other). An unmanaged one is created and used by the onFlushCallback.
    sk_sp<GrRenderTargetOpList> newRTOpList(GrRenderTargetProxy* rtp, bool managedOpList);
    sk_sp<GrTextureOpList> newTextureOpList(GrTextureProxy* textureProxy);

    GrContext* getContext() { return fContext; }

    GrAtlasTextContext* getAtlasTextContext();

    GrPathRenderer* getPathRenderer(const GrPathRenderer::CanDrawPathArgs& args,
                                    bool allowSW,
                                    GrPathRendererChain::DrawType drawType,
                                    GrPathRenderer::StencilSupport* stencilSupport = NULL);

    void flushIfNecessary() {
        if (fContext->getResourceCache()->requestsFlush()) {
            this->internalFlush(nullptr, GrResourceCache::kCacheRequested);
        }
    }

    static bool ProgramUnitTest(GrContext* context, int maxStages, int maxLevels);

    void prepareSurfaceForExternalIO(GrSurfaceProxy*);

    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);
    void testingOnly_removeOnFlushCallbackObject(GrOnFlushCallbackObject*);

private:
    GrDrawingManager(GrContext* context,
                     const GrPathRendererChain::Options& optionsForPathRendererChain,
                     GrSingleOwner* singleOwner)
        : fContext(context)
        , fOptionsForPathRendererChain(optionsForPathRendererChain)
        , fSingleOwner(singleOwner)
        , fAbandoned(false)
        , fAtlasTextContext(nullptr)
        , fPathRendererChain(nullptr)
        , fSoftwarePathRenderer(nullptr)
        , fFlushState(context->getGpu(), context->resourceProvider())
        , fFlushing(false) {
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
    friend class GrOnFlushResourceProvider; // this is just a shallow wrapper around this class

    static const int kNumPixelGeometries = 5; // The different pixel geometries
    static const int kNumDFTOptions = 2;      // DFT or no DFT

    GrContext*                        fContext;
    GrPathRendererChain::Options      fOptionsForPathRendererChain;

    // In debug builds we guard against improper thread handling
    GrSingleOwner*                    fSingleOwner;

    bool                              fAbandoned;
    SkTArray<sk_sp<GrOpList>>         fOpLists;

    std::unique_ptr<GrAtlasTextContext> fAtlasTextContext;

    GrPathRendererChain*              fPathRendererChain;
    GrSoftwarePathRenderer*           fSoftwarePathRenderer;

    GrOpFlushState                    fFlushState;
    bool                              fFlushing;

    SkTArray<GrOnFlushCallbackObject*> fOnFlushCBObjects;

    // Lazily allocated
    std::unique_ptr<gr_instanced::OpAllocator> fInstancingAllocator;
};

#endif
