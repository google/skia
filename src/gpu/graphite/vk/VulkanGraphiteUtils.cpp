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

VkShaderModule CreateVulkanShaderModule(const VulkanSharedContext* context,
                                        const std::string& spirv,
                                        VkShaderStageFlagBits stage) {
    TRACE_EVENT0("skia.shaders", "InstallVkShaderModule");
    VkShaderModuleCreateInfo moduleCreateInfo = {};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
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
            layoutBinding = {};
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

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
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

#define VK_FORMAT_MAPPING(M) \
    M(TextureFormat::kR8,             VK_FORMAT_R8_UNORM)                                  \
    M(TextureFormat::kR16,            VK_FORMAT_R16_UNORM)                                 \
    M(TextureFormat::kR16F,           VK_FORMAT_R16_SFLOAT)                                \
    M(TextureFormat::kR32F,           VK_FORMAT_R32_SFLOAT)                                \
    /*TextureFormat::kA8              unsupported */                                       \
    M(TextureFormat::kRG8,            VK_FORMAT_R8G8_UNORM)                                \
    M(TextureFormat::kRG16,           VK_FORMAT_R16G16_UNORM)                              \
    M(TextureFormat::kRG16F,          VK_FORMAT_R16G16_SFLOAT)                             \
    M(TextureFormat::kRG32F,          VK_FORMAT_R32G32_SFLOAT)                             \
    M(TextureFormat::kRGB8,           VK_FORMAT_R8G8B8_UNORM)                              \
    M(TextureFormat::kBGR8,           VK_FORMAT_B8G8R8_UNORM)                              \
    M(TextureFormat::kB5_G6_R5,       VK_FORMAT_R5G6B5_UNORM_PACK16)                       \
    M(TextureFormat::kR5_G6_B5,       VK_FORMAT_B5G6R5_UNORM_PACK16)                       \
    M(TextureFormat::kRGB16,          VK_FORMAT_R16G16B16_UNORM)                           \
    M(TextureFormat::kRGB16F,         VK_FORMAT_R16G16B16_SFLOAT)                          \
    M(TextureFormat::kRGB32F,         VK_FORMAT_R32G32B32_SFLOAT)                          \
    M(TextureFormat::kRGB8_sRGB,      VK_FORMAT_R8G8B8_SRGB)                               \
    /*TextureFormat::kBGR10_XR        unsupported */                                       \
    M(TextureFormat::kRGBA8,          VK_FORMAT_R8G8B8A8_UNORM)                            \
    M(TextureFormat::kRGBA16,         VK_FORMAT_R16G16B16A16_UNORM)                        \
    M(TextureFormat::kRGBA16F,        VK_FORMAT_R16G16B16A16_SFLOAT)                       \
    M(TextureFormat::kRGBA32F,        VK_FORMAT_R32G32B32A32_SFLOAT)                       \
    M(TextureFormat::kRGB10_A2,       VK_FORMAT_A2B10G10R10_UNORM_PACK32)                  \
    M(TextureFormat::kRGBA8_sRGB,     VK_FORMAT_R8G8B8A8_SRGB)                             \
    M(TextureFormat::kBGRA8,          VK_FORMAT_B8G8R8A8_UNORM)                            \
    M(TextureFormat::kBGR10_A2,       VK_FORMAT_A2R10G10B10_UNORM_PACK32)                  \
    M(TextureFormat::kBGRA8_sRGB,     VK_FORMAT_B8G8R8A8_SRGB)                             \
    M(TextureFormat::kABGR4,          VK_FORMAT_R4G4B4A4_UNORM_PACK16)                     \
    M(TextureFormat::kARGB4,          VK_FORMAT_B4G4R4A4_UNORM_PACK16)                     \
    /*TextureFormat::kBGRA10x6_XR     unsupported */                                       \
    M(TextureFormat::kRGB8_ETC2,      VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK)                   \
    M(TextureFormat::kRGB8_ETC2_sRGB, VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK)                    \
    M(TextureFormat::kRGB8_BC1,       VK_FORMAT_BC1_RGB_UNORM_BLOCK)                       \
    M(TextureFormat::kRGBA8_BC1,      VK_FORMAT_BC1_RGBA_UNORM_BLOCK)                      \
    M(TextureFormat::kRGBA8_BC1_sRGB, VK_FORMAT_BC1_RGBA_SRGB_BLOCK)                       \
    M(TextureFormat::kYUV8_P2_420,    VK_FORMAT_G8_B8R8_2PLANE_420_UNORM)                  \
    M(TextureFormat::kYUV8_P3_420,    VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM)                 \
    M(TextureFormat::kYUV10x6_P2_420, VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16) \
    /*TextureFormat::kExternal        VK_FORMAT_UNDEFINED w/ uint64_t sidecar */           \
    M(TextureFormat::kS8,             VK_FORMAT_S8_UINT)                                   \
    M(TextureFormat::kD16,            VK_FORMAT_D16_UNORM)                                 \
    M(TextureFormat::kD32F,           VK_FORMAT_D32_SFLOAT)                                \
    M(TextureFormat::kD24_S8,         VK_FORMAT_D24_UNORM_S8_UINT)                         \
    M(TextureFormat::kD32F_S8,        VK_FORMAT_D32_SFLOAT_S8_UINT)

// NOTE: For external formats, Vulkan stores the value outside of VkFormat and expects the VkFormat
// to be VK_FORMAT_UNDEFINED. The mapping functions below are unaware of this side car, so it
// maps VK_FORMAT_UNDEFINED to TextureFormat::kUnsupported (this is disambiguated explicitly by
// VulkanTextureInfo which has the full context). The reverse maps TextureFormat::kExternal to
// VK_FORMAT_UNDEFINED, as expected, although callers are responsible for passing along the actual
// external format separately.

TextureFormat VkFormatToTextureFormat(VkFormat format) {
#define M(TF, VK) case VK: return TF;
    switch(format) {
        VK_FORMAT_MAPPING(M)
        default: return TextureFormat::kUnsupported;
    }
#undef M
}
VkFormat TextureFormatToVkFormat(TextureFormat format) {
#define M(TF, VK) case TF: return VK;
    switch(format) {
        VK_FORMAT_MAPPING(M)
        default: return VK_FORMAT_UNDEFINED;
    }
#undef M
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
    return renderPassDesc.fColorResolveAttachment.fFormat != TextureFormat::kUnsupported &&
           renderPassDesc.fColorResolveAttachment.fLoadOp == LoadOp::kLoad;
}

} // namespace skgpu::graphite
