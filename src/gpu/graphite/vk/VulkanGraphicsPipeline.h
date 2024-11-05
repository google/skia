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
#include "include/private/base/SkTArray.h"
#include "src/gpu/Blend.h"
#include "src/gpu/graphite/DrawTypes.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"

namespace SkSL {
    class Compiler;
}

namespace skgpu::graphite {

class Attribute;
class GraphicsPipelineDesc;
class RuntimeEffectDictionary;
class VulkanResourceProvider;
class VulkanSharedContext;
struct RenderPassDesc;
class TextureInfo;
class VulkanRenderPass;

class VulkanGraphicsPipeline final : public GraphicsPipeline {
public:
    inline static constexpr unsigned int kIntrinsicUniformBufferIndex = 0;
    inline static constexpr unsigned int kRenderStepUniformBufferIndex = 1;
    inline static constexpr unsigned int kPaintUniformBufferIndex = 2;
    inline static constexpr unsigned int kGradientBufferIndex = 3;
    inline static constexpr unsigned int kNumUniformBuffers = 4;

    // For now, rigidly assign all uniform buffer descriptors to be in set 0 and all
    // texture/samplers to be in set 1.
    // TODO(b/274762935): Make the bindings and descriptor set organization more flexible.
    inline static constexpr unsigned int kUniformBufferDescSetIndex = 0;
    inline static constexpr unsigned int kTextureBindDescSetIndex = 1;
    // Currently input attachments are only used for loading MSAA from resolve, so we can use the
    // descriptor set index normally assigned to uniform desc sets.
    inline static constexpr unsigned int kInputAttachmentDescSetIndex = kUniformBufferDescSetIndex;

    inline static constexpr unsigned int kVertexBufferIndex = 0;
    inline static constexpr unsigned int kInstanceBufferIndex = 1;
    inline static constexpr unsigned int kNumInputBuffers = 2;

    inline static const DescriptorData kIntrinsicUniformBufferDescriptor = {
            DescriptorType::kUniformBuffer, /*count=*/1,
            kIntrinsicUniformBufferIndex,
            PipelineStageFlags::kVertexShader | PipelineStageFlags::kFragmentShader};

    // Currently we only ever have one input attachment descriptor by itself within a set, so its
    // binding index will always be 0.
    inline static constexpr unsigned int kInputAttachmentBindingIndex = 0;
    inline static const DescriptorData kInputAttachmentDescriptor = {
            DescriptorType::kInputAttachment, /*count=*/1,
            kInputAttachmentBindingIndex,
            PipelineStageFlags::kFragmentShader};

    static sk_sp<VulkanGraphicsPipeline> Make(VulkanResourceProvider*,
                                              const RuntimeEffectDictionary*,
                                              const GraphicsPipelineDesc&,
                                              const RenderPassDesc&,
                                              SkEnumBitMask<PipelineCreationFlags>);

    static sk_sp<VulkanGraphicsPipeline> MakeLoadMSAAPipeline(
            const VulkanSharedContext*,
            VkShaderModule vsModule,
            VkShaderModule fsModule,
            VkPipelineShaderStageCreateInfo* pipelineShaderStages,
            VkPipelineLayout,
            sk_sp<VulkanRenderPass> compatibleRenderPass,
            VkPipelineCache,
            const TextureInfo& dstColorAttachmentTexInfo);

    static bool InitializeMSAALoadPipelineStructs(
            const VulkanSharedContext*,
            VkShaderModule* outVertexShaderModule,
            VkShaderModule* outFragShaderModule,
            VkPipelineShaderStageCreateInfo* outShaderStageInfo,
            VkPipelineLayout* outPipelineLayout);

    ~VulkanGraphicsPipeline() override {}

    VkPipelineLayout layout() const {
        SkASSERT(fPipelineLayout != VK_NULL_HANDLE);
        return fPipelineLayout;
    }

    VkPipeline pipeline() const {
        SkASSERT(fPipeline != VK_NULL_HANDLE);
        return fPipeline;
    }

private:
    VulkanGraphicsPipeline(const VulkanSharedContext* sharedContext,
                           const PipelineInfo& pipelineInfo,
                           VkPipelineLayout,
                           VkPipeline,
                           bool ownsPipelineLayout,
                           skia_private::TArray<sk_sp<VulkanSampler>> immutableSamplers);

    void freeGpuData() override;

    VkPipelineLayout fPipelineLayout = VK_NULL_HANDLE;
    VkPipeline fPipeline = VK_NULL_HANDLE;
    bool fOwnsPipelineLayout = true;

    // Hold a ref to immutable samplers used such that their lifetime is properly managed.
    const skia_private::TArray<sk_sp<VulkanSampler>> fImmutableSamplers;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlGraphicsPipeline_DEFINED
