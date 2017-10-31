/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpFlushState_DEFINED
#define GrOpFlushState_DEFINED

#include "GrAppliedClip.h"
#include "GrBufferAllocPool.h"
#include "GrDeferredUpload.h"
#include "SkArenaAlloc.h"
#include "ops/GrMeshDrawOp.h"

class GrGpu;
class GrGpuCommandBuffer;
class GrGpuRTCommandBuffer;
class GrResourceProvider;

// TODO: Store uploads on GrOpFlushState rather than GrDrawOp and remove this.
class GrDrawOp::FlushStateAccess {
private:
    friend class GrOpFlushState;

    explicit FlushStateAccess(GrDrawOp* op) : fOp(op) {}

    void addInlineUpload(GrDeferredTextureUploadFn&& upload, GrDeferredUploadToken token) {
        fOp->fInlineUploads.emplace_back(std::move(upload), token);
    }

    GrDrawOp* fOp;
};

// TODO: Store draw related data on GrOpFlushState rather than GrMeshDrawOp and remove this.
class GrMeshDrawOp::FlushStateAccess {
private:
    friend class GrOpFlushState;
    using QueuedDraw = GrMeshDrawOp::QueuedDraw;

    explicit FlushStateAccess(GrMeshDrawOp* op) : fOp(op) {}

    void addMesh(const GrMesh& mesh) { fOp->fMeshes.push_back(mesh); }

    QueuedDraw* lastDraw() {
        return fOp->fQueuedDraws.empty() ? nullptr : &fOp->fQueuedDraws.back();
    }

    QueuedDraw* addDraw() { return &fOp->fQueuedDraws.push_back(); }

    GrDeferredUploadToken lastUploadToken() const {
        if (fOp->fInlineUploads.empty()) {
            return GrDeferredUploadToken::AlreadyFlushedToken();
        }
        return fOp->fInlineUploads.back().fUploadBeforeToken;
    }

    void setBaseDrawToken(GrDeferredUploadToken token) { fOp->fBaseDrawToken = token; }

    GrMeshDrawOp* fOp;
};

/** Tracks the state across all the GrOps (really just the GrDrawOps) in a GrOpList flush. */
class GrOpFlushState final : public GrDeferredUploadTarget, public GrMeshDrawOp::Target {
public:
    GrOpFlushState(GrGpu*, GrResourceProvider*);

    ~GrOpFlushState() final { this->reset(); }

    /** This is called after each op has a chance to prepare its draws and before the draws are
        issued. */
    void preIssueDraws() {
        fVertexPool.unmap();
        fIndexPool.unmap();
        int uploadCount = fASAPUploads.count();

        for (int i = 0; i < uploadCount; i++) {
            this->doUpload(fASAPUploads[i]);
        }
        fASAPUploads.reset();
    }

    void doUpload(GrDeferredTextureUploadFn&);

    GrGpuCommandBuffer* commandBuffer() { return fCommandBuffer; }
    // Helper function used by Ops that are only called via RenderTargetOpLists
    GrGpuRTCommandBuffer* rtCommandBuffer();
    void setCommandBuffer(GrGpuCommandBuffer* buffer) { fCommandBuffer = buffer; }

    GrGpu* gpu() { return fGpu; }

    void reset() {
        fVertexPool.reset();
        fIndexPool.reset();
        fPipelines.reset();
    }

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

    /** Expose base class methods for incrementing the last flushed and next draw token. */

    void flushToken() { this->GrDeferredUploadTarget::flushToken(); }

    GrDeferredUploadToken issueDrawToken() {
        return this->GrDeferredUploadTarget::issueDrawToken();
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
    SkArenaAlloc* pipelineArena() override { return &fPipelines; }

    GrGpu* fGpu;
    GrResourceProvider* fResourceProvider;
    GrGpuCommandBuffer* fCommandBuffer;
    GrVertexBufferAllocPool fVertexPool;
    GrIndexBufferAllocPool fIndexPool;
    SkSTArray<4, GrDeferredTextureUploadFn> fASAPUploads;
    OpArgs* fOpArgs;
    SkArenaAlloc fPipelines{sizeof(GrPipeline) * 100};
};

#endif
