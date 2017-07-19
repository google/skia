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
    : INHERITED(classID), fBaseDrawToken(GrDrawOpUploadToken::AlreadyFlushedToken()) {}

void GrMeshDrawOp::onPrepare(GrOpFlushState* state) {
    Target target(state, this);
    this->onPrepareDraws(&target);
}

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
    sk_sp<const GrBuffer> quadIndexBuffer(target->resourceProvider()->refQuadIndexBuffer());
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

    for (int currDrawIdx = 0; currDrawIdx < fQueuedDraws.count(); ++currDrawIdx) {
        GrDrawOpUploadToken drawToken = state->nextTokenToFlush();
        while (currUploadIdx < fInlineUploads.count() &&
               fInlineUploads[currUploadIdx].fUploadBeforeToken == drawToken) {
            state->commandBuffer()->inlineUpload(state, fInlineUploads[currUploadIdx++].fUpload,
                                                 state->drawOpArgs().fRenderTarget);
        }
        const QueuedDraw& draw = fQueuedDraws[currDrawIdx];
        SkASSERT(draw.fPipeline->getRenderTarget() == state->drawOpArgs().fRenderTarget);
        state->commandBuffer()->draw(*draw.fPipeline, *draw.fGeometryProcessor.get(),
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

//////////////////////////////////////////////////////////////////////////////

void GrMeshDrawOp::Target::draw(const GrGeometryProcessor* gp, const GrPipeline* pipeline,
                                const GrMesh& mesh) {
    GrMeshDrawOp* op = this->meshDrawOp();
    op->fMeshes.push_back(mesh);
    if (!op->fQueuedDraws.empty()) {
        // If the last draw shares a geometry processor and pipeline and there are no intervening
        // uploads, add this mesh to it.
        GrMeshDrawOp::QueuedDraw& lastDraw = op->fQueuedDraws.back();
        if (lastDraw.fGeometryProcessor == gp && lastDraw.fPipeline == pipeline &&
            (op->fInlineUploads.empty() ||
             op->fInlineUploads.back().fUploadBeforeToken != this->nextDrawToken())) {
            ++lastDraw.fMeshCnt;
            return;
        }
    }
    GrMeshDrawOp::QueuedDraw& draw = op->fQueuedDraws.push_back();
    GrDrawOpUploadToken token = this->state()->issueDrawToken();
    draw.fGeometryProcessor.reset(gp);
    draw.fPipeline = pipeline;
    draw.fMeshCnt = 1;
    if (op->fQueuedDraws.count() == 1) {
        op->fBaseDrawToken = token;
    }
}
