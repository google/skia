/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVertexBatch.h"
#include "GrBatchFlushState.h"
#include "GrResourceProvider.h"

GrVertexBatch::GrVertexBatch(uint32_t classID) : INHERITED(classID) {}

void GrVertexBatch::onPrepare(GrBatchFlushState* state) {
    Target target(state, this);
    this->onPrepareDraws(&target);
}

void* GrVertexBatch::InstancedHelper::init(Target* target, GrPrimitiveType primType,
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
    void* vertices = target->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer, &firstVertex);
    if (!vertices) {
        SkDebugf("Vertices could not be allocated for instanced rendering.");
        return nullptr;
    }
    SkASSERT(vertexBuffer);
    size_t ibSize = indexBuffer->gpuMemorySize();
    int maxInstancesPerDraw = static_cast<int>(ibSize / (sizeof(uint16_t) * indicesPerInstance));

    fMesh.initInstanced(primType, vertexBuffer, indexBuffer,
        firstVertex, verticesPerInstance, indicesPerInstance, instancesToDraw,
        maxInstancesPerDraw);
    return vertices;
}

void GrVertexBatch::InstancedHelper::recordDraw(Target* target) {
    SkASSERT(fMesh.instanceCount());
    target->draw(fMesh);
}

void* GrVertexBatch::QuadHelper::init(Target* target, size_t vertexStride,
                                      int quadsToDraw) {
    SkAutoTUnref<const GrBuffer> quadIndexBuffer(
        target->resourceProvider()->refQuadIndexBuffer());
    if (!quadIndexBuffer) {
        SkDebugf("Could not get quad index buffer.");
        return nullptr;
    }
    return this->INHERITED::init(target, kTriangles_GrPrimitiveType, vertexStride,
                                 quadIndexBuffer, kVerticesPerQuad, kIndicesPerQuad, quadsToDraw);
}

void GrVertexBatch::onDraw(GrBatchFlushState* state) {
    int uploadCnt = fInlineUploads.count();
    int currUpload = 0;

    // Iterate of all the drawArrays. Before issuing the draws in each array, perform any inline
    // uploads.
    for (DrawArrayList::Iter da(fDrawArrays); da.get(); da.next()) {
        state->advanceLastFlushedToken();
        while (currUpload < uploadCnt &&
               fInlineUploads[currUpload]->lastUploadToken() <= state->lastFlushedToken()) {
            fInlineUploads[currUpload++]->upload(state->uploader());
        }
        const GrVertexBatch::DrawArray& drawArray = *da.get();

        state->gpu()->draw(*this->pipeline(),
                           *drawArray.fPrimitiveProcessor.get(),
                           drawArray.fDraws.begin(),
                           drawArray.fDraws.count());
    }
}
