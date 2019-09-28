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
// same as GR_VK_CALL but checks for success
#ifdef SK_DEBUG
#define GR_VK_CALL_ERRCHECK(IFACE, X)                          \
    VkResult SK_MACRO_APPEND_LINE(ret) = GR_VK_CALL(IFACE, X); \
    SkASSERT(VK_SUCCESS == SK_MACRO_APPEND_LINE(ret))
#else
#define GR_VK_CALL_ERRCHECK(IFACE, X)  (void) GR_VK_CALL(IFACE, X)
#endif

bool GrVkFormatIsSupported(VkFormat);

bool GrVkFormatNeedsYcbcrSampler(VkFormat format);

#ifdef SK_DEBUG
/**
 * Returns true if the passed in VkFormat and GrColorType are compatible with each other.
 */
bool GrVkFormatColorTypePairIsValid(VkFormat, GrColorType);
#endif

bool GrSampleCountToVkSampleCount(uint32_t samples, VkSampleCountFlagBits* vkSamples);

bool GrCompileVkShaderModule(const GrVkGpu* gpu,
                             const SkSL::String& shaderString,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo,
                             const SkSL::Program::Settings& settings,
                             SkSL::String* outSPIRV,
                             SkSL::Program::Inputs* outInputs);

bool GrInstallVkShaderModule(const GrVkGpu* gpu,
                             const SkSL::String& spirv,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo);

size_t GrVkBytesPerFormat(VkFormat);

/**
 * Returns true if the format is compressed.
 */
bool GrVkFormatIsCompressed(VkFormat);

/**
 * Maps a vk format into the CompressionType enum if applicable.
 */
bool GrVkFormatToCompressionType(VkFormat vkFormat, SkImage::CompressionType* compressionType);

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
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:    return "B4G4R4A4_UNORM_PACK16";
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:    return "R4G4B4A4_UNORM_PACK16";
        case VK_FORMAT_R32G32B32A32_SFLOAT:      return "R32G32B32A32_SFLOAT";
        case VK_FORMAT_R8G8B8A8_SRGB:            return "R8G8B8A8_SRGB";
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:  return "ETC2_R8G8B8_UNORM_BLOCK";
        case VK_FORMAT_R16_UNORM:                return "R16_UNORM";
        case VK_FORMAT_R16G16_UNORM:             return "R16G16_UNORM";
        case VK_FORMAT_R16G16B16A16_UNORM:       return "R16G16B16A16_UNORM";
        case VK_FORMAT_R16G16_SFLOAT:            return "R16G16_SFLOAT";

        default:                                 return "Unknown";
    }
}

#endif
#endif
