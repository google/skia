/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrInOrderDrawBuffer.h"

GrInOrderDrawBuffer::GrInOrderDrawBuffer(GrContext* context)
    : INHERITED(context)
    , fCommands(GrCommandBuilder::Create(context->getGpu(), false))
    , fPathIndexBuffer(kPathIdxBufferMinReserve * sizeof(char)/4)
    , fPathTransformBuffer(kPathXformBufferMinReserve * sizeof(float)/4)
    , fPipelineBuffer(kPipelineBufferMinReserve)
    , fDrawID(0) {
}

GrInOrderDrawBuffer::~GrInOrderDrawBuffer() {
    this->reset();
}

void GrInOrderDrawBuffer::onDrawBatch(GrBatch* batch,
                                      const PipelineInfo& pipelineInfo) {
    State* state = this->setupPipelineAndShouldDraw(batch, pipelineInfo);
    if (!state) {
        return;
    }

    GrTargetCommands::Cmd* cmd = fCommands->recordDrawBatch(state, batch);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::onStencilPath(const GrPipelineBuilder& pipelineBuilder,
                                        const GrPathProcessor* pathProc,
                                        const GrPath* path,
                                        const GrScissorState& scissorState,
                                        const GrStencilSettings& stencilSettings) {
    GrTargetCommands::Cmd* cmd = fCommands->recordStencilPath(pipelineBuilder,
                                                              pathProc, path, scissorState,
                                                              stencilSettings);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::onDrawPath(const GrPathProcessor* pathProc,
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

void GrInOrderDrawBuffer::onDrawPaths(const GrPathProcessor* pathProc,
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

void GrInOrderDrawBuffer::onClear(const SkIRect* rect, GrColor color,
                                  bool canIgnoreRect, GrRenderTarget* renderTarget) {
    GrTargetCommands::Cmd* cmd = fCommands->recordClear(rect, color, canIgnoreRect, renderTarget);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::clearStencilClip(const SkIRect& rect,
                                           bool insideClip,
                                           GrRenderTarget* renderTarget) {
    GrTargetCommands::Cmd* cmd = fCommands->recordClearStencilClip(rect, insideClip, renderTarget);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::discard(GrRenderTarget* renderTarget) {
    if (!this->caps()->discardRenderTargetSupport()) {
        return;
    }

    GrTargetCommands::Cmd* cmd = fCommands->recordDiscard(renderTarget);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::onReset() {
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

void GrInOrderDrawBuffer::onFlush() {
    fCommands->flush(this);
    ++fDrawID;
}

void GrInOrderDrawBuffer::onCopySurface(GrSurface* dst,
                                        GrSurface* src,
                                        const SkIRect& srcRect,
                                        const SkIPoint& dstPoint) {
    SkASSERT(this->getGpu()->canCopySurface(dst, src, srcRect, dstPoint));
    GrTargetCommands::Cmd* cmd = fCommands->recordCopySurface(dst, src, srcRect, dstPoint);
    this->recordTraceMarkersIfNecessary(cmd);
}

void GrInOrderDrawBuffer::recordTraceMarkersIfNecessary(GrTargetCommands::Cmd* cmd) {
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
GrInOrderDrawBuffer::setupPipelineAndShouldDraw(const GrPrimitiveProcessor* primProc,
                                                const GrDrawTarget::PipelineInfo& pipelineInfo) {
    State* state = this->allocState(primProc);
    this->setupPipeline(pipelineInfo, state->pipelineLocation());

    if (state->getPipeline()->mustSkip()) {
        this->unallocState(state);
        return NULL;
    }

    state->fPrimitiveProcessor->initBatchTracker(&state->fBatchTracker,
                                                 state->getPipeline()->getInitBatchTracker());

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
GrInOrderDrawBuffer::setupPipelineAndShouldDraw(GrBatch* batch,
                                                const GrDrawTarget::PipelineInfo& pipelineInfo) {
    State* state = this->allocState();
    this->setupPipeline(pipelineInfo, state->pipelineLocation());

    if (state->getPipeline()->mustSkip()) {
        this->unallocState(state);
        return NULL;
    }

    batch->initBatchTracker(state->getPipeline()->getInitBatchTracker());

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
