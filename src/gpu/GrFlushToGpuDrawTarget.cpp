/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrFlushToGpuDrawTarget.h"
#include "GrContext.h"
#include "GrGpu.h"
#include "GrBufferAllocPool.h"

GrFlushToGpuDrawTarget::GrFlushToGpuDrawTarget(GrGpu* gpu,
                                               GrVertexBufferAllocPool* vertexPool,
                                               GrIndexBufferAllocPool* indexPool)
    : INHERITED(gpu->getContext())
    , fGpu(SkRef(gpu))
    , fVertexPool(vertexPool)
    , fIndexPool(indexPool)
    , fFlushing(false) {

    fCaps.reset(SkRef(fGpu->caps()));

    SkASSERT(vertexPool);
    SkASSERT(indexPool);

}

void GrFlushToGpuDrawTarget::reset() {
    fVertexPool->reset();
    fIndexPool->reset();

    this->onReset();
}

void GrFlushToGpuDrawTarget::flush() {
    if (fFlushing) {
        return;
    }
    fFlushing = true;

    fGpu->saveActiveTraceMarkers();

    this->onFlush();

    fGpu->restoreActiveTraceMarkers();

    fFlushing = false;
    this->reset();
}

bool GrFlushToGpuDrawTarget::onCanCopySurface(const GrSurface* dst,
                                              const GrSurface* src,
                                              const SkIRect& srcRect,
                                              const SkIPoint& dstPoint) {
    return getGpu()->canCopySurface(dst, src, srcRect, dstPoint);
}

bool GrFlushToGpuDrawTarget::onInitCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) {
    return getGpu()->initCopySurfaceDstDesc(src, desc);
}
