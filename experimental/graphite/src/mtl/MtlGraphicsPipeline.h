/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlGraphicsPipeline_DEFINED
#define skgpu_graphite_MtlGraphicsPipeline_DEFINED

#include "experimental/graphite/src/GraphicsPipeline.h"
#include "include/core/SkRefCnt.h"
#include "include/ports/SkCFObject.h"
#include <memory>

#import <Metal/Metal.h>

class SkShaderCodeDictionary;

namespace skgpu {
class Context;
class GraphicsPipelineDesc;
struct RenderPassDesc;
} // namespace skgpu

namespace skgpu::graphite {
class MtlGpu;
class MtlResourceProvider;

class MtlGraphicsPipeline final : public skgpu::GraphicsPipeline {
public:
    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 2;
    inline static constexpr unsigned int kVertexBufferIndex = 3;
    inline static constexpr unsigned int kInstanceBufferIndex = 4;

    static sk_sp<MtlGraphicsPipeline> Make(MtlResourceProvider*,
                                           const MtlGpu*,
                                           const skgpu::GraphicsPipelineDesc&,
                                           const skgpu::RenderPassDesc&);
    ~MtlGraphicsPipeline() override {}

    id<MTLRenderPipelineState> mtlPipelineState() const { return fPipelineState.get(); }
    id<MTLDepthStencilState> mtlDepthStencilState() const { return fDepthStencilState.get(); }
    uint32_t stencilReferenceValue() const { return fStencilReferenceValue; }
    size_t vertexStride() const { return fVertexStride; }
    size_t instanceStride() const { return fInstanceStride; }

private:
    MtlGraphicsPipeline(const skgpu::Gpu* gpu,
                        sk_cfp<id<MTLRenderPipelineState>> pso,
                        sk_cfp<id<MTLDepthStencilState>> dss,
                        uint32_t refValue,
                        size_t vertexStride,
                        size_t instanceStride)
        : skgpu::GraphicsPipeline(gpu)
        , fPipelineState(std::move(pso))
        , fDepthStencilState(dss)
        , fStencilReferenceValue(refValue)
        , fVertexStride(vertexStride)
        , fInstanceStride(instanceStride) {}

    void freeGpuData() override;

    sk_cfp<id<MTLRenderPipelineState>> fPipelineState;
    sk_cfp<id<MTLDepthStencilState>> fDepthStencilState;
    uint32_t fStencilReferenceValue;
    size_t fVertexStride = 0;
    size_t fInstanceStride = 0;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED
