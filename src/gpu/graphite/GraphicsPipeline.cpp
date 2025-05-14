/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/GraphicsPipeline.h"

#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipelineDesc.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#include "src/gpu/graphite/Renderer.h"
#include "src/gpu/graphite/ShaderCodeDictionary.h"
#include "src/gpu/graphite/ShaderInfo.h"
#include "src/utils/SkShaderUtils.h"

namespace skgpu::graphite {

GraphicsPipeline::GraphicsPipeline(const SharedContext* sharedContext,
                                   const PipelineInfo& pipelineInfo)
        : Resource(sharedContext,
                   Ownership::kOwned,
                   /*gpuMemorySize=*/0)
        , fPipelineInfo(pipelineInfo) {}

GraphicsPipeline::~GraphicsPipeline() {
#if defined(SK_PIPELINE_LIFETIME_LOGGING)
    static const char* kNames[2] = { "DeletionN", "DeletionP" };
    TRACE_EVENT_INSTANT2("skia.gpu",
                         TRACE_STR_STATIC(kNames[this->fromPrecompile()]),
                         TRACE_EVENT_SCOPE_THREAD,
                         "key", this->getPipelineInfo().fUniqueKeyHash,
                         "compilationID", this->getPipelineInfo().fCompilationID);
#endif
}

GraphicsPipeline::PipelineInfo::PipelineInfo(
            const ShaderInfo& shaderInfo,
            SkEnumBitMask<PipelineCreationFlags> pipelineCreationFlags,
            uint32_t uniqueKeyHash,
            uint32_t compilationID)
        : fDstReadStrategy(shaderInfo.dstReadStrategy())
        , fNumFragTexturesAndSamplers(shaderInfo.numFragmentTexturesAndSamplers())
        , fHasPaintUniforms(shaderInfo.hasPaintUniforms())
        , fHasStepUniforms(shaderInfo.hasStepUniforms())
        , fHasGradientBuffer(shaderInfo.hasGradientBuffer())
        , fUniqueKeyHash(uniqueKeyHash)
        , fCompilationID(compilationID)
        , fFromPrecompile(pipelineCreationFlags & PipelineCreationFlags::kForPrecompilation) {
#if defined(GPU_TEST_UTILS)
    fSkSLVertexShader = SkShaderUtils::PrettyPrint(shaderInfo.vertexSkSL());
    fSkSLFragmentShader = SkShaderUtils::PrettyPrint(shaderInfo.fragmentSkSL());
    fLabel = shaderInfo.fsLabel();
#endif
}

#if defined(GPU_TEST_UTILS)
SkString GraphicsPipelineDesc::toString(ShaderCodeDictionary* dict) const {
    SkString tmp;

    tmp.append(RenderStep::RenderStepName(fRenderStepID));
    tmp.append(" - ");

    PaintParamsKey key = dict->lookup(fPaintID);

    tmp.append(key.toString(dict));

    return tmp;
}
#endif

}  // namespace skgpu::graphite
