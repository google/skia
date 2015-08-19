/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBufferedDrawTarget.h"

// We will use the reordering buffer, unless we have NVPR.
// TODO move NVPR to batch so we can reorder
static inline bool allow_reordering(const GrCaps* caps) {
    return caps && caps->shaderCaps() && !caps->shaderCaps()->pathRenderingSupport();
}

GrBufferedDrawTarget::GrBufferedDrawTarget(GrContext* context)
    : INHERITED(context)
    , fCommands(GrCommandBuilder::Create(context->getGpu(), allow_reordering(context->caps())))
    , fPathIndexBuffer(kPathIdxBufferMinReserve * sizeof(char)/4)
    , fPathTransformBuffer(kPathXformBufferMinReserve * sizeof(float)/4)
    , fPipelineBuffer(kPipelineBufferMinReserve)
    , fDrawID(0) {
}

GrBufferedDrawTarget::~GrBufferedDrawTarget() {
    this->reset();
}

void GrBufferedDrawTarget::onDrawBatch(GrBatch* batch) {
    fCommands->recordDrawBatch(batch, *this->caps());
}

void GrBufferedDrawTarget::onDrawPaths(const GrPathProcessor* pathProc,
                                       const GrPathRange* pathRange,
                                       const void* indices,
                                       PathIndexType indexType,
                                       const float transformValues[],
                                       PathTransformType transformType,
                                       int count,
                                       const GrStencilSettings& stencilSettings,
                                       const PipelineInfo& pipelineInfo) {
    GrPipelineOptimizations opts;
    StateForPathDraw* state = this->createStateForPathDraw(pathProc, pipelineInfo, &opts);
    if (!state) {
        return;
    }
    fCommands->recordDrawPaths(state, this, pathProc, pathRange, indices, indexType,
                               transformValues, transformType, count, stencilSettings,
                               opts);
}

void GrBufferedDrawTarget::onReset() {
    fCommands->reset();
    fPathIndexBuffer.rewind();
    fPathTransformBuffer.rewind();

    fPrevState.reset(NULL);
    // Note, fPrevState points into fPipelineBuffer's allocation, so we have to reset first.
    // Furthermore, we have to reset fCommands before fPipelineBuffer too.
    if (fDrawID % kPipelineBufferHighWaterMark) {
        fPipelineBuffer.rewind();
    } else {
        fPipelineBuffer.reset();
    }
}

void GrBufferedDrawTarget::onFlush() {
    fCommands->flush(this->getGpu(), this->getContext()->resourceProvider());
    ++fDrawID;
}

GrTargetCommands::StateForPathDraw*
GrBufferedDrawTarget::createStateForPathDraw(const GrPrimitiveProcessor* primProc,
                                             const GrDrawTarget::PipelineInfo& pipelineInfo,
                                             GrPipelineOptimizations* opts) {
    StateForPathDraw* state = this->allocState(primProc);
    if (!GrPipeline::CreateAt(state->pipelineLocation(), pipelineInfo.pipelineCreateArgs(), opts)) {
        this->unallocState(state);
        return NULL;
    }

    state->fPrimitiveProcessor->initBatchTracker(&state->fBatchTracker, *opts);

    if (fPrevState && fPrevState->fPrimitiveProcessor.get() &&
        fPrevState->fPrimitiveProcessor->canMakeEqual(fPrevState->fBatchTracker,
                                                      *state->fPrimitiveProcessor,
                                                      state->fBatchTracker) &&
        GrPipeline::AreEqual(*fPrevState->getPipeline(), *state->getPipeline(), false)) {
        this->unallocState(state);
    } else {
        fPrevState.reset(state);
    }

    return fPrevState;
}
