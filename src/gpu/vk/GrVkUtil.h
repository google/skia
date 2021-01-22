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
        GPU->checkVkResult(RESULT);                                       \
    } while (false)

#define GR_VK_CALL_RESULT_NOCHECK(GPU, RESULT, X)     \
    do {                                              \
        (RESULT) = GR_VK_CALL(GPU->vkInterface(), X); \
        GPU->checkVkResult(RESULT);                   \
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
        case VK_FORMAT_R8G8B8A8_SRGB:            return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:  return kRGB_SkColorChannelFlags;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:      return kRGB_SkColorChannelFlags;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:     return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16_UNORM:                return kRed_SkColorChannelFlag;
        case VK_FORMAT_R16G16_UNORM:             return kRG_SkColorChannelFlags;
        case VK_FORMAT_R16G16B16A16_UNORM:       return kRGBA_SkColorChannelFlags;
        case VK_FORMAT_R16G16_SFLOAT:            return kRG_SkColorChannelFlags;
        case VK_FORMAT_S8_UINT:                  return 0;
        case VK_FORMAT_D24_UNORM_S8_UINT:        return 0;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:       return 0;
        default:                                 return 0;
    }
}

static constexpr size_t GrVkFormatBytesPerBlock(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:            return 4;
        case VK_FORMAT_R8_UNORM:                  return 1;
        case VK_FORMAT_B8G8R8A8_UNORM:            return 4;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:       return 2;
        case VK_FORMAT_R16G16B16A16_SFLOAT:       return 8;
        case VK_FORMAT_R16_SFLOAT:                return 2;
        case VK_FORMAT_R8G8B8_UNORM:              return 3;
        case VK_FORMAT_R8G8_UNORM:                return 2;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:  return 4;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:  return 4;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:     return 2;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:     return 2;
        case VK_FORMAT_R8G8B8A8_SRGB:             return 4;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:   return 8;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:       return 8;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:      return 8;
        case VK_FORMAT_R16_UNORM:                 return 2;
        case VK_FORMAT_R16G16_UNORM:              return 4;
        case VK_FORMAT_R16G16B16A16_UNORM:        return 8;
        case VK_FORMAT_R16G16_SFLOAT:             return 4;
        // Currently we are just over estimating this value to be used in gpu size calculations even
        // though the actually size is probably less. We should instead treat planar formats similar
        // to compressed textures that go through their own special query for calculating size.
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: return 3;
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:  return 3;
        case VK_FORMAT_S8_UINT:                   return 1;
        case VK_FORMAT_D24_UNORM_S8_UINT:         return 4;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:        return 8;

        default:                                 return 0;
    }
}

static constexpr int GrVkFormatStencilBits(VkFormat format) {
    switch (format) {
        case VK_FORMAT_S8_UINT:
            return 8;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return 8;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return 8;
        default:
            return 0;
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

#if defined(SK_DEBUG) || GR_TEST_UTILS
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
        case VK_FORMAT_S8_UINT:                  return "S8_UINT";
        case VK_FORMAT_D24_UNORM_S8_UINT:        return "D24_UNORM_S8_UINT";
        case VK_FORMAT_D32_SFLOAT_S8_UINT:       return "D32_SFLOAT_S8_UINT";

        default:                                 return "Unknown";
    }
}

#endif
#endif
