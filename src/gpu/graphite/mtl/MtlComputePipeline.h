/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlComputePipeline_DEFINED
#define skgpu_graphite_MtlComputePipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/ComputePipeline.h"

#import <Metal/Metal.h>

namespace skgpu::graphite {

class ComputePipelineDesc;
class ResourceProvider;
class MtlSharedContext;

class MtlComputePipeline final : public ComputePipeline {
public:
    using MSLFunction = std::pair<id<MTLLibrary>, std::string>;

    static sk_sp<MtlComputePipeline> Make(const MtlSharedContext*,
                                          const std::string& label,
                                          MSLFunction computeMain);
    ~MtlComputePipeline() override = default;

    id<MTLComputePipelineState> mtlPipelineState() const { return fPipelineState.get(); }

private:
    MtlComputePipeline(const SharedContext* sharedContext, sk_cfp<id<MTLComputePipelineState>> pso)
            : ComputePipeline(sharedContext)
            , fPipelineState(std::move(pso)) {}

    void freeGpuData() override;

    sk_cfp<id<MTLComputePipelineState>> fPipelineState;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_MtlComputePipeline_DEFINED
