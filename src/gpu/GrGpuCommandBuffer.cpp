/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrGpuCommandBuffer.h"

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
                              int meshCount,
                              const SkRect& bounds) {
#ifdef SK_DEBUG
    for (int i = 0; i < meshCount; ++i) {
        const GrMesh& mesh = meshes[i];

        SkASSERT(SkToBool(mesh.fIndexBuffer.get()) == SkToBool(mesh.fIndexCount));
        SkASSERT(mesh.fIndexCount >= 0);
        SkASSERT(mesh.fBaseIndex >= 0);

        SkASSERT(SkToBool(mesh.fVertexBuffer.get()) == primProc.hasVertexAttribs());
        SkASSERT(SkToBool(mesh.fVertexCount) == primProc.hasVertexAttribs());
        SkASSERT(mesh.fVertexCount >= 0);
        SkASSERT(mesh.fBaseVertex >= 0);

        SkASSERT(!primProc.hasInstanceAttribs() || this->gpu()->caps()->instancedDrawingSupport());
        SkASSERT(SkToBool(mesh.fInstanceBuffer.get()) == primProc.hasInstanceAttribs());
        SkASSERT(SkToBool(mesh.fInstanceCount) == primProc.hasInstanceAttribs());
        SkASSERT(mesh.fInstanceCount >= 0);
        SkASSERT(mesh.fBaseInstance >= 0);

        SkASSERT(mesh.fVertexBuffer || mesh.fInstanceBuffer);

        SkASSERT(1 == mesh.fPatternRepeatCount || mesh.fIndexBuffer);
        SkASSERT(mesh.fPatternRepeatCount > 0);
        SkASSERT(mesh.fMaxPatternRepetitionsInIndexBuffer > 0);
    }
#endif

    if (pipeline.isBad() || primProc.isBad()) {
        return false;
    }

    SkASSERT(pipeline.isInitialized());
    if (primProc.numAttribs() > this->gpu()->caps()->maxVertexAttributes()) {
        this->gpu()->stats()->incNumFailedDraws();
        return false;
    }
    this->onDraw(pipeline, primProc, meshes, meshCount, bounds);
    return true;
}

