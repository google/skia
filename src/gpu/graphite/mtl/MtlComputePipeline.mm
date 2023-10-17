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
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/mtl/MtlGraphiteUtilsPriv.h"
#include "src/gpu/graphite/mtl/MtlSharedContext.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"

namespace skgpu::graphite {

// static
sk_sp<MtlComputePipeline> MtlComputePipeline::Make(const MtlSharedContext* sharedContext,
                                                   const std::string& label,
                                                   MSLFunction computeMain) {
    id<MTLLibrary> library = std::get<0>(computeMain);
    if (!library) {
        return nullptr;
    }

    sk_cfp<MTLComputePipelineDescriptor*> psoDescriptor([MTLComputePipelineDescriptor new]);

    (*psoDescriptor).label = @(label.c_str());

    NSString* entryPointName = [NSString stringWithUTF8String:std::get<1>(computeMain).c_str()];
    (*psoDescriptor).computeFunction = [library newFunctionWithName:entryPointName];

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
                                       reflection:nil
                                            error:&error]);
    if (!pso) {
        SKGPU_LOG_E("Compute pipeline creation failure:\n%s", error.debugDescription.UTF8String);
        return nullptr;
    }

    return sk_sp<MtlComputePipeline>(new MtlComputePipeline(sharedContext, std::move(pso)));
}

void MtlComputePipeline::freeGpuData() { fPipelineState.reset(); }

}  // namespace skgpu::graphite
