/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/ganesh/vk/GrVkSampler.h"

#include "include/core/SkSamplingOptions.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/vk/VulkanTypes.h"
#include "include/private/base/SkTo.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkResourceProvider.h"
#include "src/gpu/ganesh/vk/GrVkSamplerYcbcrConversion.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"

#include <string.h>
#include <algorithm>
#include <atomic>
#include <optional>

static VkSamplerAddressMode wrap_mode_to_vk_sampler_address(GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case GrSamplerState::WrapMode::kRepeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case GrSamplerState::WrapMode::kClampToBorder:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
    SkUNREACHABLE;
}

static VkSamplerMipmapMode mipmap_mode_to_vk_sampler_mipmap_mode(GrSamplerState::MipmapMode mm) {
    switch (mm) {
        // There is no disable mode. We use max level to disable mip mapping.
        // It may make more sense to use NEAREST for kNone but Chrome pixel tests seam dependent
        // on subtle rendering differences introduced by switching this.
        case GrSamplerState::MipmapMode::kNone:    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case GrSamplerState::MipmapMode::kNearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case GrSamplerState::MipmapMode::kLinear:  return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    }
    SkUNREACHABLE;
}

GrVkSampler* GrVkSampler::Create(GrVkGpu* gpu,
                                 GrSamplerState samplerState,
                                 const skgpu::VulkanYcbcrConversionInfo& ycbcrInfo) {
    static VkFilter vkMinFilterModes[] = {
        VK_FILTER_NEAREST,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR
    };
    static VkFilter vkMagFilterModes[] = {
        VK_FILTER_NEAREST,
        VK_FILTER_LINEAR,
        VK_FILTER_LINEAR
    };

    VkSamplerCreateInfo createInfo;
    memset(&createInfo, 0, sizeof(VkSamplerCreateInfo));
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.magFilter = vkMagFilterModes[static_cast<int>(samplerState.filter())];
    createInfo.minFilter = vkMinFilterModes[static_cast<int>(samplerState.filter())];
    createInfo.mipmapMode = mipmap_mode_to_vk_sampler_mipmap_mode(samplerState.mipmapMode());
    createInfo.addressModeU = wrap_mode_to_vk_sampler_address(samplerState.wrapModeX());
    createInfo.addressModeV = wrap_mode_to_vk_sampler_address(samplerState.wrapModeY());
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // Shouldn't matter
    createInfo.mipLodBias = 0.0f;
    createInfo.anisotropyEnable = samplerState.isAniso() ? VK_TRUE : VK_FALSE;
    createInfo.maxAnisotropy = std::min(static_cast<float>(samplerState.maxAniso()),
                                        gpu->vkCaps().maxSamplerAnisotropy());
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_NEVER;
    // Vulkan doesn't have a direct mapping of GL's nearest or linear filters for minFilter since
    // there is always a mipmapMode. To get the same effect as GL we can set minLod = maxLod = 0.0.
    // This works since our min and mag filters are the same (this forces us to use mag on the 0
    // level mip). If the filters weren't the same we could set min = 0 and max = 0.25 to force
    // the minFilter on mip level 0.
    createInfo.minLod = 0.0f;
    bool useMipMaps = samplerState.mipmapped() == skgpu::Mipmapped::kYes;
    createInfo.maxLod = !useMipMaps ? 0.0f : 10000.0f;
    createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;

    VkSamplerYcbcrConversionInfo conversionInfo;
    GrVkSamplerYcbcrConversion* ycbcrConversion = nullptr;
    if (ycbcrInfo.isValid()) {
        SkASSERT(gpu->vkCaps().supportsYcbcrConversion());

        ycbcrConversion =
                gpu->resourceProvider().findOrCreateCompatibleSamplerYcbcrConversion(ycbcrInfo);
        if (!ycbcrConversion) {
            return nullptr;
        }

        conversionInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_INFO;
        conversionInfo.pNext = nullptr;
        conversionInfo.conversion = ycbcrConversion->ycbcrConversion();

        createInfo.pNext = &conversionInfo;

        VkFormatFeatureFlags flags = ycbcrInfo.fFormatFeatures;
        if (ycbcrConversion->requiredFilter().has_value()) {
            createInfo.magFilter = ycbcrConversion->requiredFilter().value();
            createInfo.minFilter = ycbcrConversion->requiredFilter().value();
        } else if (!SkToBool(flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            createInfo.magFilter = VK_FILTER_NEAREST;
            createInfo.minFilter = VK_FILTER_NEAREST;
        }

        // Required values when using ycbcr conversion
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.unnormalizedCoordinates = VK_FALSE;
    }

    VkSampler sampler;
    VkResult result;
    GR_VK_CALL_RESULT(gpu, result, CreateSampler(gpu->device(), &createInfo, nullptr, &sampler));
    if (result != VK_SUCCESS) {
        ycbcrConversion->unref();
        return nullptr;
    }

    return new GrVkSampler(gpu, sampler, ycbcrConversion, GenerateKey(samplerState, ycbcrInfo));
}

void GrVkSampler::freeGPUData() const {
    SkASSERT(fSampler);
    GR_VK_CALL(fGpu->vkInterface(), DestroySampler(fGpu->device(), fSampler, nullptr));
    if (fYcbcrConversion) {
        fYcbcrConversion->unref();
    }
}

GrVkSampler::Key GrVkSampler::GenerateKey(GrSamplerState samplerState,
                                          const skgpu::VulkanYcbcrConversionInfo& ycbcrInfo) {
    // In VK the max aniso value is specified in addition to min/mag/mip filters and the
    // driver is encouraged to consider the other filter settings when doing aniso.
    return {samplerState.asKey(/*anisoIsOrthogonal=*/true),
            GrVkSamplerYcbcrConversion::GenerateKey(ycbcrInfo)};
}

 uint32_t GrVkSampler::GenID() {
    static std::atomic<uint32_t> nextID{1};
    uint32_t id;
    do {
        id = nextID++;
    } while (id == SK_InvalidUniqueID);
    return id;
}
