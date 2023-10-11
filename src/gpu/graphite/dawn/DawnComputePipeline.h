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

#include "webgpu/webgpu_cpp.h"  // NO_G3_REWRITE

namespace skgpu::graphite {

class ComputePipelineDesc;
class DawnComputePipelineDesc;
class DawnSharedContext;

class DawnComputePipeline final : public ComputePipeline {
public:
    static sk_sp<DawnComputePipeline> Make(const DawnSharedContext*, const ComputePipelineDesc&);
    ~DawnComputePipeline() override = default;

    const wgpu::ComputePipeline& dawnComputePipeline() const { return fPipeline; }
    const wgpu::BindGroupLayout& dawnGroupLayout() const { return fGroupLayout; }

private:
    DawnComputePipeline(const SharedContext*, wgpu::ComputePipeline, wgpu::BindGroupLayout);

    void freeGpuData() override;

    wgpu::ComputePipeline fPipeline;
    wgpu::BindGroupLayout fGroupLayout;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_DawnComputePipeline_DEFINED
