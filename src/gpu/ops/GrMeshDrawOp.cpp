/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpuCommandBuffer.h"
#include "GrMeshDrawOp.h"
#include "GrOpFlushState.h"
#include "GrResourceProvider.h"

GrMeshDrawOp::GrMeshDrawOp(uint32_t classID)
        : INHERITED(classID), fBaseDrawToken(GrDeferredUploadToken::AlreadyFlushedToken()) {}

void GrMeshDrawOp::onPrepare(GrOpFlushState* state) { this->onPrepareDraws(state); }

void* GrMeshDrawOp::PatternHelper::init(Target* target, size_t vertexStride,
                                        const GrBuffer* indexBuffer, int verticesPerRepetition,
                                        int indicesPerRepetition, int repeatCount) {
    SkASSERT(target);
    if (!indexBuffer) {
        return nullptr;
    }
    const GrBuffer* vertexBuffer;
    int firstVertex;
    int vertexCount = verticesPerRepetition * repeatCount;
    void* vertices =
            target->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer, &firstVertex);
    if (!vertices) {
        SkDebugf("Vertices could not be allocated for instanced rendering.");
        return nullptr;
    }
    SkASSERT(vertexBuffer);
    size_t ibSize = indexBuffer->gpuMemorySize();
    int maxRepetitions = static_cast<int>(ibSize / (sizeof(uint16_t) * indicesPerRepetition));

    fMesh.setIndexedPatterned(indexBuffer, indicesPerRepetition, verticesPerRepetition,
                              repeatCount, maxRepetitions);
    fMesh.setVertexData(vertexBuffer, firstVertex);
    return vertices;
}

void GrMeshDrawOp::PatternHelper::recordDraw(Target* target, const GrGeometryProcessor* gp,
                                             const GrPipeline* pipeline) {
    target->draw(gp, pipeline, fMesh);
}

void* GrMeshDrawOp::QuadHelper::init(Target* target, size_t vertexStride, int quadsToDraw) {
    sk_sp<const GrBuffer> quadIndexBuffer = target->resourceProvider()->refQuadIndexBuffer();
    if (!quadIndexBuffer) {
        SkDebugf("Could not get quad index buffer.");
        return nullptr;
    }
    return this->INHERITED::init(target, vertexStride, quadIndexBuffer.get(), kVerticesPerQuad,
                                 kIndicesPerQuad, quadsToDraw);
}

void GrMeshDrawOp::onExecute(GrOpFlushState* state) {
    int currUploadIdx = 0;
    int currMeshIdx = 0;

    SkASSERT(fQueuedDraws.empty() || fBaseDrawToken == state->nextTokenToFlush());
    SkASSERT(state->rtCommandBuffer());

    for (int currDrawIdx = 0; currDrawIdx < fQueuedDraws.count(); ++currDrawIdx) {
        GrDeferredUploadToken drawToken = state->nextTokenToFlush();
        while (currUploadIdx < fInlineUploads.count() &&
               fInlineUploads[currUploadIdx].fUploadBeforeToken == drawToken) {
            state->rtCommandBuffer()->inlineUpload(state, fInlineUploads[currUploadIdx++].fUpload);
        }
        const QueuedDraw& draw = fQueuedDraws[currDrawIdx];
        SkASSERT(draw.fPipeline->proxy() == state->drawOpArgs().fProxy);
        state->rtCommandBuffer()->draw(*draw.fPipeline, *draw.fGeometryProcessor.get(),
                                       fMeshes.begin() + currMeshIdx, nullptr, draw.fMeshCnt,
                                       this->bounds());
        currMeshIdx += draw.fMeshCnt;
        state->flushToken();
    }
    SkASSERT(currUploadIdx == fInlineUploads.count());
    SkASSERT(currMeshIdx == fMeshes.count());
    fQueuedDraws.reset();
    fInlineUploads.reset();
}
