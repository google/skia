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
#include "include/private/base/SkMacros.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrColor.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/vk/VulkanInterface.h"
#include "src/sksl/ir/SkSLProgram.h"

namespace SkSL { struct ProgramSettings; }
class GrVkGpu;

// makes a Vk call on the interface
#define GR_VK_CALL(IFACE, X) (IFACE)->fFunctions.f##X

// Note: must be called before checkVkResult, since this does not log if the GPU is already
// considering the device to be lost.
#define GR_VK_LOG_IF_NOT_SUCCESS(GPU, RESULT, X, ...)                                   \
    do {                                                                                \
        if (RESULT != VK_SUCCESS && !GPU->isDeviceLost()) {                             \
            SkDebugf("Failed vulkan call. Error: %d, " X "\n", RESULT, ##__VA_ARGS__);  \
        }                                                                               \
    } while (false)

#define GR_VK_CALL_RESULT(GPU, RESULT, X)                                 \
    do {                                                                  \
        (RESULT) = GR_VK_CALL(GPU->vkInterface(), X);                     \
        SkASSERT(VK_SUCCESS == RESULT || VK_ERROR_DEVICE_LOST == RESULT); \
        GR_VK_LOG_IF_NOT_SUCCESS(GPU, RESULT, #X);                        \
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

static constexpr GrColorFormatDesc GrVkFormatDesc(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_R8_UNORM:
            return GrColorFormatDesc::MakeR(8, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_B8G8R8A8_UNORM:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            return GrColorFormatDesc::MakeRGB(5, 6, 5, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
            return GrColorFormatDesc::MakeRGB(5, 6, 5, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return GrColorFormatDesc::MakeRGBA(16, GrColorTypeEncoding::kFloat);
        case VK_FORMAT_R16_SFLOAT:
            return GrColorFormatDesc::MakeR(16, GrColorTypeEncoding::kFloat);
        case VK_FORMAT_R8G8B8_UNORM:
            return GrColorFormatDesc::MakeRGB(8, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_R8G8_UNORM:
            return GrColorFormatDesc::MakeRG(8, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return GrColorFormatDesc::MakeRGBA(10, 2, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            return GrColorFormatDesc::MakeRGBA(10, 2, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            return GrColorFormatDesc::MakeRGBA(4, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            return GrColorFormatDesc::MakeRGBA(4, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_R8G8B8A8_SRGB:
            return GrColorFormatDesc::MakeRGBA(8, GrColorTypeEncoding::kSRGBUnorm);
        case VK_FORMAT_R16_UNORM:
            return GrColorFormatDesc::MakeR(16, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_R16G16_UNORM:
            return GrColorFormatDesc::MakeRG(16, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_R16G16B16A16_UNORM:
            return GrColorFormatDesc::MakeRGBA(16, GrColorTypeEncoding::kUnorm);
        case VK_FORMAT_R16G16_SFLOAT:
            return GrColorFormatDesc::MakeRG(16, GrColorTypeEncoding::kFloat);

        // Compressed texture formats are not expected to have a description.
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK: return GrColorFormatDesc::MakeInvalid();
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:     return GrColorFormatDesc::MakeInvalid();
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:    return GrColorFormatDesc::MakeInvalid();

        // This type only describes color channels.
        case VK_FORMAT_S8_UINT:            return GrColorFormatDesc::MakeInvalid();
        case VK_FORMAT_D24_UNORM_S8_UINT:  return GrColorFormatDesc::MakeInvalid();
        case VK_FORMAT_D32_SFLOAT_S8_UINT: return GrColorFormatDesc::MakeInvalid();

        default: return GrColorFormatDesc::MakeInvalid();
    }
}

bool GrCompileVkShaderModule(GrVkGpu* gpu,
                             const std::string& shaderString,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo,
                             const SkSL::ProgramSettings& settings,
                             std::string* outSPIRV,
                             SkSL::Program::Interface* outInterface);

bool GrInstallVkShaderModule(GrVkGpu* gpu,
                             const std::string& spirv,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo);

#endif
