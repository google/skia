/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMeshDrawOp.h"
#include "GrBatchFlushState.h"
#include "GrResourceProvider.h"

GrMeshDrawOp::GrMeshDrawOp(uint32_t classID)
    : INHERITED(classID), fBaseDrawToken(GrDrawOpUploadToken::AlreadyFlushedToken()) {}

void GrMeshDrawOp::onPrepare(GrBatchFlushState* state) {
    Target target(state, this);
    this->onPrepareDraws(&target);
}

void* GrMeshDrawOp::InstancedHelper::init(Target* target, GrPrimitiveType primType,
                                          size_t vertexStride, const GrBuffer* indexBuffer,
                                          int verticesPerInstance, int indicesPerInstance,
                                          int instancesToDraw) {
    SkASSERT(target);
    if (!indexBuffer) {
        return nullptr;
    }
    const GrBuffer* vertexBuffer;
    int firstVertex;
    int vertexCount = verticesPerInstance * instancesToDraw;
    void* vertices =
            target->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer, &firstVertex);
    if (!vertices) {
        SkDebugf("Vertices could not be allocated for instanced rendering.");
        return nullptr;
    }
    SkASSERT(vertexBuffer);
    size_t ibSize = indexBuffer->gpuMemorySize();
    int maxInstancesPerDraw = static_cast<int>(ibSize / (sizeof(uint16_t) * indicesPerInstance));

    fMesh.initInstanced(primType, vertexBuffer, indexBuffer, firstVertex, verticesPerInstance,
                        indicesPerInstance, instancesToDraw, maxInstancesPerDraw);
    return vertices;
}

void GrMeshDrawOp::InstancedHelper::recordDraw(Target* target, const GrGeometryProcessor* gp) {
    SkASSERT(fMesh.instanceCount());
    target->draw(gp, fMesh);
}

void* GrMeshDrawOp::QuadHelper::init(Target* target, size_t vertexStride, int quadsToDraw) {
    sk_sp<const GrBuffer> quadIndexBuffer(target->resourceProvider()->refQuadIndexBuffer());
    if (!quadIndexBuffer) {
        SkDebugf("Could not get quad index buffer.");
        return nullptr;
    }
    return this->INHERITED::init(target, kTriangles_GrPrimitiveType, vertexStride,
                                 quadIndexBuffer.get(), kVerticesPerQuad, kIndicesPerQuad,
                                 quadsToDraw);
}

void GrMeshDrawOp::onDraw(GrBatchFlushState* state, const SkRect& bounds) {
    int currUploadIdx = 0;
    int currMeshIdx = 0;

    SkASSERT(fQueuedDraws.empty() || fBaseDrawToken == state->nextTokenToFlush());

    for (int currDrawIdx = 0; currDrawIdx < fQueuedDraws.count(); ++currDrawIdx) {
        GrDrawOpUploadToken drawToken = state->nextTokenToFlush();
        while (currUploadIdx < fInlineUploads.count() &&
               fInlineUploads[currUploadIdx].fUploadBeforeToken == drawToken) {
            state->commandBuffer()->inlineUpload(state, fInlineUploads[currUploadIdx++].fUpload);
        }
        const QueuedDraw& draw = fQueuedDraws[currDrawIdx];
        state->commandBuffer()->draw(*this->pipeline(), *draw.fGeometryProcessor.get(),
                                     fMeshes.begin() + currMeshIdx, draw.fMeshCnt, bounds);
        currMeshIdx += draw.fMeshCnt;
        state->flushToken();
    }
    SkASSERT(currUploadIdx == fInlineUploads.count());
    SkASSERT(currMeshIdx == fMeshes.count());
    fQueuedDraws.reset();
    fInlineUploads.reset();
}

//////////////////////////////////////////////////////////////////////////////

void GrMeshDrawOp::Target::draw(const GrGeometryProcessor* gp, const GrMesh& mesh) {
    GrMeshDrawOp* batch = this->vertexBatch();
    batch->fMeshes.push_back(mesh);
    if (!batch->fQueuedDraws.empty()) {
        // If the last draw shares a geometry processor and there are no intervening uploads,
        // add this mesh to it.
        GrMeshDrawOp::QueuedDraw& lastDraw = this->vertexBatch()->fQueuedDraws.back();
        if (lastDraw.fGeometryProcessor == gp &&
            (batch->fInlineUploads.empty() ||
             batch->fInlineUploads.back().fUploadBeforeToken != this->nextDrawToken())) {
            ++lastDraw.fMeshCnt;
            return;
        }
    }
    GrMeshDrawOp::QueuedDraw& draw = this->vertexBatch()->fQueuedDraws.push_back();
    GrDrawOpUploadToken token = this->state()->issueDrawToken();
    draw.fGeometryProcessor.reset(gp);
    draw.fMeshCnt = 1;
    if (batch->fQueuedDraws.count() == 1) {
        batch->fBaseDrawToken = token;
    }
}
