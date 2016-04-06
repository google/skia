/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkUtil.h"

bool GrPixelConfigToVkFormat(GrPixelConfig config, VkFormat* format) {
    VkFormat dontCare;
    if (!format) {
        format = &dontCare;
    }

    switch (config) {
        case kRGBA_8888_GrPixelConfig:
            *format = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case kBGRA_8888_GrPixelConfig:
            *format = VK_FORMAT_B8G8R8A8_UNORM;
            break;
        case kSRGBA_8888_GrPixelConfig:
            *format = VK_FORMAT_R8G8B8A8_SRGB;
            break;
        case kSBGRA_8888_GrPixelConfig:
            *format = VK_FORMAT_B8G8R8A8_SRGB;
            break;
        case kRGB_565_GrPixelConfig:
            *format = VK_FORMAT_R5G6B5_UNORM_PACK16;
            break;
        case kRGBA_4444_GrPixelConfig:
            *format = VK_FORMAT_R4G4B4A4_UNORM_PACK16;
            break;
        case kIndex_8_GrPixelConfig:
            // No current vulkan support for this config
            return false;
        case kAlpha_8_GrPixelConfig:
            *format = VK_FORMAT_R8_UNORM;
            break;
        case kETC1_GrPixelConfig:
            // converting to ETC2 which is a superset of ETC1
            *format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            break;
        case kLATC_GrPixelConfig:
            // No current vulkan support for this config
            return false;
        case kR11_EAC_GrPixelConfig:
            *format = VK_FORMAT_EAC_R11_UNORM_BLOCK;
            break;
        case kASTC_12x12_GrPixelConfig:
            *format = VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
            break;
        case kRGBA_float_GrPixelConfig:
            *format = VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
        case kRGBA_half_GrPixelConfig:
            *format = VK_FORMAT_R16G16B16A16_SFLOAT;
            break;
        case kAlpha_half_GrPixelConfig:
            *format = VK_FORMAT_R16_SFLOAT;
            break;
        default:
            return false;
    }
    return true;
}

bool GrVkFormatToPixelConfig(VkFormat format, GrPixelConfig* config) {
    GrPixelConfig dontCare;
    if (!config) {
        config = &dontCare;
    }

    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            *config = kRGBA_8888_GrPixelConfig;
            break;
        case VK_FORMAT_B8G8R8A8_UNORM:
            *config = kBGRA_8888_GrPixelConfig;
            break;
        case VK_FORMAT_R8G8B8A8_SRGB:
            *config = kSRGBA_8888_GrPixelConfig;
            break;
        case VK_FORMAT_B8G8R8A8_SRGB:
            *config = kSBGRA_8888_GrPixelConfig;
            break;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            *config = kRGB_565_GrPixelConfig;
            break;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            *config = kRGBA_4444_GrPixelConfig;
            break;
        case VK_FORMAT_R8_UNORM:
            *config = kAlpha_8_GrPixelConfig;
            break;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            *config = kETC1_GrPixelConfig;
            break;
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
            *config = kR11_EAC_GrPixelConfig;
            break;
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
            *config = kASTC_12x12_GrPixelConfig;
            break;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            *config = kRGBA_float_GrPixelConfig;
            break;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            *config = kRGBA_half_GrPixelConfig;
            break;
        case VK_FORMAT_R16_SFLOAT:
            *config = kAlpha_half_GrPixelConfig;
            break;
        default:
            return false;
    }
    return true;
}

bool GrSampleCountToVkSampleCount(uint32_t samples, VkSampleCountFlagBits* vkSamples) {
    switch (samples) {
        case 0: // fall through
        case 1:
            *vkSamples = VK_SAMPLE_COUNT_1_BIT;
            return true;
        case 2:
            *vkSamples = VK_SAMPLE_COUNT_2_BIT;
            return true;
        case 4:
            *vkSamples = VK_SAMPLE_COUNT_2_BIT;
            return true;
        case 8:
            *vkSamples = VK_SAMPLE_COUNT_2_BIT;
            return true;
        case 16:
            *vkSamples = VK_SAMPLE_COUNT_2_BIT;
            return true;
        case 32:
            *vkSamples = VK_SAMPLE_COUNT_2_BIT;
            return true;
        case 64:
            *vkSamples = VK_SAMPLE_COUNT_2_BIT;
            return true;
        default:
            return false;
    }
}
