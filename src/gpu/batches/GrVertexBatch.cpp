/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVertexBatch.h"
#include "GrBatchTarget.h"
#include "GrResourceProvider.h"

GrVertexBatch::GrVertexBatch() : fNumberOfDraws(0) {}

void* GrVertexBatch::InstancedHelper::init(GrBatchTarget* batchTarget, GrPrimitiveType primType,
                                     size_t vertexStride, const GrIndexBuffer* indexBuffer,
                                     int verticesPerInstance, int indicesPerInstance,
                                     int instancesToDraw) {
    SkASSERT(batchTarget);
    if (!indexBuffer) {
        return NULL;
    }
    const GrVertexBuffer* vertexBuffer;
    int firstVertex;
    int vertexCount = verticesPerInstance * instancesToDraw;
    void* vertices = batchTarget->makeVertSpace(vertexStride, vertexCount,
                                                &vertexBuffer, &firstVertex);
    if (!vertices) {
        SkDebugf("Vertices could not be allocated for instanced rendering.");
        return NULL;
    }
    SkASSERT(vertexBuffer);
    size_t ibSize = indexBuffer->gpuMemorySize();
    int maxInstancesPerDraw = static_cast<int>(ibSize / (sizeof(uint16_t) * indicesPerInstance));

    fVertices.initInstanced(primType, vertexBuffer, indexBuffer,
        firstVertex, verticesPerInstance, indicesPerInstance, instancesToDraw,
        maxInstancesPerDraw);
    return vertices;
}

void* GrVertexBatch::QuadHelper::init(GrBatchTarget* batchTarget, size_t vertexStride,
                                      int quadsToDraw) {
    SkAutoTUnref<const GrIndexBuffer> quadIndexBuffer(
        batchTarget->resourceProvider()->refQuadIndexBuffer());
    if (!quadIndexBuffer) {
        SkDebugf("Could not get quad index buffer.");
        return NULL;
    }
    return this->INHERITED::init(batchTarget, kTriangles_GrPrimitiveType, vertexStride,
                                 quadIndexBuffer, kVerticesPerQuad, kIndicesPerQuad, quadsToDraw);
}
