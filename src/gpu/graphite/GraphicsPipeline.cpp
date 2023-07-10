/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/utils/SkShaderUtils.h"

namespace skgpu::graphite {

GraphicsPipeline::GraphicsPipeline(const SharedContext* sharedContext, Shaders* pipelineShaders)
        : Resource(sharedContext, Ownership::kOwned, skgpu::Budgeted::kYes, /*gpuMemorySize=*/0) {
#if GRAPHITE_TEST_UTILS
    if (pipelineShaders) {
        fPipelineShaders.fSkSLVertexShader =
                SkShaderUtils::PrettyPrint(pipelineShaders->fSkSLVertexShader);
        fPipelineShaders.fSkSLFragmentShader =
                SkShaderUtils::PrettyPrint(pipelineShaders->fSkSLFragmentShader);
        fPipelineShaders.fNativeVertexShader = std::move(pipelineShaders->fNativeVertexShader);
        fPipelineShaders.fNativeFragmentShader = std::move(pipelineShaders->fNativeFragmentShader);
    }
#endif
}

GraphicsPipeline::~GraphicsPipeline() = default;

}  // namespace skgpu::graphite
