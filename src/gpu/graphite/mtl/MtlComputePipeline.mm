/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/mtl/MtlComputePipeline.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/gpu/graphite/mtl/MtlUtils.h"

namespace skgpu::graphite {

// static
sk_sp<MtlComputePipeline> MtlComputePipeline::Make(MtlResourceProvider* resourceProvider,
                                                   const MtlSharedContext* sharedContext,
                                                   const ComputePipelineDesc& pipelineDesc) {
    sk_cfp<MTLComputePipelineDescriptor*> psoDescriptor([MTLComputePipelineDescriptor new]);

    std::string msl;
    SkSL::Program::Inputs inputs;
    SkSL::ProgramSettings settings;

    ShaderErrorHandler* errorHandler = sharedContext->caps()->shaderErrorHandler();
    if (!SkSLToMSL(sharedContext,
                   pipelineDesc.sksl(),
                   SkSL::ProgramKind::kCompute,
                   settings,
                   &msl,
                   &inputs,
                   errorHandler)) {
        return nullptr;
    }

    sk_cfp<id<MTLLibrary>> shaderLibrary = MtlCompileShaderLibrary(sharedContext,
                                                                   msl,
                                                                   errorHandler);
    if (!shaderLibrary) {
        return nullptr;
    }

    (*psoDescriptor).label = @(pipelineDesc.name().c_str());
    (*psoDescriptor).computeFunction = [shaderLibrary.get() newFunctionWithName:@"computeMain"];

    // TODO(b/240604614): Populate input data attribute and buffer layout descriptors using the
    // `stageInputDescriptor` property based on the contents of `pipelineDesc` (on iOS 10+ or
    // macOS 10.12+).

    // TODO(b/240604614): Define input buffer mutability using the `buffers` property based on
    // the contents of `pipelineDesc` (on iOS 11+ or macOS 10.13+).

    // TODO(b/240615224): Metal docs claim that setting the
    // `threadGroupSizeIsMultipleOfThreadExecutionWidth` to YES may improve performance, IF we can
    // guarantee that the thread group size used in a dispatch command is a multiple of
    // `threadExecutionWidth` property of the pipeline state object (otherwise this will cause UB).

    NSError* error;
    sk_cfp<id<MTLComputePipelineState>> pso([sharedContext->device()
            newComputePipelineStateWithDescriptor:psoDescriptor.get()
                                          options:MTLPipelineOptionNone
                                       reflection:NULL
                                            error:&error]);
    if (!pso) {
        SKGPU_LOG_E("Compute pipeline creation failure:\n%s", error.debugDescription.UTF8String);
        return nullptr;
    }

    return sk_sp<MtlComputePipeline>(new MtlComputePipeline(sharedContext, std::move(pso)));
}

void MtlComputePipeline::freeGpuData() { fPipelineState.reset(); }

}  // namespace skgpu::graphite
