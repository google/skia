/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "vk/GrVkPipelineStateBuilder.h"

#include "vk/GrVkGpu.h"
#include "vk/GrVkRenderPass.h"

GrVkPipelineState* GrVkPipelineStateBuilder::CreatePipelineState(
                                                               GrVkGpu* gpu,
                                                               const GrPipeline& pipeline,
                                                               const GrPrimitiveProcessor& primProc,
                                                               GrPrimitiveType primitiveType,
                                                               const GrVkPipelineState::Desc& desc,
                                                               const GrVkRenderPass& renderPass) {
    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrVkPipelineStateBuilder builder(gpu, pipeline, primProc, desc.fProgramDesc);

    GrGLSLExpr4 inputColor;
    GrGLSLExpr4 inputCoverage;

    if (!builder.emitAndInstallProcs(&inputColor, &inputCoverage)) {
        builder.cleanupFragmentProcessors();
        return nullptr;
    }

    return builder.finalize(primitiveType, renderPass, desc);
}

GrVkPipelineStateBuilder::GrVkPipelineStateBuilder(GrVkGpu* gpu,
                                                   const GrPipeline& pipeline,
                                                   const GrPrimitiveProcessor& primProc,
                                                   const GrVkProgramDesc& desc)
    : INHERITED(pipeline, primProc, desc)
    , fGpu(gpu)
    , fVaryingHandler(this)
    , fUniformHandler(this) {
}

const GrCaps* GrVkPipelineStateBuilder::caps() const {
    return fGpu->caps();
}
const GrGLSLCaps* GrVkPipelineStateBuilder::glslCaps() const {
    return fGpu->vkCaps().glslCaps();
}

void GrVkPipelineStateBuilder::finalizeFragmentOutputColor(GrGLSLShaderVar& outputColor) {
    outputColor.setLayoutQualifier("location = 0, index = 0");
}

void GrVkPipelineStateBuilder::finalizeFragmentSecondaryColor(GrGLSLShaderVar& outputColor) {
    outputColor.setLayoutQualifier("location = 0, index = 1");
}

VkShaderStageFlags visibility_to_vk_stage_flags(uint32_t visibility) {
    VkShaderStageFlags flags = 0;

    if (visibility & kVertex_GrShaderFlag) {
        flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (visibility & kGeometry_GrShaderFlag) {
        flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    }
    if (visibility & kFragment_GrShaderFlag) {
        flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    return flags;
}

shaderc_shader_kind vk_shader_stage_to_shaderc_kind(VkShaderStageFlagBits stage) {
    if (VK_SHADER_STAGE_VERTEX_BIT == stage) {
        return shaderc_glsl_vertex_shader;
    }
    SkASSERT(VK_SHADER_STAGE_FRAGMENT_BIT == stage);
    return shaderc_glsl_fragment_shader;
}

bool GrVkPipelineStateBuilder::CreateVkShaderModule(const GrVkGpu* gpu,
                                                    VkShaderStageFlagBits stage,
                                                    const GrGLSLShaderBuilder& builder,
                                                    VkShaderModule* shaderModule,
                                                    VkPipelineShaderStageCreateInfo* stageInfo) {
    SkString shaderString;
    for (int i = 0; i < builder.fCompilerStrings.count(); ++i) {
        if (builder.fCompilerStrings[i]) {
            shaderString.append(builder.fCompilerStrings[i]);
            shaderString.append("\n");
        }
    }

    VkShaderModuleCreateInfo moduleCreateInfo;
    memset(&moduleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.flags = 0;

    shaderc_compilation_result_t result = nullptr;

    if (gpu->vkCaps().canUseGLSLForShaderModule()) {
        moduleCreateInfo.codeSize = strlen(shaderString.c_str());
        moduleCreateInfo.pCode = (const uint32_t*)shaderString.c_str();
    } else {

        shaderc_compiler_t compiler = gpu->shadercCompiler();

        shaderc_compile_options_t options = shaderc_compile_options_initialize();

        shaderc_shader_kind shadercStage = vk_shader_stage_to_shaderc_kind(stage);
        result = shaderc_compile_into_spv(compiler,
                                          shaderString.c_str(),
                                          strlen(shaderString.c_str()),
                                          shadercStage,
                                          "shader",
                                          "main",
                                          options);
        shaderc_compile_options_release(options);
#ifdef SK_DEBUG
        if (shaderc_result_get_num_errors(result)) {
            SkDebugf("%s\n", shaderString.c_str());
            SkDebugf("%s\n", shaderc_result_get_error_message(result));
            return false;
        }
#endif

        moduleCreateInfo.codeSize = shaderc_result_get_length(result);
        moduleCreateInfo.pCode = (const uint32_t*)shaderc_result_get_bytes(result);
    }

    VkResult err = GR_VK_CALL(gpu->vkInterface(), CreateShaderModule(gpu->device(),
                                                                     &moduleCreateInfo,
                                                                     nullptr,
                                                                     shaderModule));
    if (!gpu->vkCaps().canUseGLSLForShaderModule()) {
        shaderc_result_release(result);
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

GrVkPipelineState* GrVkPipelineStateBuilder::finalize(GrPrimitiveType primitiveType,
                                                      const GrVkRenderPass& renderPass,
                                                      const GrVkPipelineState::Desc& desc) {
    VkDescriptorSetLayout dsLayout[2];
    VkPipelineLayout pipelineLayout;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

    uint32_t numSamplers = (uint32_t)fUniformHandler.numSamplers();

    SkAutoTDeleteArray<VkDescriptorSetLayoutBinding> dsSamplerBindings(
                                                     new VkDescriptorSetLayoutBinding[numSamplers]);
    for (uint32_t i = 0; i < numSamplers; ++i) {
        const GrVkGLSLSampler& sampler =
            static_cast<const GrVkGLSLSampler&>(fUniformHandler.getSampler(i));
        SkASSERT(sampler.binding() == i);
        dsSamplerBindings[i].binding = sampler.binding();
        dsSamplerBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        dsSamplerBindings[i].descriptorCount = 1;
        dsSamplerBindings[i].stageFlags = visibility_to_vk_stage_flags(sampler.visibility());
        dsSamplerBindings[i].pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo dsSamplerLayoutCreateInfo;
    memset(&dsSamplerLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    dsSamplerLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dsSamplerLayoutCreateInfo.pNext = nullptr;
    dsSamplerLayoutCreateInfo.flags = 0;
    dsSamplerLayoutCreateInfo.bindingCount = numSamplers;
    // Setting to nullptr fixes an error in the param checker validation layer. Even though
    // bindingCount is 0 (which is valid), it still tries to validate pBindings unless it is null.
    dsSamplerLayoutCreateInfo.pBindings = numSamplers ? dsSamplerBindings.get() : nullptr;

    GR_VK_CALL_ERRCHECK(fGpu->vkInterface(),
                        CreateDescriptorSetLayout(fGpu->device(),
                                                  &dsSamplerLayoutCreateInfo,
                                                  nullptr,
                                                  &dsLayout[GrVkUniformHandler::kSamplerDescSet]));

    // This layout is not owned by the PipelineStateBuilder and thus should no be destroyed
    dsLayout[GrVkUniformHandler::kUniformBufferDescSet] = fGpu->resourceProvider().getUniDSLayout();

    // Create the VkPipelineLayout
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateFlags));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = 0;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.setLayoutCount = 2;
    layoutCreateInfo.pSetLayouts = dsLayout;
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

    GR_VK_CALL_ERRCHECK(fGpu->vkInterface(), CreatePipelineLayout(fGpu->device(),
                                                                  &layoutCreateInfo,
                                                                  nullptr,
                                                                  &pipelineLayout));

    // We need to enable the following extensions so that the compiler can correctly make spir-v
    // from our glsl shaders.
    fVS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fVS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");

    this->finalizeShaders();

    VkPipelineShaderStageCreateInfo shaderStageInfo[2];
    SkAssertResult(CreateVkShaderModule(fGpu,
                                        VK_SHADER_STAGE_VERTEX_BIT,
                                        fVS,
                                        &vertShaderModule,
                                        &shaderStageInfo[0]));

    SkAssertResult(CreateVkShaderModule(fGpu,
                                        VK_SHADER_STAGE_FRAGMENT_BIT,
                                        fFS,
                                        &fragShaderModule,
                                        &shaderStageInfo[1]));

    GrVkResourceProvider& resourceProvider = fGpu->resourceProvider();
    GrVkPipeline* pipeline = resourceProvider.createPipeline(fPipeline,
                                                             fPrimProc,
                                                             shaderStageInfo,
                                                             2,
                                                             primitiveType,
                                                             renderPass,
                                                             pipelineLayout);
    GR_VK_CALL(fGpu->vkInterface(), DestroyShaderModule(fGpu->device(), vertShaderModule,
                                                        nullptr));
    GR_VK_CALL(fGpu->vkInterface(), DestroyShaderModule(fGpu->device(), fragShaderModule,
                                                        nullptr));

    if (!pipeline) {
        GR_VK_CALL(fGpu->vkInterface(), DestroyPipelineLayout(fGpu->device(), pipelineLayout,
                                                              nullptr));
        GR_VK_CALL(fGpu->vkInterface(),
                   DestroyDescriptorSetLayout(fGpu->device(),
                                              dsLayout[GrVkUniformHandler::kSamplerDescSet],
                                              nullptr));

        this->cleanupFragmentProcessors();
        return nullptr;
    }

    return new GrVkPipelineState(fGpu,
                                 desc,
                                 pipeline,
                                 pipelineLayout,
                                 dsLayout[GrVkUniformHandler::kSamplerDescSet],
                                 fUniformHandles,
                                 fUniformHandler.fUniforms,
                                 fUniformHandler.fCurrentVertexUBOOffset,
                                 fUniformHandler.fCurrentFragmentUBOOffset,
                                 numSamplers,
                                 fGeometryProcessor,
                                 fXferProcessor,
                                 fFragmentProcessors);
}
