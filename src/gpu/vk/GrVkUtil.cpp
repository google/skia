/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkUtil.h"

#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/sksl/SkSLCompiler.h"

#ifdef SK_DEBUG
bool GrVkFormatColorTypePairIsValid(VkFormat format, GrColorType colorType) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:            return GrColorType::kRGBA_8888 == colorType ||
                                                         GrColorType::kRGB_888x == colorType;
        case VK_FORMAT_B8G8R8A8_UNORM:            return GrColorType::kBGRA_8888 == colorType;
        case VK_FORMAT_R8G8B8A8_SRGB:             return GrColorType::kRGBA_8888_SRGB == colorType;
        case VK_FORMAT_R8G8B8_UNORM:              return GrColorType::kRGB_888x == colorType;
        case VK_FORMAT_R8G8_UNORM:                return GrColorType::kRG_88 == colorType;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:  return GrColorType::kRGBA_1010102 == colorType;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:       return GrColorType::kBGR_565 == colorType;
        // R4G4B4A4 is not required to be supported so we actually
        // store RGBA_4444 data as B4G4R4A4.
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:     return GrColorType::kABGR_4444 == colorType;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:     return GrColorType::kABGR_4444 == colorType;
        case VK_FORMAT_R8_UNORM:                 return GrColorType::kAlpha_8 == colorType ||
                                                         GrColorType::kGray_8 == colorType;
        case VK_FORMAT_R32G32B32A32_SFLOAT:       return GrColorType::kRGBA_F32 == colorType;
        case VK_FORMAT_R16G16B16A16_SFLOAT:       return GrColorType::kRGBA_F16 == colorType ||
                                                         GrColorType::kRGBA_F16_Clamped == colorType;
        case VK_FORMAT_R16_SFLOAT:                return GrColorType::kAlpha_F16 == colorType;
        case VK_FORMAT_R16_UNORM:                 return GrColorType::kAlpha_16 == colorType;
        case VK_FORMAT_R16G16_UNORM:              return GrColorType::kRG_1616 == colorType;
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM: return GrColorType::kRGB_888x == colorType;
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:  return GrColorType::kRGB_888x == colorType;
        // Experimental (for Y416 and mutant P016/P010)
        case VK_FORMAT_R16G16B16A16_UNORM:        return GrColorType::kRGBA_16161616 == colorType;
        case VK_FORMAT_R16G16_SFLOAT:             return GrColorType::kRG_F16 == colorType;
        default:                                  return false;
    }

    SkUNREACHABLE;
}
#endif

bool GrVkFormatIsSupported(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        // Experimental (for Y416 and mutant P016/P010)
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16_SFLOAT:
            return true;
        default:
            return false;
    }
}

bool GrVkFormatNeedsYcbcrSampler(VkFormat format) {
    return format == VK_FORMAT_G8_B8R8_2PLANE_420_UNORM ||
           format == VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
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
        case VK_FORMAT_R16_UNORM:
            return 2;

        case VK_FORMAT_R8G8B8_UNORM:
            return 3;

        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_R16G16_UNORM:
            return 4;

        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return 8;

        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return 16;

        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return 0;

        // Experimental (for Y416 and mutant P016/P010)
        case VK_FORMAT_R16G16B16A16_UNORM:
            return 8;
        case VK_FORMAT_R16G16_SFLOAT:
            return 4;

        default:
            SK_ABORT("Invalid Vk format");
    }

    SK_ABORT("Invalid Vk format");
}

bool GrVkFormatIsCompressed(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return true;
        default:
            return false;
    }
}

bool GrVkFormatToCompressionType(VkFormat vkFormat, SkImage::CompressionType* compressionType) {
    switch (vkFormat) {
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            *compressionType = SkImage::kETC1_CompressionType;
            return true;
        default:
            return false;
    }
}
