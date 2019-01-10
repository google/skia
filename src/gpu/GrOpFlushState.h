/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpFlushState_DEFINED
#define GrOpFlushState_DEFINED

#include <utility>
#include "GrAppliedClip.h"
#include "GrBufferAllocPool.h"
#include "GrDeferredUpload.h"
#include "GrDeinstantiateProxyTracker.h"
#include "SkArenaAlloc.h"
#include "SkArenaAllocList.h"
#include "ops/GrMeshDrawOp.h"

class GrGpu;
class GrGpuCommandBuffer;
class GrGpuRTCommandBuffer;
class GrResourceProvider;

/** Tracks the state across all the GrOps (really just the GrDrawOps) in a GrOpList flush. */
class GrOpFlushState final : public GrDeferredUploadTarget {
public:
    // vertexSpace and indexSpace may either be null or an alloation of size
    // GrBufferAllocPool::kDefaultBufferSize. If the latter, then CPU memory is only allocated for
    // vertices/indices when a buffer larger than kDefaultBufferSize is required.
    GrOpFlushState(GrGpu*, GrResourceProvider*, GrTokenTracker*, void* vertexSpace,
                   void* indexSpace);

    ~GrOpFlushState() final { this->reset(); }

    /** This is called after each op has a chance to prepare its draws and before the draws are
        executed. */
    void preExecuteDraws();

    void doUpload(GrDeferredTextureUploadFn&);

    /** Called as ops are executed. Must be called in the same order as the ops were prepared. */
    void executeDrawsAndUploadsForMeshDrawOp(const GrOp* op, const SkRect& opBounds);

    GrGpuCommandBuffer* commandBuffer() { return fCommandBuffer; }
    // Helper function used by Ops that are only called via RenderTargetOpLists
    GrGpuRTCommandBuffer* rtCommandBuffer();
    void setCommandBuffer(GrGpuCommandBuffer* buffer) { fCommandBuffer = buffer; }

    GrGpu* gpu() { return fGpu; }

    void reset();

    /** Additional data required on a per-op basis when executing GrOps. */
    struct OpArgs {
        GrRenderTarget* renderTarget() const { return fProxy->peekRenderTarget(); }

        GrOp* fOp;
        // TODO: do we still need the dst proxy here?
        GrRenderTargetProxy* fProxy;
        GrAppliedClip* fAppliedClip;
        GrXferProcessor::DstProxy fDstProxy;
    };

    void setOpArgs(OpArgs* opArgs) { fOpArgs = opArgs; }

    const OpArgs& drawOpArgs() const {
        SkASSERT(fOpArgs);
        SkASSERT(fOpArgs->fOp);
        return *fOpArgs;
    }

    /** Overrides of GrDeferredUploadTarget. */

    const GrTokenTracker* tokenTracker() final { return fTokenTracker; }
    GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&&) final;
    GrDeferredUploadToken addASAPUpload(GrDeferredTextureUploadFn&&) final;

    void draw(sk_sp<const GrGeometryProcessor>,
              const GrPipeline*,
              const GrPipeline::FixedDynamicState*,
              const GrPipeline::DynamicStateArrays*,
              const GrMesh[],
              int meshCnt);

    /** Helper for drawing a single GrMesh. */
    void draw(sk_sp<const GrGeometryProcessor> gp,
              const GrPipeline* pipeline,
              const GrPipeline::FixedDynamicState* fixedDynamicState,
              const GrMesh* mesh) {
        this->draw(std::move(gp), pipeline, fixedDynamicState, nullptr, mesh, 1);
    }

    /**
     * Allocate space for a pipeline. The target ensures this pipeline lifetime is at least
     * as long as any deferred execution of draws added via draw().
     * @tparam Args
     * @param args
     * @return
     */
    template <typename... Args>
    GrPipeline* allocPipeline(Args&&... args) {
        return fArena.make<GrPipeline>(std::forward<Args>(args)...);
    }

    GrMesh* allocMesh(GrPrimitiveType primitiveType) {
        return fArena.make<GrMesh>(primitiveType);
    }

    GrMesh* allocMeshes(int n) { return fArena.makeArray<GrMesh>(n); }

    GrPipeline::FixedDynamicState* allocFixedDynamicState(const SkIRect& rect,
                                                          int numPrimitiveProcessorTextures = 0);

    GrPipeline::DynamicStateArrays* allocDynamicStateArrays(int numMeshes,
                                                            int numPrimitiveProcessorTextures,
                                                            bool allocScissors);

    GrTextureProxy** allocPrimitiveProcessorTextureArray(int n) {
        SkASSERT(n > 0);
        return fArena.makeArrayDefault<GrTextureProxy*>(n);
    }

    // Once we have C++17 structured bindings make this just be a tuple because then we can do:
    //      auto [pipeline, fixedDynamicState] = target->makePipeline(...);
    // in addition to:
    //      std::tie(flushInfo.fPipeline, flushInfo.fFixedState) = target->makePipeline(...);
    struct PipelineAndFixedDynamicState {
        const GrPipeline* fPipeline;
        GrPipeline::FixedDynamicState* fFixedDynamicState;
    };

    /**
     * Helper that makes a pipeline targeting the op's render target that incorporates the op's
     * GrAppliedClip and uses a fixed dynamic state.
     */
    PipelineAndFixedDynamicState makePipeline(uint32_t pipelineFlags, GrProcessorSet&&,
                                              GrAppliedClip&&,
                                              int numPrimitiveProcessorTextures = 0);

    void* makeVertexSpace(size_t vertexSize, int vertexCount, const GrBuffer**,
                          int* startVertex);
    uint16_t* makeIndexSpace(int indexCount, const GrBuffer**, int* startIndex);
    void* makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount, int fallbackVertexCount,
                                 const GrBuffer**, int* startVertex, int* actualVertexCount);
    uint16_t* makeIndexSpaceAtLeast(int minIndexCount, int fallbackIndexCount, const GrBuffer**,
                                    int* startIndex, int* actualIndexCount);
    void putBackIndices(int indexCount);
    void putBackVertices(int vertices, size_t vertexStride);
    GrRenderTargetProxy* proxy() const { return fOpArgs->fProxy; }
    GrAppliedClip detachAppliedClip();
    const GrXferProcessor::DstProxy& dstProxy() const { return fOpArgs->fDstProxy; }
    GrDeferredUploadTarget* deferredUploadTarget() { return this; }
    const GrCaps& caps() const;
    GrResourceProvider* resourceProvider() const { return fResourceProvider; }
    uint32_t contextUniqueID() const { return this->resourceProvider()->contextUniqueID(); }

    GrGlyphCache* glyphCache() const;

    // At this point we know we're flushing so full access to the GrAtlasManager is required (and
    // permissible).
    GrAtlasManager* atlasManager() const;

    GrDeinstantiateProxyTracker* deinstantiateProxyTracker() { return &fDeinstantiateProxyTracker; }

private:
    struct InlineUpload {
        InlineUpload(GrDeferredTextureUploadFn&& upload, GrDeferredUploadToken token)
                : fUpload(std::move(upload)), fUploadBeforeToken(token) {}
        GrDeferredTextureUploadFn fUpload;
        GrDeferredUploadToken fUploadBeforeToken;
    };

    // A set of contiguous draws that share a draw token, geometry processor, and pipeline. The
    // meshes for the draw are stored in the fMeshes array. The reason for coalescing meshes
    // that share a geometry processor into a Draw is that it allows the Gpu object to setup
    // the shared state once and then issue draws for each mesh.
    struct Draw {
        ~Draw();
        sk_sp<const GrGeometryProcessor> fGeometryProcessor;
        const GrPipeline* fPipeline = nullptr;
        const GrPipeline::FixedDynamicState* fFixedDynamicState;
        const GrPipeline::DynamicStateArrays* fDynamicStateArrays;
        const GrMesh* fMeshes = nullptr;
        const GrOp* fOp = nullptr;
        int fMeshCnt = 0;
    };

    // Storage for ops' pipelines, draws, and inline uploads.
    SkArenaAlloc fArena{sizeof(GrPipeline) * 100};

    // Store vertex and index data on behalf of ops that are flushed.
    GrVertexBufferAllocPool fVertexPool;
    GrIndexBufferAllocPool fIndexPool;

    // Data stored on behalf of the ops being flushed.
    SkArenaAllocList<GrDeferredTextureUploadFn> fASAPUploads;
    SkArenaAllocList<InlineUpload> fInlineUploads;
    SkArenaAllocList<Draw> fDraws;

    // All draws we store have an implicit draw token. This is the draw token for the first draw
    // in fDraws.
    GrDeferredUploadToken fBaseDrawToken = GrDeferredUploadToken::AlreadyFlushedToken();

    // Info about the op that is currently preparing or executing using the flush state or null if
    // an op is not currently preparing of executing.
    OpArgs* fOpArgs = nullptr;

    GrGpu* fGpu;
    GrResourceProvider* fResourceProvider;
    GrTokenTracker* fTokenTracker;
    GrGpuCommandBuffer* fCommandBuffer = nullptr;

    // Variables that are used to track where we are in lists as ops are executed
    SkArenaAllocList<Draw>::Iter fCurrDraw;
    SkArenaAllocList<InlineUpload>::Iter fCurrUpload;

    // Used to track the proxies that need to be deinstantiated after we finish a flush
    GrDeinstantiateProxyTracker fDeinstantiateProxyTracker;
};

#endif
