/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBatchBuffer_DEFINED
#define GrBatchBuffer_DEFINED

#include "GrBatchAtlas.h"
#include "GrBufferAllocPool.h"
#include "GrContext.h"
#include "GrPendingProgramElement.h"
#include "GrPipeline.h"
#include "GrTRecorder.h"
#include "GrVertices.h"

/*
 * GrBatch instances use this object to allocate space for their geometry and to issue the draws
 * that render their batch.
 */
class GrBatchTarget : public SkNoncopyable {
public:
    typedef GrBatchAtlas::BatchToken BatchToken;
    GrBatchTarget(GrGpu* gpu);

    void initDraw(const GrPrimitiveProcessor* primProc, const GrPipeline* pipeline) {
        GrNEW_APPEND_TO_RECORDER(fFlushBuffer, BufferedFlush, (primProc, pipeline));
        fNumberOfDraws++;
        fCurrentToken++;
    }

    class TextureUploader {
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

    class Uploader : public SkRefCnt {
    public:
        Uploader(BatchToken lastUploadToken) : fLastUploadToken(lastUploadToken) {}
        BatchToken lastUploadToken() const { return fLastUploadToken; }
        virtual void upload(TextureUploader)=0;

    private:
        BatchToken fLastUploadToken;
    };

    void upload(Uploader* upload) {
        if (this->asapToken() == upload->lastUploadToken()) {
            fAsapUploads.push_back().reset(SkRef(upload));
        } else {
            fInlineUploads.push_back().reset(SkRef(upload));
        }
    }

    void draw(const GrVertices& vertices) {
        fFlushBuffer.back().fVertexDraws.push_back(vertices);
    }

    bool isIssued(BatchToken token) const { return fLastFlushedToken >= token; }
    BatchToken currentToken() const { return fCurrentToken; }
    BatchToken asapToken() const { return fLastFlushedToken + 1; }

    // TODO much of this complexity goes away when batch is everywhere
    void resetNumberOfDraws() { fNumberOfDraws = 0; }
    int numberOfDraws() const { return fNumberOfDraws; }
    void preFlush() {
        this->unmapVertexAndIndexBuffers();
        int updateCount = fAsapUploads.count();
        for (int i = 0; i < updateCount; i++) {
            fAsapUploads[i]->upload(TextureUploader(fGpu));
        }
        fInlineUpdatesIndex = 0;
        fIter = FlushBuffer::Iter(fFlushBuffer);
    }
    void flushNext(int n);
    void postFlush() {
        SkASSERT(!fIter.next());
        fFlushBuffer.reset();
        fAsapUploads.reset();
        fInlineUploads.reset();
    }

    const GrCaps& caps() const { return *fGpu->caps(); }

    GrResourceProvider* resourceProvider() const { return fGpu->getContext()->resourceProvider(); }

    void* makeVertSpace(size_t vertexSize, int vertexCount,
                        const GrVertexBuffer** buffer, int* startVertex);
    uint16_t* makeIndexSpace(int indexCount,
                             const GrIndexBuffer** buffer, int* startIndex);

    // A helper for draws which overallocate and then return data to the pool
    void putBackIndices(size_t indices) { fIndexPool.putBack(indices * sizeof(uint16_t)); }

    void putBackVertices(size_t vertices, size_t vertexStride) {
        fVertexPool.putBack(vertices * vertexStride);
    }

    void reset() {
        fVertexPool.reset();
        fIndexPool.reset();    
    }

private:
    void unmapVertexAndIndexBuffers() {
        fVertexPool.unmap();
        fIndexPool.unmap();
    }

    GrGpu* fGpu;
    GrVertexBufferAllocPool fVertexPool;
    GrIndexBufferAllocPool fIndexPool;

    typedef void* TBufferAlign; // This wouldn't be enough align if a command used long double.

    struct BufferedFlush {
        BufferedFlush(const GrPrimitiveProcessor* primProc, const GrPipeline* pipeline)
            : fPrimitiveProcessor(primProc)
            , fPipeline(pipeline) {}
        typedef GrPendingProgramElement<const GrPrimitiveProcessor> ProgramPrimitiveProcessor;
        ProgramPrimitiveProcessor fPrimitiveProcessor;
        const GrPipeline* fPipeline;
        GrBatchTracker fBatchTracker;
        SkSTArray<1, GrVertices, true> fVertexDraws;
    };

    enum {
        kFlushBufferInitialSizeInBytes = 8 * sizeof(BufferedFlush),
    };

    typedef GrTRecorder<BufferedFlush, TBufferAlign> FlushBuffer;

    FlushBuffer fFlushBuffer;
    // TODO this is temporary
    FlushBuffer::Iter fIter;
    int fNumberOfDraws;
    BatchToken fCurrentToken;
    BatchToken fLastFlushedToken; // The next token to be flushed
    SkTArray<SkAutoTUnref<Uploader>, true> fAsapUploads;
    SkTArray<SkAutoTUnref<Uploader>, true> fInlineUploads;
    int fInlineUpdatesIndex;
};

#endif
