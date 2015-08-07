/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrImmediateDrawTarget.h"

#include "GrGpu.h"
#include "GrPipeline.h"
#include "GrRenderTarget.h"
#include "SkRect.h"
#include "SkTypes.h"

#include "batches/GrBatch.h"

GrImmediateDrawTarget::GrImmediateDrawTarget(GrContext* context)
    : INHERITED(context)
    , fBatchTarget(this->getGpu())
    , fDrawID(0) {
}

GrImmediateDrawTarget::~GrImmediateDrawTarget() {
    this->reset();
}

void GrImmediateDrawTarget::onDrawBatch(GrBatch* batch,
                                        const PipelineInfo& pipelineInfo) {
    SkAlignedSStorage<sizeof(GrPipeline)> pipelineStorage;
    bool shouldDraw = this->setupPipelineAndShouldDraw(pipelineStorage.get(), pipelineInfo);
    GrPipeline* pipeline = reinterpret_cast<GrPipeline*>(pipelineStorage.get());

    if (!shouldDraw) {
        pipeline->~GrPipeline();
        return;
    }

    batch->initBatchTracker(pipeline->infoForPrimitiveProcessor());

    fBatchTarget.resetNumberOfDraws();

    batch->generateGeometry(&fBatchTarget);
    batch->setNumberOfDraws(fBatchTarget.numberOfDraws());

    fBatchTarget.preFlush();
    fBatchTarget.flushNext(batch->numberOfDraws());
    fBatchTarget.postFlush();

    pipeline->~GrPipeline();
}

void GrImmediateDrawTarget::onClear(const SkIRect& rect, GrColor color,
                                    GrRenderTarget* renderTarget) {
    this->getGpu()->clear(rect, color, renderTarget);
}

void GrImmediateDrawTarget::onCopySurface(GrSurface* dst,
                                          GrSurface* src,
                                          const SkIRect& srcRect,
                                          const SkIPoint& dstPoint) {
    this->getGpu()->copySurface(dst, src, srcRect, dstPoint);
}

void GrImmediateDrawTarget::clearStencilClip(const SkIRect& rect,
                                             bool insideClip,
                                             GrRenderTarget* renderTarget) {
    this->getGpu()->clearStencilClip(rect, insideClip, renderTarget);
}

void GrImmediateDrawTarget::discard(GrRenderTarget* renderTarget) {
    if (!this->caps()->discardRenderTargetSupport()) {
        return;
    }

    this->getGpu()->discard(renderTarget);
}

void GrImmediateDrawTarget::onReset() {
    fBatchTarget.reset();
}

void GrImmediateDrawTarget::onFlush() {
    ++fDrawID;
}

bool
GrImmediateDrawTarget::setupPipelineAndShouldDraw(void* pipelineAddr,
                                                  const GrDrawTarget::PipelineInfo& pipelineInfo) {
    const GrPipeline* pipeline = this->setupPipeline(pipelineInfo, pipelineAddr);

    if (pipeline->mustSkip()) {
        return false;
    }

    this->recordXferBarrierIfNecessary(pipeline);
    return true;
}

void GrImmediateDrawTarget::recordXferBarrierIfNecessary(const GrPipeline* pipeline) {
    const GrXferProcessor& xp = *pipeline->getXferProcessor();
    GrRenderTarget* rt = pipeline->getRenderTarget();

    GrXferBarrierType barrierType;
    if (xp.willNeedXferBarrier(rt, *this->caps(), &barrierType)) {
        this->getGpu()->xferBarrier(rt, barrierType);
    }
}
