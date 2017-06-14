/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrGpuCommandBuffer.h"

#include "GrContext.h"
#include "GrCaps.h"
#include "GrFixedClip.h"
#include "GrGpu.h"
#include "GrMesh.h"
#include "GrPrimitiveProcessor.h"
#include "GrRenderTarget.h"
#include "SkRect.h"

void GrGpuCommandBuffer::submit() {
    this->gpu()->handleDirtyContext();
    this->onSubmit();
}

void GrGpuCommandBuffer::clear(GrRenderTarget* rt, const GrFixedClip& clip, GrColor color) {
#ifdef SK_DEBUG
    SkASSERT(rt);
    SkASSERT(!clip.scissorEnabled() ||
             (SkIRect::MakeWH(rt->width(), rt->height()).contains(clip.scissorRect()) &&
              SkIRect::MakeWH(rt->width(), rt->height()) != clip.scissorRect()));
#endif
    this->onClear(rt, clip, color);
}

void GrGpuCommandBuffer::clearStencilClip(GrRenderTarget* rt, const GrFixedClip& clip,
                                          bool insideStencilMask) {
    this->onClearStencilClip(rt, clip, insideStencilMask);
}

bool GrGpuCommandBuffer::draw(const GrPipeline& pipeline,
                              const GrPrimitiveProcessor& primProc,
                              const GrMesh meshes[],
                              const GrPipeline::DynamicState dynamicStates[],
                              int meshCount,
                              const SkRect& bounds) {
#ifdef SK_DEBUG
    SkASSERT(!primProc.hasInstanceAttribs() || this->gpu()->caps()->instanceAttribSupport());
    for (int i = 0; i < meshCount; ++i) {
        SkASSERT(!GrPrimTypeRequiresGeometryShaderSupport(meshes[i].primitiveType()) ||
                 this->gpu()->caps()->shaderCaps()->geometryShaderSupport());
        SkASSERT(primProc.hasVertexAttribs() == meshes[i].hasVertexData());
        SkASSERT(primProc.hasInstanceAttribs() == meshes[i].isInstanced());
    }
#endif

    if (pipeline.isBad() || !primProc.instantiate(this->gpu()->getContext()->resourceProvider())) {
        return false;
    }

    SkASSERT(pipeline.isInitialized());
    if (primProc.numAttribs() > this->gpu()->caps()->maxVertexAttributes()) {
        this->gpu()->stats()->incNumFailedDraws();
        return false;
    }
    this->onDraw(pipeline, primProc, meshes, dynamicStates, meshCount, bounds);
    return true;
}

