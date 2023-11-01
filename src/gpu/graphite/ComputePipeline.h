/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_ComputePipeline_DEFINED
#define skgpu_graphite_ComputePipeline_DEFINED

#include "src/gpu/graphite/Resource.h"

namespace skgpu::graphite {

class SharedContext;

/**
 * ComputePipeline corresponds to a backend specific pipeline used for compute (vs rendering),
 * e.g. MTLComputePipelineState (Metal),
 *      CreateComputePipeline (Dawn),
 *      CreateComputePipelineState (D3D12),
 *   or VkComputePipelineCreateInfo (Vulkan).
 */
class ComputePipeline : public Resource {
public:
    ~ComputePipeline() override = default;

    // TODO(b/240615224): The pipeline should return an optional effective local workgroup
    // size if the value was statically assigned in the shader (when it's not possible to assign
    // them via specialization constants).

    const char* getResourceType() const override { return "Compute Pipeline"; }

protected:
    explicit ComputePipeline(const SharedContext*);
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputePipeline_DEFINED
