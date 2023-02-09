/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputePassTask_DEFINED
#define skgpu_graphite_ComputePassTask_DEFINED

#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ComputeTypes.h"
#include "src/gpu/graphite/Task.h"

#include <vector>

namespace skgpu::graphite {

class Buffer;
class ComputePipeline;

/**
 * ComputePassTask records a compute kernel and its associated resources into a single compute pass
 * within a command buffer. The creation of bound resources and their binding layout is left up to
 * the caller and must be compatible with the provided the compute pipeline layout.
 */
// TODO(b/240604614): This class could facilitate some of this and also enforce layout validation
// depending on how we decide to define and combine compute shader code.
// TODO(b/240625222): This class could consume multiple ComputePass objects and encode them
// together. The individual passes could execute in parallel depending on the desired data-flow.
class ComputePassTask final : public Task {
public:
    static sk_sp<ComputePassTask> Make(std::vector<ResourceBinding> bindings,
                                       const ComputePipelineDesc&,
                                       const ComputePassDesc&);

    ~ComputePassTask() override = default;

    bool prepareResources(ResourceProvider*, const RuntimeEffectDictionary*) override;
    bool addCommands(Context*, CommandBuffer*, ReplayTargetData) override;

private:
    ComputePassTask(std::vector<ResourceBinding> bindings,
                    const ComputePipelineDesc&,
                    const ComputePassDesc&);

    const ComputePipelineDesc fPipelineDesc;
    const ComputePassDesc fComputePassDesc;
    const std::vector<ResourceBinding> fBindings;

    sk_sp<ComputePipeline> fPipeline;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputePassTask_DEFINED
