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

#include "batches/GrDrawBatch.h"
#include "batches/GrVertexBatch.h"

GrImmediateDrawTarget::GrImmediateDrawTarget(GrContext* context)
    : INHERITED(context)
    , fDrawID(0) {
}

GrImmediateDrawTarget::~GrImmediateDrawTarget() {
    this->reset();
}

void GrImmediateDrawTarget::onDrawBatch(GrBatch* batch) {

#if 0
    // TODO: encapsulate the specialization of GrVertexBatch in GrVertexBatch so that we can
    // remove this cast. Currently all GrDrawBatches are in fact GrVertexBatch.
    GrVertexBatch* vertexBatch = static_cast<GrVertexBatch*>(batch);
    vertexBatch->prepareDraws(&fBatchTarget);
    vertexBatch->setNumberOfDraws(fBatchTarget.numberOfDraws());

    fBatchTarget.preFlush();
    fBatchTarget.flushNext(vertexBatch->numberOfDraws());
    fBatchTarget.postFlush();
#endif
}

void GrImmediateDrawTarget::onCopySurface(GrSurface* dst,
                                          GrSurface* src,
                                          const SkIRect& srcRect,
                                          const SkIPoint& dstPoint) {
    this->getGpu()->copySurface(dst, src, srcRect, dstPoint);
}

void GrImmediateDrawTarget::onReset() {}

void GrImmediateDrawTarget::onFlush() {
    ++fDrawID;
}
