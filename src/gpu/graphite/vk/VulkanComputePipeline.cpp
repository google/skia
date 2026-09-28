/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanComputePipeline.h"

#include "include/private/SkAssert.h"
#include "src/gpu/graphite/ComputePipelineDesc.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/compute/ComputeStep.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/vk/VulkanUtilsPriv.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"

namespace skgpu::graphite {

sk_sp<VulkanComputePipeline> VulkanComputePipeline::Make(VulkanSharedContext* sharedContext,
                                                         const ComputePipelineDesc& desc) {
    const ComputeStep* step = desc.computeStep();
    // Should never have a compute step without resources.
    SkASSERT(!step->resources().empty());

    std::string sksl = BuildComputeSkSL(sharedContext->caps(), step, BackendApi::kVulkan);
    SkSL::ProgramSettings settings;
    SkSL::NativeShader spirv;
    SkSL::ProgramInterface outInterface;
    if (!skgpu::SkSLToSPIRV(sharedContext->caps()->shaderCaps(),
                            sksl,
                            SkSL::ProgramKind::kCompute,
                            settings,
                            &spirv,
                            &outInterface,
                            sharedContext->caps()->shaderErrorHandler())) {
        return nullptr;
    }

    VkShaderModule shaderModule =
            CreateVulkanShaderModule(sharedContext, spirv, VK_SHADER_STAGE_COMPUTE_BIT);
    if (shaderModule == VK_NULL_HANDLE) {
        return nullptr;
    }

    skia_private::TArray<DescriptorData> descriptorData;
    descriptorData.reserve_exact(step->resources().size());

    int bindingIndex = 0;
    for (const ComputeStep::ResourceDesc& r : step->resources()) {
        DescriptorType descType;
        switch (r.fType) {
            case ComputeStep::ResourceType::kUniformBuffer:
                descType = DescriptorType::kUniformBuffer;
                break;
            case ComputeStep::ResourceType::kStorageBuffer:
            case ComputeStep::ResourceType::kReadOnlyStorageBuffer:
            case ComputeStep::ResourceType::kIndirectBuffer:
                descType = DescriptorType::kStorageBuffer;
                break;
            case ComputeStep::ResourceType::kWriteOnlyStorageTexture:
                descType = DescriptorType::kStorageTexture;
                break;
            case ComputeStep::ResourceType::kReadOnlyTexture:
                descType = DescriptorType::kTexture;
                break;
            case ComputeStep::ResourceType::kSampledTexture:
                descType = DescriptorType::kCombinedTextureSampler;
                break;
            default:
                SK_ABORT("Unsupported compute resource type");
        }
        // TODO (thomsmit): If ComputeStep::ResourceDesc ever expands to support arrayed resources,
        // this count will need to dynamically map to the resource's requested size.
        descriptorData.push_back({descType,
                                  /*count=*/1,
                                  bindingIndex++,
                                  PipelineStageFlags::kCompute});
    }

    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    DescriptorDataToVkDescSetLayout(sharedContext, descriptorData, &descriptorSetLayout);
    if (descriptorSetLayout == VK_NULL_HANDLE) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyShaderModule(sharedContext->device(), shaderModule, nullptr));
        return nullptr;
    }

    // TODO (thomsmit): If compute SkSL generation ever relies on push constants for intrinsics or
    // uniforms (such as when VulkanCaps::fUsePushConstantsForIntrinsicConstants is true),
    // pipelineLayoutInfo will need to declare the appropriate VkPushConstantRange. Currently,
    // compute pipelines only rely on UBOs and SSBOs for uniforms and data.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkResult result;
    VULKAN_CALL_RESULT(
            sharedContext,
            result,
            CreatePipelineLayout(
                    sharedContext->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout));

    // DescriptorSetLayouts can be deleted after the pipeline layout is created.
    VULKAN_CALL(
            sharedContext->interface(),
            DestroyDescriptorSetLayout(sharedContext->device(), descriptorSetLayout, nullptr));

    if (result != VK_SUCCESS) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyShaderModule(sharedContext->device(), shaderModule, nullptr));
        return nullptr;
    }

    VkComputePipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    pipelineInfo.stage.module = shaderModule;
    pipelineInfo.stage.pName = "main";
    pipelineInfo.layout = pipelineLayout;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VULKAN_CALL_RESULT(sharedContext,
                       result,
                       CreateComputePipelines(sharedContext->device(),
                                              sharedContext->getPipelineCache(),
                                              1,
                                              &pipelineInfo,
                                              nullptr,
                                              &pipeline));

    VULKAN_CALL(sharedContext->interface(),
                DestroyShaderModule(sharedContext->device(), shaderModule, nullptr));

    if (result != VK_SUCCESS) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyPipelineLayout(sharedContext->device(), pipelineLayout, nullptr));
        return nullptr;
    }

    sharedContext->pipelineCompileWasRequired();

    // Both pipeline and pipelineLayout are verified valid handles; ownership is transferred
    // to VulkanComputePipeline, which cleans them up in freeGpuData().
    return sk_sp<VulkanComputePipeline>(new VulkanComputePipeline(sharedContext,
                                                                  pipeline,
                                                                  pipelineLayout,
                                                                  std::move(descriptorData)));
}

VulkanComputePipeline::VulkanComputePipeline(const SharedContext* sharedContext,
                                             VkPipeline pipeline,
                                             VkPipelineLayout pipelineLayout,
                                             skia_private::TArray<DescriptorData> descriptorData)
        : ComputePipeline(sharedContext)
        , fPipeline(pipeline)
        , fPipelineLayout(pipelineLayout)
        , fDescriptorData(std::move(descriptorData)) {}

void VulkanComputePipeline::freeGpuData() {
    auto sharedCtxt = static_cast<const VulkanSharedContext*>(this->sharedContext());
    if (fPipeline != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedCtxt->interface(),
                    DestroyPipeline(sharedCtxt->device(), fPipeline, nullptr));
        fPipeline = VK_NULL_HANDLE;
    }
    if (fPipelineLayout != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedCtxt->interface(),
                    DestroyPipelineLayout(sharedCtxt->device(), fPipelineLayout, nullptr));
        fPipelineLayout = VK_NULL_HANDLE;
    }
}

}  // namespace skgpu::graphite
