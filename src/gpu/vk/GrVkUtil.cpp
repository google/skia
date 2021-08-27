/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkUtil.h"

#include "include/gpu/GrDirectContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrDataUtils.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/sksl/SkSLCompiler.h"

bool GrVkFormatIsSupported(VkFormat format) {
    switch (format) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
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
        default:
            return false;
    }
}

SkSL::ProgramKind vk_shader_stage_to_skiasl_kind(VkShaderStageFlagBits stage) {
    if (VK_SHADER_STAGE_VERTEX_BIT == stage) {
        return SkSL::ProgramKind::kVertex;
    }
    SkASSERT(VK_SHADER_STAGE_FRAGMENT_BIT == stage);
    return SkSL::ProgramKind::kFragment;
}

bool GrCompileVkShaderModule(GrVkGpu* gpu,
                             const SkSL::String& shaderString,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo,
                             const SkSL::Program::Settings& settings,
                             SkSL::String* outSPIRV,
                             SkSL::Program::Inputs* outInputs) {
    TRACE_EVENT0("skia.shaders", "CompileVkShaderModule");
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

bool GrInstallVkShaderModule(GrVkGpu* gpu,
                             const SkSL::String& spirv,
                             VkShaderStageFlagBits stage,
                             VkShaderModule* shaderModule,
                             VkPipelineShaderStageCreateInfo* stageInfo) {
    TRACE_EVENT0("skia.shaders", "InstallVkShaderModule");
    VkShaderModuleCreateInfo moduleCreateInfo;
    memset(&moduleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = spirv.size();
    moduleCreateInfo.pCode = (const uint32_t*)spirv.c_str();

    VkResult err;
    GR_VK_CALL_RESULT(gpu, err, CreateShaderModule(gpu->device(), &moduleCreateInfo, nullptr,
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

bool GrVkFormatIsCompressed(VkFormat vkFormat) {
    switch (vkFormat) {
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            return true;
        default:
            return false;
    }
    SkUNREACHABLE;
}
