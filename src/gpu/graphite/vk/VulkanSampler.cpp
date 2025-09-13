/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanSampler.h"

#include "include/core/SkSamplingOptions.h"
#include "src/gpu/graphite/vk/VulkanCaps.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtils.h"

namespace skgpu::graphite {

VulkanSampler::VulkanSampler(const VulkanSharedContext* sharedContext,
                             const SamplerDesc& desc,
                             VkSampler sampler,
                             sk_sp<VulkanYcbcrConversion> ycbcrConversion)
        : Sampler(sharedContext)
        , fDesc(desc)
        , fSampler(sampler)
        , fYcbcrConversion(ycbcrConversion) {}

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

sk_sp<VulkanSampler> VulkanSampler::Make(
        const VulkanSharedContext* sharedContext,
        const SamplerDesc& desc,
        sk_sp<VulkanYcbcrConversion> ycbcrConversion) {
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    VkSamplerYcbcrConversionInfo conversionInfo = {};
    if (ycbcrConversion) {
        conversionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
        conversionInfo.conversion = ycbcrConversion->ycbcrConversion();
        samplerInfo.pNext = &conversionInfo;
    }

    VkFilter minMagFilter = [&] {
        switch (desc.samplingOptions().filter) {
            case SkFilterMode::kNearest: return VK_FILTER_NEAREST;
            case SkFilterMode::kLinear:  return VK_FILTER_LINEAR;
        }
        SkUNREACHABLE;
    }();
    if (ycbcrConversion && ycbcrConversion->requiredFilter().has_value()) {
        minMagFilter = ycbcrConversion->requiredFilter().value();
    }

    VkSamplerMipmapMode mipmapMode = [&] {
      switch (desc.samplingOptions().mipmap) {
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
    samplerInfo.addressModeU = tile_mode_to_vk_sampler_address(desc.tileModeX());
    samplerInfo.addressModeV = tile_mode_to_vk_sampler_address(desc.tileModeY());
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
    samplerInfo.maxLod = (desc.samplingOptions().mipmap == SkMipmapMode::kNone) ? 0.0f
                                                                                : VK_LOD_CLAMP_NONE;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    VkSampler sampler;
    VkResult result;
    VULKAN_CALL_RESULT(sharedContext,
                       result,
                       CreateSampler(sharedContext->device(), &samplerInfo, nullptr, &sampler));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    return sk_sp<VulkanSampler>(new VulkanSampler(sharedContext,
                                                  desc,
                                                  sampler,
                                                  std::move(ycbcrConversion)));
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
