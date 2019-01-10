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

//////////////////////////////////////////////////////////////////////////////

GrPipeline::FixedDynamicState* GrOpFlushState::allocFixedDynamicState(
        const SkIRect& rect, int numPrimitiveProcessorTextures) {
    auto result = fArena.make<GrPipeline::FixedDynamicState>(rect);
    if (numPrimitiveProcessorTextures) {
        result->fPrimitiveProcessorTextures =
                this->allocPrimitiveProcessorTextureArray(numPrimitiveProcessorTextures);
    }
    return result;
}

GrPipeline::DynamicStateArrays* GrOpFlushState::allocDynamicStateArrays(
        int numMeshes, int numPrimitiveProcessorTextures, bool allocScissors) {
    auto result = fArena.make<GrPipeline::DynamicStateArrays>();
    if (allocScissors) {
        result->fScissorRects = fArena.makeArray<SkIRect>(numMeshes);
    }
    if (numPrimitiveProcessorTextures) {
        result->fPrimitiveProcessorTextures = this->allocPrimitiveProcessorTextureArray(
                numPrimitiveProcessorTextures * numMeshes);
    }
    return result;
}

GrOpFlushState::PipelineAndFixedDynamicState GrOpFlushState::makePipeline(
        uint32_t pipelineFlags, GrProcessorSet&& processorSet, GrAppliedClip&& clip,
        int numPrimProcTextures) {
    GrPipeline::InitArgs pipelineArgs;
    pipelineArgs.fFlags = pipelineFlags;
    pipelineArgs.fProxy = this->proxy();
    pipelineArgs.fDstProxy = this->dstProxy();
    pipelineArgs.fCaps = &this->caps();
    pipelineArgs.fResourceProvider = this->resourceProvider();
    GrPipeline::FixedDynamicState* fixedDynamicState = nullptr;
    if (clip.scissorState().enabled() || numPrimProcTextures) {
        fixedDynamicState = this->allocFixedDynamicState(clip.scissorState().rect());
        if (numPrimProcTextures) {
            fixedDynamicState->fPrimitiveProcessorTextures =
                    this->allocPrimitiveProcessorTextureArray(numPrimProcTextures);
        }
    }
    return {this->allocPipeline(pipelineArgs, std::move(processorSet), std::move(clip)),
            fixedDynamicState};
}
