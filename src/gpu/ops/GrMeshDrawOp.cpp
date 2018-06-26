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

GrMeshDrawOp::GrMeshDrawOp(uint32_t classID) : INHERITED(classID) {}

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

void GrMeshDrawOp::PatternHelper::recordDraw(
        Target* target, const GrGeometryProcessor* gp, const GrPipeline* pipeline,
        const GrPipeline::FixedDynamicState* fixedDynamicState) {
    target->draw(gp, pipeline, fixedDynamicState, fMesh);
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
    state->executeDrawsAndUploadsForMeshDrawOp(this->uniqueID(), this->bounds());
}
