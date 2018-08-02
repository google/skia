/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrMtlGpuCommandBuffer.h"
#include "GrMtlPipelineState.h"
#include "GrMtlPipelineStateBuilder.h"
#include "GrMesh.h"

void GrMtlGpuRTCommandBuffer::onDraw(const GrPrimitiveProcessor& primProc,
                                     const GrPipeline& pipeline,
                                     const GrPipeline::FixedDynamicState* fixedDynamicState,
                                     const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                     const GrMesh mesh[],
                                     int meshCount,
                                     const SkRect& bounds) {
    // TODO: resolve textures and regenerate mipmaps as needed

    bool hasPoints = false;
    for (int i = 0; i < meshCount; ++i) {
        if (mesh[i].primitiveType() == GrPrimitiveType::kPoints) {
            hasPoints = true;
            break;
        }
    }

    GrProgramDesc desc;
    if (!GrProgramDesc::Build(&desc, primProc, hasPoints, pipeline, *fGpu->caps()->shaderCaps())) {
        return;
    }
    desc.finalize();

    std::unique_ptr<GrMtlPipelineState> pipelineState(
            GrMtlPipelineStateBuilder::CreatePipelineState(primProc, pipeline, &desc, fGpu));

    (void) pipelineState;
}
