/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanGraphicsPipeline_DEFINED
#define skgpu_graphite_VulkanGraphicsPipeline_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkSpan.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

namespace SkSL {
    class Compiler;
}

namespace skgpu::graphite {

class Attribute;
class GraphicsPipelineDesc;
class RuntimeEffectDictionary;
class VulkanSharedContext;
struct RenderPassDesc;
class VulkanRenderPass;

class VulkanGraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 2;
    inline static constexpr unsigned int kNumUniformBuffers = 3;

    inline static const DescriptorData kIntrinsicUniformDescriptor  =
            {DescriptorType::kInlineUniform,
             // For inline uniform descriptors, the descriptor count field is actually the number of
             // bytes to allocate for descriptors given this type.
             /*count=*/sizeof(float) * 4,
             VulkanGraphicsPipeline::kIntrinsicUniformBufferIndex};
    inline static const DescriptorData kRenderStepUniformDescriptor =
            {DescriptorType::kUniformBuffer,
             /*count=*/1,
             VulkanGraphicsPipeline::kRenderStepUniformBufferIndex};
    inline static const DescriptorData kPaintUniformDescriptor      =
            {DescriptorType::kUniformBuffer,
             /*count=*/1,
             VulkanGraphicsPipeline::kPaintUniformBufferIndex};

    // For now, rigidly assign all uniform buffer descriptors to be in one descriptor set in binding
    // 0 and all texture/samplers to be in binding 1.
    // TODO(b/274762935): Make the bindings and descriptor set organization more flexible.
    inline static constexpr unsigned int kUniformBufferDescSetIndex = 0;
    inline static constexpr unsigned int kTextureBindDescSetIndex = 1;

    inline static constexpr unsigned int kVertexBufferIndex = 0;
    inline static constexpr unsigned int kInstanceBufferIndex = 1;
    inline static constexpr unsigned int kNumInputBuffers = 2;

    static sk_sp<VulkanGraphicsPipeline> Make(const VulkanSharedContext*,
                                              SkSL::Compiler* compiler,
                                              const RuntimeEffectDictionary*,
                                              const GraphicsPipelineDesc&,
                                              const RenderPassDesc&,
                                              sk_sp<VulkanRenderPass> compatibleRenderPass,
                                              VkPipelineCache);

    ~VulkanGraphicsPipeline() override {}

    VkPipelineLayout layout() const {
        SkASSERT(fPipelineLayout != VK_NULL_HANDLE);
        return fPipelineLayout;
    }

    VkPipeline pipeline() const {
        SkASSERT(fPipeline != VK_NULL_HANDLE);
        return fPipeline;
    }

    bool hasFragmentUniforms() const { return fHasFragmentUniforms; }
    bool hasStepUniforms() const { return fHasStepUniforms; }
    int numTextureSamplers() const { return fNumTextureSamplers; }

private:
    VulkanGraphicsPipeline(const skgpu::graphite::SharedContext* sharedContext,
                           PipelineInfo* pipelineInfo,
                           VkPipelineLayout,
                           VkPipeline,
                           bool hasFragmentUniforms,
                           bool hasStepUniforms,
                           int numTextureSamplers);

    void freeGpuData() override;

    VkPipelineLayout fPipelineLayout = VK_NULL_HANDLE;
    VkPipeline fPipeline = VK_NULL_HANDLE;
    bool fHasFragmentUniforms = false;
    bool fHasStepUniforms = false;
    int fNumTextureSamplers = 0;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED
