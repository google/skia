/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/ganesh/vk/GrVkPipelineStateBuilder.h"

#include "include/core/SkData.h"
#include "include/core/SkFourByteTag.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/GrAutoLocaleSetter.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrPersistentCacheUtils.h"
#include "src/gpu/ganesh/GrProgramDesc.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrShaderVar.h"
#include "src/gpu/ganesh/GrThreadSafePipelineBuilder.h"
#include "src/gpu/ganesh/GrXferProcessor.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLVertexGeoBuilder.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkDescriptorSetManager.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkPipeline.h"
#include "src/gpu/ganesh/vk/GrVkPipelineState.h"
#include "src/gpu/ganesh/vk/GrVkResourceProvider.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/utils/SkShaderUtils.h"

#include <string.h>
#include <cstdint>
#include <memory>
#include <utility>

class GrCaps;

GrVkPipelineState* GrVkPipelineStateBuilder::CreatePipelineState(
        GrVkGpu* gpu,
        const GrProgramDesc& desc,
        const GrProgramInfo& programInfo,
        VkRenderPass compatibleRenderPass,
        bool overrideSubpassForResolveLoad) {

    GrVkResourceProvider& resourceProvider = gpu->resourceProvider();

    resourceProvider.pipelineStateCache()->stats()->incShaderCompilations();

    // ensure that we use "." as a decimal separator when creating SkSL code
    GrAutoLocaleSetter als("C");

    // create a builder.  This will be handed off to effects so they can use it to add
    // uniforms, varyings, textures, etc
    GrVkPipelineStateBuilder builder(gpu, desc, programInfo);

    if (!builder.emitAndInstallProcs()) {
        return nullptr;
    }

    return builder.finalize(desc, compatibleRenderPass, overrideSubpassForResolveLoad);
}

GrVkPipelineStateBuilder::GrVkPipelineStateBuilder(GrVkGpu* gpu,
                                                   const GrProgramDesc& desc,
                                                   const GrProgramInfo& programInfo)
        : INHERITED(desc, programInfo)
        , fGpu(gpu)
        , fVaryingHandler(this)
        , fUniformHandler(this) {}

const GrCaps* GrVkPipelineStateBuilder::caps() const {
    return fGpu->caps();
}

void GrVkPipelineStateBuilder::finalizeFragmentSecondaryColor(GrShaderVar& outputColor) {
    outputColor.addLayoutQualifier("location = 0, index = 1");
}

bool GrVkPipelineStateBuilder::createVkShaderModule(VkShaderStageFlagBits stage,
                                                    const std::string& sksl,
                                                    VkShaderModule* shaderModule,
                                                    VkPipelineShaderStageCreateInfo* stageInfo,
                                                    const SkSL::ProgramSettings& settings,
                                                    std::string* outSPIRV,
                                                    SkSL::Program::Interface* outInterface) {
    if (!GrCompileVkShaderModule(fGpu, sksl, stage, shaderModule,
                                 stageInfo, settings, outSPIRV, outInterface)) {
        return false;
    }
    if (outInterface->fRTFlipUniform != SkSL::Program::Interface::kRTFlip_None) {
        this->addRTFlipUniform(SKSL_RTFLIP_NAME);
    }
    return true;
}

bool GrVkPipelineStateBuilder::installVkShaderModule(VkShaderStageFlagBits stage,
                                                     const GrGLSLShaderBuilder& builder,
                                                     VkShaderModule* shaderModule,
                                                     VkPipelineShaderStageCreateInfo* stageInfo,
                                                     std::string spirv,
                                                     SkSL::Program::Interface interface) {
    if (!GrInstallVkShaderModule(fGpu, spirv, stage, shaderModule, stageInfo)) {
        return false;
    }
    if (interface.fRTFlipUniform != SkSL::Program::Interface::kRTFlip_None) {
        this->addRTFlipUniform(SKSL_RTFLIP_NAME);
    }
    return true;
}

static constexpr SkFourByteTag kSPIRV_Tag = SkSetFourByteTag('S', 'P', 'R', 'V');
static constexpr SkFourByteTag kSKSL_Tag = SkSetFourByteTag('S', 'K', 'S', 'L');

int GrVkPipelineStateBuilder::loadShadersFromCache(SkReadBuffer* cached,
                                                   VkShaderModule outShaderModules[],
                                                   VkPipelineShaderStageCreateInfo* outStageInfo) {
    std::string shaders[kGrShaderTypeCount];
    SkSL::Program::Interface interfaces[kGrShaderTypeCount];

    if (!GrPersistentCacheUtils::UnpackCachedShaders(
                cached, shaders, interfaces, kGrShaderTypeCount)) {
        return 0;
    }

    bool success = this->installVkShaderModule(VK_SHADER_STAGE_VERTEX_BIT,
                                               fVS,
                                               &outShaderModules[kVertex_GrShaderType],
                                               &outStageInfo[0],
                                               shaders[kVertex_GrShaderType],
                                               interfaces[kVertex_GrShaderType]);

    success = success && this->installVkShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT,
                                                     fFS,
                                                     &outShaderModules[kFragment_GrShaderType],
                                                     &outStageInfo[1],
                                                     shaders[kFragment_GrShaderType],
                                                     interfaces[kFragment_GrShaderType]);

    if (!success) {
        for (int i = 0; i < kGrShaderTypeCount; ++i) {
            if (outShaderModules[i]) {
                GR_VK_CALL(fGpu->vkInterface(),
                           DestroyShaderModule(fGpu->device(), outShaderModules[i], nullptr));
            }
        }
        return 0;
    }
    return 2;
}

void GrVkPipelineStateBuilder::storeShadersInCache(const std::string shaders[],
                                                   const SkSL::Program::Interface interfaces[],
                                                   bool isSkSL) {
    // Here we shear off the Vk-specific portion of the Desc in order to create the
    // persistent key. This is bc Vk only caches the SPIRV code, not the fully compiled
    // program, and that only depends on the base GrProgramDesc data.
    // The +4 is to include the kShader_PersistentCacheKeyType code the Vulkan backend adds
    // to the key right after the base key.
    sk_sp<SkData> key = SkData::MakeWithoutCopy(this->desc().asKey(),
                                                this->desc().initialKeyLength()+4);
    SkString description = GrProgramDesc::Describe(fProgramInfo, *this->caps());

    sk_sp<SkData> data = GrPersistentCacheUtils::PackCachedShaders(isSkSL ? kSKSL_Tag : kSPIRV_Tag,
                                                                   shaders,
                                                                   interfaces, kGrShaderTypeCount);

    this->gpu()->getContext()->priv().getPersistentCache()->store(*key, *data, description);
}

GrVkPipelineState* GrVkPipelineStateBuilder::finalize(const GrProgramDesc& desc,
                                                      VkRenderPass compatibleRenderPass,
                                                      bool overrideSubpassForResolveLoad) {
    TRACE_EVENT0("skia.shaders", TRACE_FUNC);

    VkDescriptorSetLayout dsLayout[GrVkUniformHandler::kDescSetCount];
    VkShaderModule shaderModules[kGrShaderTypeCount] = { VK_NULL_HANDLE,
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

    this->finalizeShaders();

    bool usePushConstants = fUniformHandler.usePushConstants();
    VkPipelineShaderStageCreateInfo shaderStageInfo[3];
    SkSL::ProgramSettings settings;
    settings.fRTFlipBinding = this->gpu()->vkCaps().getFragmentUniformBinding();
    settings.fRTFlipSet = this->gpu()->vkCaps().getFragmentUniformSet();
    settings.fSharpenTextures =
            this->gpu()->getContext()->priv().options().fSharpenMipmappedTextures;
    settings.fRTFlipOffset = fUniformHandler.getRTFlipOffset();
    settings.fUseVulkanPushConstantsForGaneshRTAdjust = usePushConstants;
    if (fFS.fForceHighPrecision) {
        settings.fForceHighPrecision = true;
    }
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
        std::string shaders[kGrShaderTypeCount];
        SkSL::Program::Interface interfaces[kGrShaderTypeCount];

        std::string* sksl[kGrShaderTypeCount] = {
            &fVS.fCompilerString,
            &fFS.fCompilerString,
        };
        std::string cached_sksl[kGrShaderTypeCount];
        if (kSKSL_Tag == shaderType) {
            if (GrPersistentCacheUtils::UnpackCachedShaders(&reader, cached_sksl, interfaces,
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
                                                  &interfaces[kVertex_GrShaderType]);

        success = success && this->createVkShaderModule(VK_SHADER_STAGE_FRAGMENT_BIT,
                                                        *sksl[kFragment_GrShaderType],
                                                        &shaderModules[kFragment_GrShaderType],
                                                        &shaderStageInfo[1],
                                                        settings,
                                                        &shaders[kFragment_GrShaderType],
                                                        &interfaces[kFragment_GrShaderType]);

        if (!success) {
            for (int i = 0; i < kGrShaderTypeCount; ++i) {
                if (shaderModules[i]) {
                    GR_VK_CALL(fGpu->vkInterface(), DestroyShaderModule(fGpu->device(),
                                                                        shaderModules[i], nullptr));
                }
            }
            return nullptr;
        }

        if (persistentCache && !cached) {
            bool isSkSL = false;
            if (fGpu->getContext()->priv().options().fShaderCacheStrategy ==
                    GrContextOptions::ShaderCacheStrategy::kSkSL) {
                for (int i = 0; i < kGrShaderTypeCount; ++i) {
                    shaders[i] = SkShaderUtils::PrettyPrint(*sksl[i]);
                }
                isSkSL = true;
            }
            this->storeShadersInCache(shaders, interfaces, isSkSL);
        }
    }

    // The vulkan spec says that if a subpass has an input attachment, then the input attachment
    // descriptor set must be bound to all pipelines in that subpass. This includes pipelines that
    // don't actually use the input attachment. Thus we look at the renderPassBarriers and not just
    // the DstProxyView barrier flags to determine if we use the input attachment.
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
    VkPushConstantRange pushConstantRange = {};
    if (usePushConstants) {
        pushConstantRange.stageFlags = fGpu->vkCaps().getPushConstantStageFlags();
        pushConstantRange.offset = 0;
        // size must be a multiple of 4
        SkASSERT(!SkToBool(fUniformHandler.currentOffset() & 0x3));
        pushConstantRange.size = fUniformHandler.currentOffset();
        layoutCreateInfo.pushConstantRangeCount = 1;
        layoutCreateInfo.pPushConstantRanges = &pushConstantRange;
    } else {
        layoutCreateInfo.pushConstantRangeCount = 0;
        layoutCreateInfo.pPushConstantRanges = nullptr;
    }

    VkPipelineLayout pipelineLayout;
    VkResult result;
    GR_VK_CALL_RESULT(fGpu, result, CreatePipelineLayout(fGpu->device(), &layoutCreateInfo, nullptr,
                                                         &pipelineLayout));
    if (result != VK_SUCCESS) {
        return nullptr;
    }

    // For the vast majority of cases we only have one subpass so we default piplines to subpass 0.
    // However, if we need to load a resolve into msaa attachment for discardable msaa then the
    // main subpass will be 1.
    uint32_t subpass = 0;
    if (overrideSubpassForResolveLoad ||
        (fProgramInfo.colorLoadOp() == GrLoadOp::kLoad &&
         fGpu->vkCaps().programInfoWillUseDiscardableMSAA(fProgramInfo))) {
        subpass = 1;
    }
    sk_sp<const GrVkPipeline> pipeline = resourceProvider.makePipeline(
            fProgramInfo, shaderStageInfo, numShaderStages, compatibleRenderPass, pipelineLayout,
            subpass);

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
                                 std::move(pipeline),
                                 samplerDSHandle,
                                 fUniformHandles,
                                 fUniformHandler.fUniforms,
                                 fUniformHandler.currentOffset(),
                                 fUniformHandler.usePushConstants(),
                                 fUniformHandler.fSamplers,
                                 std::move(fGPImpl),
                                 std::move(fXPImpl),
                                 std::move(fFPImpls));
}
