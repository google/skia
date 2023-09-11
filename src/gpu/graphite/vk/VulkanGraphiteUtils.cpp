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
        case DescriptorType::kInlineUniform:
            return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT;
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

bool vkFormatIsSupported(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

} // namespace skgpu::graphite
