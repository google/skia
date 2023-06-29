/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/graphite/vk/VulkanGraphiteUtils.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/vk/VulkanQueueManager.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/sksl/SkSLProgramSettings.h"
namespace skgpu::graphite::ContextFactory {

std::unique_ptr<Context> MakeVulkan(const VulkanBackendContext& backendContext,
                                    const ContextOptions& options) {
    sk_sp<SharedContext> sharedContext = VulkanSharedContext::Make(backendContext, options);
    if (!sharedContext) {
        return nullptr;
    }

    std::unique_ptr<QueueManager> queueManager(new VulkanQueueManager(backendContext.fQueue,
                                                                      sharedContext.get()));
    if (!queueManager) {
        return nullptr;
    }

    return ContextCtorAccessor::MakeContext(std::move(sharedContext),
                                            std::move(queueManager),
                                            options);
}

} // namespace skgpu::graphite::ContextFactory

namespace skgpu::graphite {

VkShaderModule createVulkanShaderModule(const VulkanSharedContext* context,
                                        const std::string& spirv,
                                        VkShaderStageFlagBits stage) {
    TRACE_EVENT0("skia.shaders", "InstallVkShaderModule");
    VkShaderModuleCreateInfo moduleCreateInfo;
    memset(&moduleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = spirv.size();
    moduleCreateInfo.pCode = (const uint32_t*)spirv.c_str();

    VkShaderModule shaderModule;
    VkResult result;
    VULKAN_CALL_RESULT(context->interface(),
                       result,
                       CreateShaderModule(context->device(),
                                          &moduleCreateInfo,
                                          /*const VkAllocationCallbacks*=*/nullptr,
                                          &shaderModule));
    if (result != VK_SUCCESS) {
        SKGPU_LOG_E("Failed to create VkShaderModule");
        return VK_NULL_HANDLE;
    }
    return shaderModule;
}

void DescriptorDataToVkDescSetLayout(const VulkanSharedContext* ctxt,
                                     const SkSpan<DescriptorData>& requestedDescriptors,
                                     VkDescriptorSetLayout* outLayout) {
    skia_private::STArray<kDescriptorTypeCount, VkDescriptorSetLayoutBinding> bindingLayouts;
    for (size_t i = 0; i < requestedDescriptors.size(); i++) {
        if (requestedDescriptors[i].count != 0) {
            VkDescriptorSetLayoutBinding& layoutBinding = bindingLayouts.push_back();
            memset(&layoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
            layoutBinding.binding = requestedDescriptors[i].bindingIndex;
            layoutBinding.descriptorType = DsTypeEnumToVkDs(requestedDescriptors[i].type);
            layoutBinding.descriptorCount = requestedDescriptors[i].count;
            // TODO: Obtain layout binding stage flags from visibility (vertex or shader)
            layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            // TODO: Optionally set immutableSamplers here.
            layoutBinding.pImmutableSamplers = nullptr;
        }
    }

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.bindingCount = bindingLayouts.size();
    layoutCreateInfo.pBindings = &bindingLayouts.front();

    VkResult result;
    VULKAN_CALL_RESULT(ctxt->interface(),
                       result,
                       CreateDescriptorSetLayout(ctxt->device(),
                                                 &layoutCreateInfo,
                                                 nullptr,
                                                 outLayout));
    if (result != VK_SUCCESS) {
        SkDebugf("Failed to create VkDescriptorSetLayout\n");
        outLayout = VK_NULL_HANDLE;
    }
}

VkDescriptorType DsTypeEnumToVkDs(DescriptorType type) {
    switch (type) {
        case DescriptorType::kUniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case DescriptorType::kTextureSampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::kTexture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::kCombinedTextureSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::kStorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        case DescriptorType::kInputAttachment:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    }
    SkUNREACHABLE;
}

} // namespace skgpu::graphite
