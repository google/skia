/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"

#include "include/core/SkStream.h"
#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/RenderPassDesc.h"
#include "src/gpu/graphite/TextureFormat.h"
#include "src/gpu/graphite/vk/VulkanQueueManager.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
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
    VULKAN_CALL_RESULT(context,
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
    // If requestedDescriptors is empty, that simply means we should create an empty placeholder
    // layout that doesn't actually contain any descriptors.
    skia_private::STArray<kDescriptorTypeCount, VkDescriptorSetLayoutBinding> bindingLayouts;
    for (size_t i = 0; i < requestedDescriptors.size(); i++) {
        if (requestedDescriptors[i].fCount != 0) {
            const DescriptorData& currDescriptor = requestedDescriptors[i];
            VkDescriptorSetLayoutBinding& layoutBinding = bindingLayouts.push_back();
            memset(&layoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
            layoutBinding.binding = currDescriptor.fBindingIndex;
            layoutBinding.descriptorType = DsTypeEnumToVkDs(currDescriptor.fType);
            layoutBinding.descriptorCount = currDescriptor.fCount;
            layoutBinding.stageFlags =
                    PipelineStageFlagsToVkShaderStageFlags(currDescriptor.fPipelineStageFlags);
            layoutBinding.pImmutableSamplers = currDescriptor.fImmutableSampler
                    ? (static_cast<const VulkanSampler*>(
                            currDescriptor.fImmutableSampler))->constVkSamplerPtr()
                    : nullptr;
        }
    }

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.bindingCount = bindingLayouts.size();
    layoutCreateInfo.pBindings = bindingLayouts.data();

    VkResult result;
    VULKAN_CALL_RESULT(
            ctxt,
            result,
            CreateDescriptorSetLayout(ctxt->device(), &layoutCreateInfo, nullptr, outLayout));
    if (result != VK_SUCCESS) {
        SkDebugf("Failed to create VkDescriptorSetLayout\n");
        outLayout = VK_NULL_HANDLE;
    }
}

VkDescriptorType DsTypeEnumToVkDs(DescriptorType type) {
    switch (type) {
        case DescriptorType::kUniformBuffer:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        case DescriptorType::kTextureSampler:
            return VK_DESCRIPTOR_TYPE_SAMPLER;
        case DescriptorType::kTexture:
            return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case DescriptorType::kCombinedTextureSampler:
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        case DescriptorType::kStorageBuffer:
            return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
        case DescriptorType::kInputAttachment:
            return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    }
    SkUNREACHABLE;
}

TextureFormat VkFormatToTextureFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8_UNORM:                  return TextureFormat::kR8;
        case VK_FORMAT_R16_UNORM:                 return TextureFormat::kR16;
        case VK_FORMAT_R16_SFLOAT:                return TextureFormat::kR16F;
        case VK_FORMAT_R32_SFLOAT:                return TextureFormat::kR32F;
        case VK_FORMAT_R8G8_UNORM:                return TextureFormat::kRG8;
        case VK_FORMAT_R16G16_UNORM:              return TextureFormat::kRG16;
        case VK_FORMAT_R16G16_SFLOAT:             return TextureFormat::kRG16F;
        case VK_FORMAT_R32G32_SFLOAT:             return TextureFormat::kRG32F;
        case VK_FORMAT_R8G8B8_UNORM:              return TextureFormat::kRGB8;
        case VK_FORMAT_B8G8R8_UNORM:              return TextureFormat::kBGR8;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:       return TextureFormat::kB5_G6_R5;
        case VK_FORMAT_B5G6R5_UNORM_PACK16:       return TextureFormat::kR5_G6_B5;
        case VK_FORMAT_R16G16B16_UNORM:           return TextureFormat::kRGB16;
        case VK_FORMAT_R16G16B16_SFLOAT:          return TextureFormat::kRGB16F;
        case VK_FORMAT_R32G32B32_SFLOAT:          return TextureFormat::kRGB32F;
        case VK_FORMAT_R8G8B8_SRGB:               return TextureFormat::kRGB8_sRGB;
        case VK_FORMAT_R8G8B8A8_UNORM:            return TextureFormat::kRGBA8;
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:     return TextureFormat::kRGBA8;
        case VK_FORMAT_R16G16B16A16_UNORM:        return TextureFormat::kRGBA16;
        case VK_FORMAT_R16G16B16A16_SFLOAT:       return TextureFormat::kRGBA16F;
        case VK_FORMAT_R32G32B32A32_SFLOAT:       return TextureFormat::kRGBA32F;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:  return TextureFormat::kRGB10_A2;
        case VK_FORMAT_R8G8B8A8_SRGB:             return TextureFormat::kRGBA8_sRGB;
        case VK_FORMAT_B8G8R8A8_UNORM:            return TextureFormat::kBGRA8;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:  return TextureFormat::kBGR10_A2;
        case VK_FORMAT_B8G8R8A8_SRGB:             return TextureFormat::kBGRA8_sRGB;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:     return TextureFormat::kABGR4;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:     return TextureFormat::kARGB4;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:   return TextureFormat::kRGB8_ETC2;
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:    return TextureFormat::kRGB8_ETC2_sRGB;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:       return TextureFormat::kRGB8_BC1;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:      return TextureFormat::kRGBA8_BC1;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:       return TextureFormat::kRGBA8_BC1_sRGB;
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:  return TextureFormat::kYUV8_P2_420;
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: return TextureFormat::kYUV8_P3_420;
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
            return TextureFormat::kYUV10x6_P2_420;
        case VK_FORMAT_S8_UINT:                   return TextureFormat::kS8;
        case VK_FORMAT_D16_UNORM:                 return TextureFormat::kD16;
        case VK_FORMAT_D32_SFLOAT:                return TextureFormat::kD32F;
        case VK_FORMAT_D24_UNORM_S8_UINT:         return TextureFormat::kD24_S8;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:        return TextureFormat::kD32F_S8;
        default:                                  return TextureFormat::kUnsupported;
    }
}

VkShaderStageFlags PipelineStageFlagsToVkShaderStageFlags(
        SkEnumBitMask<PipelineStageFlags> stageFlags) {
    VkShaderStageFlags vkStageFlags = 0;
    if (stageFlags & PipelineStageFlags::kVertexShader) {
        vkStageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (stageFlags & PipelineStageFlags::kFragmentShader) {
        vkStageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if (stageFlags & PipelineStageFlags::kCompute) {
        vkStageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }
    return vkStageFlags;
}

bool RenderPassDescWillLoadMSAAFromResolve(const RenderPassDesc& renderPassDesc) {
    return renderPassDesc.fColorResolveAttachment.fTextureInfo.isValid() &&
           renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
}

} // namespace skgpu::graphite
