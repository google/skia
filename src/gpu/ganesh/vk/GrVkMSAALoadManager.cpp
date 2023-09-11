/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/vk/GrVkMSAALoadManager.h"

#include "include/gpu/GrDirectContext.h"
#include "src/core/SkTraceEvent.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrResourceProvider.h"
#include "src/gpu/ganesh/vk/GrVkBuffer.h"
#include "src/gpu/ganesh/vk/GrVkCommandBuffer.h"
#include "src/gpu/ganesh/vk/GrVkDescriptorSet.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkImageView.h"
#include "src/gpu/ganesh/vk/GrVkPipeline.h"
#include "src/gpu/ganesh/vk/GrVkRenderTarget.h"
#include "src/gpu/ganesh/vk/GrVkResourceProvider.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"
#include "src/sksl/SkSLProgramSettings.h"

GrVkMSAALoadManager::GrVkMSAALoadManager()
        : fVertShaderModule(VK_NULL_HANDLE)
        , fFragShaderModule(VK_NULL_HANDLE)
        , fPipelineLayout(VK_NULL_HANDLE) {}

GrVkMSAALoadManager::~GrVkMSAALoadManager() {}

bool GrVkMSAALoadManager::createMSAALoadProgram(GrVkGpu* gpu) {
    TRACE_EVENT0("skia", TRACE_FUNC);

    std::string vertShaderText;
    vertShaderText.append(
            "layout(vulkan, set=0, binding=0) uniform vertexUniformBuffer {"
            "half4 uPosXform;"
            "};"

            "// MSAA Load Program VS\n"
            "void main() {"
            "float2 position = float2(sk_VertexID >> 1, sk_VertexID & 1);"
            "sk_Position.xy = position * uPosXform.xy + uPosXform.zw;"
            "sk_Position.zw = half2(0, 1);"
            "}");

    std::string fragShaderText;
    fragShaderText.append(
            "layout(vulkan, input_attachment_index=0, set=2, binding=0) subpassInput uInput;"

            "// MSAA Load Program FS\n"
            "void main() {"
            "sk_FragColor = subpassLoad(uInput);"
            "}");

    SkSL::ProgramSettings settings;
    std::string spirv;
    SkSL::Program::Interface interface;
    if (!GrCompileVkShaderModule(gpu, vertShaderText, VK_SHADER_STAGE_VERTEX_BIT,
                                 &fVertShaderModule, &fShaderStageInfo[0], settings, &spirv,
                                 &interface)) {
        this->destroyResources(gpu);
        return false;
    }
    SkASSERT(interface == SkSL::Program::Interface());

    if (!GrCompileVkShaderModule(gpu, fragShaderText, VK_SHADER_STAGE_FRAGMENT_BIT,
                                 &fFragShaderModule, &fShaderStageInfo[1], settings, &spirv,
                                 &interface)) {
        this->destroyResources(gpu);
        return false;
    }
    SkASSERT(interface == SkSL::Program::Interface());

    VkDescriptorSetLayout dsLayout[GrVkUniformHandler::kDescSetCount];

    GrVkResourceProvider& resourceProvider = gpu->resourceProvider();

    dsLayout[GrVkUniformHandler::kUniformBufferDescSet] = resourceProvider.getUniformDSLayout();

    // Even though we don't have a sampler we need to put a valid handle here (of zero samplers)
    // since we set up our descriptor layout to be uniform, sampler, input.
    //
    // TODO: We should have a more general way for different pipelines to describe their descriptor
    // layouts so that we don't have to use the compile time constants for the sets.
    GrVkDescriptorSetManager::Handle samplerHandle;
    resourceProvider.getZeroSamplerDescriptorSetHandle(&samplerHandle);

    dsLayout[GrVkUniformHandler::kSamplerDescSet] =
            resourceProvider.getSamplerDSLayout(samplerHandle);

    dsLayout[GrVkUniformHandler::kInputDescSet] = resourceProvider.getInputDSLayout();

    // Create the VkPipelineLayout
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    memset(&layoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateFlags));
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.setLayoutCount = GrVkUniformHandler::kDescSetCount;
    layoutCreateInfo.pSetLayouts = dsLayout;
    layoutCreateInfo.pushConstantRangeCount = 0;
    layoutCreateInfo.pPushConstantRanges = nullptr;

    VkResult err = GR_VK_CALL(
            gpu->vkInterface(),
            CreatePipelineLayout(gpu->device(), &layoutCreateInfo, nullptr, &fPipelineLayout));
    if (err) {
        this->destroyResources(gpu);
        return false;
    }

    return true;
}

bool GrVkMSAALoadManager::loadMSAAFromResolve(GrVkGpu* gpu,
                                              GrVkCommandBuffer* commandBuffer,
                                              const GrVkRenderPass& renderPass,
                                              GrAttachment* dst,
                                              GrVkImage* src,
                                              const SkIRect& rect) {
    if (!dst) {
        return false;
    }
    if (!src || !src->supportsInputAttachmentUsage()) {
        return false;
    }

    if (VK_NULL_HANDLE == fVertShaderModule) {
        SkASSERT(fFragShaderModule == VK_NULL_HANDLE && fPipelineLayout == VK_NULL_HANDLE);
        if (!this->createMSAALoadProgram(gpu)) {
            SkDebugf("Failed to create copy program.\n");
            return false;
        }
    }
    SkASSERT(fPipelineLayout != VK_NULL_HANDLE);

    GrVkResourceProvider& resourceProv = gpu->resourceProvider();

    sk_sp<const GrVkPipeline> pipeline =
            resourceProv.findOrCreateMSAALoadPipeline(renderPass, dst->numSamples(),
                                                      fShaderStageInfo, fPipelineLayout);
    if (!pipeline) {
        return false;
    }
    commandBuffer->bindPipeline(gpu, std::move(pipeline));

    // Set Dynamic viewport and stencil
    // We always use one viewport the size of the RT
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = SkIntToScalar(dst->width());
    viewport.height = SkIntToScalar(dst->height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer->setViewport(gpu, 0, 1, &viewport);

    // We assume the scissor is not enabled so just set it to the whole RT
    VkRect2D scissor;
    scissor.extent.width = dst->width();
    scissor.extent.height = dst->height();
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    commandBuffer->setScissor(gpu, 0, 1, &scissor);

    // Update and bind uniform descriptor set
    int w = rect.width();
    int h = rect.height();

    // dst rect edges in NDC (-1 to 1)
    int dw = dst->width();
    int dh = dst->height();
    float dx0 = 2.f * rect.fLeft / dw - 1.f;
    float dx1 = 2.f * (rect.fLeft + w) / dw - 1.f;
    float dy0 = 2.f * rect.fTop / dh - 1.f;
    float dy1 = 2.f * (rect.fTop + h) / dh - 1.f;

    float uniData[] = {dx1 - dx0, dy1 - dy0, dx0, dy0};  // posXform

    GrResourceProvider* resourceProvider = gpu->getContext()->priv().resourceProvider();
    // TODO: Is it worth holding onto the last used uniform buffer and tracking the width, height,
    // dst width, and dst height so that we can use the buffer again without having to update the
    // data?
    sk_sp<GrGpuBuffer> uniformBuffer = resourceProvider->createBuffer(uniData,
                                                                      sizeof(uniData),
                                                                      GrGpuBufferType::kUniform,
                                                                      kDynamic_GrAccessPattern);
    if (!uniformBuffer) {
        return false;
    }
    GrVkBuffer* vkUniformBuffer = static_cast<GrVkBuffer*>(uniformBuffer.get());
    static_assert(GrVkUniformHandler::kUniformBufferDescSet < GrVkUniformHandler::kInputDescSet);
    commandBuffer->bindDescriptorSets(gpu, fPipelineLayout,
                                      GrVkUniformHandler::kUniformBufferDescSet,
                                      /*setCount=*/1, vkUniformBuffer->uniformDescriptorSet(),
                                      /*dynamicOffsetCount=*/0, /*dynamicOffsets=*/nullptr);
    commandBuffer->addGrBuffer(std::move(uniformBuffer));

    // Update the input descriptor set
    gr_rp<const GrVkDescriptorSet> inputDS = src->inputDescSetForMSAALoad(gpu);
    if (!inputDS) {
        return false;
    }
    commandBuffer->bindDescriptorSets(gpu, fPipelineLayout,
                                      GrVkUniformHandler::kInputDescSet, /*setCount=*/1,
                                      inputDS->descriptorSet(),
                                      /*dynamicOffsetCount=*/0, /*dynamicOffsets=*/nullptr);

    // We don't need to add the src and dst resources here since those are all tracked by the main
    // render pass code out in GrVkOpsRenderPass and GrVkRenderTarget::adResources.
    commandBuffer->addRecycledResource(std::move(inputDS));

    commandBuffer->draw(gpu, 4, 1, 0, 0);

    return true;
}

void GrVkMSAALoadManager::destroyResources(GrVkGpu* gpu) {
    if (fVertShaderModule != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyShaderModule(gpu->device(), fVertShaderModule, nullptr));
        fVertShaderModule = VK_NULL_HANDLE;
    }

    if (fFragShaderModule != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyShaderModule(gpu->device(), fFragShaderModule, nullptr));
        fFragShaderModule = VK_NULL_HANDLE;
    }

    if (fPipelineLayout != VK_NULL_HANDLE) {
        GR_VK_CALL(gpu->vkInterface(),
                   DestroyPipelineLayout(gpu->device(), fPipelineLayout, nullptr));
        fPipelineLayout = VK_NULL_HANDLE;
    }
}

