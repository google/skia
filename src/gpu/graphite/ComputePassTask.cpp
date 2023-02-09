/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ComputePassTask.h"

#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/ResourceProvider.h"

namespace skgpu::graphite {

sk_sp<ComputePassTask> ComputePassTask::Make(std::vector<ResourceBinding> bindings,
                                             const ComputePipelineDesc& pipelineDesc,
                                             const ComputePassDesc& desc) {
    return sk_sp<ComputePassTask>(new ComputePassTask(std::move(bindings), pipelineDesc, desc));
}

ComputePassTask::ComputePassTask(std::vector<ResourceBinding> bindings,
                                 const ComputePipelineDesc& pipelineDesc,
                                 const ComputePassDesc& desc)
        : fPipelineDesc(pipelineDesc)
        , fComputePassDesc(desc)
        , fBindings(std::move(bindings)) {}

bool ComputePassTask::prepareResources(ResourceProvider* provider,
                                       const RuntimeEffectDictionary*) {
    fPipeline = provider->findOrCreateComputePipeline(fPipelineDesc);
    return fPipeline != nullptr;
}

bool ComputePassTask::addCommands(Context*, CommandBuffer* commandBuffer, ReplayTargetData) {
    SkASSERT(fPipeline);
    return commandBuffer->addComputePass(fComputePassDesc, fPipeline, fBindings);
}

}  // namespace skgpu::graphite
