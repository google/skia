/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrGpuCommandBuffer.h"

#include "GrCaps.h"
#include "GrGpu.h"
#include "GrPrimitiveProcessor.h"
#include "GrRenderTarget.h"
#include "SkRect.h"

void GrGpuCommandBuffer::submit(const SkIRect& bounds) {
    this->gpu()->handleDirtyContext();
    this->onSubmit(bounds);
}

void GrGpuCommandBuffer::clear(const SkIRect& rect, GrColor color, GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    SkASSERT(SkIRect::MakeWH(renderTarget->width(), renderTarget->height()).contains(rect));
    this->onClear(renderTarget, rect, color);
}

void GrGpuCommandBuffer::clearStencilClip(const SkIRect& rect,
                                          bool insideClip,
                                          GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);
    this->onClearStencilClip(renderTarget, rect, insideClip);
}


bool GrGpuCommandBuffer::draw(const GrPipeline& pipeline,
                              const GrPrimitiveProcessor& primProc,
                              const GrMesh* mesh,
                              int meshCount) {
    if (primProc.numAttribs() > this->gpu()->caps()->maxVertexAttributes()) {
        this->gpu()->stats()->incNumFailedDraws();
        return false;
    }
    this->onDraw(pipeline, primProc, mesh, meshCount);
    return true;
}

