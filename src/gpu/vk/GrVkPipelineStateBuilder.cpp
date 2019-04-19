/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "vk/GrVkPipelineStateBuilder.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrPersistentCacheUtils.h"
#include "GrShaderCaps.h"
#include "GrStencilSettings.h"
#include "GrVkRenderTarget.h"
#include "vk/GrVkDescriptorSetManager.h"
#include "vk/GrVkGpu.h"
#include "vk/GrVkRenderPass.h"

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
                                                    const GrGLSLShaderBuilder& builder,
                                                    VkShaderModule* shaderModule,
                                                    VkPipelineShaderStageCreateInfo* stageInfo,
                                                    const SkSL::Program::Settings& settings,
                                                    Desc* desc,
                                                    SkSL::String* outSPIRV,
                                                    SkSL::Program::Inputs* outInputs) {
    if (!GrCompileVkShaderModule(fGpu, builder.fCompilerString.c_str(), stage, shaderModule,
                                 stageInfo, settings, outSPIRV, outInputs)) {
        return false;
    }
    if (outInputs->fRTHeight) {
        this->addRTHeightUniform(SKSL_RTHEIGHT_NAME);
    }
    if (outInputs->fFlipY) {
        desc->setSurfaceOriginKey(GrGLSLFragmentShaderBuilder::KeyForSurfaceOrigin(
                                                     this->origin()));
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

int GrVkPipelineStateBuilder::loadShadersFromCache(const SkData& cached,
                                                   VkShaderModule* outVertShaderModule,
                                                   VkShaderModule* outFragShaderModule,
                                                   VkShaderModule* outGeomShaderModule,
                                                   VkPipelineShaderStageCreateInfo* outStageInfo) {
    SkSL::String shaders[kGrShaderTypeCount];
    SkSL::Program::Inputs inputs[kGrShaderTypeCount];

    GrPersistentCacheUtils::UnpackCachedSPIRV(&cached, shaders, inputs);

    SkAssertResult(this->installVkShaderModule(VK_SHADER_STAGE_VERTEX_BIT,
                                               fVS,
                                               outVertShaderModule,
                                               &outStageInfo[0],
                                               shaders[kVertex_GrShaderType],
                                               inputs[kVertex_GrShaderType]));

    SkAssertResult(this->installVkShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT,
                                               fFS,
                                               outFragShaderModule,
                                               &outStageInfo[1],
                                               shaders[kFragment_GrShaderType],
                                               inputs[kFragment_GrShaderType]));

    if (!shaders[kGeometry_GrShaderType].empty()) {
        SkAssertResult(this->installVkShaderModule(VK_SHADER_STAGE_GEOMETRY_BIT,
                                                   fGS,
                                                   outGeomShaderModule,
                                                   &outStageInfo[2],
                                                   shaders[kGeometry_GrShaderType],
                                                   inputs[kGeometry_GrShaderType]));
        return 3;
    } else {
        return 2;
    }
}

void GrVkPipelineStateBuilder::storeShadersInCache(const SkSL::String shaders[],
                                                   const SkSL::Program::Inputs inputs[]) {
    Desc* desc = static_cast<Desc*>(this->desc());
    sk_sp<SkData> key = SkData::MakeWithoutCopy(desc->asKey(), desc->shaderKeyLength());
    sk_sp<SkData> data = GrPersistentCacheUtils::PackCachedSPIRV(shaders, inputs);
    this->gpu()->getContext()->priv().getPersistentCache()->store(*key, *data);
}

GrVkPipelineState* GrVkPipelineStateBuilder::finalize(const GrStencilSettings& stencil,
                                                      GrPrimitiveType primitiveType,
                                                      VkRenderPass compatibleRenderPass,
                                                      Desc* desc) {
    VkDescriptorSetLayout dsLayout[2];
    VkPipelineLayout pipelineLayout;
    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule geomShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;

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
    settings.fFlipY = this->origin() != kTopLeft_GrSurfaceOrigin;
    settings.fSharpenTextures =
                        this->gpu()->getContext()->priv().options().fSharpenMipmappedTextures;
    SkASSERT(!this->fragColorIsInOut());

    sk_sp<SkData> cached;
    auto persistentCache = fGpu->getContext()->priv().getPersistentCache();
    if (persistentCache) {
        sk_sp<SkData> key = SkData::MakeWithoutCopy(desc->asKey(), desc->shaderKeyLength());
        cached = persistentCache->load(*key);
    }
    int numShaderStages;
    if (cached) {
        numShaderStages = this->loadShadersFromCache(*cached, &vertShaderModule, &fragShaderModule,
                                                     &geomShaderModule, shaderStageInfo);
    } else {
        numShaderStages = 2; // We always have at least vertex and fragment stages.
        SkSL::String shaders[kGrShaderTypeCount];
        SkSL::Program::Inputs inputs[kGrShaderTypeCount];
        SkAssertResult(this->createVkShaderModule(VK_SHADER_STAGE_VERTEX_BIT,
                                                  fVS,
                                                  &vertShaderModule,
                                                  &shaderStageInfo[0],
                                                  settings,
                                                  desc,
                                                  &shaders[kVertex_GrShaderType],
                                                  &inputs[kVertex_GrShaderType]));

        SkAssertResult(this->createVkShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT,
                                                  fFS,
                                                  &fragShaderModule,
                                                  &shaderStageInfo[1],
                                                  settings,
                                                  desc,
                                                  &shaders[kFragment_GrShaderType],
                                                  &inputs[kFragment_GrShaderType]));

        if (this->primitiveProcessor().willUseGeoShader()) {
            SkAssertResult(this->createVkShaderModule(VK_SHADER_STAGE_GEOMETRY_BIT,
                                                      fGS,
                                                      &geomShaderModule,
                                                      &shaderStageInfo[2],
                                                      settings,
                                                      desc,
                                                      &shaders[kGeometry_GrShaderType],
                                                      &inputs[kGeometry_GrShaderType]));
            ++numShaderStages;
        }
        if (persistentCache) {
            this->storeShadersInCache(shaders, inputs);
        }
    }
    GrVkPipeline* pipeline = resourceProvider.createPipeline(
            this->renderTarget()->numColorSamples(), fPrimProc, fPipeline, stencil, this->origin(),
            shaderStageInfo, numShaderStages, primitiveType, compatibleRenderPass, pipelineLayout);
    GR_VK_CALL(fGpu->vkInterface(), DestroyShaderModule(fGpu->device(), vertShaderModule,
                                                        nullptr));
    GR_VK_CALL(fGpu->vkInterface(), DestroyShaderModule(fGpu->device(), fragShaderModule,
                                                        nullptr));
    // This if check should not be needed since calling destroy on a VK_NULL_HANDLE is allowed.
    // However this is causing a crash in certain drivers (e.g. NVidia).
    if (this->primitiveProcessor().willUseGeoShader()) {
        GR_VK_CALL(fGpu->vkInterface(), DestroyShaderModule(fGpu->device(), geomShaderModule,
                                                            nullptr));
    }

    if (!pipeline) {
        GR_VK_CALL(fGpu->vkInterface(), DestroyPipelineLayout(fGpu->device(), pipelineLayout,
                                                              nullptr));
        return nullptr;
    }

    return new GrVkPipelineState(fGpu,
                                 pipeline,
                                 pipelineLayout,
                                 samplerDSHandle,
                                 fUniformHandles,
                                 fUniformHandler.fUniforms,
                                 fUniformHandler.fCurrentGeometryUBOOffset,
                                 fUniformHandler.fCurrentFragmentUBOOffset,
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
