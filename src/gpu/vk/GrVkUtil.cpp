/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrVkUtil.h"

#include "vk/GrVkGpu.h"
#include "SkSLCompiler.h"

bool GrPixelConfigToVkFormat(GrPixelConfig config, VkFormat* format) {
    VkFormat dontCare;
    if (!format) {
        format = &dontCare;
    }

    switch (config) {
        case kUnknown_GrPixelConfig:
            return false;
        case kRGBA_8888_GrPixelConfig:
            *format = VK_FORMAT_R8G8B8A8_UNORM;
            return true;
        case kBGRA_8888_GrPixelConfig:
            *format = VK_FORMAT_B8G8R8A8_UNORM;
            return true;
        case kSRGBA_8888_GrPixelConfig:
            *format = VK_FORMAT_R8G8B8A8_SRGB;
            return true;
        case kSBGRA_8888_GrPixelConfig:
            *format = VK_FORMAT_B8G8R8A8_SRGB;
            return true;
        case kRGBA_8888_sint_GrPixelConfig:
            *format = VK_FORMAT_R8G8B8A8_SINT;
            return true;
        case kRGB_565_GrPixelConfig:
            *format = VK_FORMAT_R5G6B5_UNORM_PACK16;
            return true;
        case kRGBA_4444_GrPixelConfig:
            // R4G4B4A4 is not required to be supported so we actually
            // store the data is if it was B4G4R4A4 and swizzle in shaders
            *format = VK_FORMAT_B4G4R4A4_UNORM_PACK16;
            return true;
        case kAlpha_8_GrPixelConfig: // fall through
        case kAlpha_8_as_Red_GrPixelConfig:
            *format = VK_FORMAT_R8_UNORM;
            return true;
        case kAlpha_8_as_Alpha_GrPixelConfig:
            return false;
        case kGray_8_GrPixelConfig:
        case kGray_8_as_Red_GrPixelConfig:
            *format = VK_FORMAT_R8_UNORM;
            return true;
        case kGray_8_as_Lum_GrPixelConfig:
            return false;
        case kRGBA_float_GrPixelConfig:
            *format = VK_FORMAT_R32G32B32A32_SFLOAT;
            return true;
        case kRG_float_GrPixelConfig:
            *format = VK_FORMAT_R32G32_SFLOAT;
            return true;
        case kRGBA_half_GrPixelConfig:
            *format = VK_FORMAT_R16G16B16A16_SFLOAT;
            return true;
        case kAlpha_half_GrPixelConfig: // fall through
        case kAlpha_half_as_Red_GrPixelConfig:
            *format = VK_FORMAT_R16_SFLOAT;
            return true;
    }
    SK_ABORT("Unexpected config");
    return false;
}

GrPixelConfig GrVkFormatToPixelConfig(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return kRGBA_8888_GrPixelConfig;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return kBGRA_8888_GrPixelConfig;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return kSRGBA_8888_GrPixelConfig;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return kSBGRA_8888_GrPixelConfig;
        case VK_FORMAT_R8G8B8A8_SINT:
            return kRGBA_8888_sint_GrPixelConfig;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            return kRGB_565_GrPixelConfig;
            break;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            // R4G4B4A4 is not required to be supported so we actually
            // store RGBA_4444 data as B4G4R4A4.
            return kRGBA_4444_GrPixelConfig;
        case VK_FORMAT_R8_UNORM:
            return kAlpha_8_GrPixelConfig;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return kRGBA_float_GrPixelConfig;
        case VK_FORMAT_R32G32_SFLOAT:
            return kRG_float_GrPixelConfig;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return kRGBA_half_GrPixelConfig;
        case VK_FORMAT_R16_SFLOAT:
            return kAlpha_half_GrPixelConfig;
        default:
            return kUnknown_GrPixelConfig;
    }
}

bool GrVkFormatPixelConfigPairIsValid(VkFormat format, GrPixelConfig config) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return kRGBA_8888_GrPixelConfig == config;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return kBGRA_8888_GrPixelConfig == config;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return kSRGBA_8888_GrPixelConfig == config;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return kSBGRA_8888_GrPixelConfig == config;
        case VK_FORMAT_R8G8B8A8_SINT:
            return kRGBA_8888_sint_GrPixelConfig == config;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            return kRGB_565_GrPixelConfig == config;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            // R4G4B4A4 is not required to be supported so we actually
            // store RGBA_4444 data as B4G4R4A4.
            return kRGBA_4444_GrPixelConfig == config;
        case VK_FORMAT_R8_UNORM:
            return kAlpha_8_GrPixelConfig == config ||
                   kAlpha_8_as_Red_GrPixelConfig == config ||
                   kGray_8_GrPixelConfig == config ||
                   kGray_8_as_Red_GrPixelConfig == config;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return kRGBA_float_GrPixelConfig == config;
        case VK_FORMAT_R32G32_SFLOAT:
            return kRG_float_GrPixelConfig == config;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return kRGBA_half_GrPixelConfig == config;
        case VK_FORMAT_R16_SFLOAT:
            return kAlpha_half_GrPixelConfig == config ||
                   kAlpha_half_as_Red_GrPixelConfig == config;
        default:
            return false;
    }
}

bool GrVkFormatIsSupported(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R16_SFLOAT:
            return true;
        default:
            return false;
    }
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
    SkASSERT(samples >= 1);
    switch (samples) {
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

SkSL::Program::Kind vk_shader_stage_to_skiasl_kind(VkShaderStageFlagBits stage) {
    if (VK_SHADER_STAGE_VERTEX_BIT == stage) {
        return SkSL::Program::kVertex_Kind;
    }
    if (VK_SHADER_STAGE_GEOMETRY_BIT == stage) {
        return SkSL::Program::kGeometry_Kind;
    }
    SkASSERT(VK_SHADER_STAGE_FRAGMENT_BIT == stage);
    return SkSL::Program::kFragment_Kind;
}

VkShaderStageFlagBits skiasl_kind_to_vk_shader_stage(SkSL::Program::Kind kind) {
    if (SkSL::Program::kVertex_Kind == kind) {
        return VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (SkSL::Program::kGeometry_Kind == kind) {
        return VK_SHADER_STAGE_GEOMETRY_BIT;
    }
    SkASSERT(SkSL::Program::kFragment_Kind == kind);
    return VK_SHADER_STAGE_FRAGMENT_BIT;
}

bool GrCompileVkShaderModule(const GrVkGpu* gpu,
                             const char* shaderString,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo,
                             const SkSL::Program::Settings& settings,
                             SkSL::Program::Inputs* outInputs) {
    std::unique_ptr<SkSL::Program> program = gpu->shaderCompiler()->convertProgram(
                                                              vk_shader_stage_to_skiasl_kind(stage),
                                                              SkSL::String(shaderString),
                                                              settings);
    if (!program) {
        SkDebugf("SkSL error:\n%s\n", gpu->shaderCompiler()->errorText().c_str());
        SkASSERT(false);
    }
    *outInputs = program->fInputs;
    SkSL::String code;
    if (!gpu->shaderCompiler()->toSPIRV(*program, &code)) {
        SkDebugf("%s\n", gpu->shaderCompiler()->errorText().c_str());
        return false;
    }

    VkShaderModuleCreateInfo moduleCreateInfo;
    memset(&moduleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = code.size();
    moduleCreateInfo.pCode = (const uint32_t*)code.c_str();

    VkResult err = GR_VK_CALL(gpu->vkInterface(), CreateShaderModule(gpu->device(),
                                                                     &moduleCreateInfo,
                                                                     nullptr,
                                                                     shaderModule));
    if (err) {
        return false;
    }

    memset(stageInfo, 0, sizeof(VkPipelineShaderStageCreateInfo));
    stageInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo->pNext = nullptr;
    stageInfo->flags = 0;
    stageInfo->stage = skiasl_kind_to_vk_shader_stage(program->fKind);
    stageInfo->module = *shaderModule;
    stageInfo->pName = "main";
    stageInfo->pSpecializationInfo = nullptr;

    return true;
}
