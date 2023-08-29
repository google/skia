/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlGraphicsPipeline_DEFINED
#define skgpu_graphite_MtlGraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/ports/SkCFObject.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include <memory>

#import <Metal/Metal.h>

namespace skgpu {
struct BlendInfo;
}

namespace skgpu::graphite {

class Attribute;
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

    using MSLFunction = std::pair<id<MTLLibrary>, std::string>;
    static sk_sp<MtlGraphicsPipeline> Make(const MtlSharedContext*,
                                           std::string label,
                                           MSLFunction vertexMain,
                                           SkSpan<const Attribute> vertexAttrs,
                                           SkSpan<const Attribute> instanceAttrs,
                                           MSLFunction fragmentMain,
                                           sk_cfp<id<MTLDepthStencilState>>,
                                           uint32_t stencilRefValue,
                                           const BlendInfo& blendInfo,
                                           const RenderPassDesc&,
                                           PipelineInfo* pipelineInfo);

    ~MtlGraphicsPipeline() override {}

    id<MTLRenderPipelineState> mtlPipelineState() const { return fPipelineState.get(); }
    id<MTLDepthStencilState> mtlDepthStencilState() const { return fDepthStencilState.get(); }
    uint32_t stencilReferenceValue() const { return fStencilReferenceValue; }

private:
    MtlGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                        PipelineInfo* pipelineInfo,
                        sk_cfp<id<MTLRenderPipelineState>> pso,
                        sk_cfp<id<MTLDepthStencilState>> dss,
                        uint32_t refValue);

    void freeGpuData() override;

    sk_cfp<id<MTLRenderPipelineState>> fPipelineState;
    sk_cfp<id<MTLDepthStencilState>> fDepthStencilState;
    uint32_t fStencilReferenceValue;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED
