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

void GrMeshDrawOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    state->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds);
}

//////////////////////////////////////////////////////////////////////////////

GrMeshDrawOp::PatternHelper::PatternHelper(GrOpFlushState* flushState,
                                           GrPrimitiveType primitiveType, size_t vertexStride,
                                           const GrBuffer* indexBuffer, int verticesPerRepetition,
                                           int indicesPerRepetition, int repeatCount) {
    this->init(flushState, primitiveType, vertexStride, indexBuffer, verticesPerRepetition,
               indicesPerRepetition, repeatCount);
}

void GrMeshDrawOp::PatternHelper::init(GrOpFlushState* flushState, GrPrimitiveType primitiveType,
                                       size_t vertexStride, const GrBuffer* indexBuffer,
                                       int verticesPerRepetition, int indicesPerRepetition,
                                       int repeatCount) {
    SkASSERT(flushState);
    if (!indexBuffer) {
        return;
    }
    const GrBuffer* vertexBuffer;
    int firstVertex;
    int vertexCount = verticesPerRepetition * repeatCount;
    fVertices = flushState->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer, &firstVertex);
    if (!fVertices) {
        SkDebugf("Vertices could not be allocated for patterned rendering.");
        return;
    }
    SkASSERT(vertexBuffer);
    size_t ibSize = indexBuffer->gpuMemorySize();
    int maxRepetitions = static_cast<int>(ibSize / (sizeof(uint16_t) * indicesPerRepetition));
    fMesh = flushState->allocMesh(primitiveType);
    fMesh->setIndexedPatterned(indexBuffer, indicesPerRepetition, verticesPerRepetition,
                               repeatCount, maxRepetitions);
    fMesh->setVertexData(vertexBuffer, firstVertex);
}

void GrMeshDrawOp::PatternHelper::recordDraw(
        GrOpFlushState* flushState, sk_sp<const GrGeometryProcessor> gp, const GrPipeline* pipeline,
        const GrPipeline::FixedDynamicState* fixedDynamicState) const {
    flushState->draw(std::move(gp), pipeline, fixedDynamicState, fMesh);
}

//////////////////////////////////////////////////////////////////////////////

GrMeshDrawOp::QuadHelper::QuadHelper(GrOpFlushState* flushState, size_t vertexStride,
                                     int quadsToDraw) {
    sk_sp<const GrBuffer> quadIndexBuffer = flushState->resourceProvider()->refQuadIndexBuffer();
    if (!quadIndexBuffer) {
        SkDebugf("Could not get quad index buffer.");
        return;
    }
    this->init(flushState, GrPrimitiveType::kTriangles, vertexStride, quadIndexBuffer.get(),
               kVerticesPerQuad, kIndicesPerQuad, quadsToDraw);
}
