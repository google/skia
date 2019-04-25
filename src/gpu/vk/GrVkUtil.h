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
#include "include/private/GrColor.h"
#include "include/private/SkMacros.h"
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

/**
 * Returns the vulkan texture format for the given GrPixelConfig
 */
bool GrPixelConfigToVkFormat(GrPixelConfig config, VkFormat* format);

bool GrVkFormatIsSupported(VkFormat);

#ifdef SK_DEBUG
/**
 * Returns true if the passed in VkFormat and GrPixelConfig are compatible with each other.
 */
bool GrVkFormatPixelConfigPairIsValid(VkFormat, GrPixelConfig);
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

#endif
