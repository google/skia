/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlGraphicsPipeline_DEFINED
#define skgpu_graphite_MtlGraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include <memory>

#import <Metal/Metal.h>

class SkShaderCodeDictionary;

namespace skgpu::graphite {
class Context;
class GraphicsPipelineDesc;
class MtlResourceProvider;
class MtlSharedContext;
struct RenderPassDesc;

class MtlGraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 2;
    inline static constexpr unsigned int kVertexBufferIndex = 3;
    inline static constexpr unsigned int kInstanceBufferIndex = 4;

    static sk_sp<MtlGraphicsPipeline> Make(MtlResourceProvider*,
                                           const MtlSharedContext*,
                                           const GraphicsPipelineDesc&,
                                           const RenderPassDesc&);
    ~MtlGraphicsPipeline() override {}

    id<MTLRenderPipelineState> mtlPipelineState() const { return fPipelineState.get(); }
    id<MTLDepthStencilState> mtlDepthStencilState() const { return fDepthStencilState.get(); }
    uint32_t stencilReferenceValue() const { return fStencilReferenceValue; }
    size_t vertexStride() const { return fVertexStride; }
    size_t instanceStride() const { return fInstanceStride; }

private:
    MtlGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                        sk_cfp<id<MTLRenderPipelineState>> pso,
                        sk_cfp<id<MTLDepthStencilState>> dss,
                        uint32_t refValue,
                        size_t vertexStride,
                        size_t instanceStride)
        : GraphicsPipeline(sharedContext)
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
