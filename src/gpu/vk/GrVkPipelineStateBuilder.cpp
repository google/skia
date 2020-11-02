/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkPipelineStateBuilder.h"

#include "include/gpu/GrDirectContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/GrAutoLocaleSetter.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrPersistentCacheUtils.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrShaderUtils.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/vk/GrVkDescriptorSetManager.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkRenderPass.h"
#include "src/gpu/vk/GrVkRenderTarget.h"

GrVkPipelineState* GrVkPipelineStateBuilder::CreatePipelineState(
        GrVkGpu* gpu,
        GrRenderTarget* renderTarget,
        const GrProgramDesc& desc,
        const GrProgramInfo& programInfo,
        VkRenderPass compatibleRenderPass) {

    gpu->stats()->incShaderCompilations();

    // ensure that we use "." as a decimal separator when creating SkSL code
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrVkPipelineStateBuilder builder(gpu, renderTarget, desc, programInfo);

    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }

    return builder.finalize(desc, compatibleRenderPass);
}

GrVkPipelineStateBuilder::GrVkPipelineStateBuilder(GrVkGpu* gpu,
                                                   GrRenderTarget* renderTarget,
                                                   const GrProgramDesc& desc,
                                                   const GrProgramInfo& programInfo)
        : INHERITED(renderTarget, desc, programInfo)
        , fGpu(gpu)
        , fVaryingHandler(this)
        , fUniformHandler(this) {}

const GrCaps* GrVkPipelineStateBuilder::caps() const {
    return fGpu->caps();
}

void GrVkPipelineStateBuilder::finalizeFragmentOutputColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 0");
}

void GrVkPipelineStateBuilder::finalizeFragmentSecondaryColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 1");
}

bool GrVkPipelineStateBuilder::createVkShaderModule(VkShaderStageFlagBits stage,
                                                    const SkSL::String& sksl,
                                                    VkShaderModule* shaderModule,
                                                    VkPipelineShaderStageCreateInfo* stageInfo,
                                                    const SkSL::Program::Settings& settings,
                                                    SkSL::String* outSPIRV,
                                                    SkSL::Program::Inputs* outInputs) {
    if (!GrCompileVkShaderModule(fGpu, sksl, stage, shaderModule,
                                 stageInfo, settings, outSPIRV, outInputs)) {
        return false;
    }
    if (outInputs->fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }
    return true;
}

bool GrVkPipelineStateBuilder::installVkShaderModule(VkShaderStageFlagBits stage,
                                                     const GrGLSLShaderBuilder& builder,
                                                     VkShaderModule* shaderModule,
                                                     VkPipelineShaderStageCreateInfo* stageInfo,
                                                     SkSL::String spirv,
                                                     SkSL::Program::Inputs inputs) {
    if (!GrInstallVkShaderModule(fGpu, spirv, stage, shaderModule, stageInfo)) {
        return false;
    }
    if (inputs.fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }
    return true;
}

static constexpr SkFourByteTag kSPIRV_Tag = SkSetFourByteTag('S', 'P', 'R', 'V');
static constexpr SkFourByteTag kSKSL_Tag = SkSetFourByteTag('S', 'K', 'S', 'L');

int GrVkPipelineStateBuilder::loadShadersFromCache(SkReadBuffer* cached,
                                                   VkShaderModule outShaderModules[],
                                                   VkPipelineShaderStageCreateInfo* outStageInfo) {
    SkSL::String shaders[kGrShaderTypeCount];
    SkSL::Program::Inputs inputs[kGrShaderTypeCount];

    if (!GrPersistentCacheUtils::UnpackCachedShaders(cached, shaders, inputs, kGrShaderTypeCount)) {
        return 0;
    }

    bool success = this->installVkShaderModule(VK_SHADER_STAGE_VERTEX_BIT,
                                               fVS,
                                               &outShaderModules[kVertex_GrShaderType],
                                               &outStageInfo[0],
                                               shaders[kVertex_GrShaderType],
                                               inputs[kVertex_GrShaderType]);

    success = success && this->installVkShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT,
                                                     fFS,
                                                     &outShaderModules[kFragment_GrShaderType],
                                                     &outStageInfo[1],
                                                     shaders[kFragment_GrShaderType],
                                                     inputs[kFragment_GrShaderType]);

    if (!shaders[kGeometry_GrShaderType].empty()) {
        success = success && this->installVkShaderModule(VK_SHADER_STAGE_GEOMETRY_BIT,
                                                         fGS,
                                                         &outShaderModules[kGeometry_GrShaderType],
                                                         &outStageInfo[2],
                                                         shaders[kGeometry_GrShaderType],
                                                         inputs[kGeometry_GrShaderType]);
    }

    if (!success) {
        for (int i = 0; i < kGrShaderTypeCount; ++i) {
            if (outShaderModules[i]) {
                GR_VK_CALL(fGpu->vkInterface(),
                           DestroyShaderModule(fGpu->device(), outShaderModules[i], nullptr));
            }
        }
        return 0;
    }
    return shaders[kGeometry_GrShaderType].empty() ? 2 : 3;
}

void GrVkPipelineStateBuilder::storeShadersInCache(const SkSL::String shaders[],
                                                   const SkSL::Program::Inputs inputs[],
                                                   bool isSkSL) {
    // Here we shear off the Vk-specific portion of the Desc in order to create the
    // persistent key. This is bc Vk only caches the SPIRV code, not the fully compiled
    // program, and that only depends on the base GrProgramDesc data.
    // The +4 is to include the kShader_PersistentCacheKeyType code the Vulkan backend adds
    // to the key right after the base key.
    sk_sp<SkData> key = SkData::MakeWithoutCopy(this->desc().asKey(),
                                                this->desc().initialKeyLength()+4);

    sk_sp<SkData> data = GrPersistentCacheUtils::PackCachedShaders(isSkSL ? kSKSL_Tag : kSPIRV_Tag,
                                                                   shaders,
                                                                   inputs, kGrShaderTypeCount);
    this->gpu()->getContext()->priv().getPersistentCache()->store(*key, *data);
}

GrVkPipelineState* GrVkPipelineStateBuilder::finalize(const GrProgramDesc& desc,
                                                      VkRenderPass compatibleRenderPass) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);

    VkDescriptorSetLayout dsLayout[GrVkUniformHandler::kDescSetCount];
    VkPipelineLayout pipelineLayout;
    VkShaderModule shaderModules[kGrShaderTypeCount] = { VK_NULL_HANDLE,
                                                         VK_NULL_HANDLE,
                                                         VK_NULL_HANDLE };

    GrVkResourceProvider& resourceProvider = fGpu->resourceProvider();
    // These layouts are not owned by the PipelineStateBuilder and thus should not be destroyed
    dsLayout[GrVkUniformHandler::kUniformBufferDescSet] = resourceProvider.getUniformDSLayout();

    GrVkDescriptorSetManager::Handle samplerDSHandle;
    resourceProvider.getSamplerDescriptorSetHandle(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                                   fUniformHandler, &samplerDSHandle);
    dsLayout[GrVkUniformHandler::kSamplerDescSet] =
            resourceProvider.getSamplerDSLayout(samplerDSHandle);

    dsLayout[GrVkUniformHandler::kInputDescSet] = resourceProvider.getInputDSLayout();

    bool usesInput = SkToBool(fProgramInfo.renderPassBarriers() & GrXferBarrierFlags::kTexture);
    uint32_t layoutCount =
            usesInput ? GrVkUniformHandler::kDescSetCount : (GrVkUniformHandler::kDescSetCount - 1);
    // Create the VkPipelineLayout
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateFlags));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.setLayoutCount = layoutCount;
    layoutCreateInfo.pSetLayouts = dsLayout;
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

    VkResult result;
    GR_VK_CALL_RESULT(fGpu, result, CreatePipelineLayout(fGpu->device(), &layoutCreateInfo, nullptr,
                                                         &pipelineLayout));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    // We need to enable the following extensions so that the compiler can correctly make spir-v
    // from our glsl shaders.
    fVS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_separate_shader_objects : enable\n");
    fVS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");
    fFS.extensions().appendf("#extension GL_ARB_shading_language_420pack : enable\n");

    this->finalizeShaders();

    VkPipelineShaderStageCreateInfo shaderStageInfo[3];
    SkSL::Program::Settings settings;
    settings.fRTHeightBinding = this->gpu()->vkCaps().getFragmentUniformBinding();
    settings.fRTHeightSet = this->gpu()->vkCaps().getFragmentUniformSet();
    settings.fFlipY = this->origin() != kTopLeft_GrSurfaceOrigin;
    settings.fSharpenTextures =
                        this->gpu()->getContext()->priv().options().fSharpenMipmappedTextures;
    settings.fRTHeightOffset = fUniformHandler.getRTHeightOffset();
    SkASSERT(!this->fragColorIsInOut());

    sk_sp<SkData> cached;
    SkReadBuffer reader;
    SkFourByteTag shaderType = 0;
    auto persistentCache = fGpu->getContext()->priv().getPersistentCache();
    if (persistentCache) {
        // Here we shear off the Vk-specific portion of the Desc in order to create the
        // persistent key. This is bc Vk only caches the SPIRV code, not the fully compiled
        // program, and that only depends on the base GrProgramDesc data.
        // The +4 is to include the kShader_PersistentCacheKeyType code the Vulkan backend adds
        // to the key right after the base key.
        sk_sp<SkData> key = SkData::MakeWithoutCopy(desc.asKey(), desc.initialKeyLength()+4);
        cached = persistentCache->load(*key);
        if (cached) {
            reader.setMemory(cached->data(), cached->size());
            shaderType = GrPersistentCacheUtils::GetType(&reader);
        }
    }

    int numShaderStages = 0;
    if (kSPIRV_Tag == shaderType) {
        numShaderStages = this->loadShadersFromCache(&reader, shaderModules, shaderStageInfo);
    }

    // Proceed from sources if we didn't get a SPIRV cache (or the cache was invalid)
    if (!numShaderStages) {
        numShaderStages = 2; // We always have at least vertex and fragment stages.
        SkSL::String shaders[kGrShaderTypeCount];
        SkSL::Program::Inputs inputs[kGrShaderTypeCount];

        SkSL::String* sksl[kGrShaderTypeCount] = {
            &fVS.fCompilerString,
            &fGS.fCompilerString,
            &fFS.fCompilerString,
        };
        SkSL::String cached_sksl[kGrShaderTypeCount];
        if (kSKSL_Tag == shaderType) {
            if (GrPersistentCacheUtils::UnpackCachedShaders(&reader, cached_sksl, inputs,
                                                            kGrShaderTypeCount)) {
                for (int i = 0; i < kGrShaderTypeCount; ++i) {
                    sksl[i] = &cached_sksl[i];
                }
            }
        }

        bool success = this->createVkShaderModule(VK_SHADER_STAGE_VERTEX_BIT,
                                                  *sksl[kVertex_GrShaderType],
                                                  &shaderModules[kVertex_GrShaderType],
                                                  &shaderStageInfo[0],
                                                  settings,
                                                  &shaders[kVertex_GrShaderType],
                                                  &inputs[kVertex_GrShaderType]);

        success = success && this->createVkShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT,
                                                        *sksl[kFragment_GrShaderType],
                                                        &shaderModules[kFragment_GrShaderType],
                                                        &shaderStageInfo[1],
                                                        settings,
                                                        &shaders[kFragment_GrShaderType],
                                                        &inputs[kFragment_GrShaderType]);

        if (this->primitiveProcessor().willUseGeoShader()) {
            success = success && this->createVkShaderModule(VK_SHADER_STAGE_GEOMETRY_BIT,
                                                            *sksl[kGeometry_GrShaderType],
                                                            &shaderModules[kGeometry_GrShaderType],
                                                            &shaderStageInfo[2],
                                                            settings,
                                                            &shaders[kGeometry_GrShaderType],
                                                            &inputs[kGeometry_GrShaderType]);
            ++numShaderStages;
        }

        if (!success) {
            for (int i = 0; i < kGrShaderTypeCount; ++i) {
                if (shaderModules[i]) {
                    GR_VK_CALL(fGpu->vkInterface(), DestroyShaderModule(fGpu->device(),
                                                                        shaderModules[i], nullptr));
                }
            }
            GR_VK_CALL(fGpu->vkInterface(), DestroyPipelineLayout(fGpu->device(), pipelineLayout,
                                                                  nullptr));
            return nullptr;
        }

        if (persistentCache && !cached) {
            bool isSkSL = false;
            if (fGpu->getContext()->priv().options().fShaderCacheStrategy ==
                    GrContextOptions::ShaderCacheStrategy::kSkSL) {
                for (int i = 0; i < kGrShaderTypeCount; ++i) {
                    shaders[i] = GrShaderUtils::PrettyPrint(*sksl[i]);
                }
                isSkSL = true;
            }
            this->storeShadersInCache(shaders, inputs, isSkSL);
        }
    }

    GrVkPipeline* pipeline = resourceProvider.createPipeline(fProgramInfo, shaderStageInfo,
                                                             numShaderStages, compatibleRenderPass,
                                                             pipelineLayout);
    for (int i = 0; i < kGrShaderTypeCount; ++i) {
        // This if check should not be needed since calling destroy on a VK_NULL_HANDLE is allowed.
        // However this is causing a crash in certain drivers (e.g. NVidia).
        if (shaderModules[i]) {
            GR_VK_CALL(fGpu->vkInterface(), DestroyShaderModule(fGpu->device(), shaderModules[i],
                                                                nullptr));
        }
    }

    if (!pipeline) {
        GR_VK_CALL(fGpu->vkInterface(), DestroyPipelineLayout(fGpu->device(), pipelineLayout,
                                                              nullptr));
        return nullptr;
    }

    return new GrVkPipelineState(fGpu,
                                 pipeline,
                                 samplerDSHandle,
                                 fUniformHandles,
                                 fUniformHandler.fUniforms,
                                 fUniformHandler.fCurrentUBOOffset,
                                 fUniformHandler.fSamplers,
                                 std::move(fGeometryProcessor),
                                 std::move(fXferProcessor),
                                 std::move(fFragmentProcessors));
}
