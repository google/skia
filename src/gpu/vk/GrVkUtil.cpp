/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkUtil.h"

#include "src/gpu/GrContextPriv.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/sksl/SkSLCompiler.h"

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
        case kRGB_888_GrPixelConfig:
            *format = VK_FORMAT_R8G8B8_UNORM;
            return true;
        case kRGB_888X_GrPixelConfig:
            *format = VK_FORMAT_R8G8B8A8_UNORM;
            return true;
        case kRG_88_GrPixelConfig:
            *format = VK_FORMAT_R8G8_UNORM;
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
        case kRGBA_1010102_GrPixelConfig:
            *format = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
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
        case kRGBA_half_Clamped_GrPixelConfig:
            *format = VK_FORMAT_R16G16B16A16_SFLOAT;
            return true;
        case kRGB_ETC1_GrPixelConfig:
            // converting to ETC2 which is a superset of ETC1
            *format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            return true;
        case kAlpha_half_GrPixelConfig: // fall through
        case kAlpha_half_as_Red_GrPixelConfig:
            *format = VK_FORMAT_R16_SFLOAT;
            return true;
    }
    SK_ABORT("Unexpected config");
    return false;
}

#ifdef SK_DEBUG
bool GrVkFormatPixelConfigPairIsValid(VkFormat format, GrPixelConfig config) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
            return kRGBA_8888_GrPixelConfig == config ||
                   kRGB_888X_GrPixelConfig == config;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return kBGRA_8888_GrPixelConfig == config;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return kSRGBA_8888_GrPixelConfig == config;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return kSBGRA_8888_GrPixelConfig == config;
        case VK_FORMAT_R8G8B8_UNORM:
            return kRGB_888_GrPixelConfig == config;
        case VK_FORMAT_R8G8_UNORM:
            return kRG_88_GrPixelConfig == config;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return kRGBA_1010102_GrPixelConfig == config;
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
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return kRGB_ETC1_GrPixelConfig == config;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return kRGBA_float_GrPixelConfig == config;
        case VK_FORMAT_R32G32_SFLOAT:
            return kRG_float_GrPixelConfig == config;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return kRGBA_half_GrPixelConfig == config ||
                   kRGBA_half_Clamped_GrPixelConfig == config;
        case VK_FORMAT_R16_SFLOAT:
            return kAlpha_half_GrPixelConfig == config ||
                   kAlpha_half_as_Red_GrPixelConfig == config;
        default:
            return false;
    }
}
#endif

bool GrVkFormatIsSupported(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R16_SFLOAT:
            return true;
        default:
            return false;
    }
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

bool GrCompileVkShaderModule(const GrVkGpu* gpu,
                             const SkSL::String& shaderString,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo,
                             const SkSL::Program::Settings& settings,
                             SkSL::String* outSPIRV,
                             SkSL::Program::Inputs* outInputs) {
    auto errorHandler = gpu->getContext()->priv().getShaderErrorHandler();
    std::unique_ptr<SkSL::Program> program = gpu->shaderCompiler()->convertProgram(
            vk_shader_stage_to_skiasl_kind(stage), shaderString, settings);
    if (!program) {
        errorHandler->compileError(shaderString.c_str(),
                                   gpu->shaderCompiler()->errorText().c_str());
        return false;
    }
    *outInputs = program->fInputs;
    if (!gpu->shaderCompiler()->toSPIRV(*program, outSPIRV)) {
        errorHandler->compileError(shaderString.c_str(),
                                   gpu->shaderCompiler()->errorText().c_str());
        return false;
    }

    return GrInstallVkShaderModule(gpu, *outSPIRV, stage, shaderModule, stageInfo);
}

bool GrInstallVkShaderModule(const GrVkGpu* gpu,
                             const SkSL::String& spirv,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo) {
    VkShaderModuleCreateInfo moduleCreateInfo;
    memset(&moduleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = spirv.size();
    moduleCreateInfo.pCode = (const uint32_t*)spirv.c_str();

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
    stageInfo->stage = stage;
    stageInfo->module = *shaderModule;
    stageInfo->pName = "main";
    stageInfo->pSpecializationInfo = nullptr;

    return true;
}

size_t GrVkBytesPerFormat(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_R8_UNORM:
            return 1;

        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R16_SFLOAT:
            return 2;

        case VK_FORMAT_R8G8B8_UNORM:
            return 3;

        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return 4;

        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32G32_SFLOAT:
            return 8;

        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return 16;

        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return 8;

        default:
            SK_ABORT("Invalid Vk format");
            return 0;
    }

    SK_ABORT("Invalid Vk format");
    return 0;
}

bool GrVkFormatIsCompressed(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return true;
        default:
            return false;
    }
    SK_ABORT("Invalid format");
    return false;
}

size_t GrVkFormatCompressedDataSize(VkFormat format, int width, int height) {
    SkASSERT(GrVkFormatIsCompressed(format));

    switch (format) {
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            if (width < 4) {
                SkASSERT(width == 1 || width == 2);
                width = 4;
            }
            if (height < 4) {
                SkASSERT(height == 1 || height == 2);
                height = 4;
            }
            SkASSERT((width & 3) == 0);
            SkASSERT((height & 3) == 0);
            return (width >> 2) * (height >> 2) * 8;
        default:
            SK_ABORT("Unknown compressed format");
            return 4 * width * height;
    }

    SK_ABORT("Invalid format");
    return 4 * width * height;
}

