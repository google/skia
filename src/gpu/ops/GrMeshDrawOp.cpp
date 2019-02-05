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

void GrMeshDrawOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    state->executeDrawsAndUploadsForMeshDrawOp(this, chainBounds);
}

//////////////////////////////////////////////////////////////////////////////

GrMeshDrawOp::PatternHelper::PatternHelper(Target* target, GrPrimitiveType primitiveType,
                                           size_t vertexStride, sk_sp<const GrBuffer> indexBuffer,
                                           int verticesPerRepetition, int indicesPerRepetition,
                                           int repeatCount) {
    this->init(target, primitiveType, vertexStride, std::move(indexBuffer), verticesPerRepetition,
               indicesPerRepetition, repeatCount);
}

void GrMeshDrawOp::PatternHelper::init(Target* target, GrPrimitiveType primitiveType,
                                       size_t vertexStride, sk_sp<const GrBuffer> indexBuffer,
                                       int verticesPerRepetition, int indicesPerRepetition,
                                       int repeatCount) {
    SkASSERT(target);
    if (!indexBuffer) {
        return;
    }
    sk_sp<const GrBuffer> vertexBuffer;
    int firstVertex;
    int vertexCount = verticesPerRepetition * repeatCount;
    fVertices = target->makeVertexSpace(vertexStride, vertexCount, &vertexBuffer, &firstVertex);
    if (!fVertices) {
        SkDebugf("Vertices could not be allocated for patterned rendering.");
        return;
    }
    SkASSERT(vertexBuffer);
    size_t ibSize = indexBuffer->size();
    int maxRepetitions = static_cast<int>(ibSize / (sizeof(uint16_t) * indicesPerRepetition));
    fMesh = target->allocMesh(primitiveType);
    fMesh->setIndexedPatterned(std::move(indexBuffer), indicesPerRepetition, verticesPerRepetition,
                               repeatCount, maxRepetitions);
    fMesh->setVertexData(std::move(vertexBuffer), firstVertex);
}

void GrMeshDrawOp::PatternHelper::recordDraw(
        Target* target, sk_sp<const GrGeometryProcessor> gp, const GrPipeline* pipeline,
        const GrPipeline::FixedDynamicState* fixedDynamicState) const {
    target->draw(std::move(gp), pipeline, fixedDynamicState, fMesh);
}

//////////////////////////////////////////////////////////////////////////////

GrMeshDrawOp::QuadHelper::QuadHelper(Target* target, size_t vertexStride, int quadsToDraw) {
    sk_sp<const GrGpuBuffer> quadIndexBuffer = target->resourceProvider()->refQuadIndexBuffer();
    if (!quadIndexBuffer) {
        SkDebugf("Could not get quad index buffer.");
        return;
    }
    this->init(target, GrPrimitiveType::kTriangles, vertexStride, std::move(quadIndexBuffer),
               kVerticesPerQuad, kIndicesPerQuad, quadsToDraw);
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
        result->fPrimitiveProcessorTextures = this->allocPrimitiveProcessorTextureArray(
                numPrimitiveProcessorTextures * numMeshes);
    }
    return result;
}

GrMeshDrawOp::Target::PipelineAndFixedDynamicState GrMeshDrawOp::Target::makePipeline(
        uint32_t pipelineFlags, GrProcessorSet&& processorSet, GrAppliedClip&& clip,
        int numPrimProcTextures) {
    GrPipeline::InitArgs pipelineArgs;
    pipelineArgs.fFlags = pipelineFlags;
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
