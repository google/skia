/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GraphicsPipeline.h"

#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/utils/SkShaderUtils.h"

namespace skgpu::graphite {

GraphicsPipeline::GraphicsPipeline(const SharedContext* sharedContext,
                                   const PipelineInfo& pipelineInfo)
        : Resource(sharedContext,
                   Ownership::kOwned,
                   skgpu::Budgeted::kYes,
                   /*gpuMemorySize=*/0)
        , fPipelineInfo(pipelineInfo) {}

GraphicsPipeline::~GraphicsPipeline() = default;

GraphicsPipeline::PipelineInfo::PipelineInfo(const VertSkSLInfo& vsInfo,
                                             const FragSkSLInfo& fsInfo)
        : fDstReadReq(fsInfo.fDstReadReq)
        , fNumFragTexturesAndSamplers(fsInfo.fNumTexturesAndSamplers)
        , fHasPaintUniforms(fsInfo.fHasPaintUniforms)
        , fHasStepUniforms(vsInfo.fHasStepUniforms)
        , fHasGradientBuffer(fsInfo.fHasGradientBuffer) {
#if defined(GPU_TEST_UTILS)
    fSkSLVertexShader = SkShaderUtils::PrettyPrint(vsInfo.fSkSL);
    fSkSLFragmentShader = SkShaderUtils::PrettyPrint(fsInfo.fSkSL);
    fLabel = fsInfo.fLabel;
#endif
}

}  // namespace skgpu::graphite
