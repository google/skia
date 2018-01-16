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
#include "text/GrAtlasTextContext.h"

class GrContext;
class GrCoverageCountingPathRenderer;
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
                                    GrPathRenderer::StencilSupport* stencilSupport = nullptr);

    // Returns a direct pointer to the coverage counting path renderer, or null if it is not
    // supported and turned on.
    GrCoverageCountingPathRenderer* getCoverageCountingPathRenderer();

    void flushIfNecessary() {
        if (fContext->contextPriv().getResourceCache()->requestsFlush()) {
            this->internalFlush(nullptr, GrResourceCache::kCacheRequested, 0, nullptr);
        }
    }

    static bool ProgramUnitTest(GrContext* context, int maxStages, int maxLevels);

    GrSemaphoresSubmitted prepareSurfaceForExternalIO(GrSurfaceProxy*,
                                                      int numSemaphores,
                                                      GrBackendSemaphore backendSemaphores[]);

    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);
    void testingOnly_removeOnFlushCallbackObject(GrOnFlushCallbackObject*);

private:
    GrDrawingManager(GrContext* context,
                     const GrPathRendererChain::Options& optionsForPathRendererChain,
                     const GrAtlasTextContext::Options& optionsForAtlasTextContext,
                     GrSingleOwner* singleOwner)
            : fContext(context)
            , fOptionsForPathRendererChain(optionsForPathRendererChain)
            , fOptionsForAtlasTextContext(optionsForAtlasTextContext)
            , fSingleOwner(singleOwner)
            , fAbandoned(false)
            , fAtlasTextContext(nullptr)
            , fPathRendererChain(nullptr)
            , fSoftwarePathRenderer(nullptr)
            , fFlushState(context->getGpu(), context->contextPriv().resourceProvider())
            , fFlushing(false) {}

    void abandon();
    void cleanup();

    // return true if any opLists were actually executed; false otherwise
    bool executeOpLists(int startIndex, int stopIndex, GrOpFlushState*);

    GrSemaphoresSubmitted flush(GrSurfaceProxy* proxy,
                                int numSemaphores = 0,
                                GrBackendSemaphore backendSemaphores[] = nullptr) {
        return this->internalFlush(proxy, GrResourceCache::FlushType::kExternal,
                                   numSemaphores, backendSemaphores);
    }
    GrSemaphoresSubmitted internalFlush(GrSurfaceProxy*,
                                        GrResourceCache::FlushType,
                                        int numSemaphores,
                                        GrBackendSemaphore backendSemaphores[]);

    friend class GrContext;  // for access to: ctor, abandon, reset & flush
    friend class GrContextPriv; // access to: flush
    friend class GrOnFlushResourceProvider; // this is just a shallow wrapper around this class

    static const int kNumPixelGeometries = 5; // The different pixel geometries
    static const int kNumDFTOptions = 2;      // DFT or no DFT

    GrContext*                        fContext;
    GrPathRendererChain::Options      fOptionsForPathRendererChain;
    GrAtlasTextContext::Options       fOptionsForAtlasTextContext;

    // In debug builds we guard against improper thread handling
    GrSingleOwner*                    fSingleOwner;

    bool                              fAbandoned;
    SkTArray<sk_sp<GrOpList>>         fOpLists;
    // These are the IDs of the opLists currently being flushed (in internalFlush)
    SkSTArray<8, uint32_t, true>      fFlushingOpListIDs;
    // These are the new opLists generated by the onFlush CBs
    SkSTArray<8, sk_sp<GrOpList>>     fOnFlushCBOpLists;

    std::unique_ptr<GrAtlasTextContext> fAtlasTextContext;

    GrPathRendererChain*              fPathRendererChain;
    GrSoftwarePathRenderer*           fSoftwarePathRenderer;

    GrOpFlushState                    fFlushState;
    bool                              fFlushing;

    SkTArray<GrOnFlushCallbackObject*> fOnFlushCBObjects;
};

#endif
