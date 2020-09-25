/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathShader_DEFINED
#define GrPathShader_DEFINED

#include "src/core/SkArenaAlloc.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

 // This is a common base class for shaders in the GPU tessellator.
class GrPathShader : public GrGeometryProcessor {
public:
    GrPathShader(ClassID classID, const SkMatrix& viewMatrix, GrPrimitiveType primitiveType,
                 int tessellationPatchVertexCount)
            : GrGeometryProcessor(classID)
            , fViewMatrix(viewMatrix)
            , fPrimitiveType(primitiveType)
            , fTessellationPatchVertexCount(tessellationPatchVertexCount) {
        if (fTessellationPatchVertexCount) {
            this->setWillUseTessellationShaders();
        }
    }

    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    int tessellationPatchVertexCount() const { return fTessellationPatchVertexCount; }
    const SkMatrix& viewMatrix() const { return fViewMatrix; }

    static GrProgramInfo* MakeProgramInfo(const GrPathShader* shader, SkArenaAlloc* arena,
                                          const GrSurfaceProxyView* writeView,
                                          GrPipeline::InputFlags pipelineFlags,
                                          GrProcessorSet&& processors, GrAppliedClip&& appliedClip,
                                          const GrXferProcessor::DstProxyView& dstProxyView,
                                          GrXferBarrierFlags renderPassXferBarriers,
                                          const GrUserStencilSettings* stencil,
                                          const GrCaps& caps) {
        auto* pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(
                &caps, arena, writeView->swizzle(), std::move(appliedClip), dstProxyView,
                std::move(processors), pipelineFlags);
        return MakeProgramInfo(shader, arena, writeView, pipeline, dstProxyView,
                               renderPassXferBarriers, stencil, caps);
    }

    static GrProgramInfo* MakeProgramInfo(const GrPathShader* shader, SkArenaAlloc* arena,
                                          const GrSurfaceProxyView* writeView,
                                          const GrPipeline* pipeline,
                                          const GrXferProcessor::DstProxyView& dstProxyView,
                                          GrXferBarrierFlags renderPassXferBarriers,
                                          const GrUserStencilSettings* stencil,
                                          const GrCaps& caps) {
        GrRenderTargetProxy* proxy = writeView->asRenderTargetProxy();
        return arena->make<GrProgramInfo>(proxy->numSamples(), proxy->numStencilSamples(),
                                          proxy->backendFormat(), writeView->origin(), pipeline,
                                          stencil, shader, shader->fPrimitiveType,
                                          shader->fTessellationPatchVertexCount,
                                          renderPassXferBarriers);
    }

private:
    const SkMatrix fViewMatrix;
    const GrPrimitiveType fPrimitiveType;
    const int fTessellationPatchVertexCount;

    class Impl;
};

#endif
