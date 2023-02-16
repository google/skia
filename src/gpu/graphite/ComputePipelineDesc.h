/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_ComputePipelineDesc_DEFINED
#define skgpu_graphite_ComputePipelineDesc_DEFINED

#include "src/gpu/graphite/compute/ComputeStep.h"

#include <functional>

namespace skgpu::graphite {

/**
 * ComputePipelineDesc represents the state needed to create a backend specific ComputePipeline.
 */
class ComputePipelineDesc {
public:
    // `computeStep` must outlive this `ComputePipelineDesc`.
    explicit ComputePipelineDesc(const ComputeStep* computeStep) : fComputeStep(computeStep) {}

    bool operator==(const ComputePipelineDesc& that) const {
        return fComputeStep->uniqueID() == that.fComputeStep->uniqueID();
    }

    const ComputeStep* computeStep() const { return fComputeStep; }

    uint32_t uniqueID() const { return fComputeStep->uniqueID(); }

private:
    const ComputeStep* fComputeStep;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_ComputePipelineDesc_DEFINED
