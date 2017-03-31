/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOpFlushState_DEFINED
#define GrOpFlushState_DEFINED

#include "GrBufferAllocPool.h"
#include "GrGpu.h"
#include "ops/GrMeshDrawOp.h"

class GrGpuCommandBuffer;
class GrResourceProvider;

/** Tracks the state across all the GrOps (really just the GrDrawOps) in a GrOpList flush. */
class GrOpFlushState {
public:
    GrOpFlushState(GrGpu*, GrResourceProvider*);

    ~GrOpFlushState() { this->reset(); }

    /** Inserts an upload to be executed after all ops in the flush prepared their draws but before
        the draws are executed to the backend 3D API. */
    void addASAPUpload(GrDrawOp::DeferredUploadFn&& upload) {
        fAsapUploads.emplace_back(std::move(upload));
    }

    const GrCaps& caps() const { return *fGpu->caps(); }
    GrResourceProvider* resourceProvider() const { return fResourceProvider; }

    /** Has the token been flushed to the backend 3D API. */
    bool hasDrawBeenFlushed(GrDrawOpUploadToken token) const {
        return token.fSequenceNumber <= fLastFlushedToken.fSequenceNumber;
    }

    /** Issue a token to an operation that is being enqueued. */
    GrDrawOpUploadToken issueDrawToken() {
        return GrDrawOpUploadToken(++fLastIssuedToken.fSequenceNumber);
    }

    /** Call every time a draw that was issued a token is flushed */
    void flushToken() { ++fLastFlushedToken.fSequenceNumber; }

    /** Gets the next draw token that will be issued. */
    GrDrawOpUploadToken nextDrawToken() const {
        return GrDrawOpUploadToken(fLastIssuedToken.fSequenceNumber + 1);
    }

    /** The last token flushed to all the way to the backend API. */
    GrDrawOpUploadToken nextTokenToFlush() const {
        return GrDrawOpUploadToken(fLastFlushedToken.fSequenceNumber + 1);
    }

    void* makeVertexSpace(size_t vertexSize, int vertexCount,
                          const GrBuffer** buffer, int* startVertex);
    uint16_t* makeIndexSpace(int indexCount, const GrBuffer** buffer, int* startIndex);

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

    void doUpload(GrDrawOp::DeferredUploadFn& upload) {
        GrDrawOp::WritePixelsFn wp = [this] (GrSurface* surface,
                int left, int top, int width, int height,
                GrPixelConfig config, const void* buffer,
                size_t rowBytes) -> bool {
            return this->fGpu->writePixels(surface, left, top, width, height, config, buffer,
                                           rowBytes);
        };
        upload(wp);
    }

    void putBackIndices(size_t indices) { fIndexPool.putBack(indices * sizeof(uint16_t)); }

    void putBackVertexSpace(size_t sizeInBytes) { fVertexPool.putBack(sizeInBytes); }

    GrGpuCommandBuffer* commandBuffer() { return fCommandBuffer; }
    void setCommandBuffer(GrGpuCommandBuffer* buffer) { fCommandBuffer = buffer; }

    GrGpu* gpu() { return fGpu; }

    void reset() {
        fVertexPool.reset();
        fIndexPool.reset();
    }

    /** Additional data required on a per-op basis when executing GrDrawOps. */
    struct DrawOpArgs {
        GrRenderTarget* fRenderTarget;
        const GrAppliedClip* fAppliedClip;
        GrXferProcessor::DstTexture fDstTexture;
    };

    void setDrawOpArgs(DrawOpArgs* opArgs) { fOpArgs = opArgs; }

    const DrawOpArgs& drawOpArgs() const {
        SkASSERT(fOpArgs);
        return *fOpArgs;
    }

private:
    GrGpu*                                      fGpu;
    GrResourceProvider*                         fResourceProvider;
    GrGpuCommandBuffer*                         fCommandBuffer;
    GrVertexBufferAllocPool                     fVertexPool;
    GrIndexBufferAllocPool                      fIndexPool;
    SkSTArray<4, GrDrawOp::DeferredUploadFn>    fAsapUploads;
    GrDrawOpUploadToken                         fLastIssuedToken;
    GrDrawOpUploadToken                         fLastFlushedToken;
    DrawOpArgs*                                 fOpArgs;
};

/**
 * A word about uploads and tokens: Ops should usually schedule their uploads to occur at the
 * begining of a frame whenever possible. These are called ASAP uploads. Of course, this requires
 * that there are no draws that have yet to be flushed that rely on the old texture contents. In
 * that case the ASAP upload would happen prior to the previous draw causing the draw to read the
 * new (wrong) texture data. In that case they should schedule an inline upload.
 *
 * Ops, in conjunction with helpers such as GrDrawOpAtlas, can use the token system to know
 * what the most recent draw was that referenced a resource (or portion of a resource). Each draw
 * is assigned a token. A resource (or portion) can be tagged with the most recent draw's
 * token. The target provides a facility for testing whether the draw corresponding to the token
 * has been flushed. If it has not been flushed then the op must perform an inline upload instead.
 * When scheduling an inline upload the op provides the token of the draw that the upload must occur
 * before. The upload will then occur between the draw that requires the new data but after the
 * token that requires the old data.
 *
 * TODO: Currently the token/upload interface is spread over GrDrawOp, GrMeshDrawOp,
 * GrDrawOp::Target, and GrMeshDrawOp::Target. However, the interface at the GrDrawOp level is not
 * complete and isn't useful. We should push it down to GrMeshDrawOp until it is required at the
 * GrDrawOp level.
 */

/**
 * GrDrawOp instances use this object to allocate space for their geometry and to issue the draws
 * that render their op.
 */
class GrDrawOp::Target {
public:
    Target(GrOpFlushState* state, GrDrawOp* op) : fState(state), fOp(op) {}

    /** Returns the token of the draw that this upload will occur before. */
    GrDrawOpUploadToken addInlineUpload(DeferredUploadFn&& upload) {
        fOp->fInlineUploads.emplace_back(std::move(upload), fState->nextDrawToken());
        return fOp->fInlineUploads.back().fUploadBeforeToken;
    }

    /** Returns the token of the draw that this upload will occur before. Since ASAP uploads
        are done first during a flush, this will be the first token since the most recent
        flush. */
    GrDrawOpUploadToken addAsapUpload(DeferredUploadFn&& upload) {
        fState->addASAPUpload(std::move(upload));
        return fState->nextTokenToFlush();
    }

    bool hasDrawBeenFlushed(GrDrawOpUploadToken token) const {
        return fState->hasDrawBeenFlushed(token);
    }

    /** Gets the next draw token that will be issued by this target. This can be used by an op
        to record that the next draw it issues will use a resource (e.g. texture) while preparing
        that draw. */
    GrDrawOpUploadToken nextDrawToken() const { return fState->nextDrawToken(); }

    const GrCaps& caps() const { return fState->caps(); }

    GrResourceProvider* resourceProvider() const { return fState->resourceProvider(); }

protected:
    GrDrawOp* op() { return fOp; }
    GrOpFlushState* state() { return fState; }

private:
    GrOpFlushState* fState;
    GrDrawOp* fOp;
};

/** Extension of GrDrawOp::Target for use by GrMeshDrawOp. Adds the ability to create vertex
    draws. */
class GrMeshDrawOp::Target : public GrDrawOp::Target {
public:
    Target(GrOpFlushState* state, GrMeshDrawOp* op) : INHERITED(state, op) {}

    void draw(const GrGeometryProcessor* gp, const GrPipeline* pipeline, const GrMesh& mesh);

    void* makeVertexSpace(size_t vertexSize, int vertexCount,
                          const GrBuffer** buffer, int* startVertex) {
        return this->state()->makeVertexSpace(vertexSize, vertexCount, buffer, startVertex);
    }

    uint16_t* makeIndexSpace(int indexCount, const GrBuffer** buffer, int* startIndex) {
        return this->state()->makeIndexSpace(indexCount, buffer, startIndex);
    }

    /** Helpers for ops which over-allocate and then return data to the pool. */
    void putBackIndices(int indices) { this->state()->putBackIndices(indices); }
    void putBackVertices(int vertices, size_t vertexStride) {
        this->state()->putBackVertexSpace(vertices * vertexStride);
    }

private:
    GrMeshDrawOp* meshDrawOp() { return static_cast<GrMeshDrawOp*>(this->op()); }
    typedef GrDrawOp::Target INHERITED;
};

#endif
