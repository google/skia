/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlGraphicsPipeline_DEFINED
#define skgpu_MtlGraphicsPipeline_DEFINED

#include "experimental/graphite/src/GraphicsPipeline.h"

#include "include/ports/SkCFObject.h"
#include <memory>

#import <Metal/Metal.h>

namespace skgpu {
class GraphicsPipelineDesc;
} // namespace skgpu

namespace skgpu::mtl {
class Gpu;

class GraphicsPipeline final : public skgpu::GraphicsPipeline {
public:
    inline static constexpr unsigned int kUniformBufferIndex = 0;
    inline static constexpr unsigned int kVertexBufferIndex = 1;
    inline static constexpr unsigned int kInstanceBufferIndex = 2;

    static sk_sp<GraphicsPipeline> Make(const Gpu*, const skgpu::GraphicsPipelineDesc&);
    ~GraphicsPipeline() override {}

    id<MTLRenderPipelineState> mtlPipelineState() const { return fPipelineState.get(); }
    size_t vertexStride() const { return fVertexStride; }
    size_t instanceStride() const { return fInstanceStride; }

private:
    GraphicsPipeline(sk_cfp<id<MTLRenderPipelineState>> pso,
                     size_t vertexStride,
                     size_t instanceStride)
        : fPipelineState(std::move(pso))
        , fVertexStride(vertexStride)
        , fInstanceStride(instanceStride) {}

    sk_cfp<id<MTLRenderPipelineState>> fPipelineState;
    size_t fVertexStride = 0;
    size_t fInstanceStride = 0;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlGraphicsPipeline_DEFINED
