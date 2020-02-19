/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/GrOpsRenderPass.h"

#include "include/core/SkRect.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"

void GrOpsRenderPass::clear(const GrFixedClip& clip, const SkPMColor4f& color) {
    SkASSERT(fRenderTarget);
    // A clear at this level will always be a true clear, so make sure clears were not supposed to
    // be redirected to draws instead
    SkASSERT(!this->gpu()->caps()->performColorClearsAsDraws());
    SkASSERT(!clip.scissorEnabled() || !this->gpu()->caps()->performPartialClearsAsDraws());
    fDrawPipelineStatus = DrawPipelineStatus::kNotConfigured;
    this->onClear(clip, color);
}

void GrOpsRenderPass::clearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    // As above, make sure the stencil clear wasn't supposed to be a draw rect with stencil settings
    SkASSERT(!this->gpu()->caps()->performStencilClearsAsDraws());
    fDrawPipelineStatus = DrawPipelineStatus::kNotConfigured;
    this->onClearStencilClip(clip, insideStencilMask);
}

void GrOpsRenderPass::executeDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable) {
    fDrawPipelineStatus = DrawPipelineStatus::kNotConfigured;
    this->onExecuteDrawable(std::move(drawable));
}

void GrOpsRenderPass::bindPipeline(const GrProgramInfo& programInfo, const SkRect& drawBounds) {
#ifdef SK_DEBUG
    if (programInfo.primProc().hasInstanceAttributes()) {
         SkASSERT(this->gpu()->caps()->instanceAttribSupport());
    }
    if (programInfo.pipeline().usesConservativeRaster()) {
        SkASSERT(this->gpu()->caps()->conservativeRasterSupport());
        // Conservative raster, by default, only supports triangles. Implementations can
        // optionally indicate that they also support points and lines, but we don't currently
        // query or track that info.
        SkASSERT(GrIsPrimTypeTris(programInfo.primitiveType()));
    }
    if (programInfo.pipeline().isWireframe()) {
         SkASSERT(this->gpu()->caps()->wireframeSupport());
    }
    if (GrPrimitiveType::kPatches == programInfo.primitiveType()) {
        SkASSERT(this->gpu()->caps()->shaderCaps()->tessellationSupport());
    }
    programInfo.checkAllInstantiated();
    programInfo.checkMSAAAndMIPSAreResolved();
#endif

    if (programInfo.primProc().numVertexAttributes() > this->gpu()->caps()->maxVertexAttributes()) {
        fDrawPipelineStatus = DrawPipelineStatus::kFailedToBind;
        return;
    }

    if (!this->onBindPipeline(programInfo, drawBounds)) {
        fDrawPipelineStatus = DrawPipelineStatus::kFailedToBind;
        return;
    }

#ifdef SK_DEBUG
    GrProcessor::CustomFeatures processorFeatures = programInfo.requestedFeatures();
    if (GrProcessor::CustomFeatures::kSampleLocations & processorFeatures) {
        // Verify we always have the same sample pattern key, regardless of graphics state.
        SkASSERT(this->gpu()->findOrAssignSamplePatternKey(fRenderTarget)
                         == fRenderTarget->renderTargetPriv().getSamplePatternKey());
    }
#endif

    fDrawPipelineStatus = DrawPipelineStatus::kOk;
}

void GrOpsRenderPass::drawMeshes(const GrProgramInfo& programInfo, const GrMesh meshes[],
                                 int meshCount) {
    if (DrawPipelineStatus::kOk != fDrawPipelineStatus) {
        SkASSERT(DrawPipelineStatus::kNotConfigured != fDrawPipelineStatus);
        this->gpu()->stats()->incNumFailedDraws();
        return;
    }

#ifdef SK_DEBUG
    if (int numDynamicStateArrays = programInfo.numDynamicStateArrays()) {
        SkASSERT(meshCount == numDynamicStateArrays);
    }
    for (int i = 0; i < meshCount; ++i) {
        SkASSERT(programInfo.primProc().hasVertexAttributes() ==
                 SkToBool(meshes[i].vertexBuffer()));
        SkASSERT(programInfo.primProc().hasInstanceAttributes() ==
                 SkToBool(meshes[i].instanceBuffer()));
        if (GrPrimitiveRestart::kYes == meshes[i].primitiveRestart()) {
             SkASSERT(this->gpu()->caps()->usePrimitiveRestart());
        }
    }
#endif

    if (meshCount) {
        this->onDrawMeshes(programInfo, meshes, meshCount);
    }
}
