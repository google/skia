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

#include <string>

class SkStream;
class SkWStream;

// Helper macros to call functions on the VulkanInterface without checking for errors. Note: This
// cannot require a VulkanSharedContext because driver calls are needed before the shared context
// has been initialized.
#define VULKAN_CALL(IFACE, X) (IFACE)->fFunctions.f##X

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
        (RESULT) = VULKAN_CALL((SHARED_CONTEXT)->interface(), X);         \
        SkASSERT(VK_SUCCESS == RESULT || VK_ERROR_DEVICE_LOST == RESULT); \
        VULKAN_LOG_IF_NOT_SUCCESS(SHARED_CONTEXT, RESULT, #X);            \
        (SHARED_CONTEXT)->checkVkResult(RESULT);                          \
    } while (false)

// same as VULKAN_CALL but checks for success
#define VULKAN_CALL_ERRCHECK(SHARED_CONTEXT, X) \
    VkResult SK_MACRO_APPEND_LINE(ret);         \
    VULKAN_CALL_RESULT(SHARED_CONTEXT, SK_MACRO_APPEND_LINE(ret), X)

#define VULKAN_CALL_RESULT_NOCHECK(IFACE, RESULT, X) \
    do {                                             \
        (RESULT) = VULKAN_CALL(IFACE, X);            \
    } while (false)
namespace skgpu::graphite {

class VulkanSharedContext;
struct RenderPassDesc;

VkShaderModule createVulkanShaderModule(const VulkanSharedContext*,
                                        const std::string& spirv,
                                        VkShaderStageFlagBits);

VkDescriptorType DsTypeEnumToVkDs(DescriptorType);
void DescriptorDataToVkDescSetLayout(const VulkanSharedContext*,
                                     const SkSpan<DescriptorData>&,
                                     VkDescriptorSetLayout*);

bool vkFormatIsSupported(VkFormat);

VkShaderStageFlags PipelineStageFlagsToVkShaderStageFlags(SkEnumBitMask<PipelineStageFlags>);

bool RenderPassDescWillLoadMSAAFromResolve(const RenderPassDesc& renderPassDesc);

struct VulkanTextureSpec {
    VulkanTextureSpec()
            : fFlags(0)
            , fFormat(VK_FORMAT_UNDEFINED)
            , fImageTiling(VK_IMAGE_TILING_OPTIMAL)
            , fImageUsageFlags(0)
            , fSharingMode(VK_SHARING_MODE_EXCLUSIVE)
            , fAspectMask(VK_IMAGE_ASPECT_COLOR_BIT)
            , fYcbcrConversionInfo({}) {}
    VulkanTextureSpec(const VulkanTextureInfo& info)
            : fFlags(info.fFlags)
            , fFormat(info.fFormat)
            , fImageTiling(info.fImageTiling)
            , fImageUsageFlags(info.fImageUsageFlags)
            , fSharingMode(info.fSharingMode)
            , fAspectMask(info.fAspectMask)
            , fYcbcrConversionInfo(info.fYcbcrConversionInfo) {}

    bool operator==(const VulkanTextureSpec& that) const {
        return fFlags == that.fFlags &&
               fFormat == that.fFormat &&
               fImageTiling == that.fImageTiling &&
               fImageUsageFlags == that.fImageUsageFlags &&
               fSharingMode == that.fSharingMode &&
               fAspectMask == that.fAspectMask &&
               fYcbcrConversionInfo == that.fYcbcrConversionInfo;
    }

    bool isCompatible(const VulkanTextureSpec& that) const {
        // The usages may match or the usage passed in may be a superset of the usage stored within.
        return fFlags == that.fFlags &&
               fFormat == that.fFormat &&
               fImageTiling == that.fImageTiling &&
               fSharingMode == that.fSharingMode &&
               fAspectMask == that.fAspectMask &&
               (fImageUsageFlags & that.fImageUsageFlags) == fImageUsageFlags &&
               fYcbcrConversionInfo == that.fYcbcrConversionInfo;
    }

    SkString toString() const {
        return SkStringPrintf(
                "flags=0x%08X,format=%d,imageTiling=%d,imageUsageFlags=0x%08X,sharingMode=%d,"
                "aspectMask=%u",
                fFlags,
                fFormat,
                fImageTiling,
                fImageUsageFlags,
                fSharingMode,
                fAspectMask);
    }

    bool serialize(SkWStream*) const;
    static bool Deserialize(SkStream*, VulkanTextureSpec* out);

    VkImageCreateFlags         fFlags;
    VkFormat                   fFormat;
    VkImageTiling              fImageTiling;
    VkImageUsageFlags          fImageUsageFlags;
    VkSharingMode              fSharingMode;
    VkImageAspectFlags         fAspectMask;
    VulkanYcbcrConversionInfo  fYcbcrConversionInfo;
};

VulkanTextureInfo VulkanTextureSpecToTextureInfo(const VulkanTextureSpec& vkSpec,
                                                 uint32_t sampleCount,
                                                 Mipmapped mipmapped);

namespace TextureInfos {
VulkanTextureSpec GetVulkanTextureSpec(const TextureInfo&);
VkFormat GetVkFormat(const TextureInfo&);
VkImageUsageFlags GetVkUsageFlags(const TextureInfo&);
VulkanYcbcrConversionInfo GetVulkanYcbcrConversionInfo(const TextureInfo&);
}  // namespace TextureInfos

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
