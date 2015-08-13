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

void GrImmediateDrawTarget::onDrawBatch(GrDrawBatch* batch) {
    fBatchTarget.resetNumberOfDraws();

    // TODO: encapsulate the specialization of GrVertexBatch in GrVertexBatch so that we can
    // remove this cast. Currently all GrDrawBatches are in fact GrVertexBatch.
    GrVertexBatch* vertexBatch = static_cast<GrVertexBatch*>(batch);
    vertexBatch->generateGeometry(&fBatchTarget);
    vertexBatch->setNumberOfDraws(fBatchTarget.numberOfDraws());

    fBatchTarget.preFlush();
    fBatchTarget.flushNext(vertexBatch->numberOfDraws());
    fBatchTarget.postFlush();
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
