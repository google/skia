/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkSampler.h"

#include "GrVkGpu.h"
#include "GrVkSamplerYcbcrConversion.h"

static inline VkSamplerAddressMode wrap_mode_to_vk_sampler_address(
        GrSamplerState::WrapMode wrapMode) {
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
    SK_ABORT("Unknown wrap mode.");
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
}

GrVkSampler* GrVkSampler::Create(GrVkGpu* gpu, const GrSamplerState& samplerState,
                                 const GrVkYcbcrConversionInfo& ycbcrInfo) {
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
    createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    createInfo.addressModeU = wrap_mode_to_vk_sampler_address(samplerState.wrapModeX());
    createInfo.addressModeV = wrap_mode_to_vk_sampler_address(samplerState.wrapModeY());
    createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE; // Shouldn't matter
    createInfo.mipLodBias = 0.0f;
    createInfo.anisotropyEnable = VK_FALSE;
    createInfo.maxAnisotropy = 1.0f;
    createInfo.compareEnable = VK_FALSE;
    createInfo.compareOp = VK_COMPARE_OP_NEVER;
    // Vulkan doesn't have a direct mapping of GL's nearest or linear filters for minFilter since
    // there is always a mipmapMode. To get the same effect as GL we can set minLod = maxLod = 0.0.
    // This works since our min and mag filters are the same (this forces us to use mag on the 0
    // level mip). If the filters weren't the same we could set min = 0 and max = 0.25 to force
    // the minFilter on mip level 0.
    createInfo.minLod = 0.0f;
    bool useMipMaps = GrSamplerState::Filter::kMipMap == samplerState.filter();
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

        const VkFormatFeatureFlags& flags = ycbcrInfo.fExternalFormatFeatures;

        if (!SkToBool(flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT)) {
            createInfo.magFilter = VK_FILTER_NEAREST;
            createInfo.minFilter = VK_FILTER_NEAREST;
        } else if (
                !(flags &
                  VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_SEPARATE_RECONSTRUCTION_FILTER_BIT)) {
            createInfo.magFilter = ycbcrInfo.fChromaFilter;
            createInfo.minFilter = ycbcrInfo.fChromaFilter;
        }

        // Required values when using ycbcr conversion
        createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.unnormalizedCoordinates = VK_FALSE;
    }

    VkSampler sampler;
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateSampler(gpu->device(),
                                                          &createInfo,
                                                          nullptr,
                                                          &sampler));

    return new GrVkSampler(sampler, ycbcrConversion, GenerateKey(samplerState, ycbcrInfo));
}

void GrVkSampler::freeGPUData(GrVkGpu* gpu) const {
    SkASSERT(fSampler);
    GR_VK_CALL(gpu->vkInterface(), DestroySampler(gpu->device(), fSampler, nullptr));
    if (fYcbcrConversion) {
        fYcbcrConversion->unref(gpu);
    }
}

void GrVkSampler::abandonGPUData() const {
    if (fYcbcrConversion) {
        fYcbcrConversion->unrefAndAbandon();
    }
}

GrVkSampler::Key GrVkSampler::GenerateKey(const GrSamplerState& samplerState,
                                          const GrVkYcbcrConversionInfo& ycbcrInfo) {
    const int kTileModeXShift = 2;
    const int kTileModeYShift = 4;

    SkASSERT(static_cast<int>(samplerState.filter()) <= 3);
    uint8_t samplerKey = static_cast<uint16_t>(samplerState.filter());

    SkASSERT(static_cast<int>(samplerState.wrapModeX()) <= 3);
    samplerKey |= (static_cast<uint8_t>(samplerState.wrapModeX()) << kTileModeXShift);

    SkASSERT(static_cast<int>(samplerState.wrapModeY()) <= 3);
    samplerKey |= (static_cast<uint8_t>(samplerState.wrapModeY()) << kTileModeYShift);

    return {samplerKey, GrVkSamplerYcbcrConversion::GenerateKey(ycbcrInfo)};
}

