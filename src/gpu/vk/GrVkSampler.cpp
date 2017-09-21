/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkSampler.h"

#include "GrVkGpu.h"

static inline VkSamplerAddressMode wrap_mode_to_vk_sampler_address(
        GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case GrSamplerState::WrapMode::kRepeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }
    SK_ABORT("Unknown wrap mode.");
    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
}

GrVkSampler* GrVkSampler::Create(const GrVkGpu* gpu, const GrSamplerState& samplerState,
                                 uint32_t maxMipLevel) {
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
    createInfo.pNext = 0;
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
    bool useMipMaps = GrSamplerState::Filter::kMipMap == samplerState.filter() && maxMipLevel > 0;
    createInfo.maxLod = !useMipMaps ? 0.0f : (float)(maxMipLevel);
    createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;

    VkSampler sampler;
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateSampler(gpu->device(),
                                                          &createInfo,
                                                          nullptr,
                                                          &sampler));

    return new GrVkSampler(sampler, GenerateKey(samplerState, maxMipLevel));
}

void GrVkSampler::freeGPUData(const GrVkGpu* gpu) const {
    SkASSERT(fSampler);
    GR_VK_CALL(gpu->vkInterface(), DestroySampler(gpu->device(), fSampler, nullptr));
}

uint16_t GrVkSampler::GenerateKey(const GrSamplerState& samplerState, uint32_t maxMipLevel) {
    const int kTileModeXShift = 2;
    const int kTileModeYShift = 4;
    const int kMipLevelShift = 6;

    SkASSERT(static_cast<int>(samplerState.filter()) <= 3);
    uint16_t key = static_cast<uint16_t>(samplerState.filter());

    SkASSERT(static_cast<int>(samplerState.wrapModeX()) <= 4);
    key |= (static_cast<uint16_t>(samplerState.wrapModeX()) << kTileModeXShift);

    SkASSERT(static_cast<int>(samplerState.wrapModeY()) <= 4);
    key |= (static_cast<uint16_t>(samplerState.wrapModeY()) << kTileModeYShift);

    SkASSERT(maxMipLevel < 1024);
    key |= (maxMipLevel << kMipLevelShift);

    return key;
}
