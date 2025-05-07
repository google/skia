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
class RuntimeEffectDictionary;

class MtlGraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 2;
    inline static constexpr unsigned int kStaticDataBufferIndex = 3;
    inline static constexpr unsigned int kAppendDataBufferIndex = 4;
    inline static constexpr unsigned int kGradientBufferIndex = 5;

    static sk_sp<MtlGraphicsPipeline> Make(const MtlSharedContext*,
                                           MtlResourceProvider*,
                                           const RuntimeEffectDictionary*,
                                           const UniqueKey&,
                                           const GraphicsPipelineDesc&,
                                           const RenderPassDesc&,
                                           SkEnumBitMask<PipelineCreationFlags>,
                                           uint32_t compilationID);

    static sk_sp<MtlGraphicsPipeline> MakeLoadMSAAPipeline(const MtlSharedContext*,
                                                           MtlResourceProvider*,
                                                           const RenderPassDesc&);

    ~MtlGraphicsPipeline() override {}

    id<MTLRenderPipelineState> mtlPipelineState() const { return fPipelineState.get(); }
    id<MTLDepthStencilState> mtlDepthStencilState() const { return fDepthStencilState.get(); }
    uint32_t stencilReferenceValue() const { return fStencilReferenceValue; }

private:
    MtlGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                        const PipelineInfo& pipelineInfo,
                        sk_cfp<id<MTLRenderPipelineState>> pso,
                        sk_cfp<id<MTLDepthStencilState>> dss,
                        uint32_t refValue);

    using MSLFunction = std::pair<id<MTLLibrary>, std::string>;
    static sk_sp<MtlGraphicsPipeline> Make(const MtlSharedContext*,
                                           const std::string& label,
                                           const PipelineInfo&,
                                           MSLFunction vertexMain,
                                           MTLVertexStepFunction appendStepFunc,
                                           SkSpan<const Attribute> staticAttrs,
                                           SkSpan<const Attribute> appendAttrs,
                                           MSLFunction fragmentMain,
                                           sk_cfp<id<MTLDepthStencilState>>,
                                           uint32_t stencilRefValue,
                                           const BlendInfo& blendInfo,
                                           const RenderPassDesc&);

    void freeGpuData() override;

    sk_cfp<id<MTLRenderPipelineState>> fPipelineState;
    sk_cfp<id<MTLDepthStencilState>> fDepthStencilState;
    uint32_t fStencilReferenceValue;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED
