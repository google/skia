/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "vk/GrVkProgramBuilder.h"

#include "vk/GrVkGpu.h"
#include "vk/GrVkRenderPass.h"
#include "vk/GrVkProgram.h"

GrVkProgram* GrVkProgramBuilder::CreateProgram(GrVkGpu* gpu,
                                               const DrawArgs& args,
                                               GrPrimitiveType primitiveType,
                                               const GrVkRenderPass& renderPass) {
    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrVkProgramBuilder builder(gpu, args);

    GrGLSLExpr4 inputColor;
    GrGLSLExpr4 inputCoverage;

    if (!builder.emitAndInstallProcs(&inputColor,
                                     &inputCoverage,
                                     gpu->vkCaps().maxSampledTextures())) {
        builder.cleanupFragmentProcessors();
        return nullptr;
    }

    return builder.finalize(args, primitiveType, renderPass);
}

GrVkProgramBuilder::GrVkProgramBuilder(GrVkGpu* gpu, const DrawArgs& args)
    : INHERITED(args) 
    , fGpu(gpu)
    , fVaryingHandler(this) 
    , fUniformHandler(this) {
}

const GrCaps* GrVkProgramBuilder::caps() const {
    return fGpu->caps();
}
const GrGLSLCaps* GrVkProgramBuilder::glslCaps() const {
    return fGpu->vkCaps().glslCaps();
}

void GrVkProgramBuilder::finalizeFragmentOutputColor(GrGLSLShaderVar& outputColor) {
    outputColor.setLayoutQualifier("location = 0");
}

void GrVkProgramBuilder::emitSamplers(const GrProcessor& processor,
                                      GrGLSLTextureSampler::TextureSamplerArray* outSamplers) {
    int numTextures = processor.numTextures();
    UniformHandle* localSamplerUniforms = fSamplerUniforms.push_back_n(numTextures);
    SkString name;
    for (int t = 0; t < numTextures; ++t) {
        name.printf("%d", t);
        localSamplerUniforms[t]  =
            fUniformHandler.addUniform(kFragment_GrShaderFlag,
                                       kSampler2D_GrSLType, kDefault_GrSLPrecision,
                                       name.c_str());
        outSamplers->emplace_back(localSamplerUniforms[t], processor.textureAccess(t));
    }
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

bool GrVkProgramBuilder::CreateVkShaderModule(const GrVkGpu* gpu,
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

    shaderc_compiler_t compiler = gpu->shadercCompiler();

    shaderc_compile_options_t options = shaderc_compile_options_initialize();
    shaderc_compile_options_set_forced_version_profile(options, 140, shaderc_profile_none);

    shaderc_shader_kind shadercStage = vk_shader_stage_to_shaderc_kind(stage);
    shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler,
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

    VkShaderModuleCreateInfo moduleCreateInfo;
    memset(&moduleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = nullptr;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = shaderc_result_get_length(result);
    moduleCreateInfo.pCode = (const uint32_t*)shaderc_result_get_bytes(result);

    VkResult err = GR_VK_CALL(gpu->vkInterface(), CreateShaderModule(gpu->device(),
                                                                     &moduleCreateInfo,
                                                                     nullptr,
                                                                     shaderModule));
    shaderc_result_release(result);
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

GrVkProgram* GrVkProgramBuilder::finalize(const DrawArgs& args,
                                          GrPrimitiveType primitiveType,
                                          const GrVkRenderPass& renderPass) {
    VkDescriptorSetLayout dsLayout[2];
    VkPipelineLayout pipelineLayout;
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

    uint32_t numSamplers = fSamplerUniforms.count();

    SkAutoTDeleteArray<VkDescriptorSetLayoutBinding> dsSamplerBindings(
                                                     new VkDescriptorSetLayoutBinding[numSamplers]);
    for (uint32_t i = 0; i < numSamplers; ++i) {
        UniformHandle uniHandle = fSamplerUniforms[i];
        GrVkUniformHandler::UniformInfo uniformInfo = fUniformHandler.getUniformInfo(uniHandle);
        SkASSERT(kSampler2D_GrSLType == uniformInfo.fVariable.getType());
        SkASSERT(0 == uniformInfo.fSetNumber);
        SkASSERT(uniformInfo.fBinding == i);
        dsSamplerBindings[i].binding = uniformInfo.fBinding;
        dsSamplerBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        dsSamplerBindings[i].descriptorCount = 1;
        dsSamplerBindings[i].stageFlags = visibility_to_vk_stage_flags(uniformInfo.fVisibility);
        dsSamplerBindings[i].pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo dsSamplerLayoutCreateInfo;
    memset(&dsSamplerLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    dsSamplerLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dsSamplerLayoutCreateInfo.pNext = nullptr;
    dsSamplerLayoutCreateInfo.flags = 0;
    dsSamplerLayoutCreateInfo.bindingCount = fSamplerUniforms.count();
    // Setting to nullptr fixes an error in the param checker validation layer. Even though
    // bindingCount is 0 (which is valid), it still tries to validate pBindings unless it is null.
    dsSamplerLayoutCreateInfo.pBindings = fSamplerUniforms.count() ? dsSamplerBindings.get() :
                                                                     nullptr;

    GR_VK_CALL_ERRCHECK(fGpu->vkInterface(),
                        CreateDescriptorSetLayout(fGpu->device(),
                                                  &dsSamplerLayoutCreateInfo,
                                                  nullptr,
                                                  &dsLayout[GrVkUniformHandler::kSamplerDescSet]));

    // Create Uniform Buffer Descriptor
    // We always attach uniform buffers to descriptor set 1. The vertex uniform buffer will have
    // binding 0 and the fragment binding 1.
    VkDescriptorSetLayoutBinding dsUniBindings[2];
    memset(&dsUniBindings, 0, 2 * sizeof(VkDescriptorSetLayoutBinding));
    dsUniBindings[0].binding = GrVkUniformHandler::kVertexBinding;
    dsUniBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsUniBindings[0].descriptorCount = fUniformHandler.hasVertexUniforms() ? 1 : 0;
    dsUniBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    dsUniBindings[0].pImmutableSamplers = nullptr;
    dsUniBindings[1].binding = GrVkUniformHandler::kFragBinding;
    dsUniBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    dsUniBindings[1].descriptorCount = fUniformHandler.hasFragmentUniforms() ? 1 : 0;
    dsUniBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    dsUniBindings[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo dsUniformLayoutCreateInfo;
    memset(&dsUniformLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    dsUniformLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dsUniformLayoutCreateInfo.pNext = nullptr;
    dsUniformLayoutCreateInfo.flags = 0;
    dsUniformLayoutCreateInfo.bindingCount = 2;
    dsUniformLayoutCreateInfo.pBindings = dsUniBindings;

    GR_VK_CALL_ERRCHECK(fGpu->vkInterface(), CreateDescriptorSetLayout(
                                             fGpu->device(),
                                             &dsUniformLayoutCreateInfo,
                                             nullptr,
                                             &dsLayout[GrVkUniformHandler::kUniformBufferDescSet]));

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
    GrVkPipeline* pipeline = resourceProvider.createPipeline(*args.fPipeline,
                                                             *args.fPrimitiveProcessor,
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
        GR_VK_CALL(fGpu->vkInterface(), DestroyDescriptorSetLayout(fGpu->device(), dsLayout[0],
                                                                   nullptr));
        GR_VK_CALL(fGpu->vkInterface(), DestroyDescriptorSetLayout(fGpu->device(), dsLayout[1],
                                                                   nullptr));
        return nullptr;
    }


    GrVkDescriptorPool::DescriptorTypeCounts typeCounts;
    typeCounts.setTypeCount(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
    SkASSERT(numSamplers < 256);
    typeCounts.setTypeCount(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint8_t)numSamplers);
    GrVkDescriptorPool* descriptorPool =
        fGpu->resourceProvider().findOrCreateCompatibleDescriptorPool(typeCounts);

    VkDescriptorSetAllocateInfo dsAllocateInfo;
    memset(&dsAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    dsAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAllocateInfo.pNext = nullptr;
    dsAllocateInfo.descriptorPool = descriptorPool->descPool();
    dsAllocateInfo.descriptorSetCount = 2;
    dsAllocateInfo.pSetLayouts = dsLayout;

    VkDescriptorSet descriptorSets[2];
    GR_VK_CALL_ERRCHECK(fGpu->vkInterface(), AllocateDescriptorSets(fGpu->device(),
                                                                    &dsAllocateInfo,
                                                                    descriptorSets));

    return new GrVkProgram(fGpu,
                           pipeline,
                           pipelineLayout,
                           dsLayout,
                           descriptorPool,
                           descriptorSets,
                           fUniformHandles,
                           fUniformHandler.fUniforms,
                           fUniformHandler.fCurrentVertexUBOOffset,
                           fUniformHandler.fCurrentFragmentUBOOffset,
                           numSamplers,
                           fGeometryProcessor,
                           fXferProcessor,
                           fFragmentProcessors);
}
