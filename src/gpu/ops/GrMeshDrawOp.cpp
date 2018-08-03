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
        Target* target, sk_sp<const GrGeometryProcessor> gp, const GrPipeline* pipeline,
        const GrPipeline::FixedDynamicState* fixedDynamicState) {
    target->draw(std::move(gp), pipeline, fixedDynamicState, fMesh);
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

//////////////////////////////////////////////////////////////////////////////

GrPipeline::FixedDynamicState* GrMeshDrawOp::Target::allocFixedDynamicState(
        const SkIRect& rect, int numPrimitiveProcessorTextures) {
    auto result = this->pipelineArena()->make<GrPipeline::FixedDynamicState>(rect);
    if (numPrimitiveProcessorTextures) {
        result->fPrimitiveProcessorTextures =
                this->allocPrimitiveProcessorTextureArray(numPrimitiveProcessorTextures);
    }
    return result;
}

GrPipeline::DynamicStateArrays* GrMeshDrawOp::Target::allocDynamicStateArrays(
        int numMeshes, int numPrimitiveProcessorTextures, bool allocScissors) {
    auto result = this->pipelineArena()->make<GrPipeline::DynamicStateArrays>();
    if (allocScissors) {
        result->fScissorRects = this->pipelineArena()->makeArray<SkIRect>(numMeshes);
    }
    if (numPrimitiveProcessorTextures) {
        result->fPrimitiveProcessorTextures =
                this->allocPrimitiveProcessorTextureArray(numPrimitiveProcessorTextures * numMeshes);
    }
    return result;
}

GrMeshDrawOp::Target::PipelineAndFixedDynamicState GrMeshDrawOp::Target::makePipeline(
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
