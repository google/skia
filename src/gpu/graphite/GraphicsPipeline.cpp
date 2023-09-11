/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/utils/SkShaderUtils.h"

namespace skgpu::graphite {

GraphicsPipeline::GraphicsPipeline(const SharedContext* sharedContext, PipelineInfo* pipelineInfo)
        : Resource(sharedContext, Ownership::kOwned, skgpu::Budgeted::kYes, /*gpuMemorySize=*/0) {
#if defined(GRAPHITE_TEST_UTILS)
    if (pipelineInfo) {
        fPipelineInfo.fRenderStepID = pipelineInfo->fRenderStepID;
        fPipelineInfo.fPaintID = pipelineInfo->fPaintID;
        fPipelineInfo.fSkSLVertexShader =
                SkShaderUtils::PrettyPrint(pipelineInfo->fSkSLVertexShader);
        fPipelineInfo.fSkSLFragmentShader =
                SkShaderUtils::PrettyPrint(pipelineInfo->fSkSLFragmentShader);
        fPipelineInfo.fNativeVertexShader = std::move(pipelineInfo->fNativeVertexShader);
        fPipelineInfo.fNativeFragmentShader = std::move(pipelineInfo->fNativeFragmentShader);
    }
#endif
}

GraphicsPipeline::~GraphicsPipeline() = default;

}  // namespace skgpu::graphite
