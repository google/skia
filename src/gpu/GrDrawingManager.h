/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawingManager_DEFINED
#define GrDrawingManager_DEFINED

#include <set>
#include "include/core/SkSurface.h"
#include "include/private/SkTArray.h"
#include "src/gpu/GrBufferAllocPool.h"
#include "src/gpu/GrDeferredUpload.h"
#include "src/gpu/GrPathRenderer.h"
#include "src/gpu/GrPathRendererChain.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/text/GrTextContext.h"

class GrCoverageCountingPathRenderer;
class GrOnFlushCallbackObject;
class GrOpFlushState;
class GrOpList;
class GrRecordingContext;
class GrRenderTargetContext;
class GrRenderTargetProxy;
class GrRenderTargetOpList;
class GrSoftwarePathRenderer;
class GrTextureContext;
class GrTextureOpList;
class SkDeferredDisplayList;

class GrDrawingManager {
public:
    ~GrDrawingManager();

    void freeGpuResources();

    std::unique_ptr<GrRenderTargetContext> makeRenderTargetContext(sk_sp<GrSurfaceProxy>,
                                                                   GrColorType,
                                                                   sk_sp<SkColorSpace>,
                                                                   const SkSurfaceProps*,
                                                                   bool managedOpList = true);
    std::unique_ptr<GrTextureContext> makeTextureContext(sk_sp<GrSurfaceProxy>,
                                                         GrColorType,
                                                         SkAlphaType,
                                                         sk_sp<SkColorSpace>);

    // A managed opList is controlled by the drawing manager (i.e., sorted & flushed with the
    // others). An unmanaged one is created and used by the onFlushCallback.
    sk_sp<GrRenderTargetOpList> newRTOpList(sk_sp<GrRenderTargetProxy>, bool managedOpList);
    sk_sp<GrTextureOpList> newTextureOpList(sk_sp<GrTextureProxy>);

    // Create a new, specialized, render task that will regenerate mipmap levels and/or resolve
    // MSAA (depending on GrTextureResolveFlags). This method will add the new render task to the
    // list of render tasks and make it depend on the target texture proxy. It is up to the caller
    // to add any dependencies on the new render task.
    GrRenderTask* newTextureResolveRenderTask(
            sk_sp<GrTextureProxy>, GrTextureResolveFlags, const GrCaps&);

    // Create a new render task which copies the pixels from the srcProxy into the dstBuffer. This
    // is used to support the asynchronous readback API. The srcRect is the region of the srcProxy
    // to be copied. The surfaceColorType says how we should interpret the data when reading back
    // from the source. DstColorType describes how the data should be stored in the dstBuffer.
    // DstOffset is the offset into the dstBuffer where we will start writing data.
    void newTransferFromRenderTask(sk_sp<GrSurfaceProxy> srcProxy, const SkIRect& srcRect,
                                   GrColorType surfaceColorType, GrColorType dstColorType,
                                   sk_sp<GrGpuBuffer> dstBuffer, size_t dstOffset);

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

    GrSemaphoresSubmitted flushSurfaces(GrSurfaceProxy* proxies[],
                                        int cnt,
                                        SkSurface::BackendSurfaceAccess access,
                                        const GrFlushInfo& info);
    GrSemaphoresSubmitted flushSurface(GrSurfaceProxy* proxy,
                                       SkSurface::BackendSurfaceAccess access,
                                       const GrFlushInfo& info) {
        return this->flushSurfaces(&proxy, 1, access, info);
    }

    void addOnFlushCallbackObject(GrOnFlushCallbackObject*);

#if GR_TEST_UTILS
    void testingOnly_removeOnFlushCallbackObject(GrOnFlushCallbackObject*);
#endif

    void moveRenderTasksToDDL(SkDeferredDisplayList* ddl);
    void copyRenderTasksFromDDL(const SkDeferredDisplayList*, GrRenderTargetProxy* newDest);

private:
    // This class encapsulates maintenance and manipulation of the drawing manager's DAG of
    // renderTasks.
    class RenderTaskDAG {
    public:
        RenderTaskDAG(bool sortRenderTasks);
        ~RenderTaskDAG();

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
        void removeRenderTask(int index);
        void removeRenderTasks(int startIndex, int stopIndex);

        bool empty() const { return fRenderTasks.empty(); }
        int numRenderTasks() const { return fRenderTasks.count(); }

        bool isUsed(GrSurfaceProxy*) const;

        GrRenderTask* renderTask(int index) { return fRenderTasks[index].get(); }
        const GrRenderTask* renderTask(int index) const { return fRenderTasks[index].get(); }

        GrRenderTask* back() { return fRenderTasks.back().get(); }
        const GrRenderTask* back() const { return fRenderTasks.back().get(); }

        GrRenderTask* add(sk_sp<GrRenderTask>);
        GrRenderTask* addBeforeLast(sk_sp<GrRenderTask>);
        void add(const SkTArray<sk_sp<GrRenderTask>>&);

        void swap(SkTArray<sk_sp<GrRenderTask>>* renderTasks);

        bool sortingRenderTasks() const { return fSortRenderTasks; }

    private:
        SkTArray<sk_sp<GrRenderTask>> fRenderTasks;
        bool                          fSortRenderTasks;
    };

    GrDrawingManager(GrRecordingContext*, const GrPathRendererChain::Options&,
                     const GrTextContext::Options&,
                     bool sortRenderTasks,
                     bool reduceOpListSplitting);

    bool wasAbandoned() const;

    void cleanup();

    // Closes the target's dependent render tasks (or, if not in sorting/opList-splitting-reduction
    // mode, closes fActiveOpList) in preparation for us opening a new opList that will write to
    // 'target'.
    void closeRenderTasksForNewRenderTask(GrSurfaceProxy* target);

    // return true if any opLists were actually executed; false otherwise
    bool executeRenderTasks(int startIndex, int stopIndex, GrOpFlushState*,
                            int* numRenderTasksExecuted);

    GrSemaphoresSubmitted flush(GrSurfaceProxy* proxies[],
                                int numProxies,
                                SkSurface::BackendSurfaceAccess access,
                                const GrFlushInfo&,
                                const GrPrepareForExternalIORequests&);

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

    RenderTaskDAG                     fDAG;
    GrOpList*                         fActiveOpList = nullptr;
    // These are the IDs of the opLists currently being flushed (in internalFlush)
    SkSTArray<8, uint32_t, true>      fFlushingRenderTaskIDs;
    // These are the new opLists generated by the onFlush CBs
    SkSTArray<8, sk_sp<GrOpList>>     fOnFlushCBOpLists;

    std::unique_ptr<GrTextContext>    fTextContext;

    std::unique_ptr<GrPathRendererChain> fPathRendererChain;
    sk_sp<GrSoftwarePathRenderer>     fSoftwarePathRenderer;

    GrTokenTracker                    fTokenTracker;
    bool                              fFlushing;
    bool                              fReduceOpListSplitting;

    SkTArray<GrOnFlushCallbackObject*> fOnFlushCBObjects;

    void addDDLTarget(GrSurfaceProxy* proxy) { fDDLTargets.insert(proxy); }
    bool isDDLTarget(GrSurfaceProxy* proxy) { return fDDLTargets.find(proxy) != fDDLTargets.end(); }
    void clearDDLTargets() { fDDLTargets.clear(); }

    // We play a trick with lazy proxies to retarget the base target of a DDL to the SkSurface
    // it is replayed on. Because of this remapping we need to explicitly store the targets of
    // DDL replaying.
    // Note: we do not expect a whole lot of these per flush
    std::set<GrSurfaceProxy*> fDDLTargets;
};

#endif
