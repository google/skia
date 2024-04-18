/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanSampler.h"

#include "include/core/SkSamplingOptions.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"

namespace skgpu::graphite {

VulkanSampler::VulkanSampler(const VulkanSharedContext* sharedContext,
                             VkSampler sampler)
        : Sampler(sharedContext)
        , fSampler(sampler) {}

static VkSamplerAddressMode tile_mode_to_vk_sampler_address(SkTileMode tileMode) {
    switch (tileMode) {
        case SkTileMode::kClamp:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SkTileMode::kRepeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SkTileMode::kMirror:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case SkTileMode::kDecal:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
    SkUNREACHABLE;
}

sk_sp<VulkanSampler> VulkanSampler::Make(const VulkanSharedContext* sharedContext,
                                         const SkSamplingOptions& samplingOptions,
                                         SkTileMode xTileMode,
                                         SkTileMode yTileMode) {
    VkSamplerCreateInfo samplerInfo;
    memset(&samplerInfo, 0, sizeof(VkSamplerCreateInfo));
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = nullptr;
    samplerInfo.flags = 0;

    VkFilter minMagFilter = [&] {
        switch (samplingOptions.filter) {
            case SkFilterMode::kNearest: return VK_FILTER_NEAREST;
            case SkFilterMode::kLinear:  return VK_FILTER_LINEAR;
        }
        SkUNREACHABLE;
    }();

    VkSamplerMipmapMode mipmapMode = [&] {
      switch (samplingOptions.mipmap) {
          // There is no disable mode. We use max level to disable mip mapping.
          // It may make more sense to use NEAREST for kNone but Chrome pixel tests have
          // been dependent on subtle rendering differences introduced by switching this.
          case SkMipmapMode::kNone:    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
          case SkMipmapMode::kNearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
          case SkMipmapMode::kLinear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
      }
      SkUNREACHABLE;
    }();

    samplerInfo.magFilter = minMagFilter;
    samplerInfo.minFilter = minMagFilter;
    samplerInfo.mipmapMode = mipmapMode;
    samplerInfo.addressModeU = tile_mode_to_vk_sampler_address(xTileMode);
    samplerInfo.addressModeV = tile_mode_to_vk_sampler_address(yTileMode);
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.mipLodBias = 0;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1; // TODO: when we start using aniso, need to add to key
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    // Vulkan doesn't have a direct mapping to use nearest or linear filters for minFilter since
    // there is always a mipmapMode. To get the same effect we can set minLod = maxLod = 0.0.
    // This works since our min and mag filters are the same (this forces us to use mag on the 0
    // level mip). If the filters weren't the same we could set min = 0 and max = 0.25 to force
    // the minFilter on mip level 0.
    samplerInfo.minLod = 0;
    samplerInfo.maxLod = (samplingOptions.mipmap == SkMipmapMode::kNone) ? 0.0f : VK_LOD_CLAMP_NONE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    // TODO: Add VkSamplerYcbcrConversion support by adding YCbCr conversion information to the
    // graphite-level sampler key. Currently, the ResourceProvider only generates a key based on
    // samplingOptions and tileModes.

    VkSampler sampler;
    VkResult result;
    VULKAN_CALL_RESULT(sharedContext,
                       result,
                       CreateSampler(sharedContext->device(), &samplerInfo, nullptr, &sampler));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    return sk_sp<VulkanSampler>(new VulkanSampler(sharedContext, sampler));
}

void VulkanSampler::freeGpuData() {
    const VulkanSharedContext* sharedContext =
        static_cast<const VulkanSharedContext*>(this->sharedContext());
    SkASSERT(fSampler);
    VULKAN_CALL(sharedContext->interface(),
                DestroySampler(sharedContext->device(), fSampler, nullptr));
    fSampler = VK_NULL_HANDLE;
}

} // namespace skgpu::graphite

