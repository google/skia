/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawingManager_DEFINED
#define GrDrawingManager_DEFINED

#include "GrBufferAllocPool.h"
#include "GrDeferredUpload.h"
#include "GrPathRenderer.h"
#include "GrPathRendererChain.h"
#include "GrResourceCache.h"
#include "SkSurface.h"
#include "SkTArray.h"
#include "text/GrTextContext.h"

class GrCoverageCountingPathRenderer;
class GrOnFlushCallbackObject;
class GrOpFlushState;
class GrRecordingContext;
class GrRenderTargetContext;
class GrRenderTargetProxy;
class GrRenderTargetOpList;
class GrSoftwarePathRenderer;
class GrTextureContext;
class GrTextureOpList;
class SkDeferredDisplayList;

// The GrDrawingManager allocates a new GrRenderTargetContext for each GrRenderTarget
// but all of them still land in the same GrOpList!
//
// In the future this class will allocate a new GrRenderTargetContext for
// each GrRenderTarget/GrOpList and manage the DAG.
class GrDrawingManager {
public:
    ~GrDrawingManager();

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

    GrRecordingContext* getContext() { return fContext; }

    GrTextContext* getTextContext();

    GrPathRenderer* getPathRenderer(const GrPathRenderer::CanDrawPathArgs& args,
                                    bool allowSW,
                                    GrPathRendererChain::DrawType drawType,
                                    GrPathRenderer::StencilSupport* stencilSupport = nullptr);

    GrPathRenderer* getSoftwarePathRenderer();

    // Returns a direct pointer to the coverage counting path renderer, or null if it is not
    // supported and turned on.
    GrCoverageCountingPathRenderer* getCoverageCountingPathRenderer();

    void flushIfNecessary();

    static bool ProgramUnitTest(GrContext* context, int maxStages, int maxLevels);

    GrSemaphoresSubmitted prepareSurfaceForExternalIO(GrSurfaceProxy*,
                                                      SkSurface::BackendSurfaceAccess access,
                                                      SkSurface::FlushFlags flags,
                                                      int numSemaphores,
                                                      GrBackendSemaphore backendSemaphores[]);

    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);

#if GR_TEST_UTILS
    void testingOnly_removeOnFlushCallbackObject(GrOnFlushCallbackObject*);
#endif

    void moveOpListsToDDL(SkDeferredDisplayList* ddl);
    void copyOpListsFromDDL(const SkDeferredDisplayList*, GrRenderTargetProxy* newDest);

private:
    // This class encapsulates maintenance and manipulation of the drawing manager's DAG of opLists.
    class OpListDAG {
    public:
        OpListDAG(bool explicitlyAllocating, GrContextOptions::Enable sortOpLists);
        ~OpListDAG();

        // Currently, when explicitly allocating resources, this call will topologically sort the
        // opLists.
        // MDB TODO: remove once incremental opList sorting is enabled
        void prepForFlush();

        void closeAll(const GrCaps* caps);

        // A yucky combination of closeAll and reset
        void cleanup(const GrCaps* caps);

        void gatherIDs(SkSTArray<8, uint32_t, true>* idArray) const;

        void reset();

        // These calls forceably remove an opList from the DAG. They are problematic bc they just
        // remove the opList but don't cleanup any refering pointers (i.e., dependency pointers
        // in the DAG). They work right now bc they are only called at flush time, after the
        // topological sort is complete (so the dangling pointers aren't used).
        void removeOpList(int index);
        void removeOpLists(int startIndex, int stopIndex);

        bool empty() const { return fOpLists.empty(); }
        int numOpLists() const { return fOpLists.count(); }

        GrOpList* opList(int index) { return fOpLists[index].get(); }
        const GrOpList* opList(int index) const { return fOpLists[index].get(); }

        GrOpList* back() { return fOpLists.back().get(); }
        const GrOpList* back() const { return fOpLists.back().get(); }

        void add(sk_sp<GrOpList>);
        void add(const SkTArray<sk_sp<GrOpList>>&);

        void swap(SkTArray<sk_sp<GrOpList>>* opLists);

        bool sortingOpLists() const { return fSortOpLists; }

    private:
        SkTArray<sk_sp<GrOpList>> fOpLists;
        bool                      fSortOpLists;
    };

    GrDrawingManager(GrRecordingContext*, const GrPathRendererChain::Options&,
                     const GrTextContext::Options&,
                     bool explicitlyAllocating, GrContextOptions::Enable sortRenderTargets,
                     GrContextOptions::Enable reduceOpListSplitting);

    bool wasAbandoned() const;

    void cleanup();

    // return true if any opLists were actually executed; false otherwise
    bool executeOpLists(int startIndex, int stopIndex, GrOpFlushState*, int* numOpListsExecuted);

    GrSemaphoresSubmitted flush(GrSurfaceProxy* proxy,
                                SkSurface::BackendSurfaceAccess access,
                                SkSurface::FlushFlags flags,
                                int numSemaphores,
                                GrBackendSemaphore backendSemaphores[]);

    SkDEBUGCODE(void validate() const);

    friend class GrContext; // access to: flush & cleanup
    friend class GrContextPriv; // access to: flush
    friend class GrOnFlushResourceProvider; // this is just a shallow wrapper around this class
    friend class GrRecordingContext;  // access to: ctor
    friend class SkImage; // for access to: flush

    static const int kNumPixelGeometries = 5; // The different pixel geometries
    static const int kNumDFTOptions = 2;      // DFT or no DFT

    GrRecordingContext*               fContext;
    GrPathRendererChain::Options      fOptionsForPathRendererChain;
    GrTextContext::Options            fOptionsForTextContext;
    // This cache is used by both the vertex and index pools. It reuses memory across multiple
    // flushes.
    sk_sp<GrBufferAllocPool::CpuBufferCache> fCpuBufferCache;

    OpListDAG                         fDAG;
    GrOpList*                         fActiveOpList = nullptr;
    // These are the IDs of the opLists currently being flushed (in internalFlush)
    SkSTArray<8, uint32_t, true>      fFlushingOpListIDs;
    // These are the new opLists generated by the onFlush CBs
    SkSTArray<8, sk_sp<GrOpList>>     fOnFlushCBOpLists;

    std::unique_ptr<GrTextContext>    fTextContext;

    std::unique_ptr<GrPathRendererChain> fPathRendererChain;
    sk_sp<GrSoftwarePathRenderer>     fSoftwarePathRenderer;

    GrTokenTracker                    fTokenTracker;
    bool                              fFlushing;
    bool                              fReduceOpListSplitting;

    SkTArray<GrOnFlushCallbackObject*> fOnFlushCBObjects;
};

#endif
