/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlRenderPipeline_DEFINED
#define skgpu_MtlRenderPipeline_DEFINED

#include "experimental/graphite/src/RenderPipeline.h"

#include "include/ports/SkCFObject.h"
#include <memory>

#import <Metal/Metal.h>

namespace skgpu {
class RenderPipeline;
class RenderPipelineDesc;
} // namespace skgpu

namespace skgpu::mtl {
class Gpu;

class RenderPipeline final : public skgpu::RenderPipeline {
public:
    static sk_sp<RenderPipeline> Make(const Gpu*, const skgpu::RenderPipelineDesc&);
    ~RenderPipeline() override {}

    id<MTLRenderPipelineState> mtlPipelineState() { return fPipelineState.get(); }

private:
    RenderPipeline(sk_cfp<id<MTLRenderPipelineState>> pso)
        : fPipelineState(std::move(pso)) {}

    sk_cfp<id<MTLRenderPipelineState>> fPipelineState;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlRenderPipeline_DEFINED
