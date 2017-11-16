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
#include "SkArenaAlloc.h"
#include "SkArenaAllocList.h"
#include "ops/GrMeshDrawOp.h"

class GrGpu;
class GrGpuCommandBuffer;
class GrGpuRTCommandBuffer;
class GrResourceProvider;

/** Tracks the state across all the GrOps (really just the GrDrawOps) in a GrOpList flush. */
class GrOpFlushState final : public GrDeferredUploadTarget, public GrMeshDrawOp::Target {
public:
    GrOpFlushState(GrGpu*, GrResourceProvider*);

    ~GrOpFlushState() final { this->reset(); }

    /** This is called after each op has a chance to prepare its draws and before the draws are
        executed. */
    void preExecuteDraws();

    void doUpload(GrDeferredTextureUploadFn&);

    /** Called as ops are executed. Must be called in the same order as the ops were prepared. */
    void executeDrawsAndUploadsForMeshDrawOp(uint32_t opID, const SkRect& opBounds);

    GrGpuCommandBuffer* commandBuffer() { return fCommandBuffer; }
    // Helper function used by Ops that are only called via RenderTargetOpLists
    GrGpuRTCommandBuffer* rtCommandBuffer();
    void setCommandBuffer(GrGpuCommandBuffer* buffer) { fCommandBuffer = buffer; }

    GrGpu* gpu() { return fGpu; }

    void reset();

    /** Additional data required on a per-op basis when executing GrOps. */
    struct OpArgs {
        GrRenderTarget* renderTarget() const { return fProxy->priv().peekRenderTarget(); }

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

    GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&&) final;
    GrDeferredUploadToken addASAPUpload(GrDeferredTextureUploadFn&&) final;

    /** Overrides of GrMeshDrawOp::Target. */

    void draw(const GrGeometryProcessor*, const GrPipeline*, const GrMesh&) final;
    void* makeVertexSpace(size_t vertexSize, int vertexCount, const GrBuffer**,
                          int* startVertex) final;
    uint16_t* makeIndexSpace(int indexCount, const GrBuffer**, int* startIndex) final;
    void* makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount, int fallbackVertexCount,
                                 const GrBuffer**, int* startVertex, int* actualVertexCount) final;
    uint16_t* makeIndexSpaceAtLeast(int minIndexCount, int fallbackIndexCount, const GrBuffer**,
                                    int* startIndex, int* actualIndexCount) final;
    void putBackIndices(int indexCount) final;
    void putBackVertices(int vertices, size_t vertexStride) final;
    GrRenderTargetProxy* proxy() const final { return fOpArgs->fProxy; }
    GrAppliedClip detachAppliedClip() final;
    const GrXferProcessor::DstProxy& dstProxy() const final { return fOpArgs->fDstProxy; }
    GrDeferredUploadTarget* deferredUploadTarget() final { return this; }
    const GrCaps& caps() const final;
    GrResourceProvider* resourceProvider() const final { return fResourceProvider; }

private:
    /** GrMeshDrawOp::Target override. */
    SkArenaAlloc* pipelineArena() override { return &fArena; }

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
        int fMeshCnt = 0;
        GrPendingProgramElement<const GrGeometryProcessor> fGeometryProcessor;
        const GrPipeline* fPipeline;
        uint32_t fOpID;
    };

    // Storage for ops' pipelines, draws, and inline uploads.
    SkArenaAlloc fArena{sizeof(GrPipeline) * 100};

    // Store vertex and index data on behalf of ops that are flushed.
    GrVertexBufferAllocPool fVertexPool;
    GrIndexBufferAllocPool fIndexPool;

    // Data stored on behalf of the ops being flushed.
    SkArenaAllocList<GrDeferredTextureUploadFn> fAsapUploads;
    SkArenaAllocList<InlineUpload> fInlineUploads;
    SkArenaAllocList<Draw> fDraws;
    // TODO: These should go in the arena. However, GrGpuCommandBuffer and other classes currently
    // accept contiguous arrays of meshes.
    SkSTArray<16, GrMesh> fMeshes;

    // All draws we store have an implicit draw token. This is the draw token for the first draw
    // in fDraws.
    GrDeferredUploadToken fBaseDrawToken = GrDeferredUploadToken::AlreadyFlushedToken();

    // Info about the op that is currently preparing or executing using the flush state or null if
    // an op is not currently preparing of executing.
    OpArgs* fOpArgs = nullptr;

    GrGpu* fGpu;
    GrResourceProvider* fResourceProvider;
    GrGpuCommandBuffer* fCommandBuffer = nullptr;

    // Variables that are used to track where we are in lists as ops are executed
    SkArenaAllocList<Draw>::Iter fCurrDraw;
    int fCurrMesh;
    SkArenaAllocList<InlineUpload>::Iter fCurrUpload;
};

#endif
