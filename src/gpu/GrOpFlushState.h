/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpFlushState_DEFINED
#define GrOpFlushState_DEFINED

#include "GrAppliedClip.h"
#include "GrDeferredUpload.h"
#include "GrBufferAllocPool.h"
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

    ~GrOpFlushState() final;

    /** Issue a token to an operation that is being enqueued. */
    GrDeferredUploadToken issueDrawToken() {
        return GrDeferredUploadToken(++fLastIssuedToken.fSequenceNumber);
    }

    /** Call every time a draw that was issued a token is flushed */
    void flushToken() { ++fLastFlushedToken.fSequenceNumber; }

    // TODO: SHOULD THIS BE ON GrDeferredUploadTarget.
    /** The last token flushed to all the way to the backend API. */
    GrDeferredUploadToken nextTokenToFlush() const {
        return GrDeferredUploadToken(fLastFlushedToken.fSequenceNumber + 1);
    }

    /** This is called after each op has a chance to prepare its draws and before the draws are
        issued. */
    void preIssueDraws() {
        fVertexPool.unmap();
        fIndexPool.unmap();
        int uploadCount = fAsapUploads.count();

        for (int i = 0; i < uploadCount; i++) {
            this->doUpload(fAsapUploads[i]);
        }
        fAsapUploads.reset();
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

    /** Additional data required on a per-op basis when executing GrDrawOps. */
    struct DrawOpArgs {
        GrRenderTarget* renderTarget() const { return fProxy->priv().peekRenderTarget(); }

        GrOp* fOp;
        // TODO: do we still need the dst proxy here?
        GrRenderTargetProxy* fProxy;
        GrAppliedClip* fAppliedClip;
        GrXferProcessor::DstProxy fDstProxy;
    };

    void setDrawOpArgs(DrawOpArgs* opArgs) { fOpArgs = opArgs; }

    const DrawOpArgs& drawOpArgs() const {
        SkASSERT(fOpArgs);
        SkASSERT(fOpArgs->fOp);
        return *fOpArgs;
    }

    /** Implementation of GrDeferredUploadTarget. */

    GrDeferredUploadToken addInlineUpload(GrDeferredTextureUploadFn&& upload) final {
        SkASSERT(fOpArgs);
        SkASSERT(fOpArgs->fOp);
        // Here we're dangerously relying on only GrDrawOps calling this method.
        auto op = static_cast<GrDrawOp*>(fOpArgs->fOp);
        auto token = this->nextDrawToken();
        GrDrawOp::FlushStateAccess(op).addInlineUpload(std::move(upload), token);
        return token;
    }

    GrDeferredUploadToken addAsapUpload(GrDeferredTextureUploadFn&& upload) final {
        fAsapUploads.emplace_back(std::move(upload));
        return this->nextTokenToFlush();
    }

    bool hasDrawBeenFlushed(GrDeferredUploadToken token) const final {
        return token.fSequenceNumber <= fLastFlushedToken.fSequenceNumber;
    }

    GrDeferredUploadToken nextDrawToken() const final {
        return GrDeferredUploadToken(fLastIssuedToken.fSequenceNumber + 1);
    }

    const GrCaps& caps() const final;
    GrResourceProvider* resourceProvider() const final { return fResourceProvider; }

    /** Implementation of GrMeshDrawOp::Target. */

    void draw(const GrGeometryProcessor*, const GrPipeline*, const GrMesh&) override;
    void* makeVertexSpace(size_t vertexSize, int vertexCount, const GrBuffer**, int* startVertex) override;
    uint16_t* makeIndexSpace(int indexCount, const GrBuffer**, int* startIndex) override;
    void* makeVertexSpaceAtLeast(size_t vertexSize, int minVertexCount, int fallbackVertexCount,
                                 const GrBuffer**, int* startVertex, int* actualVertexCount) override;
    uint16_t* makeIndexSpaceAtLeast(int minIndexCount, int fallbackIndexCount,
                                    const GrBuffer**, int* startIndex,
                                    int* actualIndexCount) override;
    void putBackIndices(int indexCount) override;
    void putBackVertices(int vertices, size_t vertexStride) override;
    GrRenderTargetProxy* proxy() const override { return fOpArgs->fProxy; }
    const GrAppliedClip* clip() const override { return fOpArgs->fAppliedClip; }
    GrAppliedClip detachAppliedClip() override { return std::move(*fOpArgs->fAppliedClip); }
    const GrXferProcessor::DstProxy& dstProxy() const override { return fOpArgs->fDstProxy; }
    GrDeferredUploadTarget* deferredUploadTarget() override { return this; }

private:
    /** GrMeshDrawOp::Target override */
    SkArenaAlloc* pipelineArena() override { return &fPipelines; }

    GrGpu* fGpu;
    GrResourceProvider* fResourceProvider;
    GrGpuCommandBuffer* fCommandBuffer;
    GrVertexBufferAllocPool fVertexPool;
    GrIndexBufferAllocPool fIndexPool;
    SkSTArray<4, GrDeferredTextureUploadFn> fAsapUploads;
    GrDeferredUploadToken fLastIssuedToken;
    GrDeferredUploadToken fLastFlushedToken;
    DrawOpArgs* fOpArgs;
    SkArenaAlloc fPipelines{sizeof(GrPipeline) * 100};
};

#endif
