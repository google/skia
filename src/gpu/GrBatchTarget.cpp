/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBatchTarget.h"

#include "GrBatchAtlas.h"
#include "GrPipeline.h"

static const size_t DRAW_BUFFER_VBPOOL_BUFFER_SIZE = 1 << 15;
static const int DRAW_BUFFER_VBPOOL_PREALLOC_BUFFERS = 4;

static const size_t DRAW_BUFFER_IBPOOL_BUFFER_SIZE = 1 << 11;
static const int DRAW_BUFFER_IBPOOL_PREALLOC_BUFFERS = 4;

GrBatchTarget::GrBatchTarget(GrGpu* gpu)
    : fGpu(gpu)
    , fFlushBuffer(kFlushBufferInitialSizeInBytes)
    , fIter(fFlushBuffer)
    , fNumberOfDraws(0)
    , fCurrentToken(0)
    , fLastFlushedToken(0)
    , fInlineUpdatesIndex(0) {

    fVertexPool.reset(SkNEW_ARGS(GrVertexBufferAllocPool, (fGpu,
                                             DRAW_BUFFER_VBPOOL_BUFFER_SIZE,
                                             DRAW_BUFFER_VBPOOL_PREALLOC_BUFFERS)));
    fIndexPool.reset(SkNEW_ARGS(GrIndexBufferAllocPool, (fGpu,
                                            DRAW_BUFFER_IBPOOL_BUFFER_SIZE,
                                            DRAW_BUFFER_IBPOOL_PREALLOC_BUFFERS)));
}

void GrBatchTarget::flushNext(int n)  {
    for (; n > 0; n--) {
        fLastFlushedToken++;
        SkDEBUGCODE(bool verify =) fIter.next();
        SkASSERT(verify);

        BufferedFlush* bf = fIter.get();

        // Flush all texture uploads
        int uploadCount = fInlineUploads.count();
        while (fInlineUpdatesIndex < uploadCount &&
               fInlineUploads[fInlineUpdatesIndex]->lastUploadToken() <= fLastFlushedToken) {
            fInlineUploads[fInlineUpdatesIndex++]->upload(TextureUploader(fGpu));
        }

        GrProgramDesc desc;
        const GrPipeline* pipeline = bf->fPipeline;
        const GrPrimitiveProcessor* primProc = bf->fPrimitiveProcessor.get();
        fGpu->buildProgramDesc(&desc, *primProc, *pipeline, bf->fBatchTracker);

        GrGpu::DrawArgs args(primProc, pipeline, &desc, &bf->fBatchTracker);

        int drawCount = bf->fVertexDraws.count();
        const SkSTArray<1, GrVertices, true>& vertexDraws = bf->fVertexDraws;
        for (int i = 0; i < drawCount; i++) {
            fGpu->draw(args, vertexDraws[i]);
        }
    }
}

void* GrBatchTarget::makeVertSpace(size_t vertexSize, int vertexCount,
                    const GrVertexBuffer** buffer, int* startVertex) {
    return fVertexPool->makeSpace(vertexSize, vertexCount, buffer, startVertex);
}

uint16_t* GrBatchTarget::makeIndexSpace(int indexCount,
                                        const GrIndexBuffer** buffer, int* startIndex) {
    return reinterpret_cast<uint16_t*>(fIndexPool->makeSpace(indexCount, buffer, startIndex));
}

