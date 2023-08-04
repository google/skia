/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_DawnComputePipeline_DEFINED
#define skgpu_graphite_DawnComputePipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/ComputePipeline.h"

#include "webgpu/webgpu_cpp.h"

namespace skgpu::graphite {

class ComputePipelineDesc;
class DawnComputePipelineDesc;
class DawnSharedContext;

class DawnComputePipeline final : public ComputePipeline {
public:
    static sk_sp<DawnComputePipeline> Make(const DawnSharedContext*, const ComputePipelineDesc&);
    ~DawnComputePipeline() override = default;

    const wgpu::ComputePipeline& dawnComputePipeline() const { return fPipeline; }

private:
    DawnComputePipeline(const SharedContext*, wgpu::ComputePipeline);

    void freeGpuData() override;

    wgpu::ComputePipeline fPipeline;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnComputePipeline_DEFINED
