/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkUtil_DEFINED
#define GrVkUtil_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "include/private/SkMacros.h"
#include "src/gpu/GrColor.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/vk/GrVkInterface.h"
#include "src/sksl/ir/SkSLProgram.h"

class GrVkGpu;

// makes a Vk call on the interface
#define GR_VK_CALL(IFACE, X) (IFACE)->fFunctions.f##X

#define GR_VK_CALL_RESULT(GPU, RESULT, X)                                 \
    do {                                                                  \
        (RESULT) = GR_VK_CALL(GPU->vkInterface(), X);                     \
        SkASSERT(VK_SUCCESS == RESULT || VK_ERROR_DEVICE_LOST == RESULT); \
        if (RESULT != VK_SUCCESS && !GPU->isDeviceLost()) {               \
            SkDebugf("Failed vulkan call. Error: %d," #X "\n", RESULT);   \
        }                                                                 \
        if (RESULT == VK_ERROR_DEVICE_LOST) {                             \
            GPU->setDeviceLost();                                         \
        } else if (RESULT == VK_ERROR_OUT_OF_HOST_MEMORY ||               \
                   RESULT == VK_ERROR_OUT_OF_DEVICE_MEMORY) {             \
            GPU->setOOMed();                                              \
        }                                                                 \
    } while (false)

#define GR_VK_CALL_RESULT_NOCHECK(GPU, RESULT, X)             \
    do {                                                      \
        (RESULT) = GR_VK_CALL(GPU->vkInterface(), X);         \
        if (RESULT == VK_ERROR_DEVICE_LOST) {                 \
            GPU->setDeviceLost();                             \
        } else if (RESULT == VK_ERROR_OUT_OF_HOST_MEMORY ||   \
                   RESULT == VK_ERROR_OUT_OF_DEVICE_MEMORY) { \
            GPU->setOOMed();                                  \
        }                                                     \
    } while (false)

// same as GR_VK_CALL but checks for success
#define GR_VK_CALL_ERRCHECK(GPU, X)                                  \
    VkResult SK_MACRO_APPEND_LINE(ret);                              \
    GR_VK_CALL_RESULT(GPU, SK_MACRO_APPEND_LINE(ret), X)             \


bool GrVkFormatIsSupported(VkFormat);

static constexpr uint32_t GrVkFormatChannels(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:           return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R8_UNORM:                 return kRed_SkColorChannelFlag;
        case VK_FORMAT_B8G8R8A8_UNORM:           return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:      return kRGB_SkColorChannelFlags;
        case VK_FORMAT_R16G16B16A16_SFLOAT:      return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16_SFLOAT:               return kRed_SkColorChannelFlag;
        case VK_FORMAT_R8G8B8_UNORM:             return kRGB_SkColorChannelFlags;
        case VK_FORMAT_R8G8_UNORM:               return kRG_SkColorChannelFlags;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:    return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:    return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R32G32B32A32_SFLOAT:      return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R8G8B8A8_SRGB:            return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:  return kRGB_SkColorChannelFlags;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:      return kRGB_SkColorChannelFlags;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16_UNORM:                return kRed_SkColorChannelFlag;
        case VK_FORMAT_R16G16_UNORM:             return kRG_SkColorChannelFlags;
        case VK_FORMAT_R16G16B16A16_UNORM:       return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16G16_SFLOAT:            return kRG_SkColorChannelFlags;

        default:                                 return 0;
    }
}

bool GrVkFormatNeedsYcbcrSampler(VkFormat format);

bool GrSampleCountToVkSampleCount(uint32_t samples, VkSampleCountFlagBits* vkSamples);

bool GrCompileVkShaderModule(GrVkGpu* gpu,
                             const SkSL::String& shaderString,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo,
                             const SkSL::Program::Settings& settings,
                             SkSL::String* outSPIRV,
                             SkSL::Program::Inputs* outInputs);

bool GrInstallVkShaderModule(GrVkGpu* gpu,
                             const SkSL::String& spirv,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo);

/**
 * Returns true if the format is compressed.
 */
bool GrVkFormatIsCompressed(VkFormat);

#if GR_TEST_UTILS
static constexpr const char* GrVkFormatToStr(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:           return "R8G8B8A8_UNORM";
        case VK_FORMAT_R8_UNORM:                 return "R8_UNORM";
        case VK_FORMAT_B8G8R8A8_UNORM:           return "B8G8R8A8_UNORM";
        case VK_FORMAT_R5G6B5_UNORM_PACK16:      return "R5G6B5_UNORM_PACK16";
        case VK_FORMAT_R16G16B16A16_SFLOAT:      return "R16G16B16A16_SFLOAT";
        case VK_FORMAT_R16_SFLOAT:               return "R16_SFLOAT";
        case VK_FORMAT_R8G8B8_UNORM:             return "R8G8B8_UNORM";
        case VK_FORMAT_R8G8_UNORM:               return "R8G8_UNORM";
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32: return "A2B10G10R10_UNORM_PACK32";
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32: return "A2R10G10B10_UNORM_PACK32";
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:    return "B4G4R4A4_UNORM_PACK16";
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:    return "R4G4B4A4_UNORM_PACK16";
        case VK_FORMAT_R32G32B32A32_SFLOAT:      return "R32G32B32A32_SFLOAT";
        case VK_FORMAT_R8G8B8A8_SRGB:            return "R8G8B8A8_SRGB";
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:  return "ETC2_R8G8B8_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:      return "BC1_RGB_UNORM_BLOCK";
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return "BC1_RGBA_UNORM_BLOCK";
        case VK_FORMAT_R16_UNORM:                return "R16_UNORM";
        case VK_FORMAT_R16G16_UNORM:             return "R16G16_UNORM";
        case VK_FORMAT_R16G16B16A16_UNORM:       return "R16G16B16A16_UNORM";
        case VK_FORMAT_R16G16_SFLOAT:            return "R16G16_SFLOAT";

        default:                                 return "Unknown";
    }
}

#endif
#endif
