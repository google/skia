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
class MtlResourceProvider;
class MtlSharedContext;

class MtlComputePipeline final : public ComputePipeline {
public:
    static sk_sp<MtlComputePipeline> Make(MtlResourceProvider*,
                                          const MtlSharedContext*,
                                          const ComputePipelineDesc&);
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
