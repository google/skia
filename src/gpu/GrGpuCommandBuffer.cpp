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

void GrGpuRTCommandBuffer::clear(const GrFixedClip& clip, GrColor color) {
    SkASSERT(fRenderTarget);

    this->onClear(clip, color);
}

void GrGpuRTCommandBuffer::clearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    this->onClearStencilClip(clip, insideStencilMask);
}

bool GrGpuRTCommandBuffer::draw(const GrPrimitiveProcessor& primProc, const GrPipeline& pipeline,
                                const GrPipeline::FixedDynamicState* fixedDynamicState,
                                const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                const GrMesh meshes[], int meshCount, const SkRect& bounds) {
#ifdef SK_DEBUG
    SkASSERT(!primProc.hasInstanceAttributes() || this->gpu()->caps()->instanceAttribSupport());
    for (int i = 0; i < meshCount; ++i) {
        SkASSERT(!GrPrimTypeRequiresGeometryShaderSupport(meshes[i].primitiveType()) ||
                 this->gpu()->caps()->shaderCaps()->geometryShaderSupport());
        SkASSERT(primProc.hasVertexAttributes() == meshes[i].hasVertexData());
        SkASSERT(primProc.hasInstanceAttributes() == meshes[i].isInstanced());
    }
#endif
    SkASSERT(!pipeline.isScissorEnabled() || fixedDynamicState ||
             (dynamicStateArrays && dynamicStateArrays->fScissorRects));

    auto resourceProvider = this->gpu()->getContext()->contextPriv().resourceProvider();

    if (pipeline.isBad() || !primProc.instantiate(resourceProvider)) {
        return false;
    }

    if (primProc.numVertexAttributes() > this->gpu()->caps()->maxVertexAttributes()) {
        this->gpu()->stats()->incNumFailedDraws();
        return false;
    }
    this->onDraw(primProc, pipeline, fixedDynamicState, dynamicStateArrays, meshes, meshCount,
                 bounds);
    return true;
}
