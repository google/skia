/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanGraphiteUtilsPriv_DEFINED
#define skgpu_graphite_VulkanGraphiteUtilsPriv_DEFINED

#include "include/core/SkSpan.h"
#include "include/gpu/graphite/vk/VulkanGraphiteTypes.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/DescriptorData.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/sksl/codegen/SkSLNativeShader.h"

#include <string>

class SkStream;
class SkWStream;

// Uncomment to log all Vulkan commands from Graphite
#define VULKAN_LOG(X) // SkDebugf("vk%s\n", #X);

// Helper macros to call functions on the VulkanInterface without checking for errors. Note: This
// cannot require a VulkanSharedContext because driver calls are needed before the shared context
// has been initialized.
#define VULKAN_CALL_NO_LOG(IFACE, X) (IFACE)->fFunctions.f##X
#define VULKAN_CALL(IFACE, X) VULKAN_LOG(X) VULKAN_CALL_NO_LOG(IFACE, X)

// Must be called before checkVkResult, since this does not log if the VulkanSharedContext is
// already considering the device to be lost.
#define VULKAN_LOG_IF_NOT_SUCCESS(SHARED_CONTEXT, RESULT, X, ...)                      \
    do {                                                                               \
        if (RESULT != VK_SUCCESS && !(SHARED_CONTEXT)->isDeviceLost()) {               \
            SkDebugf("Failed vulkan call. Error: %d, " X "\n", RESULT, ##__VA_ARGS__); \
        }                                                                              \
    } while (false)

#define VULKAN_CALL_RESULT(SHARED_CONTEXT, RESULT, X)                     \
    do {                                                                  \
        VULKAN_LOG(X)                                                     \
        (RESULT) = VULKAN_CALL_NO_LOG((SHARED_CONTEXT)->interface(), X);  \
        SkASSERT(VK_SUCCESS == RESULT || VK_ERROR_DEVICE_LOST == RESULT); \
        VULKAN_LOG_IF_NOT_SUCCESS(SHARED_CONTEXT, RESULT, #X);            \
        (SHARED_CONTEXT)->checkVkResult(RESULT);                          \
    } while (false)

// same as VULKAN_CALL but checks for success
#define VULKAN_CALL_ERRCHECK(SHARED_CONTEXT, X) \
    VkResult SK_MACRO_APPEND_LINE(ret);         \
    VULKAN_CALL_RESULT(SHARED_CONTEXT, SK_MACRO_APPEND_LINE(ret), X)

#define VULKAN_CALL_RESULT_NOCHECK(IFACE, RESULT, X)        \
    do {                                                    \
        VULKAN_LOG(X)                                       \
        (RESULT) = VULKAN_CALL_NO_LOG(IFACE, X);            \
    } while (false)
namespace skgpu::graphite {

class VulkanSharedContext;
struct RenderPassDesc;
enum class TextureFormat : uint8_t;

VkShaderModule CreateVulkanShaderModule(const VulkanSharedContext*,
                                        const SkSL::NativeShader& spirv,
                                        VkShaderStageFlagBits);

VkDescriptorType DsTypeEnumToVkDs(DescriptorType);
void DescriptorDataToVkDescSetLayout(const VulkanSharedContext*,
                                     const SkSpan<DescriptorData>&,
                                     VkDescriptorSetLayout*);

TextureFormat VkFormatToTextureFormat(VkFormat);
VkFormat TextureFormatToVkFormat(TextureFormat);

VkImageAspectFlags GetVkImageAspectFlags(TextureFormat);

VkShaderStageFlags PipelineStageFlagsToVkShaderStageFlags(SkEnumBitMask<PipelineStageFlags>);

// When multisampling is used, Graphite never retains the multisampled data at the end of the render
// pass. It is always resolved to the single sampled color attachment. If the next multisampled
// render pass requires the single sampled data to be loaded (for example to blend with), then the
// MSAA data is initialized from the resolve attachment. In that case,
// RenderPassDescWillLoadMSAAFromResolve returns true.
// When VK_EXT_multisampled_render_to_single_sampled is available, this load is automatically done
// by the driver/hardware (in which case RenderPassDescWillImplicitlyLoadMSAA returns true).
// Otherwise Graphite has to manually do that with a draw call.
bool RenderPassDescWillLoadMSAAFromResolve(const RenderPassDesc& renderPassDesc);
bool RenderPassDescWillImplicitlyLoadMSAA(const RenderPassDesc& renderPassDesc);
// Adjust the load/store ops of the render pass to something common, since they do not affect the
// pipeline and the render pass remains compatible.
RenderPassDesc MakePipelineCompatibleRenderPass(const RenderPassDesc& renderPassDesc);

namespace BackendTextures {
VkImage GetVkImage(const BackendTexture&);
VkImageLayout GetVkImageLayout(const BackendTexture&);
uint32_t GetVkQueueFamilyIndex(const BackendTexture&);
VulkanAlloc GetMemoryAlloc(const BackendTexture&);

void SetMutableState(BackendTexture*, const skgpu::MutableTextureState&);
sk_sp<skgpu::MutableTextureState> GetMutableState(const BackendTexture&);
}  // namespace BackendTextures

} // namespace skgpu::graphite

#endif // skgpu_graphite_VulkanGraphiteUtilsPriv_DEFINED
