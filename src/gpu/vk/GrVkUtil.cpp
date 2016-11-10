/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkUtil.h"

#include "vk/GrVkGpu.h"
#if USE_SKSL
#include "SkSLCompiler.h"
#endif

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
            // R4G4B4A4 is not required to be supported so we actually
            // store the data is if it was B4G4R4A4 and swizzle in shaders
            *format = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
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
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            // R4G4B4A4 is not required to be supported so we actually
            // store RGBA_4444 data as B4G4R4A4.
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

bool GrVkFormatIsSRGB(VkFormat format, VkFormat* linearFormat) {
    VkFormat linearFmt = format;
    switch (format) {
        case VK_FORMAT_R8_SRGB:
            linearFmt = VK_FORMAT_R8_UNORM;
            break;
        case VK_FORMAT_R8G8_SRGB:
            linearFmt = VK_FORMAT_R8G8_UNORM;
            break;
        case VK_FORMAT_R8G8B8_SRGB:
            linearFmt = VK_FORMAT_R8G8B8_UNORM;
            break;
        case VK_FORMAT_B8G8R8_SRGB:
            linearFmt = VK_FORMAT_B8G8R8_UNORM;
            break;
        case VK_FORMAT_R8G8B8A8_SRGB:
            linearFmt = VK_FORMAT_R8G8B8A8_UNORM;
            break;
        case VK_FORMAT_B8G8R8A8_SRGB:
            linearFmt = VK_FORMAT_B8G8R8A8_UNORM;
            break;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            linearFmt = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
            break;
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            linearFmt = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
            break;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
            linearFmt = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            break;
        case VK_FORMAT_BC2_SRGB_BLOCK:
            linearFmt = VK_FORMAT_BC2_UNORM_BLOCK;
            break;
        case VK_FORMAT_BC3_SRGB_BLOCK:
            linearFmt = VK_FORMAT_BC3_UNORM_BLOCK;
            break;
        case VK_FORMAT_BC7_SRGB_BLOCK:
            linearFmt = VK_FORMAT_BC7_UNORM_BLOCK;
            break;
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            break;
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
            break;
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
            break;
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
            linearFmt = VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
            break;
        default:
            break;
    }
    if (linearFormat) {
        *linearFormat = linearFmt;
    }
    return (linearFmt != format);
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
            *vkSamples = VK_SAMPLE_COUNT_4_BIT;
            return true;
        case 8:
            *vkSamples = VK_SAMPLE_COUNT_8_BIT;
            return true;
        case 16:
            *vkSamples = VK_SAMPLE_COUNT_16_BIT;
            return true;
        case 32:
            *vkSamples = VK_SAMPLE_COUNT_32_BIT;
            return true;
        case 64:
            *vkSamples = VK_SAMPLE_COUNT_64_BIT;
            return true;
        default:
            return false;
    }
}

#if USE_SKSL
SkSL::Program::Kind vk_shader_stage_to_skiasl_kind(VkShaderStageFlagBits stage) {
    if (VK_SHADER_STAGE_VERTEX_BIT == stage) {
        return SkSL::Program::kVertex_Kind;
    }
    SkASSERT(VK_SHADER_STAGE_FRAGMENT_BIT == stage);
    return SkSL::Program::kFragment_Kind;
}
#else
shaderc_shader_kind vk_shader_stage_to_shaderc_kind(VkShaderStageFlagBits stage) {
    if (VK_SHADER_STAGE_VERTEX_BIT == stage) {
        return shaderc_glsl_vertex_shader;
    }
    SkASSERT(VK_SHADER_STAGE_FRAGMENT_BIT == stage);
    return shaderc_glsl_fragment_shader;
}
#endif

bool GrCompileVkShaderModule(const GrVkGpu* gpu,
                             const char* shaderString,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo) {
    VkShaderModuleCreateInfo moduleCreateInfo;
    memset(&moduleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.flags = 0;

#if USE_SKSL
    std::string code;
#else
    shaderc_compilation_result_t result = nullptr;
#endif

    if (gpu->vkCaps().canUseGLSLForShaderModule()) {
        moduleCreateInfo.codeSize = strlen(shaderString);
        moduleCreateInfo.pCode = (const uint32_t*)shaderString;
    } else {

#if USE_SKSL
        bool result = gpu->shaderCompiler()->toSPIRV(vk_shader_stage_to_skiasl_kind(stage),
                                                     std::string(shaderString),
                                                     &code);
        if (!result) {
            SkDebugf("%s\n", gpu->shaderCompiler()->errorText().c_str());
            return false;
        }
        moduleCreateInfo.codeSize = code.size();
        moduleCreateInfo.pCode = (const uint32_t*)code.c_str();
#else
        shaderc_compiler_t compiler = gpu->shadercCompiler();

        shaderc_compile_options_t options = shaderc_compile_options_initialize();

        shaderc_shader_kind shadercStage = vk_shader_stage_to_shaderc_kind(stage);
        result = shaderc_compile_into_spv(compiler,
                                          shaderString,
                                          strlen(shaderString),
                                          shadercStage,
                                          "shader",
                                          "main",
                                          options);
        shaderc_compile_options_release(options);
#ifdef SK_DEBUG
        if (shaderc_result_get_num_errors(result)) {
            SkDebugf("%s\n", shaderString);
            SkDebugf("%s\n", shaderc_result_get_error_message(result));
            return false;
        }
#endif // SK_DEBUG

        moduleCreateInfo.codeSize = shaderc_result_get_length(result);
        moduleCreateInfo.pCode = (const uint32_t*)shaderc_result_get_bytes(result);
#endif // USE_SKSL
    }

    VkResult err = GR_VK_CALL(gpu->vkInterface(), CreateShaderModule(gpu->device(),
                                                                     &moduleCreateInfo,
                                                                     nullptr,
                                                                     shaderModule));

    if (!gpu->vkCaps().canUseGLSLForShaderModule()) {
#if !USE_SKSL
        shaderc_result_release(result);
#endif
    }
    if (err) {
        return false;
    }

    memset(stageInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    stageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo->pNext = nullptr;
    stageInfo->flags = 0;
    stageInfo->stage = stage;
    stageInfo->module = *shaderModule;
    stageInfo->pName = "main";
    stageInfo->pSpecializationInfo = nullptr;

    return true;
}
