/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBatchBuffer_DEFINED
#define GrBatchBuffer_DEFINED

#include "GrBufferAllocPool.h"
#include "batches/GrVertexBatch.h"

class GrResourceProvider;

/** Simple class that performs the upload on behalf of a GrBatchUploader. */
class GrBatchUploader::TextureUploader {
public:
    TextureUploader(GrGpu* gpu) : fGpu(gpu) { SkASSERT(gpu); }

    /**
        * Updates the pixels in a rectangle of a texture.
        *
        * @param left          left edge of the rectangle to write (inclusive)
        * @param top           top edge of the rectangle to write (inclusive)
        * @param width         width of rectangle to write in pixels.
        * @param height        height of rectangle to write in pixels.
        * @param config        the pixel config of the source buffer
        * @param buffer        memory to read pixels from
        * @param rowBytes      number of bytes between consecutive rows. Zero
        *                      means rows are tightly packed.
        */
    bool writeTexturePixels(GrTexture* texture,
                            int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t rowBytes) {
        return fGpu->writePixels(texture, left, top, width, height, config, buffer, rowBytes);
    }

private:
    GrGpu* fGpu;
};

/** Tracks the state across all the GrBatches in a GrDrawTarget flush. */
class GrBatchFlushState {
public:
    GrBatchFlushState(GrGpu*, GrResourceProvider*);

    ~GrBatchFlushState() { this->reset(); }

    void advanceToken() { ++fCurrentToken; }

    void advanceLastFlushedToken() { ++fLastFlushedToken; }

    /** Inserts an upload to be executred after all batches in the flush prepared their draws
        but before the draws are executed to the backend 3D API. */
    void addASAPUpload(GrBatchUploader* upload) {
        fAsapUploads.push_back().reset(SkRef(upload));
    }

    const GrCaps& caps() const { return *fGpu->caps(); }
    GrResourceProvider* resourceProvider() const { return fResourceProvider; }

    /** Has the token been flushed to the backend 3D API. */
    bool hasTokenBeenFlushed(GrBatchToken token) const { return fLastFlushedToken >= token; }

    /** The current token advances once for every contiguous set of uninterrupted draws prepared
        by a batch. */
    GrBatchToken currentToken() const { return fCurrentToken; }

    /** The last token flushed to all the way to the backend API. */
    GrBatchToken lastFlushedToken() const { return fLastFlushedToken; }

    /** This is a magic token that can be used to indicate that an upload should occur before
        any draws for any batch in the current flush execute. */
    GrBatchToken asapToken() const { return fLastFlushedToken + 1; }

    void* makeVertexSpace(size_t vertexSize, int vertexCount,
                          const GrVertexBuffer** buffer, int* startVertex);
    uint16_t* makeIndexSpace(int indexCount, const GrIndexBuffer** buffer, int* startIndex);

    /** This is called after each batch has a chance to prepare its draws and before the draws
        are issued. */
    void preIssueDraws() {
        fVertexPool.unmap();
        fIndexPool.unmap();
        int uploadCount = fAsapUploads.count();
        for (int i = 0; i < uploadCount; i++) {
            fAsapUploads[i]->upload(&fUploader);
        }
        fAsapUploads.reset();
    }

    void putBackIndices(size_t indices) { fIndexPool.putBack(indices * sizeof(uint16_t)); }

    void putBackVertexSpace(size_t sizeInBytes) { fVertexPool.putBack(sizeInBytes); }

    GrBatchUploader::TextureUploader* uploader() { return &fUploader; }

    GrGpu* gpu() { return fGpu; }

    void reset() {
        fVertexPool.reset();
        fIndexPool.reset();
    }

private:
    GrGpu*                                          fGpu;
    GrBatchUploader::TextureUploader                fUploader;

    GrResourceProvider*                             fResourceProvider;

    GrVertexBufferAllocPool                         fVertexPool;
    GrIndexBufferAllocPool                          fIndexPool;

    SkTArray<SkAutoTUnref<GrBatchUploader>, true>   fAsapUploads;

    GrBatchToken                                    fCurrentToken;

    GrBatchToken                                    fLastFlushedToken;
};

/**
 * GrDrawBatch instances use this object to allocate space for their geometry and to issue the draws
 * that render their batch.
 */
class GrDrawBatch::Target {
public:
    Target(GrBatchFlushState* state, GrDrawBatch* batch) : fState(state), fBatch(batch) {}

    void upload(GrBatchUploader* upload) {
        if (this->asapToken() == upload->lastUploadToken()) {
            fState->addASAPUpload(upload);
        } else {
            fBatch->fInlineUploads.push_back().reset(SkRef(upload));
        }
    }

    bool hasTokenBeenFlushed(GrBatchToken token) const {
        return fState->hasTokenBeenFlushed(token);
    }
    GrBatchToken currentToken() const { return fState->currentToken(); }
    GrBatchToken asapToken() const { return fState->asapToken(); }

    const GrCaps& caps() const { return fState->caps(); }

    GrResourceProvider* resourceProvider() const { return fState->resourceProvider(); }

protected:
    GrDrawBatch* batch() { return fBatch; }
    GrBatchFlushState* state() { return fState; }

private:
    GrBatchFlushState*  fState;
    GrDrawBatch*        fBatch;
};

/** Extension of GrDrawBatch::Target for use by GrVertexBatch. Adds the ability to create vertex
    draws. */
class GrVertexBatch::Target : public GrDrawBatch::Target {
public:
    Target(GrBatchFlushState* state, GrVertexBatch* batch) : INHERITED(state, batch) {}

    void initDraw(const GrPrimitiveProcessor* primProc, const GrPipeline* pipeline) {
        GrVertexBatch::DrawArray* draws = this->vertexBatch()->fDrawArrays.addToTail();
        draws->fPrimitiveProcessor.reset(primProc);
        this->state()->advanceToken();
    }

    void draw(const GrVertices& vertices) {
        this->vertexBatch()->fDrawArrays.tail()->fDraws.push_back(vertices);
    }

    void* makeVertexSpace(size_t vertexSize, int vertexCount,
                          const GrVertexBuffer** buffer, int* startVertex) {
        return this->state()->makeVertexSpace(vertexSize, vertexCount, buffer, startVertex);
    }

    uint16_t* makeIndexSpace(int indexCount, const GrIndexBuffer** buffer, int* startIndex) {
        return this->state()->makeIndexSpace(indexCount, buffer, startIndex);
    }

    /** Helpers for batches which over-allocate and then return data to the pool. */
    void putBackIndices(int indices) { this->state()->putBackIndices(indices); }
    void putBackVertices(int vertices, size_t vertexStride) {
        this->state()->putBackVertexSpace(vertices * vertexStride);
    }

private:
    GrVertexBatch* vertexBatch() { return static_cast<GrVertexBatch*>(this->batch()); }
    typedef GrDrawBatch::Target INHERITED;
};

#endif
