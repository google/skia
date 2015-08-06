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

void GrBufferedDrawTarget::onDrawBatch(GrBatch* batch,
                                       const PipelineInfo& pipelineInfo) {
    State* state = this->setupPipelineAndShouldDraw(batch, pipelineInfo);
    if (!state) {
        return;
    }

    GrTargetCommands::Cmd* cmd = fCommands->recordDrawBatch(state, batch);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrBufferedDrawTarget::onStencilPath(const GrPipelineBuilder& pipelineBuilder,
                                         const GrPathProcessor* pathProc,
                                         const GrPath* path,
                                         const GrScissorState& scissorState,
                                         const GrStencilSettings& stencilSettings) {
    GrTargetCommands::Cmd* cmd = fCommands->recordStencilPath(pipelineBuilder,
                                                              pathProc, path, scissorState,
                                                              stencilSettings);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrBufferedDrawTarget::onDrawPath(const GrPathProcessor* pathProc,
                                      const GrPath* path,
                                      const GrStencilSettings& stencilSettings,
                                      const PipelineInfo& pipelineInfo) {
    State* state = this->setupPipelineAndShouldDraw(pathProc, pipelineInfo);
    if (!state) {
        return;
    }
    GrTargetCommands::Cmd* cmd = fCommands->recordDrawPath(state, pathProc, path, stencilSettings);
    this->recordTraceMarkersIfNecessary(cmd);
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
    State* state = this->setupPipelineAndShouldDraw(pathProc, pipelineInfo);
    if (!state) {
        return;
    }
    GrTargetCommands::Cmd* cmd = fCommands->recordDrawPaths(state, this, pathProc, pathRange,
                                                            indices, indexType, transformValues,
                                                            transformType, count,
                                                            stencilSettings, pipelineInfo);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrBufferedDrawTarget::onClear(const SkIRect& rect, GrColor color,
                                   GrRenderTarget* renderTarget) {
    GrTargetCommands::Cmd* cmd = fCommands->recordClear(rect, color, renderTarget);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrBufferedDrawTarget::clearStencilClip(const SkIRect& rect,
                                            bool insideClip,
                                            GrRenderTarget* renderTarget) {
    GrTargetCommands::Cmd* cmd = fCommands->recordClearStencilClip(rect, insideClip, renderTarget);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrBufferedDrawTarget::discard(GrRenderTarget* renderTarget) {
    if (!this->caps()->discardRenderTargetSupport()) {
        return;
    }

    GrTargetCommands::Cmd* cmd = fCommands->recordDiscard(renderTarget);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrBufferedDrawTarget::onReset() {
    fCommands->reset();
    fPathIndexBuffer.rewind();
    fPathTransformBuffer.rewind();
    fGpuCmdMarkers.reset();

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
    fCommands->flush(this);
    ++fDrawID;
}

void GrBufferedDrawTarget::onCopySurface(GrSurface* dst,
                                         GrSurface* src,
                                         const SkIRect& srcRect,
                                         const SkIPoint& dstPoint) {
    GrTargetCommands::Cmd* cmd = fCommands->recordCopySurface(dst, src, srcRect, dstPoint);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrBufferedDrawTarget::recordTraceMarkersIfNecessary(GrTargetCommands::Cmd* cmd) {
    if (!cmd) {
        return;
    }
    const GrTraceMarkerSet& activeTraceMarkers = this->getActiveTraceMarkers();
    if (activeTraceMarkers.count() > 0) {
        if (cmd->isTraced()) {
            fGpuCmdMarkers[cmd->markerID()].addSet(activeTraceMarkers);
        } else {
            cmd->setMarkerID(fGpuCmdMarkers.count());
            fGpuCmdMarkers.push_back(activeTraceMarkers);
        }
    }
}

GrTargetCommands::State*
GrBufferedDrawTarget::setupPipelineAndShouldDraw(const GrPrimitiveProcessor* primProc,
                                                 const GrDrawTarget::PipelineInfo& pipelineInfo) {
    State* state = this->allocState(primProc);
    this->setupPipeline(pipelineInfo, state->pipelineLocation());

    if (state->getPipeline()->mustSkip()) {
        this->unallocState(state);
        return NULL;
    }

    state->fPrimitiveProcessor->initBatchTracker(
        &state->fBatchTracker, state->getPipeline()->infoForPrimitiveProcessor());

    if (fPrevState && fPrevState->fPrimitiveProcessor.get() &&
        fPrevState->fPrimitiveProcessor->canMakeEqual(fPrevState->fBatchTracker,
                                                      *state->fPrimitiveProcessor,
                                                      state->fBatchTracker) &&
        fPrevState->getPipeline()->isEqual(*state->getPipeline())) {
        this->unallocState(state);
    } else {
        fPrevState.reset(state);
    }

    this->recordTraceMarkersIfNecessary(
            fCommands->recordXferBarrierIfNecessary(*fPrevState->getPipeline(), *this->caps()));
    return fPrevState;
}

GrTargetCommands::State*
GrBufferedDrawTarget::setupPipelineAndShouldDraw(GrBatch* batch,
                                                 const GrDrawTarget::PipelineInfo& pipelineInfo) {
    State* state = this->allocState();
    this->setupPipeline(pipelineInfo, state->pipelineLocation());

    if (state->getPipeline()->mustSkip()) {
        this->unallocState(state);
        return NULL;
    }

    batch->initBatchTracker(state->getPipeline()->infoForPrimitiveProcessor());

    if (fPrevState && !fPrevState->fPrimitiveProcessor.get() &&
        fPrevState->getPipeline()->isEqual(*state->getPipeline())) {
        this->unallocState(state);
    } else {
        fPrevState.reset(state);
    }

    this->recordTraceMarkersIfNecessary(
            fCommands->recordXferBarrierIfNecessary(*fPrevState->getPipeline(), *this->caps()));
    return fPrevState;
}
