/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/GrGpuCommandBuffer.h"

#include "include/core/SkRect.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrRenderTarget.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrPrimitiveProcessor.h"

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

    if (pipeline.isBad()) {
        return false;
    }
    for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
        if (!fixedDynamicState->fPrimitiveProcessorTextures[i]->instantiate(resourceProvider)) {
            return false;
        }
    }

    if (primProc.numVertexAttributes() > this->gpu()->caps()->maxVertexAttributes()) {
        this->gpu()->stats()->incNumFailedDraws();
        return false;
    }
    this->onDraw(primProc, pipeline, fixedDynamicState, dynamicStateArrays, meshes, meshCount,
                 bounds);
    return true;
}
