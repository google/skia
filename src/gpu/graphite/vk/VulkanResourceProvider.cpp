/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanResourceProvider.h"

#include "include/core/SkSpan.h"
#include "include/gpu/ShaderErrorHandler.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "src/core/SkSLTypeShared.h"
#include "src/gpu/PipelineUtils.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/ComputePipeline.h"
#include "src/gpu/graphite/ContextUtils.h"
#include "src/gpu/graphite/GraphicsPipeline.h"
#include "src/gpu/graphite/RendererProvider.h"
#include "src/gpu/graphite/Sampler.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/vk/VulkanBuffer.h"
#include "src/gpu/graphite/vk/VulkanCommandBuffer.h"
#include "src/gpu/graphite/vk/VulkanDescriptorPool.h"
#include "src/gpu/graphite/vk/VulkanDescriptorSet.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSampler.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "src/gpu/graphite/vk/VulkanTexture.h"
#include "src/sksl/SkSLProgramKind.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLProgram.h"

namespace skgpu::graphite {

VkDescriptorSetLayout VulkanResourceProvider::DescTypeAndCountToVkDescSetLayout(
        const VulkanSharedContext* ctxt,
        SkSpan<DescTypeAndCount> requestedDescriptors) {

    VkDescriptorSetLayout layout;
    skia_private::STArray<kDescriptorTypeCount, VkDescriptorSetLayoutBinding> bindingLayouts;

    for (size_t i = 0, j = 0; i < requestedDescriptors.size(); i++) {
        if (requestedDescriptors[i].count != 0) {
            VkDescriptorSetLayoutBinding* layoutBinding = &bindingLayouts.at(j++);
            memset(layoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
            layoutBinding->binding = 0;
            layoutBinding->descriptorType =
                    VulkanDescriptorSet::DsTypeEnumToVkDs(requestedDescriptors[i].type);
            layoutBinding->descriptorCount = requestedDescriptors[i].count;
            // TODO: Obtain layout binding stage flags from visibility (vertex or shader)
            layoutBinding->stageFlags = 0;
            // TODO: Optionally set immutableSamplers here.
            layoutBinding->pImmutableSamplers = nullptr;
        }
    }

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.bindingCount = bindingLayouts.size();
    layoutCreateInfo.pBindings = &bindingLayouts.front();

    VkResult result;
    VULKAN_CALL_RESULT(ctxt->interface(),
                       result,
                       CreateDescriptorSetLayout(ctxt->device(),
                                                 &layoutCreateInfo,
                                                 nullptr,
                                                 &layout));
    if (result != VK_SUCCESS) {
        SkDebugf("Failed to create VkDescriptorSetLayout\n");
        layout = VK_NULL_HANDLE;
    }
    return layout;
}

VulkanResourceProvider::VulkanResourceProvider(SharedContext* sharedContext,
                                               SingleOwner* singleOwner,
                                               uint32_t recorderID)
        : ResourceProvider(sharedContext, singleOwner, recorderID) {}

VulkanResourceProvider::~VulkanResourceProvider() {}

const VulkanSharedContext* VulkanResourceProvider::vulkanSharedContext() {
    return static_cast<const VulkanSharedContext*>(fSharedContext);
}

sk_sp<Texture> VulkanResourceProvider::createWrappedTexture(const BackendTexture&) {
    return nullptr;
}

sk_sp<GraphicsPipeline> VulkanResourceProvider::createGraphicsPipeline(
        const RuntimeEffectDictionary* runtimeDict,
        const GraphicsPipelineDesc& pipelineDesc,
        const RenderPassDesc& renderPassDesc) {
    SkSL::Program::Interface vsInterface, fsInterface;
    SkSL::ProgramSettings settings;

    settings.fForceNoRTFlip = true; // TODO: Confirm

    auto compiler = this->skslCompiler();
    ShaderErrorHandler* errorHandler = fSharedContext->caps()->shaderErrorHandler();

    const RenderStep* step =
            fSharedContext->rendererProvider()->lookup(pipelineDesc.renderStepID());

    bool useShadingSsboIndex =
            fSharedContext->caps()->storageBufferPreferred() && step->performsShading();

    const FragSkSLInfo fsSkSLInfo = GetSkSLFS(fSharedContext->caps(),
                                              fSharedContext->shaderCodeDictionary(),
                                              runtimeDict,
                                              step,
                                              pipelineDesc.paintParamsID(),
                                              useShadingSsboIndex,
                                              renderPassDesc.fWriteSwizzle);
    const std::string& fsSkSL = fsSkSLInfo.fSkSL;
    const bool localCoordsNeeded = fsSkSLInfo.fRequiresLocalCoords;

    bool hasFragment = !fsSkSL.empty();
    std::string vsSPIRV, fsSPIRV;
    VkShaderModule fsModule = VK_NULL_HANDLE, vsModule = VK_NULL_HANDLE;

    if (hasFragment) {
        if (!SkSLToSPIRV(compiler,
                         fsSkSL,
                         SkSL::ProgramKind::kGraphiteFragment,
                         settings,
                         &fsSPIRV,
                         &fsInterface,
                         errorHandler)) {
            return {};
        }

        fsModule = createVulkanShaderModule(this->vulkanSharedContext(),
                                            fsSPIRV,
                                            VK_SHADER_STAGE_FRAGMENT_BIT);

        if (!fsModule) {
            return {};
        }
    }

    if (!SkSLToSPIRV(compiler,
                     GetSkSLVS(fSharedContext->caps()->resourceBindingRequirements(),
                               step,
                               useShadingSsboIndex,
                               localCoordsNeeded),
                     SkSL::ProgramKind::kGraphiteVertex,
                     settings,
                     &vsSPIRV,
                     &vsInterface,
                     errorHandler)) {
        return {};
    }

    vsModule = createVulkanShaderModule(this->vulkanSharedContext(),
                                        vsSPIRV,
                                        VK_SHADER_STAGE_VERTEX_BIT);
    if (!vsModule) {
        return {};
    }

    // TODO: Generate depth-stencil state, blend info
    return VulkanGraphicsPipeline::Make(this->vulkanSharedContext(),
                                        vsModule,
                                        step->vertexAttributes(),
                                        step->instanceAttributes(),
                                        fsModule,
                                        step->depthStencilSettings(),
                                        step->primitiveType(),
                                        fsSkSLInfo.fBlendInfo,
                                        renderPassDesc);
}

sk_sp<ComputePipeline> VulkanResourceProvider::createComputePipeline(const ComputePipelineDesc&) {
    return nullptr;
}

sk_sp<Texture> VulkanResourceProvider::createTexture(SkISize size, const TextureInfo& info,
                                                     skgpu::Budgeted budgeted) {
    return VulkanTexture::Make(this->vulkanSharedContext(), size, info, budgeted);
}

sk_sp<Buffer> VulkanResourceProvider::createBuffer(size_t size,
                                                   BufferType type,
                                                   AccessPattern accessPattern) {
    return VulkanBuffer::Make(this->vulkanSharedContext(), size, type, accessPattern);
}

sk_sp<Sampler> VulkanResourceProvider::createSampler(const SkSamplingOptions& samplingOptions,
                                                     SkTileMode xTileMode,
                                                     SkTileMode yTileMode) {
    return VulkanSampler::Make(this->vulkanSharedContext(), samplingOptions, xTileMode, yTileMode);
}

BackendTexture VulkanResourceProvider::onCreateBackendTexture(SkISize dimensions,
                                                              const TextureInfo&) {
    return {};
}

VulkanDescriptorSet* VulkanResourceProvider::findOrCreateDescriptorSet(
        SkSpan<DescTypeAndCount> requestedDescriptors) {
    GraphiteResourceKey key;
    // TODO(nicolettep): Optimize key structure. It is horrendously inefficient but functional.
    // Fow now, have each descriptor type and quantity take up an entire uint32_t, with an
    // additional uint32_t added to include a unique identifier for different descriptor sets that
    // have the same set layout.
    static const int kNum32DataCnt = (kDescriptorTypeCount * 2) + 1;
    static const ResourceType kType = GraphiteResourceKey::GenerateResourceType();

    Resource* descSet = nullptr;
    // Search for available descriptor sets by assembling the last part of the key with a unique set
    // ID (which ranges from 0 to kMaxNumSets - 1). Start the search at 0 and continue until an
    // available set is found.
    // TODO(nicolettep): Explore ways to optimize this traversal.
    for (uint32_t i = 0; i < VulkanDescriptorPool::kMaxNumSets; i++) {
        GraphiteResourceKey::Builder builder(&key, kType, kNum32DataCnt, Shareable::kNo);
        // Assemble the base component of a descriptor set key which is determined by the type and
        // quantity of requested descriptors.
        for (size_t j = 0, k = 0; k < requestedDescriptors.size(); j = j + 2, k++) {
            builder[j+1] = static_cast<uint32_t>(requestedDescriptors[k].type);
            builder[j] = requestedDescriptors[k].count;
        }
        builder[kNum32DataCnt - 1] = i;
        builder.finish();

        if ((descSet = fResourceCache->findAndRefResource(key, skgpu::Budgeted::kNo))) {
            // A non-null resource pointer indicates we have found an available descriptor set.
            return static_cast<VulkanDescriptorSet*>(descSet);
        }
        key.reset();
    }

    // If we did not find an existing avilable desc set, allocate sets with the appropriate layout
    // and add them to the cache.
    auto pool = VulkanDescriptorPool::Make(this->vulkanSharedContext(), requestedDescriptors);
    VkDescriptorSetLayout layout = DescTypeAndCountToVkDescSetLayout(
            this->vulkanSharedContext(),
            requestedDescriptors);

    // Store the key of the first descriptor set so it can be easily accessed later.
    GraphiteResourceKey firstSetKey;
    // Allocate the maximum number of sets so they can be easily accessed as needed from the cache.
    for (int i = 0; i < VulkanDescriptorPool::kMaxNumSets ; i++) {
        GraphiteResourceKey::Builder builder(&key, kType, kNum32DataCnt, Shareable::kNo);
        descSet = VulkanDescriptorSet::Make(this->vulkanSharedContext(), pool, &layout).get();
        // Assemble the base component of a descriptor set key which is determined by the type and
        // quantity of requested descriptors.
        for (size_t j = 0, k = 0; k < requestedDescriptors.size(); j = j + 2, k++) {
            builder[j+1] = static_cast<uint32_t>(requestedDescriptors[k].type);
            builder[j] = requestedDescriptors[k].count;
        }
        builder[kNum32DataCnt - 1] = i;
        builder.finish();
        descSet->setKey(key);
        fResourceCache->insertResource(descSet);
        if (i == 0) {
            firstSetKey = key;
        }
        key.reset();
    }
    descSet = fResourceCache->findAndRefResource(firstSetKey, skgpu::Budgeted::kNo);
    return descSet ? static_cast<VulkanDescriptorSet*>(descSet) : nullptr;
}
} // namespace skgpu::graphite
