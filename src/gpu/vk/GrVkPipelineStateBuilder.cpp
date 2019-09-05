/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrPersistentCacheUtils.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrShaderUtils.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/vk/GrVkDescriptorSetManager.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkPipelineStateBuilder.h"
#include "src/gpu/vk/GrVkRenderPass.h"
#include "src/gpu/vk/GrVkRenderTarget.h"

typedef size_t shader_size;

GrVkPipelineState* GrVkPipelineStateBuilder::CreatePipelineState(
        GrVkGpu* gpu,
        GrRenderTarget* renderTarget, GrSurfaceOrigin origin,
        const GrPrimitiveProcessor& primProc,
        const GrTextureProxy* const primProcProxies[],
        const GrPipeline& pipeline,
        const GrStencilSettings& stencil,
        GrPrimitiveType primitiveType,
        Desc* desc,
        VkRenderPass compatibleRenderPass) {
    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrVkPipelineStateBuilder builder(gpu, renderTarget, origin, pipeline, primProc,
                                     primProcProxies, desc);

    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }

    return builder.finalize(stencil, primitiveType, compatibleRenderPass, desc);
}

GrVkPipelineStateBuilder::GrVkPipelineStateBuilder(GrVkGpu* gpu,
                                                   GrRenderTarget* renderTarget,
                                                   GrSurfaceOrigin origin,
                                                   const GrPipeline& pipeline,
                                                   const GrPrimitiveProcessor& primProc,
                                                   const GrTextureProxy* const primProcProxies[],
                                                   GrProgramDesc* desc)
        : INHERITED(renderTarget, origin, primProc, primProcProxies, pipeline, desc)
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
                                                    Desc* desc,
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

int GrVkPipelineStateBuilder::loadShadersFromCache(SkReader32* cached,
                                                   VkShaderModule outShaderModules[],
                                                   VkPipelineShaderStageCreateInfo* outStageInfo) {
    SkSL::String shaders[kGrShaderTypeCount];
    SkSL::Program::Inputs inputs[kGrShaderTypeCount];

    GrPersistentCacheUtils::UnpackCachedShaders(cached, shaders, inputs, kGrShaderTypeCount);

    SkAssertResult(this->installVkShaderModule(VK_SHADER_STAGE_VERTEX_BIT,
                                               fVS,
                                               &outShaderModules[kVertex_GrShaderType],
                                               &outStageInfo[0],
                                               shaders[kVertex_GrShaderType],
                                               inputs[kVertex_GrShaderType]));

    SkAssertResult(this->installVkShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT,
                                               fFS,
                                               &outShaderModules[kFragment_GrShaderType],
                                               &outStageInfo[1],
                                               shaders[kFragment_GrShaderType],
                                               inputs[kFragment_GrShaderType]));

    if (!shaders[kGeometry_GrShaderType].empty()) {
        SkAssertResult(this->installVkShaderModule(VK_SHADER_STAGE_GEOMETRY_BIT,
                                                   fGS,
                                                   &outShaderModules[kGeometry_GrShaderType],
                                                   &outStageInfo[2],
                                                   shaders[kGeometry_GrShaderType],
                                                   inputs[kGeometry_GrShaderType]));
        return 3;
    } else {
        return 2;
    }
}

void GrVkPipelineStateBuilder::storeShadersInCache(const SkSL::String shaders[],
                                                   const SkSL::Program::Inputs inputs[],
                                                   bool isSkSL,
                                                   const SkSL::Program::Settings& settings) {
    Desc* desc = static_cast<Desc*>(this->desc());
    sk_sp<SkData> key = SkData::MakeWithoutCopy(desc->asKey(), desc->shaderKeyLength());
    sk_sp<SkData> data = GrPersistentCacheUtils::PackCachedShaders(isSkSL ? kSKSL_Tag : kSPIRV_Tag,
                                                                   shaders,
                                                                   inputs, kGrShaderTypeCount,
                                                                   &settings);
    this->gpu()->getContext()->priv().getPersistentCache()->store(*key, *data);
}

GrVkPipelineState* GrVkPipelineStateBuilder::finalize(const GrStencilSettings& stencil,
                                                      GrPrimitiveType primitiveType,
                                                      VkRenderPass compatibleRenderPass,
                                                      Desc* desc) {
    VkDescriptorSetLayout dsLayout[2];
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

    VkPipelineShaderStageCreateInfo shaderStageInfo[3];
    SkSL::Program::Settings settings;
    settings.fCaps = this->caps()->shaderCaps();
    settings.fVkCaps = &this->gpu()->vkCaps();
    settings.fFlipY = this->origin() != kTopLeft_GrSurfaceOrigin;
    settings.fSharpenTextures =
                        this->gpu()->getContext()->priv().options().fSharpenMipmappedTextures;
    settings.fRTHeightOffset = fUniformHandler.getRTHeightOffset();
    SkASSERT(!this->fragColorIsInOut());

    sk_sp<SkData> cached;
    SkReader32 reader;
    SkFourByteTag shaderType = 0;
    auto persistentCache = fGpu->getContext()->priv().getPersistentCache();
    if (persistentCache) {
        sk_sp<SkData> key = SkData::MakeWithoutCopy(desc->asKey(), desc->shaderKeyLength());
        cached = persistentCache->load(*key);
        if (cached) {
            reader.setMemory(cached->data(), cached->size());
            shaderType = reader.readU32();
        }
    }

    int numShaderStages = 0;
    if (kSPIRV_Tag == shaderType) {
        numShaderStages = this->loadShadersFromCache(&reader, shaderModules, shaderStageInfo);
    } else {
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
            GrPersistentCacheUtils::UnpackCachedShaders(&reader, cached_sksl, inputs,
                                                        kGrShaderTypeCount);
            for (int i = 0; i < kGrShaderTypeCount; ++i) {
                sksl[i] = &cached_sksl[i];
            }
        }

        bool success = this->createVkShaderModule(VK_SHADER_STAGE_VERTEX_BIT,
                                                  *sksl[kVertex_GrShaderType],
                                                  &shaderModules[kVertex_GrShaderType],
                                                  &shaderStageInfo[0],
                                                  settings,
                                                  desc,
                                                  &shaders[kVertex_GrShaderType],
                                                  &inputs[kVertex_GrShaderType]);

        success = success && this->createVkShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT,
                                                        *sksl[kFragment_GrShaderType],
                                                        &shaderModules[kFragment_GrShaderType],
                                                        &shaderStageInfo[1],
                                                        settings,
                                                        desc,
                                                        &shaders[kFragment_GrShaderType],
                                                        &inputs[kFragment_GrShaderType]);

        if (this->primitiveProcessor().willUseGeoShader()) {
            success = success && this->createVkShaderModule(VK_SHADER_STAGE_GEOMETRY_BIT,
                                                            *sksl[kGeometry_GrShaderType],
                                                            &shaderModules[kGeometry_GrShaderType],
                                                            &shaderStageInfo[2],
                                                            settings,
                                                            desc,
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
            this->storeShadersInCache(shaders, inputs, isSkSL, settings);
        }
    }
    GrVkPipeline* pipeline = resourceProvider.createPipeline(
            this->renderTarget()->numSamples(), fPrimProc, fPipeline, stencil, this->origin(),
            shaderStageInfo, numShaderStages, primitiveType, compatibleRenderPass, pipelineLayout);
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
                                 std::move(fFragmentProcessors),
                                 fFragmentProcessorCnt);
}

//////////////////////////////////////////////////////////////////////////////

bool GrVkPipelineStateBuilder::Desc::Build(Desc* desc,
                                           GrRenderTarget* renderTarget,
                                           const GrPrimitiveProcessor& primProc,
                                           const GrPipeline& pipeline,
                                           const GrStencilSettings& stencil,
                                           GrPrimitiveType primitiveType,
                                           GrVkGpu* gpu) {
    if (!INHERITED::Build(desc, renderTarget, primProc,
                          primitiveType == GrPrimitiveType::kPoints, pipeline, gpu)) {
        return false;
    }

    GrProcessorKeyBuilder b(&desc->key());

    b.add32(GrVkGpu::kShader_PersistentCacheKeyType);
    int keyLength = desc->key().count();
    SkASSERT(0 == (keyLength % 4));
    desc->fShaderKeyLength = SkToU32(keyLength);

    GrVkRenderTarget* vkRT = (GrVkRenderTarget*)renderTarget;
    vkRT->simpleRenderPass()->genKey(&b);

    stencil.genKey(&b);

    b.add32(pipeline.getBlendInfoKey());

    b.add32((uint32_t)primitiveType);

    return true;
}
